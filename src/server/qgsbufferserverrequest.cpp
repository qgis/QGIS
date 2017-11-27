/***************************************************************************
                          qgsbufferserverrequest.cpp

  Define response wrapper for bbuffer response
  -------------------
  begin                : 2017-01-03
  copyright            : (C) 2017 by David Marteau
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

#include "qgsbufferserverrequest.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"

#include <QDebug>

QgsBufferServerRequest::QgsBufferServerRequest( const QString &url, Method method, const QgsServerRequest::Headers &headers, QByteArray *data )
  : QgsServerRequest( url, method, headers )
{
  if ( data )
  {
    mData = *data;
  }
}

QgsBufferServerRequest::QgsBufferServerRequest( const QUrl &url, Method method, const QgsServerRequest::Headers &headers, QByteArray *data )
  : QgsServerRequest( url, method, headers )
{
  if ( data )
  {
    mData = *data;
  }
}
