/***************************************************************************
    qgsstacasset.cpp
    ---------------------
    begin                : October 2024
    copyright            : (C) 2024 by Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsstacasset.h"

#include <QUrl>

QgsStacAsset::QgsStacAsset( const QString &href,
                            const QString &title,
                            const QString &description,
                            const QString &mediaType,
                            const QStringList &roles )
  : mHref( href )
  , mTitle( title )
  , mDescription( description )
  , mMediaType( mediaType )
  , mRoles( roles )
{
}

QString QgsStacAsset::href() const
{
  return mHref;
}

QString QgsStacAsset::title() const
{
  return mTitle;
}

QString QgsStacAsset::description() const
{
  return mDescription;
}

QString QgsStacAsset::mediaType() const
{
  return mMediaType;
}

QStringList QgsStacAsset::roles() const
{
  return mRoles;
}

bool QgsStacAsset::isCloudOptimized() const
{
  const QString format = formatName();
  return format == QLatin1String( "COG" ) ||
         format == QLatin1String( "COPC" ) ||
         format == QLatin1String( "EPT" );
}

QString QgsStacAsset::formatName() const
{
  if ( mMediaType == QLatin1String( "image/tiff; application=geotiff; profile=cloud-optimized" ) ||
       mMediaType == QLatin1String( "image/vnd.stac.geotiff; cloud-optimized=true" ) )
    return QStringLiteral( "COG" );
  else if ( mMediaType == QLatin1String( "application/vnd.laszip+copc" ) )
    return QStringLiteral( "COPC" );
  else if ( mHref.endsWith( QLatin1String( "/ept.json" ) ) )
    return QStringLiteral( "EPT" );
  return QString();
}

QgsMimeDataUtils::Uri QgsStacAsset::uri() const
{
  QgsMimeDataUtils::Uri uri;
  QUrl url( href() );
  if ( formatName() == QLatin1String( "COG" ) )
  {
    uri.layerType = QStringLiteral( "raster" );
    uri.providerKey = QStringLiteral( "gdal" );
    if ( href().startsWith( QLatin1String( "http" ), Qt::CaseInsensitive ) ||
         href().startsWith( QLatin1String( "ftp" ), Qt::CaseInsensitive ) )
    {
      uri.uri = QStringLiteral( "/vsicurl/%1" ).arg( href() );
    }
    else if ( href().startsWith( QLatin1String( "s3://" ), Qt::CaseInsensitive ) )
    {
      uri.uri = QStringLiteral( "/vsis3/%1" ).arg( href().mid( 5 ) );
    }
    else
    {
      uri.uri = href();
    }
  }
  else if ( formatName() == QLatin1String( "COPC" ) )
  {
    uri.layerType = QStringLiteral( "pointcloud" );
    uri.providerKey = QStringLiteral( "copc" );
    uri.uri = href();
  }
  else if ( formatName() == QLatin1String( "EPT" ) )
  {
    uri.layerType = QStringLiteral( "pointcloud" );
    uri.providerKey = QStringLiteral( "ept" );
    uri.uri = href();
  }
  else
  {
    return {};
  }

  uri.name = title().isEmpty() ? url.fileName() : title();

  return uri;
}
