/*
   FALCON - The Falcon Programming Language.
   FILE: mxml_ext.cpp

   MXML module main file - extension implementation.
   -------------------------------------------------------------------
   Author: Giancarlo Niccolai
   Begin: Thu, 06 Mar 2008 19:44:41 +0100

   -------------------------------------------------------------------
   (C) Copyright 2004: the FALCON developers (see list in AUTHORS file)

   See LICENSE file for licensing details.
*/

/** \file
   MXML module main file - extension implementation.
*/

#include <falcon/vm.h>
#include <falcon/transcoding.h>
#include <falcon/fstream.h>
#include <falcon/lineardict.h>
#include "mxml_ext.h"
#include "mxml_mod.h"
#include "mxml_st.h"

#include "mxml.h"

/*#
   @beginmodule feathers.mxml
*/

namespace MXML {

Falcon::CoreObject *Node::makeShell( Falcon::VMachine *vm )
{
   static Falcon::Item *node_class = 0;

   if( m_objOwner != 0 )
      return m_objOwner;

   if( node_class == 0 )
      node_class = vm->findWKI( "MXMLNode" );

   fassert( node_class != 0 );

   Falcon::CoreObject *co = node_class->asClass()->createInstance();
   co->setUserData( new Falcon::Ext::NodeCarrier( this, co ) );
   return co;
}

}

namespace Falcon {
namespace Ext {

static MXML::Node *internal_getNodeParameter( VMachine *vm, int pid )
{
   Item *i_child = vm->param(pid);

   if ( i_child == 0 || ! i_child->isObject() || ! i_child->asObject()->derivedFrom( "MXMLNode" ) )
   {
      throw new  ParamError( ErrorParam( e_inv_params, __LINE__ ).
         extra( "MXMLNode" ) );
      return 0;
   }

   return static_cast<NodeCarrier *>( i_child->asObject()->getUserData() )->node();
}

/*#
   @class MXMLDocument
   @brief Encapsulates a complete XML document.
   @optparam encoding Encoding suggested for document load or required for document write.
   @optparam style Style required in document write.
   Class containing a logical XML file representation.

   To work with MXML, you need to instantiate at least one object from this class;
   this represents the logic of a document. It is derived for an element, as
   an XML document must still be valid as an XML element of another document,
   but it implements some data specific for handling documents.

   It is possible to specify a @b encoding parameter which must be one of the
   encoding names know by Falcon (see the @b TranscoderFactory function in the
   RTL documentation). In this version, this parameter is ignored if the object
   is used to deserialize an XML document, but it's used as output encoding (and
   set in the "encoding" field of the XML heading) when writing the document.

   The @b style parameter requires that a particular formatting is used when
   writing the document. It can be overridden in the @a MXMLDocument.write method,
   and if not provided there, the default set in the constructor will be used.

   The @b style parameter must be in @a MXMLStyle enumeration.

   @note It is not necessary to create and serialize a whole XML document to
   create just XML compliant data representations. Single nodes can be serialized
   with the @a MXMLNode.serialize method; in this way it is possible to create
   small xml valid fragments for storage, network streaming, template filling
   etc. At the same time, it is possible to de-serialize a single XML node
   through the @a MXMLNode.deserialize method, which tries to decode an XML
   document fragment configuring the node and eventually re-creating its subtree.


   @section mxml_doc_struct MXML document structure.

   The XML document, as seen by the MXML module, is a tree of nodes. Some nodes have
   meta-informative value, and are meant to be used by the XML parser programs to
   determine how the tree is expected to be built.

   The tree itself has a topmost node (called top node), which is the parent for every
   other node, and a node called "root" which is actually the root of the "informative
   hierarchy" of the XML document, called 'tag nodes'.

   Tag nodes can have some "attributes", zero or more children and a partent.
   It is also possible to access the previous node at the same level of the tree,
   or the next one, and it is possible to insert nodes in any position. Tag nodes
   can have other tag nodes, data nodes or comment nodes as children. Processing Instruction
   nodes can also be placed at any level of the XML tree.

   A valid XML document can have only one root node, or in other words, it can declare
   only one tag node at top level. In example, the following is a valid XML document:
   @code
      <?xml encoding="utf-8" version="1.0" ?>
      <!-- The above was an XML special PI node, and this is a comment -->
      <!DOCTYPE greeting SYSTEM "hello.dtd">
      <!-- We see a doctype above -->
      <MyDocumentRootTag>
         ...
      </MyDocumentRootTag>
   @endcode

   In the above document, the top node would hold a comment, a DOCTYPE node, another comment
   and then a tag node, which is also the "root" node.

   The special XML instruction at the beginning is not translated into a node; instead,
   its attribute becomes readable properties of the MXMLDocument instance (or are written
   taking them from the instance properties, if the document is being written).

   Falcon MXML node allows to create automatically simple data nodes attacched to tag nodes
   by specifying a "data" value for the node. In example,
   @code
      <some_tag>Some data</some_tag>
   @endcode
   this node can be generated by creating a either a "some_tag" node and adding a data
   child to it, or setting its @a MXMLNode.data to "Some tag". Falcon will automatically
   import data for such nodes in the data field of the parent tag node.

   On the other hand, it is possible to create combination of data and
   tag children as in the following sample:
   @code
      <p>A paragraph <b>with bold text</b>.</p>
   @endcode
   In this case, it is necessary to create a "p" tag node with a child data node
   containing "A paragraph ", a tag "b" node having "with bold text" as single data and
   a data node containing a single "." character. The data in the "p" tag node will
   be empty.
*/


FALCON_FUNC MXMLDocument_init( ::Falcon::VMachine *vm )
{
   CoreObject *self = vm->self().asObject();
   Item *i_encoding = vm->param(0);
   Item *i_style = vm->param(1);

   if ( ( i_encoding != 0 && ! i_encoding->isString() && ! i_encoding->isNil() ) ||
      ( i_style != 0 && ! i_style->isInteger()) )
   {
       throw new  ParamError( ErrorParam( e_inv_params, __LINE__ ).
         extra( "[S,I]" ) );
      return;
   }

   int style = i_style == 0 ? 0 : (int) i_style->forceInteger();
   MXML::Document *doc;
   if( i_encoding == 0 || i_encoding->isNil() )
      doc = new MXML::Document( "C", style );
   else
      doc = new MXML::Document( *i_encoding->asString(), style );

   self->setUserData( new MXML::DocumentCarrier( doc ) );
}

/*#
   @method deserialize MXMLDocument
   @brief Loads an XML document from a stream.
   @param istream A Falcon Stream instance opened for input.
   @raise MXMLError on load error.

   Loads a document from a Falcon Stream. After a succesful call,
   the document is ready for inspection and can be modified.

   @see MXMLDocument.read
*/

FALCON_FUNC MXMLDocument_deserialize( ::Falcon::VMachine *vm )
{
   CoreObject *self = vm->self().asObject();
   Item *i_stream = vm->param(0);

   if ( i_stream == 0 || ! i_stream->isObject() || ! i_stream->asObject()->derivedFrom( "Stream" ) )
   {
      throw new  ParamError( ErrorParam( e_inv_params, __LINE__ ).
         extra( "Stream" ) );
      return;
   }

   Stream *stream = static_cast<Stream *>( i_stream->asObject()->getUserData() );
   MXML::Document *doc = static_cast<MXML::DocumentCarrier *>( self->getUserData() )->document();

   try
   {
      doc->read( *stream );
      vm->retval( true );
   }
   catch( MXML::MalformedError &err )
   {
      throw new MXMLError( ErrorParam( FALCON_MXML_ERROR_BASE + err.numericCode(), __LINE__ )
      .desc( err.description() )
      .extra( err.describeLine() ) );
   }
   catch( MXML::IOError &err )
   {
      throw new IoError( ErrorParam( FALCON_MXML_ERROR_BASE + err.numericCode(), __LINE__ )
      .desc( err.description() )
      .extra( err.describeLine() ) );
   }
}

/*#
   @method serialize MXMLDocument
   @brief Stores the document as an XML file.
   @param ostream A Falcon Stream opened for output.
   @raise MXMLError on write error.

   This method stores an XML document created using the
   three in this instance on the stream passed as parameter.

   @see MXMLDocument.write
*/
FALCON_FUNC MXMLDocument_serialize( ::Falcon::VMachine *vm )
{
   CoreObject *self = vm->self().asObject();
   Item *i_stream = vm->param(0);

   if ( i_stream == 0 || ! i_stream->isObject() || ! i_stream->asObject()->derivedFrom( "Stream" ) )
   {
      throw new  ParamError( ErrorParam( e_inv_params, __LINE__ ).
         extra( "Stream" ) );
      return;
   }

   Stream *stream = static_cast<Stream *>( i_stream->asObject()->getUserData() );
   MXML::Document *doc = static_cast<MXML::DocumentCarrier *>( self->getUserData() )->document();

   try
   {
      doc->write( *stream, doc->style() );
      vm->retval( true );
   }
   catch( MXML::MalformedError &err )
   {
      throw new MXMLError( ErrorParam( FALCON_MXML_ERROR_BASE + err.numericCode(), __LINE__ )
      .desc( err.description() )
      .extra( err.describeLine() ) );
   }
   catch( MXML::IOError &err )
   {
      throw new IoError( ErrorParam( FALCON_MXML_ERROR_BASE + err.numericCode(), __LINE__ )
      .desc( err.description() )
      .extra( err.describeLine() ) );
   }
}

/*#
   @method style MXMLDocument
   @brief Reads or changes the style applied to this XML document.
   @optparam setting If given, changes the style.
   @return The current style settings.

   This method allows to read or change the style used for serialization
   and deserialization of this document, which is usually set in the
   constructor.

   The @b setting paramter must be in @a MXMLStyle enumeration.

   The method returns the current style as a combination of the bitfields
   from the @a MXMLStyle enumeration.

   @see MXMLDocument.init
*/
FALCON_FUNC MXMLDocument_style( ::Falcon::VMachine *vm )
{
   CoreObject *self = vm->self().asObject();
   MXML::Document *doc = static_cast<MXML::DocumentCarrier *>( self->getUserData() )->document();
   Item *i_style = vm->param(0);

   // read immediately the style, we always return it
   vm->retval( (int64) doc->style() );
   if ( i_style == 0 )
   {
      return;
   }

   if ( i_style == 0 || ! i_style->isInteger() )
   {
      throw new  ParamError( ErrorParam( e_inv_params, __LINE__ ).
         extra( "N" ) );
      return;
   }

   doc->style( (int) i_style->asInteger() );
}

/*#
   @method top MXMLDocument
   @brief Retrieves the topmost node in the document.
   @return The overall topmost node of the XML document.

   This method returns the topmost node of the document;
   this is actually an invisible node which is used as a
   "container" for the top nodes in the document: comments,
   directives as DOCTYPE and the "root" tag node.

   @see MXMLDocument.root
   @see MXMLDocument
*/
FALCON_FUNC MXMLDocument_top( ::Falcon::VMachine *vm )
{
   CoreObject *self = vm->self().asObject();
   MXML::Document *doc = static_cast<MXML::DocumentCarrier *>( self->getUserData() )->document();
   MXML::Node *root = doc->root();
   vm->retval( root->getShell( vm ) );
}

/*#
   @method root MXMLDocument
   @brief Retrieves the root tag node in the document.
   @return The root tag node of the XML document.

   This method returns the "root" node, which is the unique
   topmost node of type "tag", and that defines the information
   contents of the XML document.

   The default name for this node is "root"; the implementor
   should change the name to something more sensible before
   serializing a document generated from this instance.

   As a valid XML document must have exactly one root node,
   an instance for this node is always generated when then
   document is created, so it is always available.

   @see MXMLDocument.top
   @see MXMLDocument
*/
FALCON_FUNC MXMLDocument_root( ::Falcon::VMachine *vm )
{
   CoreObject *self = vm->self().asObject();
   MXML::Document *doc = static_cast<MXML::DocumentCarrier *>( self->getUserData() )->document();
   MXML::Node *root = doc->main();
   // if we don't have a root (main) node, create it.
   if ( root == 0 ) {
      root = new MXML::Node( MXML::Node::typeTag, "root" );
      doc->root()->addBelow( root );
   }

   vm->retval( root->getShell( vm ) );
}


/*#
   @method find MXMLDocument
   @brief Finds the first (tag) node matching a certain criterion.
   @param name Tag name of the searched node.
   @optparam attrib Name of one of the attributes of the searched node.
   @optparam value Value for one of the attributes in the node.
   @optparam data Part of the data stored in the searched tag node.
   @return The node matching the given criterion or nil if not found.

   This method performs a search in the XML tree, starting from the root,
   from a tag node with the given name, attribute (eventually having a certain
   value) and specified data portion. All the paramters are optional, and
   can be substituted with a nil or not given to match "everything".

   The @a MXMLDocument.findNext method will repeat the search starting from
   the last matching node; direction of the search is down towards the leaves
   of the tree, then forward towards the next siblings. When the nodes matching
   the criterion are exhausted, the two methods return nil.

   In example, to search in a tree for all the nodes named "interesting", the
   following code can be used:
   @code
      // doc is a MXMLDocument
      node = doc.find( "interesting" )
      while node != nil
         > "Found an interesting node:", node.path()
         ...
         node = doc.findNext()
      end
   @endcode

   To find a node which has an attribute named "cute" (at which value
   we're not interested), and which data node contains the word "suspect",
   the following code can be used:
   @code
      node = doc.find( nil, "cute", nil, "suspect" )
      while node != nil
         > "Found a suspect node:", node.path()
         ...
         node = doc.findNext()
      end
   @endcode

   @note Checks are case sensitive.

   @see MXMLDocument.findNext
*/

FALCON_FUNC MXMLDocument_find( ::Falcon::VMachine *vm )
{
   Item *i_name = vm->param(0);
   Item *i_attrib = vm->param(1);
   Item *i_valattr = vm->param(2);
   Item *i_data = vm->param(3);
   CoreObject *self = vm->self().asObject();

   // parameter sanity check
   if( ( i_name == 0 || (! i_name->isString() && ! i_name->isNil() )) ||
       ( i_attrib != 0 && (! i_attrib->isString() && ! i_attrib->isNil() )) ||
       ( i_valattr != 0 && (! i_valattr->isString() && ! i_valattr->isNil() )) ||
       ( i_data != 0 && (! i_data->isString() && ! i_data->isNil() ))
   )
   {
      throw new  ParamError( ErrorParam( e_inv_params, __LINE__ ).
         extra( "S,[S,S,S]" ) );
      return;
   }

   String dummy;
   String *sName, *sValue, *sValAttr, *sData;

   sName = i_name == 0 || i_name->isNil() ? &dummy : i_name->asString();
   sValue = i_attrib == 0 || i_attrib->isNil() ? &dummy : i_attrib->asString();
   sValAttr = i_valattr == 0 || i_valattr->isNil() ? &dummy : i_valattr->asString();
   sData = i_data == 0 || i_data->isNil() ? &dummy : i_data->asString();

   MXML::Document *doc = static_cast<MXML::DocumentCarrier *>( self->getUserData() )->document();
   // the real find
   MXML::Node *node = doc->find( *sName, *sValue, *sValAttr, *sData );
   if ( node == 0 )
      vm->retnil();
   else
      vm->retval( node->getShell( vm ) );
}


/*#
   @method findNext MXMLDocument
   @brief Finds the next (tag) node matching a certain criterion.
   @return The next node matching the given criterion or nil if not found.

   This method is meant to be used after a @a MXMLDocument.find call has
   returned a valid node to iterate through all the matching nodes in a tree.

   @see MXMLDocument.find
*/
FALCON_FUNC MXMLDocument_findNext( ::Falcon::VMachine *vm )
{
   CoreObject *self = vm->self().asObject();
   MXML::Document *doc = static_cast<MXML::DocumentCarrier *>( self->getUserData() )->document();
   // the real find
   MXML::Node *node = doc->findNext();
   if ( node == 0 )
      vm->retnil();
   else
      vm->retval( node->getShell( vm ) );
}

/*#
   @method findPath MXMLDocument
   @brief Finds the first (tag) node matching a certain XML path.
   @param path The XML path to be searched.
   @return The next node matching the given criterion or nil if not found.

   This method provides limited (at this time, very limited) support for xpath.
   A tag node can be found through a virtual "path" staring from the root node and
   leading to it; each element of the path is a tag parent node. In example, the
   path for the node "inner" in the following example:
   @code
      <root>
         <outer>
            <middle>
               <inner>Inner content</inner>
            </middle>
         </outer>
      </root>
   @endcode

   would be "/root/outer/middle/inner".

   Paths are not unique keys for the nodes; in the above example, more than one "inner" node may
   be stacked inside the "middle" node, and all of them would have the same path.

   This method allows to use a "*" wildcard to substitute a level of the path with "anything". In
   example, the path "/root/\*\/middle/inner" would find the inner node in the above sample no matter
   what the second-topmost node name was.

   If the path cannot match any node in the three, the method returns nil. It is possible to iterate
   through all the nodes having the same path (or matching wildcard paths) in a tree by using the
   @a MXMLDocument.findPathNext method. In example, the following code would find all the nodes
   which have exactly two parents:

   @code
      node = doc.findPath( "/\*\/\*\/\*" )
      while node != nil
         > "Found a node at level 3:", node.path()
         ...
         node = doc.findPathNext()
      end
   @endcode

   @see MXMLDocument.findPathNext
*/
FALCON_FUNC MXMLDocument_findPath( ::Falcon::VMachine *vm )
{
Item *i_name = vm->param(0);
   CoreObject *self = vm->self().asObject();

   // parameter sanity check
   if( i_name == 0 || ! i_name->isString() )
   {
      throw new  ParamError( ErrorParam( e_inv_params, __LINE__ ).
         extra( "S" ) );
      return;
   }

   MXML::Document *doc = static_cast<MXML::DocumentCarrier *>( self->getUserData() )->document();
   // the real find
   MXML::Node *node = doc->findPath( *i_name->asString() );
   if ( node == 0 )
      vm->retnil();
   else
      vm->retval( node->getShell( vm ) );
}

/*#
   @method findPathNext MXMLDocument
   @brief Finds the next (tag) node matching a certain XML path.
   @return The next node matching the given criterion or nil if not found.

   This method is meant to be used together with @a MXMLDocument.findPath
   method to traverse a tree in search of nodes matching certain paths.

   @see MXMLDocument.findPath
*/
FALCON_FUNC MXMLDocument_findPathNext( ::Falcon::VMachine *vm )
{
  CoreObject *self = vm->self().asObject();
   MXML::Document *doc = static_cast<MXML::DocumentCarrier *>( self->getUserData() )->document();
   // the real find
   MXML::Node *node = doc->findNextPath();
   if ( node == 0 )
      vm->retnil();
   else
      vm->retval( node->getShell( vm ) );
}


/*#
   @method write MXMLDocument
   @brief Stores a document to an XML file on a filesystem.
   @param filename Name of the destination XML file.
   @raise MXMLError on error during the serialization.

   This method saves the XML document to a file on a
   reachable filesystem. It takes care of opening a suitable
   stream, transcoding it using the chosen encoding and
   performing a complete serialization through
   @a MXMLDocument.serialize.

   @see MXMLDocument.setEncoding
*/
FALCON_FUNC MXMLDocument_save( ::Falcon::VMachine *vm )
{
   CoreObject *self = vm->self().asObject();
   Item *i_uri = vm->param(0);

   if ( i_uri == 0 || ! i_uri->isString() )
   {
      throw new  ParamError( ErrorParam( e_inv_params, __LINE__ ).
         extra( "N" ) );
      return;
   }

   String &uri = *i_uri->asString();
   MXML::Document *doc = static_cast<MXML::DocumentCarrier *>( self->getUserData() )->document();
   
   vm->idle();
   //TODO: use parsing uri
   FileStream out;
   if ( out.create( uri, BaseFileStream::e_aUserRead | BaseFileStream::e_aUserWrite | BaseFileStream::e_aGroupRead | BaseFileStream::e_aOtherRead  ) )
   {

      Stream *output = &out;

      if ( doc->encoding() != "C" )
      {
         output = TranscoderFactory( doc->encoding(), &out, false );
         if ( output == 0 )
         {
            vm->unidle();
            throw new MXMLError( ErrorParam( e_inv_params, __LINE__ )
               .extra( "Invalid encoding " + doc->encoding() ) );
            return;
         }
      }

      try
      {
         doc->write( *output, doc->style() );
         vm->unidle();
         vm->retval( true );
      }
      catch( MXML::MalformedError &err )
      {
         vm->unidle();
         throw new MXMLError( ErrorParam( FALCON_MXML_ERROR_BASE + err.numericCode(), __LINE__ )
            .desc( err.description() )
            .extra( err.describeLine() ) );
      }
      catch( MXML::IOError &err )
      {
         vm->unidle();
         throw new IoError( ErrorParam( FALCON_MXML_ERROR_BASE + err.numericCode(), __LINE__ )
         .desc( err.description() )
         .extra( err.describeLine() ) );
      }
   }
   else
   {
      vm->unidle();
      throw new IoError( ErrorParam(
         FALCON_MXML_ERROR_BASE + (int) MXML::Error::errIo , __LINE__ )
         .desc( FAL_STR( MXML_ERR_IO ) ) );
   }
}


/*#
   @method read MXMLDocument
   @brief Loads a document to an XML file on a filesystem.
   @param filename Name of the source XML file.
   @raise MXMLError on error during the deserialization.

   This method loads the XML document from a file on a
   reachable filesystem. It takes care of opening a suitable
   stream, transcoding it using the chosen encoding and
   performing a complete deserialization through
   @a MXMLDocument.deserialize.

   @see MXMLDocument.setEncoding
*/
FALCON_FUNC MXMLDocument_load( ::Falcon::VMachine *vm )
{
   CoreObject *self = vm->self().asObject();
   Item *i_uri = vm->param(0);

   if ( i_uri == 0 || ! i_uri->isString() )
   {
      throw new  ParamError( ErrorParam( e_inv_params, __LINE__ ).
         extra( "S" ) );
      return;
   }

   String &uri = *i_uri->asString();
   MXML::Document *doc = static_cast<MXML::DocumentCarrier *>( self->getUserData() )->document();
   
   vm->idle();
   
   //TODO: use parsing uri
   FileStream in;
   if ( in.open( uri ) )
   {
      Stream *input = &in;

      if ( doc->encoding() != "C" )
      {
         input = TranscoderFactory( doc->encoding(), &in, false );
         if ( input == 0 )
         {
            vm->unidle();
            throw new MXMLError( ErrorParam( e_inv_params, __LINE__ )
               .extra( FAL_STR( MXML_ERR_INVENC ) + doc->encoding() ) );
            return;
         }

      }

      try
      {
         doc->read( *input );
         vm->unidle();
         vm->retval( true );
      }
      catch( MXML::MalformedError &err )
      {
         vm->unidle();
         throw new MXMLError(
            ErrorParam( FALCON_MXML_ERROR_BASE + err.numericCode(), __LINE__ )
            .desc( err.description() )
            .extra( err.describeLine() ) );
      }
      catch( MXML::IOError &err )
      {
         vm->unidle();
         throw new IoError(
            ErrorParam( FALCON_MXML_ERROR_BASE + err.numericCode(), __LINE__ )
            .desc( err.description() )
            .extra( err.describeLine() ) );
      }

      in.close();
      return;
   }
   
   if ( ! in.good() )
   {
      throw new IoError( ErrorParam(
         FALCON_MXML_ERROR_BASE + (int) MXML::Error::errIo , __LINE__ )
         .desc( FAL_STR( MXML_ERR_IO ) ) );
   }

   in.close();
}

/*#
   @method setEncoding MXMLDocument
   @brief Changes the document encoding for stream operations.
   @param encoding A valid falcon encoding name
   @raise ParamError if the encoding name is unknown.

   This method sets the encoding used for I/O operations on this
   XML document. It also determines the value of the "encoding"
   attribute that will be set in the the special PI ?xml at
   document heading.

   @see MXMLDocument.getEncoding
*/
FALCON_FUNC MXMLDocument_setEncoding( ::Falcon::VMachine *vm )
{
   CoreObject *self = vm->self().asObject();
   Item *i_encoding = vm->param(0);

   if ( i_encoding == 0 || ! i_encoding->isString() )
   {
      throw new  ParamError( ErrorParam( e_inv_params, __LINE__ ).
         extra( "S" ) );
      return;
   }

   String &encoding = *i_encoding->asString();
   Transcoder *tr = TranscoderFactory( encoding );
   if ( tr == 0 )
   {
      throw new  ParamError( ErrorParam( e_param_range, __LINE__ ).
         extra( encoding ) );
      return;
   }
   delete tr;

   MXML::Document *doc = static_cast<MXML::DocumentCarrier *>( self->getUserData() )->document();
   doc->encoding( encoding );
}


/*#
   @method getEncoding MXMLDocument
   @brief Returns the encoding for stream operations.
   @return Previously set or read XML encoding.

   This method returns the encoding that has been previously set
   either at document creation or through the @a MXMLDocument.setEncoding.

   Also, after a succesful deserialization, this method will return the
   value of the "encoding" attribute of the ?xml PI heading directive,
   if provided.

   If still unset, this method will return nil.

   @see MXMLDocument.setEncoding
*/
FALCON_FUNC MXMLDocument_getEncoding( ::Falcon::VMachine *vm )
{
   CoreObject *self = vm->self().asObject();
   MXML::Document *doc = static_cast<MXML::DocumentCarrier *>( self->getUserData() )->document();
   vm->retval( new CoreString( doc->encoding() ) );
}

/*#
   @class MXMLNode
   @param type One of the node type defined by the @a MXMLType enumeration.
   @optparam name Name of the node, if this is a tag node.
   @optparam data Optional data content attached to this node..
   @brief Minimal entity of the XML document.
   @raise ParamError if the type is invalid.

   This class encapsulates a minimal adressable entity in an XML document.
   Nodes can be of different types, some of which, like CDATA, tag and comment nodes
   can have a simple textual data attached to them (equivalent to a single data node
   being their only child).

   Nodes can be attached and detached from trees or serialized on their own. The
   subtrees of child nodes stays attached to its parent also when the MXMLDocument
   they are attached to is changed. Also, serializing a node directly allows to
   write mini xml valid fragments which may be used for network transmissions,
   database storage, template filling etc., without the need to build a whole XML
   document and writing the ?xml heading declarator.

   The @b type must be one of the @a MXMLType enumeration elements.
   The @b name of the node is relevant only for Processing Instruction
   nodes and tag node, while data can be specified for comment, tag and
   data nodes.

   If the node is created just to be de-serialized, create it as an empty
   comment and then deserialize the node from a stream.

*/


FALCON_FUNC MXMLNode_init( ::Falcon::VMachine *vm )
{
   CoreObject *self = vm->self().asObject();
   Item *i_type = vm->param(0);
   Item *i_name = vm->param(1);
   Item *i_data = vm->param(2);

   if ( ( i_type != 0 && ! i_type->isInteger() ) ||
      ( i_name != 0 && ! (i_name->isString() || i_name->isNil()) ) ||
      ( i_data != 0 && ! i_data->isString() )  )
   {
      throw new  ParamError( ErrorParam( e_inv_params, __LINE__ ).
         extra( "[N,S,S]" ) );
      return;
   }

   // verify type range
   int type = i_type != 0 ? (int) i_type->asInteger() : 0;

   if ( type < 0 || type > (int) MXML::Node::typeFakeClosing )
   {
      throw new  ParamError( ErrorParam( e_inv_params, __LINE__ ).
         extra( "Invalid type" ) );
      return;
   }

   String dummy;
   String *name = i_name == 0 || i_name->isNil() ? &dummy : i_name->asString();
   String *data = i_data == 0 ? &dummy : i_data->asString();

   MXML::Node *node = new MXML::Node( (MXML::Node::type) type, *name, *data );
   self->setUserData( new NodeCarrier( node, self ) );
}

/*#
   @method serialize MXMLNode
   @brief Stores an XML node on a stream.
   @raise MXMLError If the serialization failed.

   This method allows the storage of a single node and all its
   children in an XML compliant format. The resulting data
   is an valid XML fragment that may be included verbatim in
   an XML document.
*/
FALCON_FUNC MXMLNode_serialize( ::Falcon::VMachine *vm )
{
   CoreObject *self = vm->self().asObject();
   Item *i_stream = vm->param(0);

   if ( i_stream == 0 || ! i_stream->isObject() || ! i_stream->asObject()->derivedFrom( "Stream" ) )
   {
      throw new  ParamError( ErrorParam( e_inv_params, __LINE__ ).
         extra( "Stream" ) );
      return;
   }

   Stream *stream = static_cast<Stream *>( i_stream->asObject()->getUserData() );
   MXML::Node *node = static_cast<NodeCarrier *>( self->getUserData() )->node();

   try
   {
      node->write( *stream, 0 );
      vm->retval( true );
   }
   catch( MXML::MalformedError &err )
   {
      throw new MXMLError(
      ErrorParam( FALCON_MXML_ERROR_BASE + err.numericCode(), __LINE__ )
         .desc( err.description() )
         .extra( err.describeLine() ) );
   }
   catch( MXML::IOError &err )
   {
      throw new MXMLError(
         ErrorParam( FALCON_MXML_ERROR_BASE + err.numericCode(), __LINE__ )
         .desc( err.description() )
         .extra( err.describeLine() ) );
   }

}

/*#
   @method deserialize MXMLNode
   @brief Stores an XML node on a stream.
   @raise MXMLError If the serialization failed.

   This method allows the storage of a single node and all its
   children in an XML compliant format. The resulting data
   is an valid XML fragment that may be included verbatim in
   an XML document.
*/
FALCON_FUNC MXMLNode_deserialize( ::Falcon::VMachine *vm )
{
   CoreObject *self = vm->self().asObject();
   Item *i_stream = vm->param(0);

   if ( i_stream == 0 || ! i_stream->isObject() || ! i_stream->asObject()->derivedFrom( "Stream" ) )
   {
      throw new  ParamError( ErrorParam( e_inv_params, __LINE__ ).
         extra( "Stream" ) );
      return;
   }

   Stream *stream = static_cast<Stream *>( i_stream->asObject()->getUserData() );
   delete static_cast<NodeCarrier *>( self->getUserData() );

   MXML::Node *node = new MXML::Node();

   try
   {
      node->read( *stream );
      self->setUserData( new NodeCarrier( node, self ) );
      vm->retval( self );
   }
   catch( MXML::MalformedError &err )
   {
      throw new MXMLError(
         ErrorParam( FALCON_MXML_ERROR_BASE + err.numericCode(), __LINE__ )
         .desc( err.description() )
         .extra( err.describeLine() ) );
      delete node;
   }
   catch( MXML::IOError &err )
   {
      throw new MXMLError(
         ErrorParam( FALCON_MXML_ERROR_BASE + err.numericCode(), __LINE__ )
         .desc( err.description() )
         .extra( err.describeLine() ) );
      delete node;
   }
}


/*#
   @method nodeType MXMLNode
   @brief Returns the type of this node.
   @return A value in @a MXMLType enumeration.
*/
FALCON_FUNC MXMLNode_nodeType( ::Falcon::VMachine *vm )
{
   CoreObject *self = vm->self().asObject();
   MXML::Node *node = static_cast<NodeCarrier *>( self->getUserData() )->node();
   vm->retval( (int64)node->nodeType() );
}


/*#
   @method name MXMLNode
   @brief Set and/or return the name of this node.
   @optparam name If provided, the new name of this node.
   @return If a new @b name is not given, the current node name.

   A name can be assigned to any node, but it will be meaningful only
   for tag and PI nodes.
*/
FALCON_FUNC MXMLNode_name( ::Falcon::VMachine *vm )
{
   Item *i_name = vm->param(0);

   if ( i_name != 0 && ! i_name->isString() )
   {
      throw new  ParamError( ErrorParam( e_inv_params, __LINE__ ).
         extra( "[S]" ) );
      return;
   }

   CoreObject *self = vm->self().asObject();
   MXML::Node *node = static_cast<NodeCarrier *>( self->getUserData() )->node();

   if ( i_name == 0 )
      vm->retval( new CoreString( node->name() ) );
   else
      node->name( *i_name->asString() );
}


/*#
   @method data MXMLNode
   @brief Set and/or return the content of this node.
   @optparam data If provided, the new data of this node.
   @return If a new @b data is not given, the current node data.

   A data can be assigned to any node, but it will be meaningful only
   for data, tag, comment and CDATA nodes. Moreover, tag nodes can have
   also other children; in this case, the data set with this method will
   be serialized as if it was a first child data node.
*/
FALCON_FUNC MXMLNode_data( ::Falcon::VMachine *vm )
{
   Item *i_data = vm->param(0);

   if ( i_data != 0 && ! i_data->isString() )
   {
      throw new  ParamError( ErrorParam( e_inv_params, __LINE__ ).
         extra( "[S]" ) );
      return;
   }

   CoreObject *self = vm->self().asObject();
   MXML::Node *node = static_cast<NodeCarrier *>( self->getUserData() )->node();
   if ( i_data == 0 )
      vm->retval( new CoreString( node->data() ) );
   else
      node->data( *i_data->asString() );
}

/*#
   @method setAttribute MXMLNode
   @brief Sets an XML attribute of this node to a given value.
   @param attribute The XML attribute to be set.
   @param value The value for this XML attribute.

   This method sets the value for an XML attribute of the node.
   Attributes can be assigned to PI, Tag and DOCTYPE nodes.

   The @b value parameter can be any Falcon type; if it's not
   a string, the @b FBOM.toString method will be applied to transform
   it into a string.

   If the attribute doesn't exist, it is added, otherwise it's value
   is changed.

   @note Don't confuse XML attributes with Falcon attributes.
*/
FALCON_FUNC MXMLNode_setAttribute( ::Falcon::VMachine *vm )
{
   CoreObject *self = vm->self().asObject();
   MXML::Node *node = static_cast<NodeCarrier *>( self->getUserData() )->node();

   Item *i_attrName = vm->param(0);
   Item *i_attrValue = vm->param(1);

   if ( i_attrName == 0 || ! i_attrName->isString() ||
        i_attrValue == 0
      )
   {
      throw new  ParamError( ErrorParam( e_inv_params, __LINE__ ).
         extra( "S,X" ) );
      return;
   }

   String content;
   String *value;
   if ( i_attrValue->isString() )
   {
      value = i_attrValue->asString();
   }
   else {
      vm->itemToString( content, i_attrValue );
      value = &content;
   }

   const String &attrName = *i_attrName->asString();
   if( ! node->hasAttribute( attrName ) )
   {
      node->addAttribute( new MXML::Attribute( attrName, *value ) );
   }

   node->setAttribute( attrName, *value );
}

/*#
   @method getAttribute MXMLNode
   @brief Gets the value of an XML attribute of this node.
   @param attribute The XML attribute to be read.
   @return The value for this XML attribute (as a string).

   This method retreives the value for an XML attribute of the node.
   Attributes can be assigned to PI, Tag and DOCTYPE nodes.

   If the attribute doesn't exist, nil is returned.

   @note Don't confuse XML attributes with Falcon attributes.
*/
FALCON_FUNC MXMLNode_getAttribute( ::Falcon::VMachine *vm )
{
   CoreObject *self = vm->self().asObject();
   MXML::Node *node = static_cast<NodeCarrier *>( self->getUserData() )->node();

   Item *i_attrName = vm->param(0);

   if ( i_attrName == 0 || ! i_attrName->isString() )
   {
      throw new ParamError( ErrorParam( e_inv_params, __LINE__ ).
         extra( "S" ) );
      return;
   }

   if ( ! node->hasAttribute( *i_attrName->asString() ) )
   {
      vm->retnil();
      return;
   }

   const String &val = node->getAttribute( *i_attrName->asString() );
   vm->retval( new CoreString( val ) );
}

/*#
   @method getAttribs MXMLNode
   @brief Gets the all the XML attributes of this node.
   @return A dictionary containing all the XML attributes and their values.

   This method retreives all the attributes of the node, and stores them
   in a dictionary as a pair of key => value strings.

   Attributes can be assigned to PI, Tag and DOCTYPE nodes.

   If the node doesn't have any XML attribute, an empty dictionary is
   returned.

   The dictionary is read-only; values in the dictionary can be changed,
   but this won't change the values of the original XML attributes in
   the source node.

   @note Don't confuse XML attributes with Falcon attributes.
*/
FALCON_FUNC MXMLNode_getAttribs( ::Falcon::VMachine *vm )
{
   CoreObject *self = vm->self().asObject();
   MXML::Node *node = static_cast<NodeCarrier *>( self->getUserData() )->node();

   const MXML::AttribList &attribs = node->attribs();

   LinearDict *dict = new LinearDict( attribs.size() );

   MXML::AttribList::const_iterator iter = attribs.begin();
   while( iter != attribs.end() )
   {
      dict->put( new CoreString( (*iter)->name()),
         new CoreString( (*iter)->value()) );
      ++iter;
   }

   vm->retval( new CoreDict(dict) );
}

/*#
   @method getChildren MXMLNode
   @brief Gets the all the children nodes of this node.
   @return An array containing all the children node.

   This method stores all the children of an XML node in an
   array.

   If the node doesn't have any child, an empty array is
   returned.

   The array is read-only; it is possible to change it but
   inserting or removing nodes from it won't change the children
   list of the source node.
*/
FALCON_FUNC MXMLNode_getChildren( ::Falcon::VMachine *vm )
{
   CoreObject *self = vm->self().asObject();
   MXML::Node *node = static_cast<NodeCarrier *>( self->getUserData() )->node();

   CoreArray *arr = new CoreArray;

   node = node->child();
   while( node != 0 )
   {
      arr->append( node->getShell( vm ) );
      node = node->next();
   }

   vm->retval( arr );
}

/*#
   @method unlink MXMLNode
   @brief Removes a node from its parent tree.

   This method removes a node from the list of node of
   its parent node. The node is removed together with all
   its children and their whole subtree.

   After an unlink, it is possible to insert the node into
   another place of the same tree or of another tree.

   Actually, all the insertion routines perform an @b unlink on
   the node that is going to be inserted, so it is not
   necessary to call @b unlink from the falcon script before
   adding it elsewhere. However, explicitly unlinked node may be
   kept elsewhere (i.e. in a script maintained dictionary) for
   later usage.
*/
FALCON_FUNC MXMLNode_unlink( ::Falcon::VMachine *vm )
{
   CoreObject *self = vm->self().asObject();
   MXML::Node *node = static_cast<NodeCarrier *>( self->getUserData() )->node();

   node->unlink();
}

/*#
   @method removeChild MXMLNode
   @brief Removes a child from its parent tree.
   @param child The child node to be removed.
   @return True if the @b child node is actually a child of this node, false otherwise.

   This method is equivalent to @b MXMLNode.unlink applied to the child node,
   but it checks if the removed node is really a child of this node before actually
   removing it.

   If the @b child parameter is really a child of this node it is unlinked and the
   method returns true, otherwise the node is untouched and the method returns false.
*/
FALCON_FUNC MXMLNode_removeChild( ::Falcon::VMachine *vm )
{
   MXML::Node *child = internal_getNodeParameter( vm, 0 );
   if ( child == 0 )
      return;

   CoreObject *self = vm->self().asObject();
   MXML::Node *node = static_cast<NodeCarrier *>( self->getUserData() )->node();

   try {
      node->removeChild( child );
      vm->retval(true);
   }
   catch( ... )
   {
      vm->retval( false );
   }
}

/*#
   @method parent MXMLNode
   @brief Return the parent node of this node.
   @return The parent node or nil if this node has no parents.

   This method returns the node that is currently
   parent of this node in the XML tree.

   The method returns nil if the node hasn't a parent; this may mean
   that this node is the topmost node in an XMLDocument (the node
   returned by @a MXMLDocument.top ) or if it has not still been added
   to a tree, or if it has been removed with @a MXMLNode.removeChild or
   @a MXMLNode.unlink.
*/
FALCON_FUNC MXMLNode_parent( ::Falcon::VMachine *vm )
{
   CoreObject *self = vm->self().asObject();
   MXML::Node *node = static_cast<NodeCarrier *>( self->getUserData() )->node();
   MXML::Node *parent = node->parent();
   if ( parent != 0 )
      vm->retval( parent->getShell( vm ) );
   else
      vm->retnil();
}


/*#
   @method firstChild MXMLNode
   @brief Return the first child of a node.
   @return The first child of this node or nil if the node hasn't any child.

   This method returns the first child of a node; it's the node that will
   be delivered for first in the rendering of the final XML document, and that
   will appare on the topmost postition between the nodes below the current
   one.

   To iterate through all the child nodes of a node, it is possible to
   get the first child and the iteratively @a MXMLNode.nextSibling
   until it returns nil. In example:

   @code
      // node is an MXMLNode...
      child = node.firstChild()
      while child != nil
         > "Child of ", node.name(), ": ", child.name()
         child = child.nextSibling()
      end
   @endcode
*/
FALCON_FUNC MXMLNode_firstChild( ::Falcon::VMachine *vm )
{
   CoreObject *self = vm->self().asObject();
   MXML::Node *node = static_cast<NodeCarrier *>( self->getUserData() )->node();
   MXML::Node *child = node->child();

   if ( child != 0 )
      vm->retval( child->getShell( vm ) );
   else
      vm->retnil();
}

/*#
   @method nextSibling MXMLNode
   @brief Return the next node child of the same parent.
   @return The next node at the same level, or nil.

   This method returns the next node that would be found in
   the rendered XML document right after this one, at the same level.
   If such node doesn't exist, it returns nil.

   @see MXMLNode.firstChild
*/
FALCON_FUNC MXMLNode_nextSibling( ::Falcon::VMachine *vm )
{
   CoreObject *self = vm->self().asObject();
   MXML::Node *node = static_cast<NodeCarrier *>( self->getUserData() )->node();
   MXML::Node *sibling = node->next();

   if ( sibling != 0 )
      vm->retval( sibling->getShell( vm ) );
   else
      vm->retnil();
}

/*#
   @method prevSibling MXMLNode
   @brief Return the previous node child of the same parent.
   @return The previous node at the same level, or nil.

   This method returns the previous node that would be found in
   the rendered XML document right after this one, at the same level.
   If such node doesn't exist, it returns nil.

   @see MXMLNode.lastChild
*/
FALCON_FUNC MXMLNode_prevSibling( ::Falcon::VMachine *vm )
{
   CoreObject *self = vm->self().asObject();
   MXML::Node *node = static_cast<NodeCarrier *>( self->getUserData() )->node();
   MXML::Node *sibling = node->prev();

   if ( sibling != 0 )
      vm->retval( sibling->getShell( vm ) );
   else
      vm->retnil();
}


/*#
   @method lastChild MXMLNode
   @brief Return the last child of a node.
   @return The last child of this node or nil if the node hasn't any child.

   This method returns the last child of a node; it's the node that will
   be delivered for last in the rendering of the final XML document, and that
   will appare on the lowermost postition between the nodes below the current
   one.

   To iterate through all the child nodes of a node in reverse order,
   it is possible to get the last child and the iteratively
   @a MXMLNode.prevSibling
   until it returns nil. In example:

   @code
      // node is an MXMLNode...
      child = node.lastChild()
      while child != nil
         > "Child of ", node.name(), " reverse: ", child.name()
         child = child.prevSibling()
      end
   @endcode
*/
FALCON_FUNC MXMLNode_lastChild( ::Falcon::VMachine *vm )
{
   CoreObject *self = vm->self().asObject();
   MXML::Node *node = static_cast<NodeCarrier *>( self->getUserData() )->node();
   MXML::Node *sibling = node->lastChild();

   if ( sibling != 0 )
      vm->retval( sibling->getShell( vm ) );
   else
      vm->retnil();
}

/*#
   @method addBelow MXMLNode
   @brief Adds a node below this one.
   @param node The node to be added below this one.

   This method appends the given @b node as the last child
   of this node, eventually removing it from a prevuious tree
   structure to which it was linked if needed.

   After this method returns, @b node can be retreived calling the
   @a MXMLNode.lastChild on this node, until another @b addBelow
   adds another node at the end of the children list.
*/
FALCON_FUNC MXMLNode_addBelow( ::Falcon::VMachine *vm )
{
   MXML::Node *child = internal_getNodeParameter( vm, 0 );
   if ( child == 0 )
      return;

   CoreObject *self = vm->self().asObject();
   MXML::Node *node = static_cast<NodeCarrier *>( self->getUserData() )->node();

   // just to be sure
   child->unlink();
   node->addBelow( child );
}

/*#
   @method insertBelow MXMLNode
   @brief Inserts a node below this one.
   @param node The node to be added below this one.

   This method prepends the given @b node as the first child
   of this node, eventually removing it from a prevuious tree
   structure to which it was linked if needed.

   After this method returns, @b node can be retreived calling the
   @a MXMLNode.firstChild on this node, until another @b insertBelow
   adds another node at the beginning of the children list.
*/
FALCON_FUNC MXMLNode_insertBelow( ::Falcon::VMachine *vm )
{
   MXML::Node *child = internal_getNodeParameter( vm, 0 );
   if ( child == 0 )
      return;

   CoreObject *self = vm->self().asObject();
   MXML::Node *node = static_cast<NodeCarrier *>( self->getUserData() )->node();

   // just to be sure
   child->unlink();
   node->insertBelow( child );
}

/*#
   @method insertBefore MXMLNode
   @brief Inserts a node before this one.
   @param node The node to be added before this one.

   This method prepends the given @b node in front of this one
   in the list of sibling nodes, eventually removing it from a prevuious tree
   structure to which it was linked if needed. This is equivalent to inserting
   the node exactly before this one, at the same level, in the final
   XML document.

   If this node was the first child of its parent, the inserted node
   becomes the new first child.
*/
FALCON_FUNC MXMLNode_insertBefore( ::Falcon::VMachine *vm )
{
   MXML::Node *child = internal_getNodeParameter( vm, 0 );
   if ( child == 0 )
      return;

   CoreObject *self = vm->self().asObject();
   MXML::Node *node = static_cast<NodeCarrier *>( self->getUserData() )->node();

   // just to be sure
   child->unlink();
   node->insertBefore( child );
}


/*#
   @method insertAfter MXMLNode
   @brief Inserts a node after this one.
   @param node The node to be added after this one.

   This method prepends the given @b node after of this one
   in the list of sibling nodes, eventually removing it from a prevuious tree
   structure to which it was linked if needed. This is equivalent to inserting
   the node exactly after this one, at the same level, in the final
   XML document.

   If this node was the last child of its parent, the inserted node
   becomes the new last child.
*/
FALCON_FUNC MXMLNode_insertAfter( ::Falcon::VMachine *vm )
{
   MXML::Node *child = internal_getNodeParameter( vm, 0 );
   if ( child == 0 )
      return;

   CoreObject *self = vm->self().asObject();
   MXML::Node *node = static_cast<NodeCarrier *>( self->getUserData() )->node();

   // just to be sure
   child->unlink();
   node->insertAfter( child );
}

/*#
   @method depth MXMLNode
   @brief Calculates the depth of this node.
   @return The depth of this node.

   This method returns the number of steps needed to find a
   node without parents in the parent hierarchy of this node.

   The dept for a topmost tree node is 0, for a root node in a tree
   is 1 and for its direct child is 2.
*/
FALCON_FUNC MXMLNode_depth( ::Falcon::VMachine *vm )
{
   CoreObject *self = vm->self().asObject();
   MXML::Node *node = static_cast<NodeCarrier *>( self->getUserData() )->node();
   vm->retval( (int64) node->depth() );
}


/*#
   @method path MXMLNode
   @brief Returns the path from the root to this node.
   @return The path of this node in its XML document tree.

   The path of a node is the list of parent node names separated
   by a slash "/", starting from the root node (or from the first
   node of a separate tree) and terminating with the node itself.

   In example, the path of the node "item" in the following XML document:
   @code
      <root>
         <content>
            <item/>
         </content>
      </root>
   @endcode
   would be "/root/content/item"

   @see MXMLDocument.findPath
*/
FALCON_FUNC MXMLNode_path( ::Falcon::VMachine *vm )
{
   CoreObject *self = vm->self().asObject();
   MXML::Node *node = static_cast<NodeCarrier *>( self->getUserData() )->node();
   CoreString *gs = new CoreString( node->path() );
   gs->bufferize();
   vm->retval( gs );
}

/*#
   @method clone MXMLNode
   @brief Clones a whole XML hierarcy starting from this node.
   @return A copy of this node, with all its children copied.
*/
FALCON_FUNC MXMLNode_clone( ::Falcon::VMachine *vm )
{
   CoreObject *self = vm->self().asObject();
   MXML::Node *node = static_cast<NodeCarrier *>( self->getUserData() )->node();
   vm->retval( node->clone()->getShell( vm ) );
}

//=======================================================================
// MXML error class
//

/*#
   @class MXMLError
   @brief Error raised by the MXML module in case of problems.
   @optparam code A numeric error code.
   @optparam description A textual description of the error code.
   @optparam extra A descriptive message explaining the error conditions.
   @from Error code, description, extra

   An instance of this class is raised whenever some problem is
   found. The error codes generated by this module are in the
   @a MXMLErrorCode enumeration.
*/

FALCON_FUNC  MXMLError_init ( ::Falcon::VMachine *vm )
{
   CoreObject *einst = vm->self().asObject();
   if( einst->getUserData() == 0 )
      einst->setUserData( new MXMLError );

   ::Falcon::core::Error_init( vm );
}

}
}

/* end of mxml_ext.cpp */
