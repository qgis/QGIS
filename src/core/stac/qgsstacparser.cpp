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
#include "qgsjsonutils.h"
#include "qgslogger.h"


void QgsStacParser::setData( const QByteArray &data )
{
  mError = QString();
  try
  {
    mData = nlohmann::json::parse( data.data() );
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
    else
    {
      mType = QgsStacObject::Type::Unknown;
    }
  }
  catch ( nlohmann::json::exception &ex )
  {
    mError = QStringLiteral( "Error parsing JSON" );
    QgsDebugError( QStringLiteral( "Error parsing JSON : %1" ).arg( ex.what() ) );
    mType = QgsStacObject::Type::Unknown;
  }
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
  try
  {
    const QString ver( QString::fromStdString( mData.at( "stac_version" ) ) );
    if ( !isSupportedStacVersion( ver ) )
    {
      mError = QStringLiteral( "Unsupported STAC version: %1" ).arg( ver );
      return nullptr;
    }

    const QString id( QString::fromStdString( mData.at( "id" ) ) );
    const QString description( QString::fromStdString( mData.at( "description" ) ) );

    QVector< QgsStacLink > links = parseLinks( mData.at( "links" ) );

    std::unique_ptr< QgsStacCatalog > catalog = std::make_unique< QgsStacCatalog >( id,
        ver,
        description,
        links );

    if ( mData.contains( "title" ) )
      catalog->setTitle( QString::fromStdString( mData["title"] ) );

    if ( mData.contains( "conformsTo" ) )
    {
      for ( const auto &conformanceClass : mData["conformsTo"] )
      {
        catalog->addConformanceClass( QString::fromStdString( conformanceClass ) );
      }
    }

    if ( mData.contains( "stac_extensions" ) )
    {
      QStringList extensions;
      for ( const auto &extension : mData["stac_extensions"] )
      {
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
  try
  {
    const QString ver( QString::fromStdString( mData.at( "stac_version" ) ) );
    if ( !isSupportedStacVersion( ver ) )
    {
      mError = QStringLiteral( "Unsupported STAC version: %1" ).arg( ver );
      return nullptr;
    }

    const QString id( QString::fromStdString( mData.at( "id" ) ) );
    const QString description( QString::fromStdString( mData.at( "description" ) ) );
    const QString license( QString::fromStdString( mData.at( "license" ) ) );

    QgsStacExtent stacExtent;
    int totalExtents = 0;
    for ( const auto &e : mData.at( "extent" ).at( "spatial" ).at( "bbox" ) )
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
    for ( const auto &e : mData.at( "extent" ).at( "temporal" ).at( "interval" ) )
    {
      if ( !e.is_array() ||
           e.size() != 2 )
      {
        mError = QStringLiteral( "Malformed STAC collection temporal extents" );
        QgsDebugError( mError );
        return nullptr;
      }
      const QDateTime start = e[0].is_null() ? QDateTime() : QDateTime::fromString( QString::fromStdString( e[0] ), Qt::ISODateWithMs );
      const QDateTime end = e[1].is_null() ? QDateTime() : QDateTime::fromString( QString::fromStdString( e[1] ), Qt::ISODateWithMs );

      if ( ++totalExtents == 1 )
        stacExtent.setTemporalExtent( QgsDateTimeRange( start, end ) );
      else
        stacExtent.addDetailedTemporalExtent( QgsDateTimeRange( start, end ) );
    }

    QVector< QgsStacLink > links = parseLinks( mData.at( "links" ) );

    std::unique_ptr< QgsStacCollection > collection = std::make_unique< QgsStacCollection >( id,
        ver,
        description,
        links,
        license,
        stacExtent );

    if ( mData.contains( "title" ) )
      collection->setTitle( QString::fromStdString( mData["title"] ) );

    if ( mData.contains( "stac_extensions" ) )
    {
      QStringList extensions;
      for ( const auto &extension : mData["stac_extensions"] )
      {
        extensions.append( QString::fromStdString( extension ) );
      }
      collection->setStacExtensions( extensions );
    }

    if ( mData.contains( "keywords" ) )
    {
      QStringList keywords;
      for ( const auto &kw : mData["keywords"] )
      {
        keywords.append( QString::fromStdString( kw ) );
      }
      collection->setKeywords( keywords );
    }

    if ( mData.contains( "providers" ) )
    {
      QVector< QgsStacProvider > providers;
      for ( const auto &p : mData["providers"] )
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
            roles.append( QString::fromStdString( role ) );
          }
        }
        const QgsStacProvider provider( QString::fromStdString( p["name"] ),
                                        p.contains( "description" ) ? QString::fromStdString( p["description"] ) : QString(),
                                        roles,
                                        p.contains( "url" ) ? QString::fromStdString( p["url"] ) : QString() );



        providers.append( provider );
      }
      collection->setProviders( providers );
    }

    if ( mData.contains( "summaries" ) )
    {
      const QVariant summ = QgsJsonUtils::jsonToVariant( mData["summaries"] );
      collection->setSummaries( summ.toMap() );
    }

    if ( mData.contains( "assets" ) )
    {
      QMap< QString, QgsStacAsset > assets = parseAssets( mData["assets"] );
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
        extensions.append( QString::fromStdString( extension ) );
      }
      item->setStacExtensions( extensions );
    }

    if ( data.contains( "collection" ) )
      item->setCollection( QString::fromStdString( data["collection"] ) );

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
    const QgsStacLink l( QString::fromStdString( link.at( "href" ) ),
                         QString::fromStdString( link.at( "rel" ) ),
                         link.contains( "type" ) ? QString::fromStdString( link["type"] ) : QString(),
                         link.contains( "title" ) ? QString::fromStdString( link["title"] ) : QString() );
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
    const QgsStacAsset a( QString::fromStdString( value.at( "href" ) ),
                          value.contains( "title" ) ? QString::fromStdString( value["title"] ) : QString(),
                          value.contains( "description" ) ? QString::fromStdString( value["description"] ) : QString(),
                          value.contains( "type" ) ? QString::fromStdString( value["type"] ) : QString(),
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
