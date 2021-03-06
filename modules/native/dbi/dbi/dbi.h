/*
 * FALCON - The Falcon Programming Language.
 * FILE: dbi.h
 *
 * Short description
 * -------------------------------------------------------------------
 * Author: Giancarlo Niccolai and Jeremy Cowgar
 * Begin: Sun, 23 Dec 2007 20:33:57 +0100
 *
 * -------------------------------------------------------------------
 * (C) Copyright 2004: the FALCON developers (see list in AUTHORS file)
 *
 * See LICENSE file for licensing details.
 */

#ifndef DBI_H
#define DBI_H

#include <falcon/modloader.h>
#include <falcon/srv/dbi_service.h>

namespace Falcon
{

/**
 * Load the DBI driver.
 */
class DBILoaderImpl: public DBILoader
{
public:
   DBILoaderImpl();
   ~DBILoaderImpl();

   virtual DBIService *loadDbProvider( VMachine *vm, const String &provName );

};

}

// Singleton instance.
extern Falcon::DBILoaderImpl theDBIService;

#endif

/* end of dbi.h */

