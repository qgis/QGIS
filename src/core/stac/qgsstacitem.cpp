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

QgsStacObject::Type QgsStacItem::type() const
{
  return QgsStacObject::Type::Item;
}

QString QgsStacItem::toHtml() const
{
  QString html = QStringLiteral( "<html><head></head>\n<body>\n" );

  html += QStringLiteral( "<h1>%1</h1>\n<hr>\n" ).arg( QLatin1String( "Item" ) );
  html += QLatin1String( "<table class=\"list-view\">\n" );
  html += QStringLiteral( "<tr><td class=\"highlight\">%1</td><td>%2</td></tr>\n" ).arg( QStringLiteral( "id" ), id() );
  html += QStringLiteral( "<tr><td class=\"highlight\">%1</td><td>%2</td></tr>\n" ).arg( QStringLiteral( "stac_version" ), stacVersion() );
  html += QStringLiteral( "<tr><td class=\"highlight\">%1</td><td>%2</td></tr>\n" ).arg( QStringLiteral( "collection" ), collection() );
  html += QLatin1String( "</table>\n" );

  if ( !mStacExtensions.isEmpty() )
  {
    html += QStringLiteral( "<h1>%1</h1>\n<hr>\n" ).arg( QLatin1String( "Extensions" ) );
    html += QLatin1String( "<ul>\n" );
    for ( const QString &extension : mStacExtensions )
    {
      html += QStringLiteral( "<li>%1</li>\n" ).arg( extension );
    }
    html += QLatin1String( "</ul>\n" );
  }

  html += QStringLiteral( "<h1>%1</h1>\n<hr>\n" ).arg( QLatin1String( "Geometry" ) );
  html += QLatin1String( "<table class=\"list-view\">\n" );
  html += QStringLiteral( "<tr><td class=\"highlight\">%1</td><td>%2</td></tr>\n" ).arg( QStringLiteral( "WKT" ), mGeometry.asWkt() );
  html += QLatin1String( "</table>\n" );

  html += QStringLiteral( "<h1>%1</h1>\n<hr>\n" ).arg( QLatin1String( "Bounding Box" ) );
  html += QLatin1String( "<ul>\n" );
  html += QStringLiteral( "<li>%1</li>\n" ).arg( mBbox.is2d() ? mBbox.toRectangle().toString() : mBbox.toString() );
  html += QLatin1String( "</ul>\n" );

  if ( ! mProperties.isEmpty() )
  {
    html += QStringLiteral( "<h1>%1</h1>\n<hr>\n" ).arg( QLatin1String( "Properties" ) );
    html += QLatin1String( "<table class=\"list-view\">\n" );
    for ( auto it = mProperties.constBegin(); it != mProperties.constEnd(); ++it )
    {
      html += QStringLiteral( "<tr><td class=\"highlight\">%1</td><td>%2</td></tr>\n" ).arg( it.key(), it.value().toString() );
    }
    html += QLatin1String( "</table><br/>\n" );
  }

  html += QStringLiteral( "<h1>%1</h1>\n<hr>\n" ).arg( QLatin1String( "Links" ) );
  for ( const QgsStacLink &link : mLinks )
  {
    html += QLatin1String( "<table class=\"list-view\">\n" );
    html += QStringLiteral( "<tr><td class=\"highlight\">%1</td><td>%2</td></tr>\n" ).arg( QStringLiteral( "relation" ), link.relation() );
    html += QStringLiteral( "<tr><td class=\"highlight\">%1</td><td>%2</td></tr>\n" ).arg( QStringLiteral( "title" ), link.title() );
    html += QStringLiteral( "<tr><td class=\"highlight\">%1</td><td><a href=\"%2\">%2</a></td></tr>\n" ).arg( QStringLiteral( "url" ), link.href() );
    html += QStringLiteral( "<tr><td class=\"highlight\">%1</td><td>%2</td></tr>\n" ).arg( QStringLiteral( "type" ), link.mediaType() );
    html += QLatin1String( "</table><br/>\n" );
  }

  if ( ! mAssets.isEmpty() )
  {
    html += QStringLiteral( "<h1>%1</h1>\n<hr>\n" ).arg( QLatin1String( "Assets" ) );
    for ( auto it = mAssets.constBegin(); it != mAssets.constEnd(); ++it )
    {
      html += QLatin1String( "<table class=\"list-view\">\n" );
      html += QStringLiteral( "<tr><td class=\"highlight\">%1</td><td>%2</td></tr>\n" ).arg( QStringLiteral( "id" ), it.key() );
      html += QStringLiteral( "<tr><td class=\"highlight\">%1</td><td>%2</td></tr>\n" ).arg( QStringLiteral( "title" ), it->title() );
      html += QStringLiteral( "<tr><td class=\"highlight\">%1</td><td>%2</td></tr>\n" ).arg( QStringLiteral( "description" ), it->description() );
      html += QStringLiteral( "<tr><td class=\"highlight\">%1</td><td><a href=\"%2\">%2</a></td></tr>\n" ).arg( QStringLiteral( "url" ), it->href() );
      html += QStringLiteral( "<tr><td class=\"highlight\">%1</td><td>%2</td></tr>\n" ).arg( QStringLiteral( "type" ), it->mediaType() );
      html += QStringLiteral( "<tr><td class=\"highlight\">%1</td><td>%2</td></tr>\n" ).arg( QStringLiteral( "roles" ), it->roles().join( ',' ) );
      html += QLatin1String( "</table><br/>\n" );
    }
  }

  html += QLatin1String( "\n</body>\n</html>\n" );
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
  return QDateTime::fromString( mProperties.value( QStringLiteral( "datetime" ), QStringLiteral( "null" ) ).toString() );
}

bool QgsStacItem::hasDateTimeRange() const
{
  return mProperties.contains( QStringLiteral( "start_datetime" ) ) &&
         mProperties.contains( QStringLiteral( "end_datetime" ) );
}

QgsDateTimeRange QgsStacItem::dateTimeRange() const
{
  const QDateTime start = QDateTime::fromString( mProperties.value( QStringLiteral( "start_datetime" ), QStringLiteral( "null" ) ).toString() );
  const QDateTime end = QDateTime::fromString( mProperties.value( QStringLiteral( "end_datetime" ), QStringLiteral( "null" ) ).toString() );
  return QgsDateTimeRange( start, end );
}

QString QgsStacItem::title() const
{
  return mProperties.value( QStringLiteral( "title" ) ).toString();
}

QString QgsStacItem::description() const
{
  return mProperties.value( QStringLiteral( "description" ) ).toString();
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
