/***************************************************************************
    qgsstacparser.cpp
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

#include "qgsstacparser.h"
#include "qgsstacitem.h"
#include "qgsstaccatalog.h"
#include "qgsstaccollection.h"
#include "qgsstaccollections.h"
#include "qgsstacitemcollection.h"
#include "qgsjsonutils.h"
#include "qgslogger.h"


void QgsStacParser::setData( const QByteArray &data )
{
  mError = QString();
  mType = QgsStacObject::Type::Unknown;
  try
  {
    mData = nlohmann::json::parse( data.data() );
    if ( mData.contains( "stac_version" ) )
    {
      const QString ver( QString::fromStdString( mData.at( "stac_version" ) ) );
      if ( !isSupportedStacVersion( ver ) )
      {
        mError = QStringLiteral( "Unsupported STAC version: %1" ).arg( ver );
        return;
      }
    }
  }
  catch ( nlohmann::json::exception &ex )
  {
    mError = QStringLiteral( "Error parsing JSON" );
    QgsDebugError( QStringLiteral( "Error parsing JSON : %1" ).arg( ex.what() ) );
    return;
  }

  try
  {
    if ( mData.at( "type" ) == "Catalog" )
    {
      mType = QgsStacObject::Type::Catalog;
    }
    else if ( mData.at( "type" ) == "Collection" )
    {
      mType = QgsStacObject::Type::Collection;
    }
    else if ( mData.at( "type" ) == "Feature" )
    {
      mType = QgsStacObject::Type::Item;
    }
  }
  catch ( nlohmann::json::exception &ex )
  {
    // might still be FeatureCollection or Collections Collection
  }
}

void QgsStacParser::setBaseUrl( const QUrl &url )
{
  mBaseUrl = url;
}

QgsStacObject::Type QgsStacParser::type() const
{
  return mType;
}

QString QgsStacParser::error() const
{
  return mError;
}

QgsStacCatalog *QgsStacParser::catalog()
{
  return parseCatalog( mData );
}

QgsStacCatalog *QgsStacParser::parseCatalog( const nlohmann::json &data )
{
  try
  {
    const QString ver( QString::fromStdString( data.at( "stac_version" ) ) );
    if ( !isSupportedStacVersion( ver ) )
    {
      mError = QStringLiteral( "Unsupported STAC version: %1" ).arg( ver );
      return nullptr;
    }

    const QString id( QString::fromStdString( data.at( "id" ) ) );
    const QString description( getString( data.at( "description" ) ) );

    QVector< QgsStacLink > links = parseLinks( data.at( "links" ) );

    std::unique_ptr< QgsStacCatalog > catalog = std::make_unique< QgsStacCatalog >( id,
        ver,
        description,
        links );

    if ( data.contains( "title" ) )
      catalog->setTitle( getString( data["title"] ) );

    if ( data.contains( "conformsTo" ) )
    {
      for ( const auto &conformanceClass : data["conformsTo"] )
      {
        if ( conformanceClass.is_string() )
          catalog->addConformanceClass( QString::fromStdString( conformanceClass ) );
      }
    }

    if ( data.contains( "stac_extensions" ) )
    {
      QStringList extensions;
      for ( const auto &extension : data["stac_extensions"] )
      {
        if ( extension.is_string() )
          extensions.append( QString::fromStdString( extension ) );
      }
      catalog->setStacExtensions( extensions );
    }

    return catalog.release();
  }
  catch ( nlohmann::json::exception &ex )
  {
    mError = QStringLiteral( "Error parsing Collection" );
    QgsDebugError( QStringLiteral( "Error parsing Collection: %1" ).arg( ex.what() ) );
    return nullptr;
  }
}

QgsStacCollection *QgsStacParser::collection()
{
  return parseCollection( mData );
}

QgsStacCollection *QgsStacParser::parseCollection( const nlohmann::json &data )
{
  try
  {
    const QString ver( QString::fromStdString( data.at( "stac_version" ) ) );
    if ( !isSupportedStacVersion( ver ) )
    {
      mError = QStringLiteral( "Unsupported STAC version: %1" ).arg( ver );
      return nullptr;
    }

    const QString id( QString::fromStdString( data.at( "id" ) ) );
    const QString description( getString( data.at( "description" ) ) );
    const QString license( getString( data.at( "license" ) ) );

    QgsStacExtent stacExtent;
    int totalExtents = 0;
    for ( const auto &e : data.at( "extent" ).at( "spatial" ).at( "bbox" ) )
    {
      QgsBox3D extent;
      if ( e.size() == 4 )
      {
        extent = QgsBox3D( e[0].get<double>(),
                           e[1].get<double>(),
                           0.,
                           e[2].get<double>(),
                           e[3].get<double>(),
                           0. );
      }
      else if ( e.size() == 6 )
      {
        extent = QgsBox3D( e[0].get<double>(),
                           e[1].get<double>(),
                           e[2].get<double>(),
                           e[3].get<double>(),
                           e[4].get<double>(),
                           e[5].get<double>() );
      }
      else
      {
        mError = QStringLiteral( "Malformed STAC collection spatial extents" );
        QgsDebugError( mError );
        return nullptr;
      }
      if ( ++totalExtents == 1 )
        stacExtent.setSpatialExtent( extent );
      else
        stacExtent.addDetailedSpatialExtent( extent );
    }

    totalExtents = 0;
    for ( const auto &e : data.at( "extent" ).at( "temporal" ).at( "interval" ) )
    {
      if ( !e.is_array() ||
           e.size() != 2 )
      {
        mError = QStringLiteral( "Malformed STAC collection temporal extents" );
        QgsDebugError( mError );
        return nullptr;
      }
      const QDateTime start = QDateTime::fromString( getString( e[0] ), Qt::ISODateWithMs );
      const QDateTime end = QDateTime::fromString( getString( e[1] ), Qt::ISODateWithMs );

      if ( ++totalExtents == 1 )
        stacExtent.setTemporalExtent( QgsDateTimeRange( start, end ) );
      else
        stacExtent.addDetailedTemporalExtent( QgsDateTimeRange( start, end ) );
    }

    QVector< QgsStacLink > links = parseLinks( data.at( "links" ) );

    std::unique_ptr< QgsStacCollection > collection = std::make_unique< QgsStacCollection >( id,
        ver,
        description,
        links,
        license,
        stacExtent );

    if ( data.contains( "title" ) )
      collection->setTitle( getString( data["title"] ) );

    if ( data.contains( "stac_extensions" ) )
    {
      QStringList extensions;
      for ( const auto &extension : data["stac_extensions"] )
      {
        if ( extension.is_string() )
          extensions.append( QString::fromStdString( extension ) );
      }
      collection->setStacExtensions( extensions );
    }

    if ( data.contains( "keywords" ) )
    {
      QStringList keywords;
      for ( const auto &kw : data["keywords"] )
      {
        if ( kw.is_string() )
          keywords.append( QString::fromStdString( kw ) );
      }
      collection->setKeywords( keywords );
    }

    if ( data.contains( "providers" ) )
    {
      QVector< QgsStacProvider > providers;
      for ( const auto &p : data["providers"] )
      {
        if ( !p.contains( "name" ) ||
             ( p.contains( "roles" ) && !p["roles"].is_array() ) )
        {
          QgsDebugError( QStringLiteral( "Malformed STAC provider object" ) );
          continue;
        }

        QStringList roles;
        if ( p.contains( "roles" ) )
        {
          for ( const auto &role : p["roles"] )
          {
            if ( role.is_string() )
              roles.append( QString::fromStdString( role ) );
          }
        }
        const QgsStacProvider provider( QString::fromStdString( p["name"] ),
                                        p.contains( "description" ) ? getString( p["description"] ) : QString(),
                                        roles,
                                        p.contains( "url" ) ? getString( p["url"] ) : QString() );



        providers.append( provider );
      }
      collection->setProviders( providers );
    }

    if ( data.contains( "summaries" ) )
    {
      const QVariant summ = QgsJsonUtils::jsonToVariant( data["summaries"] );
      collection->setSummaries( summ.toMap() );
    }

    if ( data.contains( "assets" ) )
    {
      QMap< QString, QgsStacAsset > assets = parseAssets( data["assets"] );
      collection->setAssets( assets );
    }

    return collection.release();
  }
  catch ( nlohmann::json::exception &ex )
  {
    mError = QStringLiteral( "Error parsing Collection" );
    QgsDebugError( QStringLiteral( "Error parsing Collection: %1" ).arg( ex.what() ) );
    return nullptr;
  }
}

QgsStacItem *QgsStacParser::item()
{
  return parseItem( mData );
}

QgsStacItem *QgsStacParser::parseItem( const nlohmann::json &data )
{
  try
  {
    const QString ver = QString::fromStdString( data.at( "stac_version" ) );
    if ( !isSupportedStacVersion( ver ) )
    {
      mError = QStringLiteral( "Unsupported STAC version: %1" ).arg( ver );
      return nullptr;
    }

    const QString id = QString::fromStdString( data.at( "id" ) );
    const QgsGeometry geom = QgsJsonUtils::geometryFromGeoJson( data.at( "geometry" ) );

    QgsBox3D bbox;
    if ( !geom.isNull() )
    {
      auto b = data.at( "bbox" );
      if ( b.size() == 4 )
      {
        bbox.setXMinimum( b[0] );
        bbox.setYMinimum( b[1] );
        bbox.setXMaximum( b[2] );
        bbox.setYMaximum( b[3] );
      }
      else if ( b.size() == 6 )
      {
        bbox.setXMinimum( b[0] );
        bbox.setYMinimum( b[1] );
        bbox.setZMinimum( b[2] );
        bbox.setXMaximum( b[3] );
        bbox.setYMaximum( b[4] );
        bbox.setZMaximum( b[5] );
      }
    }

    auto datetime = data.at( "properties" ).at( "datetime" );
    if ( !datetime.is_string() ||
         !QDateTime::fromString( QString::fromStdString( datetime ), Qt::ISODate ).isValid() )
    {
      auto s = data.at( "properties" ).at( "start_datetime" );
      QString ss = QString::fromStdString( s );
      const QDateTime start = QDateTime::fromString( ss, Qt::ISODate );

      auto e = data.at( "properties" ).at( "end_datetime" );
      const QString ee = QString::fromStdString( e );
      const QDateTime end = QDateTime::fromString( ee, Qt::ISODate );
      if ( start.isNull() ||
           end.isNull() )
      {
        // invalid datetime
        mError = QStringLiteral( "Invalid STAC item temporal range" );
        return nullptr;
      }
    }
    const QVariantMap properties = QgsJsonUtils::jsonToVariant( data.at( "properties" ) ).toMap();

    QVector< QgsStacLink > links = parseLinks( data.at( "links" ) );

    QMap< QString, QgsStacAsset > assets = parseAssets( data.at( "assets" ) );

    std::unique_ptr< QgsStacItem > item = std::make_unique< QgsStacItem >( id,
                                          ver,
                                          geom,
                                          properties,
                                          links,
                                          assets,
                                          bbox );

    if ( data.contains( "stac_extensions" ) )
    {
      QStringList extensions;
      for ( const auto &extension : data["stac_extensions"] )
      {
        if ( extension.is_string() )
          extensions.append( QString::fromStdString( extension ) );
      }
      item->setStacExtensions( extensions );
    }

    if ( data.contains( "collection" ) )
      item->setCollection( getString( data["collection"] ) );

    return item.release();
  }
  catch ( nlohmann::json::exception &ex )
  {
    mError = QStringLiteral( "Error parsing Item" );
    QgsDebugError( QStringLiteral( "Error parsing Item: %1" ).arg( ex.what() ) );
    return nullptr;
  }
}

QVector<QgsStacLink> QgsStacParser::parseLinks( const json &data )
{
  QVector< QgsStacLink > links;
  links.reserve( static_cast<int>( data.size() ) );
  for ( const auto &link : data )
  {
    QUrl linkUrl( QString::fromStdString( link.at( "href" ) ) );
    if ( linkUrl.isRelative() )
      linkUrl = mBaseUrl.resolved( linkUrl );

    const QgsStacLink l( linkUrl.toString(),
                         QString::fromStdString( link.at( "rel" ) ),
                         link.contains( "type" ) ? getString( link["type"] ) : QString(),
                         link.contains( "title" ) ? getString( link["title"] ) : QString() );
    links.append( l );
  }
  return links;
}

QMap<QString, QgsStacAsset> QgsStacParser::parseAssets( const json &data )
{
  QMap< QString, QgsStacAsset > assets;
  for ( const auto &asset : data.items() )
  {
    const json value = asset.value();
    QUrl assetUrl( QString::fromStdString( value.at( "href" ) ) );
    if ( assetUrl.isRelative() )
      assetUrl = mBaseUrl.resolved( assetUrl );

    const QgsStacAsset a( assetUrl.toString(),
                          value.contains( "title" ) ? getString( value["title"] ) : QString(),
                          value.contains( "description" ) ? getString( value["description"] ) : QString(),
                          value.contains( "type" ) ? getString( value["type"] ) : QString(),
                          value.contains( "roles" ) ? QgsJsonUtils::jsonToVariant( value["roles"] ).toStringList() : QStringList() );
    assets.insert( QString::fromStdString( asset.key() ), a );
  }
  return assets;
}

bool QgsStacParser::isSupportedStacVersion( const QString &version )
{
  const int maxMajor = 1;
  const int minMajor = 1;
  const int minMinor = 0;
  const int minRelease = 0;

  const thread_local QRegularExpression r( QStringLiteral( "^(\\d+).(\\d+).(\\d+)(-.*)*" ) );
  const QRegularExpressionMatch match = r.match( version );

  const QStringList m = match.capturedTexts();

  // m[0] is the whole matched string, so version 1.0.0 has a length of 4, 1.0.0-rc1 has a length of 5
  if ( m.length() < 4 )
    return false;

  const int major = m[1].toInt();
  const int minor = m[2].toInt();
  const int release = m[3].toInt();

  if ( major > maxMajor ||
       major < minMajor ||
       ( major == minMajor && minor < minMinor ) ||
       ( major == minMajor && minor == minMinor && release < minRelease ) ||
       ( major == minMajor && minor == minMinor && release == minRelease && m.length() == 5 ) )
  {
    return false;
  }
  return true;
}

QString QgsStacParser::getString( const nlohmann::json &data )
{
  return data.is_null() ? QString() : QString::fromStdString( data );
}

QgsStacItemCollection *QgsStacParser::itemCollection()
{
  try
  {
    QVector< QgsStacLink > links = parseLinks( mData.at( "links" ) );

    QVector< QgsStacItem * > items;
    items.reserve( static_cast<int>( mData.at( "features" ).size() ) );
    for ( auto &item : mData.at( "features" ) )
    {
      QgsStacItem *i = parseItem( item );
      if ( i )
        items.append( i );
    }

    const int numberMatched = mData.contains( "numberMatched" ) ? mData["numberMatched"].get<int>() : -1;

    return new QgsStacItemCollection( items, links, numberMatched );
  }
  catch ( nlohmann::json::exception &ex )
  {
    mError = QStringLiteral( "Error parsing ItemCollection" );
    QgsDebugError( QStringLiteral( "Error parsing ItemCollection: %1" ).arg( ex.what() ) );
    return nullptr;
  }
}

QgsStacCollections *QgsStacParser::collections()
{
  try
  {
    QVector< QgsStacLink > links = parseLinks( mData.at( "links" ) );

    QVector< QgsStacCollection * > cols;
    cols.reserve( static_cast<int>( mData.at( "collections" ).size() ) );
    for ( auto &col : mData.at( "collections" ) )
    {
      QgsStacCollection *c = parseCollection( col );
      if ( c )
        cols.append( c );
    }

    const int numberMatched = mData.contains( "numberMatched" ) ? mData["numberMatched"].get<int>() : -1;

    return new QgsStacCollections( cols, links, numberMatched );
  }
  catch ( nlohmann::json::exception &ex )
  {
    mError = QStringLiteral( "Error parsing ItemCollection" );
    QgsDebugError( QStringLiteral( "Error parsing ItemCollection: %1" ).arg( ex.what() ) );
    return nullptr;
  }
}
