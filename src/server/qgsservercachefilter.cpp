/***************************************************************************
                          qgsservercachefilter.cpp
                          ------------------------
 Cache interface for Qgis Server plugins

  begin                : 2018-07-05
  copyright            : (C) 2018 by René-Luc D'Hont
  email                : rldhont at 3liz dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsservercachefilter.h"

#include <QDomDocument>

QgsServerCacheFilter::QgsServerCacheFilter( const QgsServerInterface *serverInterface )
  : mServerInterface( serverInterface )
{
}

QByteArray QgsServerCacheFilter::getCachedDocument( const QgsProject *project, const QgsServerRequest &request, const QString &key ) const
{
  Q_UNUSED( project );
  Q_UNUSED( request );
  Q_UNUSED( key );
  return QByteArray();
}

bool QgsServerCacheFilter::setCachedDocument( const QDomDocument *doc, const QgsProject *project, const QgsServerRequest &request, const QString &key ) const
{
  Q_UNUSED( doc );
  Q_UNUSED( project );
  Q_UNUSED( request );
  Q_UNUSED( key );
  return false;
}

bool QgsServerCacheFilter::deleteCachedDocument( const QgsProject *project, const QgsServerRequest &request, const QString &key ) const
{
  Q_UNUSED( project );
  Q_UNUSED( request );
  Q_UNUSED( key );
  return false;
}

bool QgsServerCacheFilter::deleteCachedDocuments( const QgsProject *project ) const
{
  Q_UNUSED( project );
  return false;
}

QByteArray QgsServerCacheFilter::getCachedImage( const QgsProject *project, const QgsServerRequest &request, const QString &key ) const
{
  Q_UNUSED( project );
  Q_UNUSED( request );
  Q_UNUSED( key );
  return QByteArray();
}

bool QgsServerCacheFilter::setCachedImage( const QByteArray *img, const QgsProject *project, const QgsServerRequest &request, const QString &key ) const
{
  Q_UNUSED( img );
  Q_UNUSED( project );
  Q_UNUSED( request );
  Q_UNUSED( key );
  return false;
}

bool QgsServerCacheFilter::deleteCachedImage( const QgsProject *project, const QgsServerRequest &request, const QString &key ) const
{
  Q_UNUSED( project );
  Q_UNUSED( request );
  Q_UNUSED( key );
  return false;
}

bool QgsServerCacheFilter::deleteCachedImages( const QgsProject *project ) const
{
  Q_UNUSED( project );
  return false;
}
