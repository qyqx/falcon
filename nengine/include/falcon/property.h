/*
   FALCON - The Falcon Programming Language.
   FILE: property.h

   Encapsulation for user-defined properties in ClassUser.
   -------------------------------------------------------------------
   Author: Giancarlo Niccolai
   Begin: Sun, 07 Aug 2011 12:11:01 +0200

   -------------------------------------------------------------------
   (C) Copyright 2011: the FALCON developers (see list in AUTHORS file)

   See LICENSE file for licensing details.
*/

#ifndef _FALCON_PROPERTY_H_
#define _FALCON_PROPERTY_H_

#include <falcon/setup.h>
#include <falcon/string.h>
#include <falcon/usercarrier.h>

namespace Falcon {

class ClassUser;
class Item;


class Property
{
public:   
   Property( ClassUser* uc, const String &name, bool bCarried = false );
   virtual ~Property();
   
   const String& name() const { return m_name; }
   ClassUser* owner() const { return m_owner; }
   
   virtual void set( void* instance, const Item& value ) = 0;
   virtual void get( void* instance, Item& target ) = 0;
  
   bool isCarried() const { return m_bCarried; }

   void checkType( bool ok, const String& required );
private:
   String m_name;
   ClassUser* m_owner;
   bool m_bCarried;
};


class PropertyCarried: public Property
{
   
public:
   PropertyCarried( ClassUser* uc, const String &name );
   virtual ~PropertyCarried() {}   
   
   uint32 carrierPos() const { return m_carrierPos; }
   
   Item* carried( UserCarrier* uc ) const { return uc->dataAt(m_carrierPos); }
   
   const static uint32 not_carried = (uint32)-1;
   
private:
   uint32 m_carrierPos;
   friend class ClassUser;   
};


class PropertyString: public PropertyCarried
{
public:
   PropertyString( ClassUser* uc, const String &name ):
      PropertyCarried( uc, name )
      {}
      
   virtual ~PropertyString() {}   
   
   virtual void get( void* instance, Item& target );   
   virtual const String& getString( void* instance ) = 0;
};

class PropertyData: public PropertyCarried
{
public:
   PropertyData( ClassUser* uc, const String &name ):
      PropertyCarried( uc, name )
      {}
      
   virtual ~PropertyData() {}   
   
   virtual void set( void* instance, const Item& value );
   virtual void get( void* instance, Item& target );
   
   virtual void initCacheItem( Item& cache ) = 0;
   virtual void update( void* data, Item& cache ) = 0;
   virtual void fetch( void* data ) = 0;
};

class PropertyReflect: public PropertyCarried
{
public:
   typedef enum {
      e_rt_none,
      e_rt_bool,
      e_rt_char,
      e_rt_uchar,
      e_rt_short,
      e_rt_ushort,
      e_rt_int,
      e_rt_uint,
      e_rt_long,
      e_rt_ulong,         
      e_rt_int64,
      e_rt_uint64,         
      e_rt_float,
      e_rt_double,
      e_rt_ldouble,         
      e_rt_charp,
      e_rt_wcharp,
      e_rt_buffer,
      e_rt_string
   }
   t_rtype;
   
   const static uint32 not_displaced = (uint32)-1;

   PropertyReflect( ClassUser* uc, const String &name );
   virtual ~PropertyReflect() {}
   
   virtual void set( void* instance, const Item& value );
   virtual void get( void* instance, Item& target );

   void reflect( void* base, bool& value );
   void reflect( void* base, char& value );
   void reflect( void* base, unsigned char& value );
   
   void reflect( void* base, short& value );
   void reflect( void* base, unsigned short& value );
   void reflect( void* base, int& value );
   void reflect( void* base, unsigned int& value );
   void reflect( void* base, long& value );
   void reflect( void* base, unsigned long& value );
   void reflect( void* base, int64& value );
   void reflect( void* base, uint64& value );

   void reflect( void* base, float& value );
   void reflect( void* base, double& value );
   void reflect( void* base, long double& value );

   void reflect( void* base, char*& value, uint32 max_size, bool bIsBuffer = false );
   void reflect( void* base, wchar_t*& value, uint32 max_size );   
   void reflect( void* base, String& value );
   
   void displace( void* base, void*& displacePtr );
   
private:   
   uint32 m_displacement;
   uint32 m_offset;
   uint32 m_max_size;
   t_rtype m_type;
};


class PropertyReflectRO: public PropertyReflect
{
public:
   
   PropertyReflectRO( ClassUser* uc, const String &name );   
   virtual ~PropertyReflectRO() {};
   virtual void set( void* instance, const Item& value );
};


class PropertyReflectWO: public PropertyReflect
{
public:
   
   PropertyReflectWO( ClassUser* uc, const String &name );   
   virtual ~PropertyReflectWO() {};
   virtual void get( void* instance, Item& value );
};

}

#endif	/* PROPERTY_H */

/* end of property.h */