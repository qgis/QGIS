/***************************************************************************
    qgswfsrequest.cpp
    ---------------------
    begin                : February 2016
    copyright            : (C) 2011 by Martin Dobias
                           (C) 2016 by Even Rouault
    email                : wonder dot sk at gmail dot com
                           even.rouault at spatialys.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgswfsrequest.h"
#include "moc_qgswfsrequest.cpp"

#include "qgslogger.h"

QgsWfsRequest::QgsWfsRequest( const QgsWFSDataSourceURI &uri )
  : QgsBaseNetworkRequest( uri.auth(), tr( "WFS" ) )
  , mUri( uri )
{
  QgsDebugMsgLevel( QStringLiteral( "theUri = " ) + uri.uri(), 4 );
}

QUrl QgsWfsRequest::requestUrl( const QString &request ) const
{
  return mUri.requestUrl( request );
}
