/*
   FALCON - The Falcon Programming Language.
   FILE: stdsteps.h

   Standard misc PSteps commonly used in the virtual machine.
   -------------------------------------------------------------------
   Author: Giancarlo Niccolai
   Begin: Sun, 31 Jul 2011 00:39:23 +0200

   -------------------------------------------------------------------
   (C) Copyright 2011: the FALCON developers (see list in AUTHORS file)

   See LICENSE file for licensing details.
*/

#ifndef _FALCON_STDSTEPS_H
#define _FALCON_STDSTEPS_H

#include <falcon/setup.h>
#include <falcon/pstep.h>

namespace Falcon 
{

class VMContext;

/** Collection of commonly used standard PSteps.
 
 This class holds some common use PSteps that can be freely pushed
 in any VM.
 
 This obviate the needs to locally create and destroy this PSteps, or
 to create Private PSteps doing repetitive work.

 */
class StdSteps
{
public:
   
   //======================================================
   // Expression versions.
   //
   
   /** Duplicate the topmost item in the stack. */
   class PStepDupliTop: public PStep
   {
   public:
      PStepDupliTop() { apply = apply_; }
      virtual ~PStepDupliTop() {}
      static void apply_( const PStep*, VMContext* ctx );
      virtual void describeTo( String& ) const;
   };
   PStepDupliTop m_dupliTop;
   
   /** Duplicate the topmost 2 items in the stack.  */
   class PStepDupliTop2: public PStep
   {
   public:
      PStepDupliTop2() { apply = apply_; }
      virtual ~PStepDupliTop2() {}
      static void apply_( const PStep*, VMContext* ctx );
      virtual void describeTo( String& ) const;
   };
   PStepDupliTop2 m_dupliTop2;
   
   /** Duplicate the topmost 3 items in the stack.  */
   class PStepDupliTop3: public PStep
   {
   public:
      PStepDupliTop3() { apply = apply_; }
      virtual ~PStepDupliTop3() {}
      static void apply_( const PStep*, VMContext* ctx );
      virtual void describeTo( String& ) const;
   };
   PStepDupliTop3 m_dupliTop3;

   
   /** Swap the topmost item and the second-topmost one in the stack. */
   class PStepSwapTop: public PStep
   {
   public:
      PStepSwapTop() { apply = apply_; }
      virtual ~PStepSwapTop() {}
      static void apply_( const PStep*, VMContext* ctx );
      virtual void describeTo( String& ) const;
   };
   
   PStepSwapTop m_swapTop;
   
   /** Swap the topmost item putting it below the topmost 2. 
    
    This is used when having evaluated an expression that must be sent to
    an array setIndex, as setIndex wants \b value, \b array, \b index
    in the stack in this order.
    */   
   class PStepSwapTopWith2: public PStep
   {
   public:
      PStepSwapTopWith2() { apply = apply_; }
      virtual ~PStepSwapTopWith2() {}
      static void apply_( const PStep*, VMContext* ctx );
      virtual void describeTo( String& ) const;
   };
   
   PStepSwapTopWith2 m_swapTopWith2;
   
   /** Base class for copy-down steps. 
    Copy-down steps copy the topmost item to the nth item in the stack.
    
    \note the VM model only ensures that the items in the current call frame
    are accessible randomly in the data stack, but no check is performed here.
    
    */
   class PStepCopyDown2: public PStep
   {
   public:
      PStepCopyDown2() { apply = apply_; }
      virtual ~PStepCopyDown2() {}
      static void apply_( const PStep*, VMContext* ctx );
      virtual void describeTo( String& ) const;
   };
   PStepCopyDown2 m_copyDown2;
   
   class PStepCopyDown3: public PStep
   {
   public:
      PStepCopyDown3() { apply = apply_; }
      virtual ~PStepCopyDown3() {}
      static void apply_( const PStep*, VMContext* ctx );
      virtual void describeTo( String& ) const;
   };
   PStepCopyDown3 m_copyDown3;
   
   class PStepCopyDown4: public PStep
   {
   public:
      PStepCopyDown4() { apply = apply_; }
      virtual ~PStepCopyDown4() {}
      static void apply_( const PStep*, VMContext* ctx );
      virtual void describeTo( String& ) const;
   };
   PStepCopyDown4 m_copyDown4;
   
   class PStepCopyDown5: public PStep
   {
   public:
      PStepCopyDown5() { apply = apply_; }
      virtual ~PStepCopyDown5() {}
      static void apply_( const PStep*, VMContext* ctx );
      virtual void describeTo( String& ) const;
   };
   PStepCopyDown5 m_copyDown5;

   /** Cleans the topmost value */
   class PStepPop: public PStep
   {
   public:
      PStepPop() { apply = apply_; }
      virtual ~PStepPop() {}
      static void apply_( const PStep*, VMContext* ctx );
      virtual void describeTo( String& ) const;
   };
   PStepPop m_pop;
   
   /** Cleans the topmost 2 values */
   class PStepPop2: public PStep
   {
   public:
      PStepPop2() { apply = apply_; }
      virtual ~PStepPop2() {}
      static void apply_( const PStep*, VMContext* ctx );
      virtual void describeTo( String& ) const;
   };
   PStepPop2 m_pop2;
   
   /** Cleans the topmost 3 values */
   class PStepPop3: public PStep
   {
   public:
      PStepPop3() { apply = apply_; }
      virtual ~PStepPop3() {}
      static void apply_( const PStep*, VMContext* ctx );
      virtual void describeTo( String& ) const;
   };
   PStepPop3 m_pop3;
   
   /** Pops the topmost value of the stack and copies it on the previous item. */
   class PStepDragDown: public PStep
   {
   public:
      PStepDragDown() { apply = apply_; }
      virtual ~PStepDragDown() {}
      static void apply_( const PStep*, VMContext* ctx );
      virtual void describeTo( String& ) const;
   };
   PStepDragDown m_dragDown;

   /** Adds one empty space in top of the stack.
    The space added is left as is, and not nilled!
    */
   class PStepAddSpace: public PStep
   {
   public:
      PStepAddSpace() {apply = apply_; }
      virtual ~PStepAddSpace() {}
      static void apply_( const PStep*, VMContext* ctx );
      virtual void describeTo( String& ) const;
   };
   PStepAddSpace m_addSpace;
   
   /** Pushes a nil item on the top of the stack.
    The space added is nilled for safety.
    */
   class PStepPushNil: public PStep
   {
   public:
      PStepPushNil() {apply = apply_; }
      virtual ~PStepPushNil() {}
      static void apply_( const PStep*, VMContext* ctx );
      virtual void describeTo( String& ) const;
   };
   PStepPushNil m_pushNil;
   
   
   /** Pstep immediately invoking return from current frame.
    Use as a safeguard in functional programming.
    
    This is pushed by functions that have just started and are introducing sequences 
    that may or may not complete immediately. By adding this explicit request to
    the VM to return, the functions can make sure that the frame is returned
    when the pushed sequences are completed.
    
    This PStep has not a "keep self" version because returning the frame will
    discard itself and any other PStep up to the nearest call frame.
    */
   class PStepReturnFrame: public PStep
   {
   public:
      PStepReturnFrame() {apply = apply_; }
      virtual ~PStepReturnFrame() {}
      static void apply_( const PStep*, VMContext* ctx );
      virtual void describeTo( String& ) const;
   };
   PStepReturnFrame m_returnFrame;
   
   
   class PStepReturnFrameWithTop: public PStep
   {
   public:
      PStepReturnFrameWithTop() {apply = apply_; }
      virtual ~PStepReturnFrameWithTop() {}
      static void apply_( const PStep*, VMContext* ctx );
      virtual void describeTo( String& ) const;
   };
   PStepReturnFrameWithTop m_returnFrameWithTop;
   
      class PStepReturnFrameWithTopDoubt: public PStep
   {
   public:
      PStepReturnFrameWithTopDoubt() {apply = apply_; }
      virtual ~PStepReturnFrameWithTopDoubt() {}
      static void apply_( const PStep*, VMContext* ctx );
      virtual void describeTo( String& ) const;
   };
   PStepReturnFrame m_returnFrameWithTopDoubt;
};

}

#endif	/* _FALCON_STDSTEPS_H */

/* end of stdsteps.h */