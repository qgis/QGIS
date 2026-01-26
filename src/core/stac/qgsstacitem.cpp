/***************************************************************************
    qgsstacitem.cpp
    ---------------------
    begin                : August 2024
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

#include "qgsstacitem.h"

QgsStacItem::QgsStacItem( const QString &id,
                          const QString &version,
                          const QgsGeometry &geometry,
                          const QVariantMap &properties,
                          const QVector< QgsStacLink > &links,
                          const QMap< QString, QgsStacAsset > &assets,
                          const QgsBox3D &bbox )
  : QgsStacObject( id, version, links )
  ,  mGeometry( geometry )
  ,  mBbox( bbox )
  ,  mProperties( properties )
  ,  mAssets( assets )
{
}

Qgis::StacObjectType QgsStacItem::type() const
{
  return Qgis::StacObjectType::Item;
}

QString QgsStacItem::toHtml() const
{
  QString html = u"<h1>%1</h1>\n<hr>\n"_s.arg( "Item"_L1 );
  html += "<table class=\"list-view\">\n"_L1;
  html += u"<tr><td class=\"highlight\">%1</td><td>%2</td></tr>\n"_s.arg( u"id"_s, id() );
  html += u"<tr><td class=\"highlight\">%1</td><td>%2</td></tr>\n"_s.arg( u"stac_version"_s, stacVersion() );
  html += u"<tr><td class=\"highlight\">%1</td><td>%2</td></tr>\n"_s.arg( u"collection"_s, collection() );
  html += "</table>\n"_L1;

  if ( !mStacExtensions.isEmpty() )
  {
    html += u"<h1>%1</h1>\n<hr>\n"_s.arg( "Extensions"_L1 );
    html += "<ul>\n"_L1;
    for ( const QString &extension : mStacExtensions )
    {
      html += u"<li>%1</li>\n"_s.arg( extension );
    }
    html += "</ul>\n"_L1;
  }

  html += u"<h1>%1</h1>\n<hr>\n"_s.arg( "Geometry"_L1 );
  html += "<table class=\"list-view\">\n"_L1;
  html += u"<tr><td class=\"highlight\">%1</td><td>%2</td></tr>\n"_s.arg( u"WKT"_s, mGeometry.asWkt() );
  html += "</table>\n"_L1;

  html += u"<h1>%1</h1>\n<hr>\n"_s.arg( "Bounding Box"_L1 );
  html += "<ul>\n"_L1;
  html += u"<li>%1</li>\n"_s.arg( mBbox.is2d() ? mBbox.toRectangle().toString() : mBbox.toString() );
  html += "</ul>\n"_L1;

  if ( ! mProperties.isEmpty() )
  {
    html += u"<h1>%1</h1>\n<hr>\n"_s.arg( "Properties"_L1 );
    html += "<table class=\"list-view\">\n"_L1;
    for ( auto it = mProperties.constBegin(); it != mProperties.constEnd(); ++it )
    {
      html += u"<tr><td class=\"highlight\">%1</td><td>%2</td></tr>\n"_s.arg( it.key(), it.value().toString() );
    }
    html += "</table><br/>\n"_L1;
  }

  html += u"<h1>%1</h1>\n<hr>\n"_s.arg( "Links"_L1 );
  for ( const QgsStacLink &link : mLinks )
  {
    html += "<table class=\"list-view\">\n"_L1;
    html += u"<tr><td class=\"highlight\">%1</td><td>%2</td></tr>\n"_s.arg( u"relation"_s, link.relation() );
    html += u"<tr><td class=\"highlight\">%1</td><td>%2</td></tr>\n"_s.arg( u"title"_s, link.title() );
    html += u"<tr><td class=\"highlight\">%1</td><td><a href=\"%2\">%2</a></td></tr>\n"_s.arg( u"url"_s, link.href() );
    html += u"<tr><td class=\"highlight\">%1</td><td>%2</td></tr>\n"_s.arg( u"type"_s, link.mediaType() );
    html += "</table><br/>\n"_L1;
  }

  if ( ! mAssets.isEmpty() )
  {
    html += u"<h1>%1</h1>\n<hr>\n"_s.arg( "Assets"_L1 );
    for ( auto it = mAssets.constBegin(); it != mAssets.constEnd(); ++it )
    {
      html += it->toHtml( it.key() );
    }
  }
  return html;
}

QgsGeometry QgsStacItem::geometry() const
{
  return mGeometry;
}

void QgsStacItem::setGeometry( const QgsGeometry &geometry )
{
  mGeometry = geometry;
}

QgsBox3D QgsStacItem::boundingBox() const
{
  return mBbox;
}

void QgsStacItem::setBoundingBox( const QgsBox3D &bbox )
{
  mBbox = bbox;
}

QVariantMap QgsStacItem::properties() const
{
  return mProperties;
}

void QgsStacItem::setProperties( const QVariantMap &properties )
{
  mProperties = properties;
}

QMap< QString, QgsStacAsset > QgsStacItem::assets() const
{
  return mAssets;
}

void QgsStacItem::setAssets( const QMap< QString, QgsStacAsset > &assets )
{
  mAssets = assets;
}

QString QgsStacItem::collection() const
{
  return mCollection;
}

void QgsStacItem::setCollection( const QString &collection )
{
  mCollection = collection;
}

QDateTime QgsStacItem::dateTime() const
{
  return QDateTime::fromString( mProperties.value( u"datetime"_s, u"null"_s ).toString() );
}

bool QgsStacItem::hasDateTimeRange() const
{
  return mProperties.contains( u"start_datetime"_s ) &&
         mProperties.contains( u"end_datetime"_s );
}

QgsDateTimeRange QgsStacItem::dateTimeRange() const
{
  const QDateTime start = QDateTime::fromString( mProperties.value( u"start_datetime"_s, u"null"_s ).toString() );
  const QDateTime end = QDateTime::fromString( mProperties.value( u"end_datetime"_s, u"null"_s ).toString() );
  return QgsDateTimeRange( start, end );
}

QString QgsStacItem::title() const
{
  return mProperties.value( u"title"_s ).toString();
}

QString QgsStacItem::description() const
{
  return mProperties.value( u"description"_s ).toString();
}

QgsMimeDataUtils::UriList QgsStacItem::uris() const
{
  QgsMimeDataUtils::UriList uris;
  for ( const QgsStacAsset &asset : std::as_const( mAssets ) )
  {
    QgsMimeDataUtils::Uri uri = asset.uri();

    // skip assets with incompatible formats
    if ( uri.uri.isEmpty() )
      continue;

    uris.append( uri );
  }
  return uris;
}
