/*
   FALCON - The Falcon Programming Language.
   FILE: classstring.cpp

   String type handler.
   -------------------------------------------------------------------
   Author: Giancarlo Niccolai
   Begin: Sat, 15 Jan 2011 19:09:07 +0100

   -------------------------------------------------------------------
   (C) Copyright 2011: the FALCON developers (see list in AUTHORS file)

   See LICENSE file for licensing details.
*/

#undef SRC
#define SRC "engine/classes/classstring.cpp"

#include <falcon/classes/classstring.h>
#include <falcon/range.h>
#include <falcon/itemid.h>
#include <falcon/vmcontext.h>
#include <falcon/optoken.h>
#include <falcon/errors/accesserror.h>

#include <falcon/datareader.h>
#include <falcon/datawriter.h>

namespace Falcon {

ClassString::ClassString():
   Class("String", FLC_CLASS_ID_STRING )
{
}


ClassString::~ClassString()
{
}


void ClassString::dispose( void* self ) const
{
   String* f = static_cast<String*>(self);
   delete f;
}


void* ClassString::clone( void* source ) const
{
   String* s = static_cast<String*>(source);
   return new String(*s);
}


void ClassString::store( VMContext*, DataWriter* dw, void* data ) const
{
   String* str = static_cast<String*>(data);
   dw->write( *str );
}


void ClassString::restore( VMContext* , DataReader* dr, void*& data ) const
{
   String* str = new String;
   try{
      dr->read( *str );
      data = str;
   }
   catch( ... )
   {
      delete str;
      throw;
   }
}


void ClassString::describe( void* instance, String& target, int, int maxlen ) const
{
   String* self = static_cast<String*>(instance);
   target.size(0);
   target.append('"');
   if( (int) self->length() > maxlen )
   {
      target += self->subString(0, maxlen);
      target += "...";
   }
   else
   {
      target += *self;
   }
   
   target.append('"');
}

void ClassString::enumerateProperties( void*, Class::PropertyEnumerator& cb ) const
{
   // TODO: More
   cb("len", false );
   cb("len_", false );
}

void ClassString::enumeratePV( void* self, Class::PVEnumerator& cb ) const
{
   // TODO: More
   String* str = static_cast<String*>(self); 
   Item temp;
   
   temp = ((int64)str->length());
   cb("len_", temp );
}

bool ClassString::hasProperty( void*, const String& prop ) const
{
   // TODO: More
   return 
      prop == "len"
      || prop == "len_";
}


void ClassString::gcMark( void* instance, uint32 mark ) const
{
   static_cast<String*>(instance)->gcMark(mark);
}


bool ClassString::gcCheck( void* instance, uint32 mark ) const
{
   return static_cast<String*>(instance)->currentMark() >= mark;
}

//=======================================================================
// Addition

void ClassString::op_add( VMContext* ctx, void* self ) const
{
   String* str = static_cast<String*>(self);
   Item* op1, *op2;
   ctx->operands(op1, op2);

   Class* cls;
   void* inst;
   if( ! op2->asClassInst( cls, inst ) )
   {
      String* copy = new String(*str);
      copy->append(op2->describe());
      ctx->stackResult(2, copy->garbage() );
      return;
   }

   if ( cls->typeID() == typeID() )
   {
      // it's a string!
      String *copy = new String(*str);
      copy->append( *static_cast<String*>(inst) );
      ctx->stackResult(2, copy->garbage() );
      return;
   }

   // else we surrender, and we let the virtual system to find a way.
   ctx->pushCode( &m_nextOp );

   // this will transform op2 slot into its string representation.
   cls->op_toString( ctx, inst );
   
   if( ! ctx->wentDeep( &m_nextOp ) )
   {
      ctx->popCode();
      // op2 has been transformed
      String* deep = (String*)op2->asInst();
      deep->prepend( *str );
   }
}

//=======================================================================
// Auto Addition
//

void ClassString::op_create( VMContext* ctx, int pcount ) const
{
   // no param?
   if( pcount == 0 )
   {
      // create a string.
      String* s = new String;
      ctx->stackResult(1, s->garbage() );
   }
   else
   {
      // the parameter is a string?
      Item* itm = ctx->opcodeParams(pcount);
      if( itm->isString() )
      {
         // copy it.
         String* s = new String( *itm->asString() );
         ctx->stackResult( pcount + 1, s->garbage() );
      }
      else
      {
         // apply the op_toString on the item.
         Item cpy = *itm;
         ctx->stackResult( pcount + 1, cpy );

         Class* cls;
         void* data;
         cpy.forceClassInst( cls, data );
         cls->op_toString( ctx, data );
      }
   }
}


void ClassString::op_aadd( VMContext* ctx, void* self ) const
{
   String* str = static_cast<String*>(self);
   Item* op1, *op2;
   ctx->operands(op1, op2);

   Class* cls;
   void* inst;
   if( ! op2->asClassInst( cls, inst ) )
   {
      if( op1->copied() )
      {
         String* copy = new String(*str);
         copy->append(op2->describe());
         ctx->stackResult(2, copy->garbage() );
      }
      else
      {
         op1->asString()->append(op2->describe());
      }

      return;
   }

   if ( cls->typeID() == typeID() )
   {
      // it's a string!
      if( op1->copied() )
      {
         String *copy = new String(*str);
         copy->append( *static_cast<String*>(inst) );
         ctx->stackResult(2, copy->garbage() );
      }
      else
      {
         op1->asString()->append( *static_cast<String*>(inst) );
      }
      return;
   }

   // else we surrender, and we let the virtual system to find a way.
   ctx->pushCode( &m_nextOp );

   // this will transform op2 slot into its string representation.
   cls->op_toString( ctx, inst );

   if( ! ctx->wentDeep(&m_nextOp) )
   {
      ctx->popCode();
      // op2 has been transformed
      String* deep = (String*) op2->asInst();
      deep->prepend( *str );
   }
}

ClassString::NextOp::NextOp()
{
   apply = apply_;
}


void ClassString::NextOp::apply_( const PStep*, VMContext* ctx )
{
   // The result of a deep call is in A
   Item* op1, *op2;
   ctx->operands(op1, op2); // we'll discard op2

   String* deep = ctx->regA().asString();
   String* self = op1->asString();

   if( op1->copied() )
   {
      String* copy = new String(*self);
      copy->append( *deep );
      ctx->stackResult( 2, copy->garbage() );
   }
   else
   {
      ctx->popData();
      self->append(*deep);
   }
}

void ClassString::op_getIndex( VMContext* ctx, void* self ) const
{
   Item *index, *stritem;

   ctx->operands( stritem, index );

   String& str = *static_cast<String*>(self);

   if (index->isOrdinal())
   {
      int64 v = index->forceInteger();

      if ( v < 0 ) v = str.length() + v;

      if ( v >= str.length() )
      {
         throw new AccessError( ErrorParam( e_arracc, __LINE__ ).extra("out of range") );
      }

      String *s = new String();

      s->append(str.getCharAt(v));
      ctx->stackResult(2, s->garbage() );
   }
   else if ( index->isUser() ) // index is a range
   {
      // if range is moving from a smaller number to larger (start left move right in the array)
      //      give values in same order as they appear in the array
      // if range is moving from a larger number to smaller (start right move left in the array)
      //      give values in reverse order as they appear in the array

      Class *cls;
      void *udata; 

      if ( ! index->asClassInst( cls, udata ) )
      {
         throw new AccessError( ErrorParam( e_arracc, __LINE__ ).extra("type error") );
      }

      // Confirm we have a range
      if ( ! cls->typeID() == FLC_CLASS_ID_RANGE )
      {
         throw new AccessError( ErrorParam( e_arracc, __LINE__ ).extra("unknown index") );
      }

      Range& rng = *static_cast<Range*>(udata);

      int64 step = ( rng.step() == 0 ) ? 1 : rng.step(); // assume 1 if no step given
      int64 start = rng.start();
      int64 end = rng.end();
      int64 strLen = str.length();

      // do some validation checks before proceeding
      if ( start > strLen || start < (strLen * -1)  || end > strLen || end < (strLen * -1) )
      {
         throw new AccessError( ErrorParam( e_arracc, __LINE__ ).extra("index out of range") );
      }

      bool reverse = false;

      String *s = new String();

      if ( rng.isOpen() )
      {
         // If negative number count from the end of the array
         if ( start < 0 ) start = strLen + start;

         end = strLen;
      }
      else // non-open range
      {
         if ( start < 0 ) start = strLen + start;

         if ( end < 0 ) end = strLen + end;

         if ( start == end ) end++; // handle [1:1] range

         if ( start > end )
         {
            reverse = true;
            if ( rng.step() == 0 ) step = -1;
         }
      }

      if ( reverse )
      {
         while ( start >= end )
         {
            s->append( str.getCharAt( start ) );
            start += step;
         }
      }
      else
      {
         while ( start < end )
         {
            s->append( str.getCharAt( start ) );
            start += step;
         }
      }

      ctx->stackResult( 2, s->garbage() );
   }
   else
   {
      throw new AccessError( ErrorParam( e_arracc, __LINE__ ).extra("invalid index") );
   }
}

//=======================================================================
// Comparation
//

void ClassString::op_compare( VMContext* ctx, void* self ) const
{
   Item* op1, *op2;
   OpToken token( ctx, op1, op2 );
   String* string = static_cast<String*>(self);

   Class* otherClass;
   void* otherData;

   if( op2->asClassInst( otherClass, otherData ) )
   {
      if( otherClass->typeID() == typeID() )
      {
         token.exit( string->compare(*static_cast<String*>(otherData) ) );
      }
      else
      {
         token.exit( typeID() - otherClass->typeID() );
      }
   }
   else
   {
      token.exit( typeID() - op2->type() );
   }
}


void ClassString::op_toString( VMContext* ctx, void* data ) const
{
   // this op is generally called for temporary items,
   // ... so, even if we shouldn't be marked,
   // ... we won't be marked long if we're temporary.
   ctx->topData().setUser( this, data, true ); 
}

void ClassString::op_isTrue( VMContext* ctx, void* str ) const
{
   ctx->topData().setBoolean( static_cast<String*>(str)->size() != 0 );
}

}

/* end of classstring.cpp */