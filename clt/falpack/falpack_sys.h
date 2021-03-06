/*
   FALCON - The Falcon Programming Language.
   FILE: falpack_sys.h

   System specific extensions for Falcon
   -------------------------------------------------------------------
   Author: Giancarlo Niccolai
   Begin: Sun, 31 Jan 2010 11:29:07 +0100

   -------------------------------------------------------------------
   (C) Copyright 2010: the FALCON developers (see list in AUTHORS file)

   See LICENSE file for licensing details.
*/

#ifndef FALPACK_SYS_H_
#define FALPACK_SYS_H_

#include <falcon/string.h>
#include <vector>

namespace Falcon
{

class Options;

bool transferSysFiles( Options &options, bool bJustScript );
bool copyDynlibs( Options& options, const String& modpath, const std::vector<String>& dynlibs );

}

#endif

/* end of falpack_sys.h */
