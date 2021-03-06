/*
   The Falcon Programming Language
   FILE: dynlib_sys_gcc_amd64.cpp

   Direct dynamic library interface for Falcon
   System specific extensions - GCC on intel32
   -------------------------------------------------------------------
   Author: Giancarlo Niccolai
   Begin:

   -------------------------------------------------------------------
   (C) Copyright 2010 Giancarlo Niccolai

   See the LICENSE file distributed with this package for licensing details.
*/

#include "dynlib_sys.h"
namespace Falcon {
namespace Sys {



void dynlib_call( void *faddress, void** parameters, int* sizes, byte* retval )
{
   int count = 0;
   __asm__ __volatile__(
      "pushl   %%ebx \n"            /* Save used registers */
      "pushl   %%esi \n"
      "pushl   %%edi \n"
      "movl    %%esp, %%edi\n"      /* Create a fake frame */

      "1: movl %3, %%esi \n" /* Get next parameter's size */
      "addl    %4, %%esi \n" /* Move count ptrs forward */
      "movl    (%%esi), %%ebx \n"
      "andl    $0x7f, %%ebx \n"   /* We're not interested in knowing the value is float */
      "orl     %%ebx, %%ebx \n" /* Check if this was the last size */
      "jz      2f\n"
      "movl    %2, %%esi \n" /* Get next parameter */
      "addl    %4, %%esi \n" /* Move count ptrs forward */
      "movl    (%%esi), %%esi \n"  /* Go to the N-int buffero position */
      "addl    %%ebx, %%esi \n"  /* Start from bottom */
      "4: subl $4,%%esi\n" /* Prepare to read next bytes, if we're not done... */
      "movl    (%%esi), %%edx \n"
      "pushl   %%edx\n" /* Push them on the stack */
      "subl    $4,%%ebx\n" /* More bytes to be pushed? */
      "jnz     4b\n"       /* yes -- loop again */
      "addl    $4,%4\n"    /* Next parameter (int/ptr size fwd)... */
      "jmp     1b\n"

      "2: call *%1\n"         /* perform the call */

      "movl  %0, %%esi\n"     /* Get the return format */
      "movl  (%%esi), %%ebx\n"
      "orl   %%ebx, %%ebx\n"  /* No return? */
      "jz    100f\n"          /* Go Away */

      "cmpl  $0x8c, %%ebx\n"  /* 0x80 + 0x0c -- long double 80 bits */
      "jne   5f\n"
      "movl  %%eax, (%%esi) \n"
      "movl  %%edx, 4(%%esi) \n"
      "movl  %%ecx, 8(%%esi) \n"
      "jmp   100f \n"

      "5: cmpl  $0x88, %%ebx\n"  /* 0x80 + 0x8 -- double 64 bits */
      "jne   6f\n"
      "fstpl (%%esi) \n"
      "jmp   100f \n"

      "6: cmpl  $0x84, %%ebx\n"  /* 0x80 + 0x4 -- float 64 bits */
      "jne   7f\n"
      "fstps (%%esi) \n"
      "jmp   100f \n"

      "7: cmpl   $0x8, %%ebx\n"  /* long long -- 8 bytes */
      "jne   8f\n"
      "movl  %%eax, (%%esi) \n"
      "movl  %%edx, 4(%%esi) \n"
      "jmp   100f \n"

      "8: cmpl  $0x4, %%ebx\n"  /* long/int/everything else -- 4 bytes */
      "jne   9f\n"
      "movl  %%eax, (%%esi) \n"
      "jmp   100f \n"

      "9: movl $0, (%%esi) \n"       /* zero the return */

      "100: movl  %%edi, %%esp\n"      /* Restore my fake frame */
      "popl  %%edi\n"
      "popl  %%esi \n"
      "popl  %%ebx \n"

      :"=m"(retval) /* output */
      :"m"(faddress), "m"(parameters), "m"(sizes), "m"(count)  /* input */
      :"%esp"         /* clobbered register */
   );
}

}
}

/* end of dynlib_sys_gcc_amd64.cpp */
