/*
   FALCON - The Falcon Programming Language.
   FILE: process_sys_unix.h

   Unix implementation of process handle
   -------------------------------------------------------------------
   Author: Giancarlo Niccolai
   Begin: Sat Jan 29 2005

   -------------------------------------------------------------------
   (C) Copyright 2004: the FALCON developers (see list in AUTHORS file)

   See LICENSE file for licensing details.
*/

/** \file
   Unix implementation of process handle
*/

#ifndef flc_process_sys_unix_H
#define flc_process_sys_unix_H

#include <sys/types.h>
#include "process_sys.h"

namespace Falcon {

class FileService;

namespace Sys {

class UnixProcessHandle: public ProcessHandle
{
   int m_file_des_in[2];
   int m_file_des_out[2];
   int m_file_des_err[2];

   pid_t m_pid;

   friend ProcessHandle *openProcess( String **args, bool sinkin, bool sinkout, bool sinkerr, bool mergeErr, bool bg );
public:
   UnixProcessHandle():
      ProcessHandle()
   {}

   virtual ~UnixProcessHandle();

   pid_t pid() const { return m_pid; }

   virtual ::Falcon::Stream *getInputStream();
   virtual ::Falcon::Stream *getOutputStream();
   virtual ::Falcon::Stream *getErrorStream();

   virtual bool close();
   virtual bool wait( bool block );
   virtual bool terminate( bool severe = false );
};

}
}

#endif

/* end of process_sys_unix.h */