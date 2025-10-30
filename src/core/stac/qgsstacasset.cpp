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

#include "qgsgdalprovider.h"
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
  return !formatName().isEmpty();
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
  else if ( mMediaType.contains( QLatin1String( "cloud-optimized=true" ), Qt::CaseInsensitive ) )
  {
    // could use GDAL identify, but PDAL does not have the equivalent so split string
    const QStringList parts = mMediaType.split( QRegExp( "[/;]+" ), Qt::SkipEmptyParts );
    if ( parts.size() > 1 )
      return parts[1];
  }
  return QString();
}

QgsMimeDataUtils::Uri QgsStacAsset::uri() const
{
  QgsMimeDataUtils::Uri uri;
  QUrl url( href() );

  if ( isCloudOptimized() )
  {
    if ( href().startsWith( QLatin1String( "http" ), Qt::CaseInsensitive ) ||
         href().startsWith( QLatin1String( "ftp" ), Qt::CaseInsensitive ) )
    {
      uri.uri = QStringLiteral( "/vsicurl/%1" ).arg( href() );
    }
    else if ( href().startsWith( QLatin1String( "s3://" ), Qt::CaseInsensitive ) )
    {
      uri.uri = QStringLiteral( "/vsis3/%1" ).arg( href().mid( 5 ) );
    }
    else if ( href().startsWith( QLatin1String( "azure://" ), Qt::CaseInsensitive ) )
    {
      uri.uri = QStringLiteral( "/vsiaz/%1" ).arg( href().mid( 8 ) );
    }
    else if ( href().startsWith( QLatin1String( "gcp://" ), Qt::CaseInsensitive ) )
    {
      uri.uri = QStringLiteral( "/vsigs/%1" ).arg( href().mid( 6 ) );
    }
    else
    {
      uri.uri = href();
    }
  }
  else
  {
    return {};
  }

  QString errMsg;

  if ( ( formatName() == QLatin1String( "COG" ) ) ||
       QgsGdalProvider::isValidRasterFileName( uri.uri, errMsg ) )
  {
    uri.layerType = QStringLiteral( "raster" );
    uri.providerKey = QStringLiteral( "gdal" );
  }
  else
  {
    uri.layerType = QStringLiteral( "pointcloud" );
    uri.providerKey = formatName().toLower();
#ifndef PDAL_2_9_OR_HIGHER
    // reset and don't use /vsi/ as not supported
    uri.uri = href();
#endif
  }

  uri.name = title().isEmpty() ? url.fileName() : title();

  return uri;
}
