/*
   FALCON - The Falcon Programming Language.
   FILE: exprcall.h

   Expression controlling item (function) call
   -------------------------------------------------------------------
   Author: Giancarlo Niccolai
   Begin: Sat, 11 Jun 2011 21:19:26 +0200

   -------------------------------------------------------------------
   (C) Copyright 2011: the FALCON developers (see list in AUTHORS file)

   See LICENSE file for licensing details.
*/

#ifndef _FALCON_EXPRCALL_H_
#define _FALCON_EXPRCALL_H_

#include <falcon/pseudofunc.h>
#include <falcon/psteps/exprvector.h>

namespace Falcon {


/** Call expression.
 
 This expression represents the invocation of a functional structure, or
 "call", defined as:
 
 \code
 &lt;Callee expression&gt;( &lt;expression list&gt; ); 
 \endcode
 
 The callee expression is the selector, while the arity of the expression
 depends on the size of the parameter expression list.
   
 If the item generated by the callee expression is an ETA function or method,
 the parameters are untranslated and passed as-is to the function; otherwise,
 after evaluating the callee, each parameter is evaluated in turn and its
 result is passed to the function.
 
 */
class FALCON_DYN_CLASS ExprCall: public ExprVector
{
public:
   ExprCall( int line=0, int chr=0 );
   ExprCall( Expression* op1, int line=0, int chr=0 );
   ExprCall( const ExprCall& other );
   virtual ~ExprCall();

   inline virtual ExprCall* clone() const { return new ExprCall( *this ); }
   virtual bool simplify( Item& value ) const;
   virtual void describeTo( String&, int depth=0 ) const;   

   inline virtual bool isStandAlone() const { return false; }

   virtual bool isStatic() const { return false; }

   virtual Expression* selector() const;
   virtual bool selector( Expression* e ); 
   
protected:
   Expression* m_callExpr;

private:
   
   static void apply_( const PStep*, VMContext* ctx );   
};

}

#endif

/* end of exprcall.h */