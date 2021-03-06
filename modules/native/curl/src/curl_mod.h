/*
   FALCON - The Falcon Programming Language.
   FILE: curl_ext.cpp

   cURL library binding for Falcon
   Interface extension functions
   -------------------------------------------------------------------
   Author: Giancarlo Niccolai
   Begin: Thu, 27 Nov 2009 16:31:15 +0100

   -------------------------------------------------------------------
   (C) Copyright 2009: The above AUTHOR

         Licensed under the Falcon Programming Language License,
      Version 1.1 (the "License"); you may not use this file
      except in compliance with the License. You may obtain
      a copy of the License at

         http://www.falconpl.org/?page_id=license_1_1

      Unless required by applicable law or agreed to in writing,
      software distributed under the License is distributed on
      an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
      KIND, either express or implied. See the License for the
      specific language governing permissions and limitations
      under the License.

*/

/** \file
   cURL library binding for Falcon
   Internal logic functions - declarations.
*/

#ifndef curl_mod_H
#define curl_mod_H

#include <falcon/falconobject.h>
#include <falcon/itemarray.h>
#include <falcon/error.h>
#include <falcon/coreslot.h>
#include <curl/curl.h>

namespace Falcon {
namespace Mod {

class CurlHandle: public CacheObject
{
public:
   CurlHandle( const CoreClass* cls, bool bDeser = false );
   CurlHandle( const CurlHandle &other );

   virtual ~CurlHandle();
   virtual CurlHandle* clone() const;

   CURL* handle() const { return m_handle; }

   virtual void gcMark( uint32 mark );

   virtual bool serialize( Stream *stream, bool bLive ) const;
   virtual bool deserialize( Stream *stream, bool bLive );

   static CoreObject* Factory( const CoreClass *cls, void *data, bool );

   void setOnDataCallback( const Item& callable );
   void setOnDataStream( Stream* s );
   void setOnDataMessage( const String& msgName );
   void setOnDataGetString();
   void setOnDataStdOut();

   void setReadCallback( const Item& callable );
   void setReadStream( Stream* read );

   CoreString* getData();

   void cleanup();

   /** Creates a curl_slist from a Falcon array of strings.
    *  The list is stored here and destroyed by cleanup().
    *  Returns zero if some elements of ca are not strings.
    *
    */
   struct curl_slist* slistFromArray( CoreArray* ca );

   /** Stores data for post operations.
    * Saves a copy of the string in a local buffer, that is destroyed
    * at cleanup(), and sets the POSTFIELDS and CURLOPT_POSTFIELDSIZE_LARGE
    * options correctly.
    *
    * Multiple operations will cause the previous buffer to be discarded.
    */
   void postData( const String& str );

protected:
   /** Callback modes.
    *
    */
   typedef enum
   {
      e_cbmode_stdout,
      e_cbmode_string,
      e_cbmode_stream,
      e_cbmode_slot,
      e_cbmode_callback
   } t_cbmode;

private:

   static size_t write_stdout( void *ptr, size_t size, size_t nmemb, void *data);
   static size_t write_stream( void *ptr, size_t size, size_t nmemb, void *data);
   static size_t write_msg( void *ptr, size_t size, size_t nmemb, void *data);
   static size_t write_string( void *ptr, size_t size, size_t nmemb, void *data);
   static size_t write_callback( void *ptr, size_t size, size_t nmemb, void *data);

   static size_t read_callback( void *ptr, size_t size, size_t nmemb, void *data);
   static size_t read_stream( void *ptr, size_t size, size_t nmemb, void *data);

   CURL* m_handle;

   Item m_iDataCallback;
   CoreString* m_sReceived;
   Stream* m_dataStream;
   String m_sSlot;

   /** Callback mode, determining which of the method to notify the app is used. */
   t_cbmode m_cbMode;

   Item m_iReadCallback;
   Stream* m_readStream;

   // lists of lists to be destroyed at exit.
   List m_slists;

   void* m_pPostBuffer;
};



class CurlMultiHandle: public CacheObject
{
public:
   CurlMultiHandle( const CoreClass* cls, bool bDeser = false );
   CurlMultiHandle( const CurlMultiHandle &other );

   virtual ~CurlMultiHandle();
   virtual CurlMultiHandle* clone() const;

   CURLM* handle() const { return m_handle; }

   virtual bool serialize( Stream *stream, bool bLive ) const;
   virtual bool deserialize( Stream *stream, bool bLive );

   static CoreObject* Factory( const CoreClass *cls, void *data, bool );

   virtual void gcMark( uint32 mark );

   bool addHandle( CurlHandle* h );
   bool removeHandle( CurlHandle* );

private:
   CURLM* m_handle;
   Mutex* m_mtx;
   int* m_refCount;

   ItemArray m_handles;
};

class CurlError: public ::Falcon::Error
{
public:
   CurlError():
      Error( "CurlError" )
   {}

   CurlError( const ErrorParam &params  ):
      Error( "CurlError", params )
      {}
};

}
}

#endif

/* end of curl_mod.h */
