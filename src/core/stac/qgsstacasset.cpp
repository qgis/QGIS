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
  return format == "COG"_L1 ||
         format == "COPC"_L1 ||
         format == "EPT"_L1 ||
         format == "Zarr"_L1 ||
         format == "Parquet"_L1;
}

QString QgsStacAsset::formatName() const
{
  if ( mMediaType == "image/tiff; application=geotiff; profile=cloud-optimized"_L1 ||
       mMediaType == "image/vnd.stac.geotiff; cloud-optimized=true"_L1 )
    return u"COG"_s;
  else if ( mMediaType == "application/vnd.laszip+copc"_L1 )
    return u"COPC"_s;
  else if ( mHref.endsWith( "/ept.json"_L1 ) )
    return u"EPT"_s;
  else if ( mMediaType == "application/vnd+zarr"_L1 )
    return u"Zarr"_s;
  else if ( mMediaType == "application/vnd.apache.parquet"_L1 )
    return u"Parquet"_s;
  return QString();
}


QgsMimeDataUtils::Uri QgsStacAsset::uri( const QString &authcfg ) const
{
  QgsMimeDataUtils::Uri uri;
  QUrl url( href() );
  if ( formatName() == "COG"_L1 )
  {
    uri.layerType = u"raster"_s;
    uri.providerKey = u"gdal"_s;
    if ( href().startsWith( "http"_L1, Qt::CaseInsensitive ) ||
         href().startsWith( "ftp"_L1, Qt::CaseInsensitive ) )
    {
      uri.uri = u"/vsicurl/%1"_s.arg( href() );
      if ( !authcfg.isEmpty() )
        uri.uri.append( u" authcfg='%1'"_s.arg( authcfg ) );
    }
    else if ( href().startsWith( "s3://"_L1, Qt::CaseInsensitive ) )
    {
      uri.uri = u"/vsis3/%1"_s.arg( href().mid( 5 ) );
    }
    else
    {
      uri.uri = href();
    }
  }
  else if ( formatName() == "COPC"_L1 )
  {
    uri.layerType = u"pointcloud"_s;
    uri.providerKey = u"copc"_s;
    uri.uri = href();
    if ( !authcfg.isEmpty() )
      uri.uri.append( u" authcfg='%1'"_s.arg( authcfg ) );
  }
  else if ( formatName() == "EPT"_L1 )
  {
    uri.layerType = u"pointcloud"_s;
    uri.providerKey = u"ept"_s;
    uri.uri = href();
    if ( !authcfg.isEmpty() )
      uri.uri.append( u" authcfg='%1'"_s.arg( authcfg ) );
  }
  else if ( formatName() == "Zarr"_L1 )
  {
    uri.layerType = u"raster"_s;
    uri.providerKey = u"gdal"_s;
    if ( href().startsWith( "http"_L1, Qt::CaseInsensitive ) ||
         href().startsWith( "ftp"_L1, Qt::CaseInsensitive ) )
    {
      uri.uri = u"ZARR:\"/vsicurl/%1\""_s.arg( href() );
      if ( !authcfg.isEmpty() )
        uri.uri.append( u" authcfg='%1'"_s.arg( authcfg ) );
    }
    else if ( href().startsWith( "s3://"_L1, Qt::CaseInsensitive ) )
    {
      // Remove the s3:// protocol prefix for compatibility with GDAL's /vsis3
      uri.uri = u"ZARR:\"/vsis3/%1\""_s.arg( href().mid( 5 ) );
    }
    else
    {
      uri.uri = href();
    }
  }
  else if ( formatName() == "Parquet"_L1 )
  {
    uri.layerType = u"vector"_s;
    uri.providerKey = u"ogr"_s;
    if ( href().startsWith( "http"_L1, Qt::CaseInsensitive ) ||
         href().startsWith( "ftp"_L1, Qt::CaseInsensitive ) )
    {
      uri.uri = u"/vsicurl/%1"_s.arg( href() );
      if ( !authcfg.isEmpty() )
        uri.uri.append( u" authcfg='%1'"_s.arg( authcfg ) );
    }
    else if ( href().startsWith( "s3://"_L1, Qt::CaseInsensitive ) )
    {
      uri.uri = u"/vsis3/%1"_s.arg( href().mid( 5 ) );
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
  QString html = u"<h1>%1</h1>\n<hr>\n"_s.arg( "Asset"_L1 );
  html += "<table class=\"list-view\">\n"_L1;
  html += u"<tr><td class=\"highlight\">%1</td><td>%2</td></tr>\n"_s.arg( u"id"_s, assetId );
  html += u"<tr><td class=\"highlight\">%1</td><td>%2</td></tr>\n"_s.arg( u"title"_s, title() );
  html += u"<tr><td class=\"highlight\">%1</td><td>%2</td></tr>\n"_s.arg( u"description"_s, description() );
  html += u"<tr><td class=\"highlight\">%1</td><td><a href=\"%2\">%2</a></td></tr>\n"_s.arg( u"url"_s, href() );
  html += u"<tr><td class=\"highlight\">%1</td><td>%2</td></tr>\n"_s.arg( u"type"_s, mediaType() );
  html += u"<tr><td class=\"highlight\">%1</td><td>%2</td></tr>\n"_s.arg( u"roles"_s, roles().join( ',' ) );
  html += "</table><br/>\n"_L1;
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
  if ( formatName() == "Zarr"_L1 )
  {
    return false;
  }

  return true;
}
