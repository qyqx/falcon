/*
   FALCON - The Falcon Programming Language.
   FILE: coremodule.cpp

   Core module -- main file.
   -------------------------------------------------------------------
   Author: Giancarlo Niccolai
   Begin: Sat, 30 Apr 2011 12:25:36 +0200

   -------------------------------------------------------------------
   (C) Copyright 2011: the FALCON developers (see list in AUTHORS file)

   See LICENSE file for licensing details.
*/

#undef SRC
#define SRC "falcon/cm/coremodule.cpp"

#include <falcon/cm/coremodule.h>
#include <falcon/cm/iff.h>
#include <falcon/cm/inspect.h>
#include <falcon/cm/print.h>
#include <falcon/cm/uri.h>
#include <falcon/cm/path.h>
#include <falcon/cm/storer.h>
#include <falcon/cm/restorer.h>
#include <falcon/cm/stream.h>
#include <falcon/cm/textstream.h>
#include <falcon/cm/textwriter.h>
#include <falcon/cm/textreader.h>
#include <falcon/cm/datawriter.h>
#include <falcon/cm/datareader.h>
#include <falcon/cm/parallel.h>

// the standard error classes
#include <falcon/errorclasses.h>


namespace Falcon {

CoreModule::CoreModule():
   Module("core")
{
   Ext::ClassStream* classStream = new Ext::ClassStream;
   
   *this
      // Standard functions
      << new Ext::FuncPrintl
      << new Ext::FuncPrint
      << new Ext::Inspect
      << new Ext::Iff
      
      // Standard classes
      << new Ext::ClassURI
      << new Ext::ClassPath
      << new Ext::ClassRestorer
      << new Ext::ClassStorer
      << new Ext::ClassParallel
      << classStream
      << new Ext::ClassTextStream( classStream )
      << new Ext::ClassTextWriter( classStream )
      << new Ext::ClassTextReader( classStream )
      << new Ext::ClassDataWriter( classStream )
      << new Ext::ClassDataReader( classStream )
      ;
}

}

/* end of coremodule.cpp */