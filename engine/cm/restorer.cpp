/*
   FALCON - The Falcon Programming Language.
   FILE: restorer.cpp

   Falcon core module -- Interface to Restorer class.
   -------------------------------------------------------------------
   Author: Giancarlo Niccolai
   Begin: Wed, 07 Dec 2011 23:31:54 +0100

   -------------------------------------------------------------------
   (C) Copyright 2011: the FALCON developers (see list in AUTHORS file)

   See LICENSE file for licensing details.
*/

#undef SRC
#define SRC "falcon/cm/restorer.cpp"

#include <falcon/cm/restorer.h>
#include <falcon/restorer.h>

#include <falcon/vm.h>
#include <falcon/vmcontext.h>
#include <falcon/path.h>
#include <falcon/errors/paramerror.h>
#include <falcon/errors/codeerror.h>
#include <falcon/stdsteps.h>
#include <falcon/usercarrier.h>

#include <falcon/module.h>
#include <falcon/modspace.h>

#include <falcon/cm/stream.h>

namespace Falcon {
namespace Ext {

class RestorerCarrier: public UserCarrierT<Restorer> 
{
   
public:
   RestorerCarrier( Restorer* data ):
      UserCarrierT<Restorer> (data),
      m_streamc(0),
      m_modSpace(0)
   {}
      
   virtual ~RestorerCarrier()
   {
   }
   
   void setStream( StreamCarrier* stc )
   {      
      m_streamc = stc;
   }
   
   virtual void gcMark( uint32 mark )
   {
      if ( m_gcMark != mark )
      {
         m_gcMark = mark;
         m_streamc->m_gcMark = mark;
         if( m_modSpace != 0 ) {
            m_modSpace->gcMark( mark );
         }
      }
   }
   
private:
   StreamCarrier* m_streamc;
   ModSpace* m_modSpace;
};


ClassRestorer::ClassRestorer():
   ClassUser("Restorer"),
   FALCON_INIT_PROPERTY( hasNext ),
   FALCON_INIT_METHOD( next ),
   FALCON_INIT_METHOD( restore )
{}

ClassRestorer::~ClassRestorer()
{}

bool ClassRestorer::op_init( VMContext* ctx, void* instance, int32 ) const
{
   RestorerCarrier* carrier = static_cast<RestorerCarrier*>(instance);
   carrier->carried()->context( ctx );
   return false;
}


void* ClassRestorer::createInstance() const
{ 
   return new RestorerCarrier(new Restorer(0) );
}

/*
void ClassRestorer::op_iter( VMContext* ctx, void* instance ) const
{
   
}


void ClassRestorer::op_next( VMContext* ctx, void* instance ) const
{
   
}
*/
//====================================================
// Properties.
//

FALCON_DEFINE_PROPERTY_GET_P( ClassRestorer, hasNext )
{ 
   RestorerCarrier* stc = static_cast<RestorerCarrier*>(instance);
   Restorer* restorer = stc->carried();
   
   value.setBoolean( restorer->hasNext() );
}

FALCON_DEFINE_PROPERTY_SET( ClassRestorer, hasNext )( void*, const Item& )
{ 
   throw new ParamError( ErrorParam(e_prop_ro, __LINE__, SRC ).extra(name()));
}

FALCON_DEFINE_METHOD_P1( ClassRestorer, restore )
{
   static Class* clsStream = methodOf()->module()->getClass( "Stream" );
   static PStep* retStep = &Engine::instance()->stdSteps()->m_returnFrame;
   
   fassert( clsStream != 0 );
   
   Item* i_item = ctx->param(0);
   if( i_item == 0 )
   {
      throw paramError();
   }      

   Class* cls; void* data;
   i_item->forceClassInst( cls, data ); 
   
   if( ! cls->isDerivedFrom( clsStream ) )
   {
      throw paramError();
   }
   
   RestorerCarrier* stc = static_cast<RestorerCarrier*>(ctx->self().asInst());
   Restorer* restorer = stc->carried();
   StreamCarrier* streamc = static_cast<StreamCarrier*>(cls->getParentData(clsStream,data));
   stc->setStream( streamc );

   // prepare not to return the frame now but later.
   ctx->pushCode( retStep );
   bool complete =  restorer->restore(streamc->m_underlying, 
                           ctx->vm()->modSpace() );
   if( complete )
   {
      ctx->returnFrame();
   }
}


FALCON_DEFINE_METHOD_P1( ClassRestorer, next )
{  
   static Collector* coll = Engine::instance()->collector();
   
   RestorerCarrier* stc = static_cast<RestorerCarrier*>(ctx->self().asInst());
   Restorer* restorer = stc->carried();
   Class* cls;
   void* data;
   bool first;
   if( restorer->next( cls, data, first ) )
   {
      if (cls->isFlatInstance())
      {
         ctx->returnFrame( *static_cast<Item*>(data) );
      }
      else {
         // send to the garbage the items that have been returned for the first time.
         if( first )
            ctx->returnFrame( FALCON_GC_STORE(coll, cls, data) );
         else
            ctx->returnFrame( Item(cls, data) );         
      }
   }
   else
   {
      ctx->raiseError(new CodeError( ErrorParam(e_arracc, __LINE__, SRC )
         .origin( ErrorParam::e_orig_runtime )
         .extra( "No more items in next()") ) );
   }
}

   
}
}

/* end of restorer.cpp */