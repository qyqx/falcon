/*
   FALCON - The Falcon Programming Language.
   FILE: falpack.cpp

   Packager for Falcon stand-alone applications
   -------------------------------------------------------------------
   Author: Giancarlo Niccolai
   Begin: Fri, 22 Jan 2010 20:18:36 +0100

   -------------------------------------------------------------------
   (C) Copyright 2010: the FALCON developers (see list in AUTHORS file)

   See LICENSE file for licensing details.
*/

#include <falcon/engine.h>
#include <falcon/sys.h>
#include "options.h"
#include "utils.h"
#include "falpack_sys.h"

#include <vector>
#include <set>

using namespace Falcon;

String load_path;
String io_encoding;
bool ignore_defpath = false;


// Forward decl
bool transferModules( Options &options, const String& mainScript );

static void version()
{
   stdOut->writeString( "Falcon application packager.\n" );
   stdOut->writeString( "Version " );
   stdOut->writeString( FALCON_VERSION " (" FALCON_VERSION_NAME ")" );
   stdOut->writeString( "\n" );
   stdOut->flush();
}

static void usage()
{
   stdOut->writeString( "Usage: falpack [options] <main_script>\n" );
   stdOut->writeString( "\n" );
   stdOut->writeString( "Options:\n" );
   stdOut->writeString( "   -b <module> Blacklists this module (by module name)\n" );
   stdOut->writeString( "   --bin <dir> Specify directory where falcon binary resides\n" );
   stdOut->writeString( "   -e <enc>    Source files encoding\n" );
   stdOut->writeString( "   -h, -?      This help\n" );
   stdOut->writeString( "   --lib <dir> Specify directory where falcon runtime library resides\n" );
   stdOut->writeString( "   -L <dir>    Redefine FALCON_LOAD_PATH\n" );
   stdOut->writeString( "   -M          Pack also pre-compiled modules\n" );
   stdOut->writeString( "   -P <dir>    Save the package in this directory\n" );
   stdOut->writeString( "   -r <name>   Install <name> instead of \"falcon\" as interpreter\n" );
   stdOut->writeString( "   -R <dir>    Change root for system data into <dir> (dflt: _system)\n" );
   stdOut->writeString( "   -s          Strip sources\n" );
   stdOut->writeString( "   -S          Do not store system files (engine + runner)\n" );
   stdOut->writeString( "   -v          Prints version and exit\n" );
   stdOut->writeString( "   -V          Verbose mode\n" );
   stdOut->writeString( "\n" );
   stdOut->flush();
}

bool copyAllResources( Options& options, const Path& from, const Path& tgtPath )
{
   // do we have an extension filter?
   bool bHasExt = from.getExtension() !=  "";

   VFSProvider* file = Engine::getVFS("file");
   if( file == 0 )
   {
      error( "Can't find FILE resource" );
      return false;
   }

   DirEntry *entry = file->openDir( from.getFullLocation() );
   if( entry == 0 )
   {
      warning( "Can't open directory " + from.getFullLocation() );
      return false;
   }

   String fname;
   while( entry->read( fname ) )
   {
      if( fname == ".." || fname == "." )
      {
         continue;
      }

      FileStat fs;
      if ( ! Sys::fal_stats( from.getFullLocation() + "/" + fname, fs ) )
      {
         continue;
      }

      if ( fs.m_type == FileStat::t_normal || fs.m_type == FileStat::t_link )
      {
         // do we filter the extension?
         if( bHasExt )
         {
            if ( ! fname.endsWith( "." + from.getExtension(), true ) )
            {
               continue;
            }
         }

         // TODO: Jail resources under modpath
         if ( ! copyFile( from.getFullLocation() + "/" + fname, tgtPath.getFullLocation() + "/" + fname ) )
         {
            warning( "Cannot copy resource " +
                  from.getFullLocation() + "/" + fname
                  + " into "
                  + tgtPath.getFullLocation() + "/" + fname );
            entry->close();
            delete entry;
            return false;
         }

         /*
         // descend
         Path nfrom( from );
         nfrom.setFullLocation( from.getFullLocation() + "/" + fname );
         if( ! copyAllResources( options, nfrom, modPath, tgtPath ) )
         {
            return false;
         }
         */
      }
   }

   entry->close();
   delete entry;

   return true;
}


bool copyResource( Options& options, const String& resource, const Path& modPath, const Path& tgtPath )
{
   message( "Storing resource " + resource );
   Path resPath( resource );

   Path modResPath( modPath );
   Path tgtResPath( tgtPath );

   if( resPath.isAbsolute() )
   {
      warning( "Resource " + resource + " has an absolute path." );
      modResPath.setFullLocation( modPath.getFullLocation() + resPath.getFullLocation() );
      tgtResPath.setFullLocation( tgtPath.getFullLocation() + resPath.getFullLocation() );
   }
   else
   {
      modResPath.setFullLocation( modPath.getFullLocation() +"/"+ resPath.getFullLocation() );
      tgtResPath.setFullLocation( tgtPath.getFullLocation() +"/"+ resPath.getFullLocation() );
   }

   modResPath.setFilename( resPath.getFilename() );
   tgtResPath.setFilename( resPath.getFilename() );

   // create target path
   int32 fsStatus;
   if( ! Sys::fal_mkdir( tgtResPath.getFullLocation(), fsStatus, true ) )
   {
      warning( "Cannot create path " + tgtResPath.getFullLocation()
            + " for resource " + modResPath.get() );

      return false;
   }

   if( resPath.getFile() == "*" )
   {
     Path from( resPath );
     from.setFullLocation( modPath.getFullLocation() + "/" + from.getFullLocation() );
     if ( ! copyAllResources( options, from, tgtResPath ) )
     {
        return false;
     }
   }
   else
   {
      // TODO: Jail resources under modpath
      if ( ! copyFile( modResPath.get(), tgtResPath.get() ) )
      {
         warning( "Cannot copy resource " + modResPath.get() + " into " + tgtResPath.get() );
         return false;
      }
   }

   return true;
}

bool copyFtr( const Path& src, const Path &tgt )
{
   VFSProvider* file = Engine::getVFS("file");
   fassert( file != 0 );
   DirEntry *entry = file->openDir( src.getFullLocation() );
   if( entry == 0 )
   {
      warning( "Can't open directory " + src.getFullLocation() );
      return false;
   }

   String fname;
   String module = src.getFile();
   Path orig( src );
   Path target( tgt );

   while( entry->read( fname ) )
   {
      if( fname.startsWith( module ) && fname.endsWith( ".ftt" ) )
      {
         orig.setFilename( fname );
         target.setFilename( fname );
         if( ! copyFile( orig.get(), target.get() ) )
         {
            warning( "Can't copy source FTT file " + orig.get() );
         }
      }
   }

   entry->close();
   delete entry;

   return true;
}

static void relativize( const Path& source, Path& target )
{
   if ( ! target.isAbsolute() )
   {
      return;
   }

   String sFull1 = source.getFullLocation();
   String sFull2 = target.getFullLocation();

   // checking the equality, check also if the last path in sFull2 matches sFull1 completely
   if( sFull1.length() < sFull2.length() &&
         sFull2.subString(0, sFull1.length()+1 ) == sFull1 + "/" )
   {
      target.setFullLocation( sFull2.subString( sFull1.length() + 1 ) );
   }
   else if( sFull1 == sFull2 )
   {
      target.setFullLocation( "" );
   }
   else
   {
      // there is nothing we can do.
   }

}

void addPlugins( const Options& options_main, const String& parentModule, const String& path )
{
   message( "Loading plugin \"" + path +"\" for module " + parentModule );

   Path modPath( parentModule );
   modPath = modPath.getFullLocation() + "/" + path;

   // prepare the target plugin path
   Path outputPath( modPath );
   outputPath.setFilename("");
   Path mainPath;
   mainPath.setFullLocation( options_main.m_sMainScriptPath );
   relativize( mainPath, outputPath );
   outputPath.setFullLocation(
         options_main.m_sTargetDir +"/"+ outputPath.getLocation() );


   // topmost location of the plugin must be
   if( path.endsWith("*") )
   {
      VFSProvider* file = Engine::getVFS("file");
      fassert( file != 0 );
      DirEntry *entry = file->openDir( modPath.getFullLocation() );
      if( entry == 0 )
      {
         warning( "Can't open plugin directory \"" + modPath.getFullLocation() + "\" for module "
               + parentModule );
      }

      String fname;
      while( entry->read( fname ) )
      {

         // binary?
         if ( fname.endsWith(".fam") )
         {
            // do we have also a source?
            modPath.setFilename( fname );
            modPath.setExtension( "fal" );
            FileStat famStats;

            if( Sys::fal_stats( modPath.get(), famStats ) )
            {
               // we have already a fal that has been transferred or will be transferred later,
               // so wait for that.
               continue;
               // otherwise, go on transferring the source.
            }

            // same for ftd
            modPath.setExtension( "ftd" );
            if( Sys::fal_stats( modPath.get(), famStats ) )
            {
               continue;
            }

         }
         else if( fname.endsWith( DllLoader::dllExt() ) )
         {
            //Transfer DLLS as they are.
         }
         // source?
         else if( fname.endsWith( ".fal" ) || fname.endsWith(".ftd") )
         {
               // go on, transfer the source.
         }
         else
         {
            // we don't know how to manage other plugins
            continue;
         }

         // copy our options, so that transferModule doesn't pollute them
         Options options( options_main );

         options.m_sTargetDir = outputPath.get();
         // ok, transfer the thing
         modPath.setFilename( fname );
         transferModules( options, modPath.get() );
      }

      entry->close();
      delete entry;
   }
   else
   {
      // copy our options, so that transferModule doesn't pollute them
      Options options( options_main );
      options.m_sTargetDir = outputPath.get();
      transferModules( options, modPath.get() );
   }
}


bool storeModule( Options& options, Module* mod )
{
   // this is the base path for the module
   Path modPath( mod->path() );
   Path tgtPath;
   tgtPath.setFullLocation( options.m_sTargetDir );

   // normalize module path
   while( modPath.getFullLocation().startsWith("./") )
   {
      modPath.setFullLocation( modPath.getFullLocation().subString(2) );
   }

   message( String("Processing module ").A( modPath.get() ) );

   // strip the main script path from the module path.
   String modloc = modPath.getFullLocation();

   if ( modloc.find( options.m_sMainScriptPath ) == 0 )
   {
      // The thing came from below the main script.
      modloc = modloc.subString(options.m_sMainScriptPath.length() );
      if ( modloc != "" && modloc.getCharAt(0) == '/' )
      {
         modloc = modloc.subString(1);
      }
      tgtPath.setFullLocation( tgtPath.get() + "/" + modloc );
   }
   else
   {
      // if it's coming from somewhere else in the loadpath hierarcy,
      // we must store it below the topmost dir.
      tgtPath.setFullLocation( tgtPath.get() + "/" + options.m_sSystemRoot );

      // Find the path in LoadPath that caused this module to load,
      // strip it away and reproduce it below the SystemRoot.
      // For example, strip /usr/lib/falcon/ from system scripts.
      std::vector<String> paths;
      splitPaths( options.m_sLoadPath, paths );

      for( uint32 i = 0; i < paths.size(); ++i )
      {
         if( modloc.startsWith( paths[i] ) )
         {
            String sSysPath = modloc.subString( paths[i].size() + 1 );
            if( sSysPath != "" )
            {
               tgtPath.setFullLocation( tgtPath.get() + "/" + sSysPath );
            }
            break;
         }
      }
   }

   // store it
   int fsStatus;
   if ( ! Sys::fal_mkdir( tgtPath.getFullLocation(), fsStatus, true ) )
   {
      error( String("Can't create ") + tgtPath.getFullLocation() );
      return false;
   }

   tgtPath.setFilename( modPath.getFilename() );

   // should we store just sources, just fam or both?
   if( modPath.getExtension() != "fam" && modPath.getExtension() != DllLoader::dllExt() )
   {
      // it's a source file.
      if ( ! options.m_bStripSources )
      {
         if( ! copyFile( modPath.get(), tgtPath.get() ) )
         {
            error( String("Can't copy \"") + modPath.get() + "\" into \"" +
                  tgtPath.get() + "\"" );
            return false;
         }
      }

      // should we save the fam?
      if( options.m_bStripSources || options.m_bPackFam )
      {
         tgtPath.setExtension("fam");
         FileStream famFile;

         if ( ! famFile.create( tgtPath.get(), (Falcon::BaseFileStream::t_attributes) 0644  )
             || ! mod->save(&famFile) )
         {
            error( "Can't create \"" + tgtPath.get() + "\"" );
            return false;
         }
         famFile.flush();
         famFile.close();

      }
   }
   else
   {
      // just blindly copy everything else.
      if( ! copyFile( modPath.get(), tgtPath.get() ) )
      {
         error( "Can't copy \"" + modPath.get() + "\" into \"" + tgtPath.get() + "\"" );
         return false;
      }
   }

   // now copy .ftr files, if any.
   modPath.setExtension( "ftr" );
   FileStat ftrStat;

   if ( Sys::fal_stats( modPath.get(), ftrStat ) )
   {
      message( "Copying translation file " + modPath.get() );

      tgtPath.setExtension( "ftr" );
      // just blindly copy everything else.
      if( ! copyFile( modPath.get(), tgtPath.get() ) )
      {
         warning( "Can't copy \"" + modPath.get() + "\" into \"" + tgtPath.get() + "\"\n" );
      }
   }

   // Should we store .ftt as well?
   if ( ! options.m_bStripSources )
   {
      copyFtr( modPath, tgtPath );
   }

   // and now, the resources.
   std::vector<String> reslist;
   if( getAttribute( mod, "resources", reslist ) )
   {
      for ( uint32 i = 0; i < reslist.size(); ++i )
      {
         copyResource( options, reslist[i], modPath, tgtPath );
      }
   }

   // and finally, the dynamic libraries associated with this module.
   std::vector<String> dynliblist;
   if( getAttribute( mod, "dynlib", dynliblist ) )
   {
      copyDynlibs( options, mod->path(), dynliblist );
   }

   return true;
}


bool transferModules( Options &options, const String& mainScript )
{
   ModuleLoader ml("");

   ml.alwaysRecomp( true );
   ml.saveModules( false );

   if( options.m_sEncoding != "" )
      ml.sourceEncoding( options.m_sEncoding );

   // prepare the load path.
   if( options.m_sLoadPath != "" )
   {
      ml.addSearchPath( options.m_sLoadPath );
   }
   else
   {
      ml.addFalconPath();
   }

   // add script path (always)
   Path scriptPath( mainScript );
   if( scriptPath.getFullLocation() != "" )
      ml.addSearchPath( scriptPath.getFullLocation() );

   // and communicate to the rest of the program the search path we used.
   options.m_sLoadPath = ml.getSearchPath();
   message( "Falcon load path set to: " + options.m_sLoadPath );

   // Load the main script alone; It's useful for two reasons:
   // 1) if the main script fails to laod, we can issue a more precise error
   // 2) using the path() from the main script we have a normalized representation
   //    of the topmost directory where the other sub-modules will be found.
   // Note -- throws on error; for now, we don't use the ability to signal
   //         a different error.

   Module* mainMod = ml.loadFile( mainScript );

   // update the script path with the normalized one.
   options.m_sMainScript = mainMod->path();
   options.m_sMainScriptPath = Path(mainMod->path()).getFullLocation();
   message( "Normalized main script path: " + options.m_sMainScriptPath );

   // load the rest of the application.
   Runtime rt( &ml );
   rt.addModule( mainMod );

   const ModuleVector* mv = rt.moduleVector();
   for( uint32 i = 0; i < mv->size(); i ++ )
   {
      Module *mod = mv->moduleAt(i);

      if( options.isBlackListed(mod->name()) )
      {
         message( "Skipping module \"" + mod->name() +  "\" because it's blacklisted" );
         continue;
      }

      if( options.m_bNoSysFile && options.isSysModule(mod->name()) )
      {
         message( "Skipping module \"" + mod->name() +  "\" because it's a system module" );
         continue;
      }

      if( ! storeModule( options, mod ) )
         return false;

      // add the plugins.
      std::vector<String> reslist;
      if( getAttribute( mod, "plugins", reslist ) )
      {
         for ( uint32 i = 0; i < reslist.size(); ++i )
         {
            addPlugins( options, mod->path(), reslist[i] );
         }
      }
   }

   return true;
}


int main( int argc, char *argv[] )
{
   Falcon::GetSystemEncoding( io_encoding );

   if ( io_encoding != "" )
   {
      Transcoder *trans = TranscoderFactory( io_encoding, 0, true );
      if ( trans == 0 )
      {
         stdOut = new StdOutStream();
         stdOut->writeString( "Unrecognized system encoding '" + io_encoding + "'; falling back to C.\n\n" );
         stdOut->flush();
         stdErr = new StdErrStream;
      }
      else
      {
         stdOut = AddSystemEOL( TranscoderFactory( io_encoding, new StdOutStream, true ), true );
         stdErr = AddSystemEOL( TranscoderFactory( io_encoding, new StdErrStream, true ), true );
      }
   }

   Options options;

   if ( ! options.parse( argc-1, argv+1 ) )
   {
      stdOut->writeString( "Fatal: invalid parameters.\nUse -h for help.\n\n" );
      return 1;
   }

   if( options.m_bVersion )
   {
      version();
   }

   if( options.m_bHelp )
   {
      usage();
   }

   setVerbose( options.m_bVerbose );

   if ( ! options.m_sMainScript )
   {
      stdOut->writeString( "falpack: Nothing to do.\n\n" );
      return 0;
   }

   Engine::Init();

   // by default store the application in a subdirectory equal to the name of the
   // application.
   Path target( options.m_sMainScript );
   if( options.m_sTargetDir == "" )
   {
      options.m_sTargetDir = target.getFile();
   }

   //===============================================================
   // We need a runtime and a module loader to load all the modules.
   bool bResult;

   try
   {
      bResult = transferModules( options, options.m_sMainScript );

      if ( bResult )
      {
         bResult = transferSysFiles( options, options.m_bNoSysFile );
      }
   }
   catch( Error* err )
   {
      // We had a compile time problem, very probably
      bResult = false;
      error( String( "Compilation error.\n" ) + err->toString() );
      err->decref();
   }

   delete stdOut;
   delete stdErr;

   Engine::Shutdown();

   return bResult ? 0 : 1;
}


/* end of falpack.cpp */
