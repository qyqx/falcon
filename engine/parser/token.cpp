/*
   FALCON - The Falcon Programming Language.
   FILE: parser/token.cpp

   Token for the parser subsystem.
   -------------------------------------------------------------------
   Author: Giancarlo Niccolai
   Begin: Sun, 03 Apr 2011 17:16:35 +0200

   -------------------------------------------------------------------
   (C) Copyright 2011: the FALCON developers (see list in AUTHORS file)

   See LICENSE file for licensing details.
*/

#define SRC "engine/parser/token.cpp"

#include <falcon/parser/token.h>
#include <falcon/parser/tokeninstance.h>

namespace Falcon {
namespace Parsing {

#define HASH_SEED 0xF3DE3EA3

Token::Token(uint32 nID, const String& name, int prio, bool bRightAssoc ):
   m_bNonTerminal( false ),
   m_bRightAssoc( bRightAssoc ),
   m_prio(prio),
   m_nID( nID ),
   m_name( name )
{}

Token::Token(const String& name, int prio, bool bRightAssoc):
   m_bNonTerminal(false),
   m_bRightAssoc( bRightAssoc ),
   m_prio(prio),
   m_name(name)
{
   m_nID = simpleHash( name );
}

Token::Token()
{
   m_nID = 0;
}

void Token::name( const String& n )
{
    m_nID = simpleHash( n );
    m_name = n;
}

uint32 Token::simpleHash( const String& v )
{
   uint32 h = HASH_SEED;
   
   for( length_t i = 0; i < v.length(); ++i )
   {
      char_t chr = v.getCharAt(i);
      if( chr > 0xFFFF )
      {
         h += chr;
      }
      else if( chr > 0xFF )
      {
         h += chr << (16*(i%2));
      }
      else
      {
         h += chr << (8*(i%3));
      }
   }

   return h;
}

Token::~Token()
{
}

TokenInstance* Token::makeInstance( int line, int chr )
{
   TokenInstance* ti = TokenInstance::alloc( line, chr, *this);
   return ti;
}

TokenInstance* Token::makeInstance( int line, int chr, void* data, deletor d )
{
   TokenInstance* ti = TokenInstance::alloc( line, chr, *this);
   ti->setValue( data, d );
   return ti;
}

TokenInstance* Token::makeInstance( int line, int chr, int32 v )
{
   TokenInstance* ti = TokenInstance::alloc( line, chr, *this);
   ti->setValue( v );
   return ti;
}

TokenInstance* Token::makeInstance( int line, int chr, uint32 v )
{
   TokenInstance* ti = TokenInstance::alloc( line, chr, *this);
   ti->setValue( v );
   return ti;
}

TokenInstance* Token::makeInstance( int line, int chr, int64 v )
{
   TokenInstance* ti = TokenInstance::alloc(  line, chr, *this);
   ti->setValue( v );
   return ti;
}

TokenInstance* Token::makeInstance( int line, int chr, numeric v )
{
   TokenInstance* ti = TokenInstance::alloc( line, chr, *this);
   ti->setValue( v );
   return ti;
}

TokenInstance* Token::makeInstance( int line, int chr, bool v )
{
   TokenInstance* ti = TokenInstance::alloc( line, chr, *this);
   ti->setValue( v );
   return ti;
}

TokenInstance* Token::makeInstance( int line, int chr, const String& v )
{
   TokenInstance* ti = TokenInstance::alloc( line, chr, *this);
   ti->setValue( v );
   return ti;
}

}
}

/* end of parser/token.cpp */