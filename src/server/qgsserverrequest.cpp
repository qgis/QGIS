/***************************************************************************
                          qgsserverrequest.cpp

  Define ruquest class for getting request contents
  -------------------
  begin                : 2016-12-05
  copyright            : (C) 2016 by David Marteau
  email                : david dot marteau at 3liz dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsserverrequest.h"


QgsServerRequest::QgsServerRequest( const QString& url, Method method )
: mUrl(url)
, mMethod(method)
{

}

QgsServerRequest::QgsServerRequest( const QUrl& url, Method method )
: mUrl(url)
, mMethod(method)
{

}

//! destructor
QgsServerRequest::~QgsServerRequest()
{
    
}

const QUrl& QgsServerRequest::url() const 
{
    return mUrl;
}

QgsServerRequest::Method QgsServerRequest::method() const
{
    return mMethod;
}

const QByteArray* QgsServerRequest::data() const
{
    return nullptr;
}


