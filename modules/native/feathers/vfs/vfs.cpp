/*
   FALCON - The Falcon Programming Language.
   FILE: vfs.cpp

   Interface to Falcon Virtual File System -- main file.
   -------------------------------------------------------------------
   Author: Giancarlo Niccolai
   Begin: Mon, 24 Oct 2011 14:34:31 +0200

   -------------------------------------------------------------------
   (C) Copyright 2011: the FALCON developers (see list in AUTHORS file)

   See LICENSE file for licensing details.
*/

#undef SRC
#define SRC "falcon/moduels/native/feathers/vfs.cpp"

#include <falcon/symbol.h>
#include <falcon/item.h>
#include <falcon/vfsprovider.h>
#include <falcon/errors/linkerror.h>

#include "vfs.h"
#include "open.h"
#include "create.h"

namespace Falcon {

VFSModule::VFSModule():
   Module("vfs")
{
   // We want the URI core class.
   addImportRequest( &onURIResolved, "URI" );
   // and, of course, we need the stream class.
   addImportRequest( &onStreamResolved, "Stream" );
   
   *this
      // Standard functions
      << new Ext::Open( this )
      << new Ext::Create( this )
      
      // Standard classes
      ;
   
   // constants
   this->addConstant( "O_RD", (int64)VFSProvider::OParams::e_oflag_rd );
   this->addConstant( "O_WR", (int64)VFSProvider::OParams::e_oflag_wr );
   this->addConstant( "O_APPEND", (int64)VFSProvider::OParams::e_oflag_append );
   this->addConstant( "O_TRUNC", (int64)VFSProvider::OParams::e_oflag_trunc );
   this->addConstant( "O_RAW", (int64)FALCON_VFS_MODE_FLAG_RAW );
   
   this->addConstant( "SH_NR", (int64)VFSProvider::OParams::e_sflag_nr );
   this->addConstant( "SH_NW", (int64)VFSProvider::OParams::e_sflag_nw );

   this->addConstant( "C_NOOVR", (int64)VFSProvider::CParams::e_cflag_noovr );
   this->addConstant( "C_NOSTREAM", (int64)VFSProvider::CParams::e_cflag_nostream );
   this->addConstant( "C_RAW", (int64)FALCON_VFS_MODE_FLAG_RAW );
   
   

}

VFSModule::~VFSModule()
{}

Falcon::Error* VFSModule::onURIResolved( Falcon::Module* requester, const Falcon::Module* , const Falcon::Symbol* sym )
{   
   // printl should really be a function in a global symbol ,but...
   if( ! sym->defaultValue().isClass() )
   {
      return new Falcon::LinkError( Falcon::ErrorParam( 
            Falcon::e_link_error, __LINE__, requester->name() )
         .extra( "Class URI not found" ) );
   }

   // We know the requester is an instance of our module.
   static_cast<VFSModule*>(requester)->m_uriClass = (Class*)sym->defaultValue().asInst();

   // we have no error to signal. 
   return 0;
}

Falcon::Error* VFSModule::onStreamResolved( Falcon::Module* requester, const Falcon::Module* , const Falcon::Symbol* sym )
{   
   // printl should really be a function in a global symbol ,but...
   if( ! sym->defaultValue().isClass() )
   {
      return new Falcon::LinkError( Falcon::ErrorParam( 
            Falcon::e_link_error, __LINE__, requester->name() )
         .extra( "Class Stream not found" ) );
   }

   // We know the requester is an instance of our module.
   static_cast<VFSModule*>(requester)->m_streamClass = (Class*)sym->defaultValue().asInst();

   // we have no error to signal. 
   return 0;
}

}

//=================================
// Export function.

FALCON_MODULE_DECL 
{
   Falcon::Module* mod = new Falcon::VFSModule;
   return mod;
}

/* end of vfs.cpp */