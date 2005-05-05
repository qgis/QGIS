/***************************************************************************
                          qgsproject.cpp -  description
                             -------------------
  begin                : August 31, 2004
  copyright            : (C) 2004 by Mark Coletti
  email                : mcoletti at gmail.com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */
#ifndef QGSEXCEPTION_H
#define QGSEXCEPTION_H

#include <exception>
#include <string>
#include <list>

#include <qdom.h> 
#include <qstring.h>

/**
   Defines a qgis exception class
 */
class QgsException : public std::exception
{
public:

    QgsException( std::string const & what )
        : what_( what )
    {}

    QgsException( QString const & what )
        : what_( (const char *)what )
    {}

    virtual ~QgsException() throw()
    {}

    const char* what() const throw()
    {
        return what_.c_str();
    }
    
private:

    /// description of exception
    std::string what_;

}; // class QgsException


/** for Qgis I/O related exceptions 

  @note usually thrown for opening file's that don't exist, and the like.

*/
class QgsIOException : public QgsException
{
public:

    QgsIOException( std::string const & what )
        : QgsException( what )
        {}

    QgsIOException( QString const & what )
        : QgsException( what )
        {}

}; // class QgsIOException



/** for files missing from layers while reading project files

*/
class QgsProjectBadLayerException : public QgsException
{
public:

    QgsProjectBadLayerException( std::list<QDomNode> const & layers )
        : QgsException(std::string(msg_)),
          mBrokenLayers( layers )
    {}
    
    ~QgsProjectBadLayerException() throw()
    {}
    
    std::list<QDomNode> const & layers() const
    {
        return mBrokenLayers;
    }

private:

    /** QDomNodes representing the state of a layer that couldn't be loaded

    The layer data was either relocated or deleted.  The DOM node also
    contains ancillary data such as line widths and the like.

     */
    std::list<QDomNode> mBrokenLayers;

    static const char * msg_;

}; // class QgsProjectBadLayerException



#endif
