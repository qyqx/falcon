/*
   FALCON - The Falcon Programming Language.
   FILE: classnil.h

   Nil object handler.
   -------------------------------------------------------------------
   Author: Giancarlo Niccolai
   Begin: Sun, 13 Feb 2011 15:11:01 +0100

   -------------------------------------------------------------------
   (C) Copyright 2011: the FALCON developers (see list in AUTHORS file)

   See LICENSE file for licensing details.
*/

#ifndef _FALCON_CLASSNIL_H_
#define _FALCON_CLASSNIL_H_

#include <falcon/setup.h>
#include <falcon/class.h>

namespace Falcon
{

/**
 Class handling a NIL as an item in a falcon script.
 */

class FALCON_DYN_CLASS ClassNil: public Class
{
public:

   ClassNil();
   virtual ~ClassNil();

   virtual void dispose( void* self ) const;
   virtual void* clone( void* source ) const;
   virtual void* createInstance() const;
   
   virtual void store( VMContext* ctx, DataWriter* stream, void* instance ) const;
   virtual void restore( VMContext* ctx, DataReader* stream, void*& empty ) const;

   virtual void describe( void* instance, String& target, int maxDepth = 3, int maxLength = 60 ) const;

   //=============================================================

   virtual bool op_init( VMContext* ctx, void* instance, int32 pcount ) const;
   virtual void op_isTrue( VMContext* ctx, void* self ) const;
};

}

#endif /* _FALCON_CLASSNIL_H_ */

/* end of classnil.h */