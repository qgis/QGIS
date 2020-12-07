/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2019 Peter Petrik (zilolv at gmail dot com)
*/

#ifndef MDAL_XML_HPP
#define MDAL_XML_HPP

#include <string>
#include <vector>
#include <stdio.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

//! C++ Wrapper around libxml2 library
class XMLFile
{
  public:
    //! Create file with invalid handle
    XMLFile();
    //! Closes the file
    ~XMLFile();

    //! Opens the XML file
    void openFile( const std::string &fileName );

    //! Gets next child with a correct name
    //! if force, throws an exception if not found
    xmlNodePtr getCheckChild( xmlNodePtr parent, const std::string &name, bool force = true ) const;

    //! Gets next sibling with a correct name
    //! if force, throws an exception if not found
    xmlNodePtr getCheckSibling( xmlNodePtr parent, const std::string &name, bool force = true ) const;

    //! Gets root element and check that has correct name
    xmlNodePtr getCheckRoot( const std::string &name ) const;

    //! Checks if attribute matches the string.
    bool checkAttribute( xmlNodePtr parent, const std::string  &name, const std::string &expectedVal ) const;

    //! Checks if attribute matches the string. Throws if not
    void checkAttribute( xmlNodePtr parent, const std::string  &name, const std::string &expectedVal, const std::string &err ) const;

    //! Checks strings are equal. Throws exception
    void checkEqual( const xmlChar *xmlString, const std::string &str, const std::string &err ) const;

    //! Checks strings are equal
    bool checkEqual( const xmlChar *xmlString, const std::string &str ) const;

    //! Gets string attribute
    std::string attribute( xmlNodePtr node, std::string name ) const;

    //! Gets double attribute
    double queryDoubleAttribute( xmlNodePtr elem, std::string name ) const;

    //! Gets size_t attribute
    size_t querySizeTAttribute( xmlNodePtr elem, std::string name ) const;

    //! Gets element text
    std::string content( xmlNodePtr node ) const;

    //! Gets root element
    xmlNodePtr root() const;

    //! Converts xmlString to string.
    std::string toString( const xmlChar *xmlString ) const;

  private:
    //! Throws an error
    [[ noreturn ]] void error( const std::string &str ) const;

    xmlDocPtr mXmlDoc;
    std::string mFileName;
};

#endif // MDAL_XML_HPP
