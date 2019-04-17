/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2019 Peter Petrik (zilolv at gmail dot com)
*/

#include <string>
#include <vector>
#include <assert.h>
#include "mdal_xml.hpp"
#include "mdal.h"
#include "mdal_utils.hpp"

class XMLString
{
  private:
    xmlChar *mXmlChar;

  public:
    XMLString( const std::string &str )
    {
      mXmlChar = xmlCharStrdup( str.c_str() );
    }
    XMLString( xmlChar *str )
    {
      mXmlChar = str;
    }

    ~XMLString()
    {
      if ( mXmlChar )
        xmlFree( mXmlChar );
    }

    xmlChar *get() {return mXmlChar;}
};

XMLFile::XMLFile() = default;

XMLFile::~XMLFile()
{
  if ( mXmlDoc )
  {
    xmlFreeDoc( mXmlDoc );
    mXmlDoc = nullptr;
  }
}

void XMLFile::error( const std::string &str ) const
{
  MDAL::debug( str + "(" + mFileName + ")" );
  throw MDAL_Status::Err_UnknownFormat;
}

void XMLFile::openFile( const std::string &fileName )
{
  mFileName = fileName;
  mXmlDoc = xmlParseFile( fileName.c_str() );
  if ( mXmlDoc == nullptr )
  {
    error( "XML Document not parsed successfully " + fileName );
  }
}

void XMLFile::checkEqual( const xmlChar *xmlString, const std::string &str, const std::string &err ) const
{
  assert( xmlString );

  if ( xmlStrcmp( xmlString, XMLString( str.c_str() ).get() ) != 0 )
  {
    std::string str2 = toString( xmlString );
    error( err );
  }
}

bool XMLFile::checkEqual( const xmlChar *xmlString, const std::string &str ) const
{
  assert( xmlString );
  return ( xmlStrcmp( xmlString, XMLString( str.c_str() ).get() ) == 0 );
}

xmlNodePtr XMLFile::getCheckRoot( const std::string &name ) const
{
  xmlNodePtr rootNode = root();
  checkEqual( rootNode->name, name, "Root element is not" + name );
  return rootNode;
}

xmlNodePtr XMLFile::getCheckChild( xmlNodePtr parent, const std::string &name, bool force ) const
{
  assert( parent );
  xmlNodePtr ret;
  ret = parent->xmlChildrenNode;
  while ( ret != nullptr )
  {
    if ( checkEqual( ret->name, name ) )
    {
      return ret;
    }
    ret = ret->next;
  }

  if ( force && !ret )
    error( "Element " + toString( parent->name ) + " does not have a child " + name );

  return ret;
}

xmlNodePtr XMLFile::getCheckSibling( xmlNodePtr parent, const std::string &name, bool force ) const
{
  assert( parent );

  xmlNodePtr ret = xmlNextElementSibling( parent );
  while ( ret != nullptr )
  {
    if ( checkEqual( ret->name, name ) )
    {
      return ret;
    }
    ret = xmlNextElementSibling( ret );
  }

  if ( force && !ret )
    error( "Element " + toString( parent->name ) + " does not have a sibling " + name );

  return ret;
}

bool XMLFile::checkAttribute( xmlNodePtr parent, const std::string &name, const std::string &expectedVal ) const
{
  assert( parent );

  xmlChar *uri = xmlGetProp( parent, XMLString( name.c_str() ).get() );
  if ( !uri )
    return false;

  bool ret = checkEqual( uri, expectedVal );
  xmlFree( uri );

  return ret;
}

void XMLFile::checkAttribute( xmlNodePtr parent, const std::string  &name, const std::string &expectedVal, const std::string &err ) const
{
  assert( parent );
  xmlChar *uri = xmlGetProp( parent, XMLString( name.c_str() ).get() );
  if ( !uri )
    error( err );

  XMLString str( uri );
  checkEqual( uri, expectedVal, err );
}

xmlNodePtr XMLFile::root() const
{
  assert( mXmlDoc );

  xmlNodePtr cur = xmlDocGetRootElement( mXmlDoc );
  if ( cur == nullptr )
    error( "XML Document is empty" );

  return cur;
}

std::string XMLFile::toString( const xmlChar *xmlString ) const
{
  if ( !xmlString )
    error( "Name of XML element is empty" );

  // Works only for ASCII strings
  std::string ret( reinterpret_cast<const char *>( xmlString ) );
  return ret;
}

double XMLFile::queryDoubleAttribute( xmlNodePtr elem, std::string name ) const
{
  std::string valStr = attribute( elem, name );
  double val = MDAL::toDouble( valStr );
  return val;
}

size_t XMLFile::querySizeTAttribute( xmlNodePtr elem, std::string name ) const
{
  std::string valStr = attribute( elem, name );
  size_t val = MDAL::toSizeT( valStr );
  return val;
}

std::string XMLFile::attribute( xmlNodePtr node, std::string name ) const
{
  std::string ret;
  assert( node );

  xmlChar *uri = xmlGetProp( node, XMLString( name ).get() );
  XMLString str( uri );
  if ( !uri )
  {
    error( "Unable to get attribute " + name );
  }
  ret = toString( uri );
  return ret;
}

std::string XMLFile::content( xmlNodePtr node ) const
{
  assert( node );

  std::string ret;

  XMLString content( xmlNodeGetContent( node ) );
  ret = toString( content.get() );

  return ret;
}
