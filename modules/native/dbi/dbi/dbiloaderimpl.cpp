/*
 * FALCON - The Falcon Programming Language.
 * FILE: dbiloaderimpl.cpp
 *
 * Implementation of the DBI loader service, used mainly by this module.
 * -------------------------------------------------------------------
 * Author: Giancarlo Niccolai
 *  Begin: Sun, 23 Dec 2007 20:33:57 +0100
 *
 * -------------------------------------------------------------------
 * (C) Copyright 2007: the FALCON developers (see list in AUTHORS file)
 *
 * See LICENSE file for licensing details.
 */

#include "dbi.h"
#include "dbi_st.h"

#include <falcon/srv/dbi_service.h>
#include <falcon/dbi_error.h>
#include <falcon/globals.h>

namespace Falcon
{

DBILoaderImpl::DBILoaderImpl():
   DBILoader( "DBILOADER" )
{
}

DBILoaderImpl::~DBILoaderImpl()
{}

DBIService *DBILoaderImpl::loadDbProvider( VMachine *vm, const String &provName )
{
   ModuleLoader* loader = new ModuleLoader("");
   DBIService *serv = static_cast<DBIService *>( vm->getService( "DBI_" + provName ) );
   loader->addFalconPath();

   Module *mod = 0;
   if ( serv == 0 )
   {
      // ok, let's try to load the service
      try {
         mod = loader->loadName( "dbi."+ provName );
         vm->link( mod, false );
      }
      catch( Error * )
      {
         delete loader;
         if ( mod != 0 )
            mod->decref();
         throw;
      }

      // the VM has linked the module, we get rid of it.
      mod->decref();
      delete loader;

      //NOTE: we must actually have a local map, as we may be fed with different VMs.
      //We should load only one module for each type, and give to each VM a pre-loaded
      // module, if available.

      // everything went fine; the service is in the vm, but we can access it also
      // from the module
      serv = static_cast<DBIService *>( mod->getService(  "DBI_" + provName ) );

      if ( serv == 0 )
      {
         throw new DBIError( ErrorParam( FALCON_DBI_ERROR_INVALID_DRIVER, __LINE__ )
               .desc( FAL_STR( dbi_msg_driver_not_found ) )
               .extra( "DBI_" + provName )
         );
      }

   }

   serv->init();
   return serv;
}

}

/* end of dbiloaderimpl.cpp */

