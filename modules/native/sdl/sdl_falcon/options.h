/*
   FALCON - The Falcon Programming Language.
   FILE: options.h

   Options storage for falcon compiler.
   -------------------------------------------------------------------
   Author: Giancarlo Niccolai
   Begin: ven set 10 2004

   -------------------------------------------------------------------
   (C) Copyright 2004: the FALCON developers (see list in AUTHORS file)

   See LICENSE file for licensing details.
*/

/** \file
   Options storage for falcon compiler.
*/

#ifndef flc_options_H
#define flc_options_H

#include <falcon/string.h>
#include <falcon/genericlist.h>

/** Options storage for falcon compiler
   This class is just a nice place to store options for the compiler and their defaults.
*/

class FalconOptions
{
   void modalGiven();
   bool m_modal;
   bool m_justinfo;

public:

   Falcon::String input;
   Falcon::String output;
   Falcon::String load_path;
   Falcon::String io_encoding;
   Falcon::String source_encoding;
   Falcon::String module_language;
   Falcon::List preloaded;
   Falcon::List directives;
   Falcon::List defines;

   bool compile_only;
   bool run_only;
   bool tree_out;
   bool assemble_out;
   bool search_path;
   bool force_recomp;
   bool check_memory;

   bool comp_memory;
   bool recompile_on_load;
   bool save_modules;
   bool wait_after;
   bool parse_ftd;

   bool compile_tltable;
   bool interactive;

   bool ignore_syspath;

   FalconOptions();

   void parse( int argc, char **argv, int &script_pos );
   void usage( bool deep = false );
   void version();

   /** Returns true if the parsed options required an immediate exit. */
   bool wasJustInfo() const { return m_justinfo; }
};

#endif

/* end of options.h */
