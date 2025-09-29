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
         format == QLatin1String( "EPT" ) ||
         format == QLatin1String( "Zarr" );
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
  else if ( mMediaType == QLatin1String( "application/vnd+zarr" ) )
    return QStringLiteral( "Zarr" );
  return QString();
}

QgsMimeDataUtils::Uri QgsStacAsset::uri() const
{
  return uri( QString() );
}


QgsMimeDataUtils::Uri QgsStacAsset::uri( const QString &authcfg ) const
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
      if ( !authcfg.isEmpty() )
        uri.uri.append( QStringLiteral( " authcfg='%1'" ).arg( authcfg ) );
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
    if ( !authcfg.isEmpty() )
      uri.uri.append( QStringLiteral( " authcfg='%1'" ).arg( authcfg ) );
  }
  else if ( formatName() == QLatin1String( "EPT" ) )
  {
    uri.layerType = QStringLiteral( "pointcloud" );
    uri.providerKey = QStringLiteral( "ept" );
    uri.uri = href();
    if ( !authcfg.isEmpty() )
      uri.uri.append( QStringLiteral( " authcfg='%1'" ).arg( authcfg ) );
  }
  else if ( formatName() == QLatin1String( "Zarr" ) )
  {
    uri.layerType = QStringLiteral( "raster" );
    uri.providerKey = QStringLiteral( "gdal" );
    if ( href().startsWith( QLatin1String( "http" ), Qt::CaseInsensitive ) ||
         href().startsWith( QLatin1String( "ftp" ), Qt::CaseInsensitive ) )
    {
      uri.uri = QStringLiteral( "ZARR:\"/vsicurl/%1\"" ).arg( href() );
      if ( !authcfg.isEmpty() )
        uri.uri.append( QStringLiteral( " authcfg='%1'" ).arg( authcfg ) );
    }
    else if ( href().startsWith( QLatin1String( "s3://" ), Qt::CaseInsensitive ) )
    {
      // Remove the s3:// protocol prefix for compatibility with GDAL's /vsis3
      uri.uri = QStringLiteral( "ZARR:\"/vsis3/%1\"" ).arg( href().mid( 5 ) );
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

  uri.name = title().isEmpty() ? url.fileName() : title();

  return uri;
}

QString QgsStacAsset::toHtml( const QString &assetId ) const
{
  QString html = QStringLiteral( "<h1>%1</h1>\n<hr>\n" ).arg( QLatin1String( "Asset" ) );
  html += QStringLiteral( "<table class=\"list-view\">\n" );
  html += QStringLiteral( "<tr><td class=\"highlight\">%1</td><td>%2</td></tr>\n" ).arg( QStringLiteral( "id" ), assetId );
  html += QStringLiteral( "<tr><td class=\"highlight\">%1</td><td>%2</td></tr>\n" ).arg( QStringLiteral( "title" ), title() );
  html += QStringLiteral( "<tr><td class=\"highlight\">%1</td><td>%2</td></tr>\n" ).arg( QStringLiteral( "description" ), description() );
  html += QStringLiteral( "<tr><td class=\"highlight\">%1</td><td><a href=\"%2\">%2</a></td></tr>\n" ).arg( QStringLiteral( "url" ), href() );
  html += QStringLiteral( "<tr><td class=\"highlight\">%1</td><td>%2</td></tr>\n" ).arg( QStringLiteral( "type" ), mediaType() );
  html += QStringLiteral( "<tr><td class=\"highlight\">%1</td><td>%2</td></tr>\n" ).arg( QStringLiteral( "roles" ), roles().join( ',' ) );
  html += QStringLiteral( "</table><br/>\n" );
  return html;
}

bool QgsStacAsset::isDownloadable() const
{
  /*
   * Directory-based data types like Zarr should not offer downloads.
   * Download attempts might
   * - fail with 4xx,
   * - succeed but download an HTML directory listing response, or
   * - something else that does not meet the user's needs.
   */
  if ( formatName() == QLatin1String( "Zarr" ) )
  {
    return false;
  }

  return true;
}
