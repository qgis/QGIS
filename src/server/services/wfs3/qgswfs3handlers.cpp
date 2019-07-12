/***************************************************************************
                              qgswfs3handlers.cpp
                              -------------------------
  begin                : May 3, 2019
  copyright            : (C) 2019 by Alessandro Pasotti
  email                : elpaso at itopen dot it
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgswfs3handlers.h"
#include "qgsserverogcapi.h"
#include "qgsserverapicontext.h"
#include "qgsserverrequest.h"
#include "qgsserverresponse.h"
#include "qgsserverapiutils.h"
#include "qgsfeaturerequest.h"
#include "qgsjsonutils.h"
#include "qgsvectorlayer.h"
#include "qgsmessagelog.h"

#include <QMimeDatabase>

Wfs3APIHandler::Wfs3APIHandler()
{
}

void Wfs3APIHandler::handleRequest( const QgsServerApiContext &context ) const
{
  // TODO
  json data
  {
    { "api", "This page will contain the API documentation for this service." }
  };
  json navigation = json::array();
  const auto url { context.request()->url() };
  navigation.push_back( {{ "title",  "Landing page" }, { "href", parentLink( url, 1 ) }} ) ;
  write( data, context, {{ "pageTitle", linkTitle() }, { "navigation", navigation }} );
}

Wfs3LandingPageHandler::Wfs3LandingPageHandler()
{

}

void Wfs3LandingPageHandler::handleRequest( const QgsServerApiContext &context ) const
{
  json data
  {
    { "links", links( context ) }
  };
  // Append links to APIs
  data["links"].push_back(
  {
    { "href", href( context, "/collections" )},
    { "rel", QgsServerOgcApi::relToString( QgsServerOgcApi::Rel::data ) },
    { "type", QgsServerOgcApi::mimeType( QgsServerOgcApi::ContentType::JSON ) },
    { "title", "Feature collections" },
  } );
  data["links"].push_back(
  {
    { "href", href( context, "/conformance" )},
    { "rel", QgsServerOgcApi::relToString( QgsServerOgcApi::Rel::conformance ) },
    { "type", QgsServerOgcApi::mimeType( QgsServerOgcApi::ContentType::JSON ) },
    { "title", "WFS 3.0 conformance classes" },
  } );
  data["links"].push_back(
  {
    { "href", href( context, "/api" )},
    { "rel", QgsServerOgcApi::relToString( QgsServerOgcApi::Rel::service_desc ) },
    { "type", QgsServerOgcApi::mimeType( QgsServerOgcApi::ContentType::OPENAPI3 ) },
    { "title", "API definition" },
  } );
  write( data, context, {{ "pageTitle", linkTitle() }, { "navigation", json::array() }} );
}


Wfs3ConformanceHandler::Wfs3ConformanceHandler()
{
}

void Wfs3ConformanceHandler::handleRequest( const QgsServerApiContext &context ) const
{
  json data
  {
    { "links", links( context ) },
    {
      "conformsTo", { "http://www.opengis.net/spec/wfs-1/3.0/req/core",
        "http://www.opengis.net/spec/wfs-1/3.0/req/oas30",
        "http://www.opengis.net/spec/wfs-1/3.0/req/html",
        "http://www.opengis.net/spec/wfs-1/3.0/req/gmlsf2",
        "http://www.opengis.net/spec/wfs-1/3.0/req/geojson"
      }
    }
  };
  json navigation = json::array();
  const auto url { context.request()->url() };
  navigation.push_back( {{ "title",  "Landing page" }, { "href", parentLink( url, 1 ) }} ) ;
  write( data, context, {{ "pageTitle", linkTitle() }, { "navigation", navigation }} );
}

Wfs3CollectionsHandler::Wfs3CollectionsHandler()
{
}

void Wfs3CollectionsHandler::handleRequest( const QgsServerApiContext &context ) const
{
  json data
  {
    {
      "links", links( context )
    },  // TODO: add XSD or other schema?
    { "collections", json::array() },
    {
      "crs", {
        "http://www.opengis.net/def/crs/OGC/1.3/CRS84",
        "http://www.opengis.net/def/crs/EPSG/0/4326"
      }
    }
  };

  if ( context.project() )
  {
    // TODO: include meshes?
    for ( const auto &layer : context.project()->layers<QgsVectorLayer *>( ) )
    {
      const auto title { layer->title().isEmpty() ? layer->name().toStdString() : layer->title().toStdString() };
      const auto shortName { layer->shortName().isEmpty() ? layer->name() : layer->shortName() };
      data["collections"].push_back(
      {
        // identifier of the collection used, for example, in URIs
        { "name", shortName.toStdString() },
        // human readable title of the collection
        { "title", title },
        // a description of the features in the collection
        { "description", layer->abstract().toStdString() },
        {
          "crs", {
            "http://www.opengis.net/def/crs/OGC/1.3/CRS84",
            "http://www.opengis.net/def/crs/EPSG/0/4326"
          }
        },
        // TODO: "relations" ?
        {
          "extent",  {
            { "crs", "http://www.opengis.net/def/crs/OGC/1.3/CRS84" },
            { "spatial", QgsServerApiUtils::layerExtent( layer ) }
          }
        },
        {
          "links", {
            {
              { "href", href( context, QStringLiteral( "/%1/items" ).arg( shortName ), QgsServerOgcApi::contentTypeToExtension( QgsServerOgcApi::ContentType::JSON ) )  },
              { "rel", QgsServerOgcApi::relToString( QgsServerOgcApi::Rel::item ) },
              { "type", QgsServerOgcApi::mimeType( QgsServerOgcApi::ContentType::GEOJSON ) },
              { "title", title + " as GeoJSON" }
            },
            {
              { "href", href( context, QStringLiteral( "/%1/items" ).arg( shortName ), QgsServerOgcApi::contentTypeToExtension( QgsServerOgcApi::ContentType::HTML ) )  },
              { "rel", QgsServerOgcApi::relToString( QgsServerOgcApi::Rel::item ) },
              { "type", QgsServerOgcApi::mimeType( QgsServerOgcApi::ContentType::HTML )  },
              { "title", title + " as HTML" }
            }/* TODO: not sure what these "concepts" are about, neither if they are mandatory
            {
              { "href", href( api, context.request(), QStringLiteral( "/%1/concepts" ).arg( shortName ) )  },
              { "rel", QgsServerOgcApi::relToString( QgsServerOgcApi::Rel::item ) },
              { "type", "text/html" },
              { "title", "Describe " + title }
            }
            */
          }
        },
      } );
    }
  }
  json navigation = json::array();
  const auto url { context.request()->url() };
  navigation.push_back( {{ "title",  "Landing page" }, { "href", parentLink( url, 1 ) }} ) ;
  write( data, context, {{ "pageTitle", linkTitle() }, { "navigation", navigation }} );
}

Wfs3DescribeCollectionHandler::Wfs3DescribeCollectionHandler()
{
}

void Wfs3DescribeCollectionHandler::handleRequest( const QgsServerApiContext &context ) const
{
  if ( ! context.project() )
  {
    throw QgsServerApiImproperlyConfiguredException( QStringLiteral( "Project is invalid or undefined" ) );
  }
  // Check collectionId
  const auto match { path().match( context.request()->url().path( ) ) };
  if ( ! match.hasMatch() )
  {
    throw QgsServerApiNotFoundError( QStringLiteral( "Collection was not found" ) );
  }
  const auto collectionId { match.captured( QStringLiteral( "collectionId" ) ) };
  // May throw if not found
  const auto mapLayer { layerFromCollection( context, collectionId ) };
  Q_ASSERT( mapLayer );

  const auto title { mapLayer->title().isEmpty() ? mapLayer->name().toStdString() : mapLayer->title().toStdString() };
  const auto shortName { mapLayer->shortName().isEmpty() ? mapLayer->name() : mapLayer->shortName() };
  json _links { links( context ) };
  _links.push_back(
  {
    { "href", href( context, QStringLiteral( "/items" ), QgsServerOgcApi::contentTypeToExtension( QgsServerOgcApi::ContentType::JSON ) )  },
    { "rel", QgsServerOgcApi::relToString( QgsServerOgcApi::Rel::items ) },
    { "type", QgsServerOgcApi::mimeType( QgsServerOgcApi::ContentType::JSON ) },
    { "title", title }
  } );

  _links.push_back(
  {
    { "href", href( context, QStringLiteral( "/items" ), QgsServerOgcApi::contentTypeToExtension( QgsServerOgcApi::ContentType::HTML ) )  },
    { "rel", QgsServerOgcApi::relToString( QgsServerOgcApi::Rel::items ) },
    { "type", QgsServerOgcApi::mimeType( QgsServerOgcApi::ContentType::HTML ) },
    { "title", title }
  }
  /* TODO: not sure what these "concepts" are about, neither if they are mandatory
  ,{
    { "href", href( api, *context.request() , QStringLiteral( "/concepts" ), QStringLiteral( "html") )  },
    { "rel", QgsServerOgcApi::relToString( QgsServerOgcApi::Rel::item ) },
    { "type", "text/html" },
    { "title", "Describe " + title }
  }
  */
  );
  json data
  {
    { "name", mapLayer->name().toStdString() },
    { "title", title },
    // TODO: check if we need to expose other advertised CRS here
    {
      "crs", {
        "http://www.opengis.net/def/crs/OGC/1.3/CRS84",
        "http://www.opengis.net/def/crs/EPSG/0/4326"
      }
    },
    // TODO: "relations" ?
    {
      "extent",  {
        { "crs", "http://www.opengis.net/def/crs/OGC/1.3/CRS84" },
        { "spatial", QgsServerApiUtils::layerExtent( mapLayer ) }
      }
    },
    {
      "links", _links
    }
  };
  json navigation = json::array();
  const auto url { context.request()->url() };
  navigation.push_back( {{ "title",  "Landing page" }, { "href", parentLink( url, 2 ) }} ) ;
  navigation.push_back( {{ "title",  "Collections" }, { "href", parentLink( url, 1 ) }} ) ;
  write( data, context, {{ "pageTitle", title }, { "navigation", navigation }} );
}

Wfs3CollectionsItemsHandler::Wfs3CollectionsItemsHandler()
{
}

QList<QgsServerQueryStringParameter> Wfs3CollectionsItemsHandler::parameters( const QgsServerApiContext &context ) const
{
  QList<QgsServerQueryStringParameter> params;

// Limit
  QgsServerQueryStringParameter limit { QStringLiteral( "limit" ), false,
                                        QgsServerQueryStringParameter::Type::Int,
                                        QStringLiteral( "Number of features to retrieve [0-10000]" ),
                                        10 };
  limit.setCustomValidator( [ ]( const QgsServerApiContext &, QVariant & value ) -> bool
  {
    return value >= 0 && value <= 10000;   // TODO: make this configurable!
  } );
  params.push_back( limit );

// Offset
  QgsServerQueryStringParameter offset { QStringLiteral( "offset" ), false,
                                         QgsServerQueryStringParameter::Type::Int,
                                         QStringLiteral( "Offset for features to retrieve [0-<number of features in the collection>]" ),
                                         0 };

  bool offsetValidatorSet = false;

// I'm not yet sure if we should get here without a project,
// but parameters() may be called to document the API - better safe than sorry.
  if ( context.project() )
  {
    // Fields filters
    const auto mapLayer { layerFromContext( context ) };
    if ( mapLayer )
    {
      offset.setCustomValidator( [ = ]( const QgsServerApiContext &, QVariant & value ) -> bool
      {
        const auto longVal { value.toLongLong( ) };
        return longVal >= 0 && longVal <= mapLayer->featureCount( );
      } );
      offset.setDescription( QStringLiteral( "Offset for features to retrieve [0-%1]" ).arg( mapLayer->featureCount( ) ) );
      offsetValidatorSet = true;

      const auto constFields { QgsServerApiUtils::publishedFields( mapLayer ) };
      for ( const auto &f : constFields )
      {
        QgsServerQueryStringParameter::Type t;
        switch ( f.type() )
        {
          case QVariant::Int:
          case QVariant::LongLong:
            t = QgsServerQueryStringParameter::Type::Int;
            break;
          case QVariant::Double:
            t = QgsServerQueryStringParameter::Type::Double;
            break;
          // TODO: date & time
          default:
            t = QgsServerQueryStringParameter::Type::String;
            break;
        }
        QgsServerQueryStringParameter fieldParam { f.name(), false,
            t, QStringLiteral( "Retrieve features filtered by: %1 (%2)" ).arg( f.name() )
            .arg( QgsServerQueryStringParameter::typeName( t ) ) };
        params.push_back( fieldParam );
      }
    }
  }

  if ( ! offsetValidatorSet )
  {
    offset.setCustomValidator( [ ]( const QgsServerApiContext &, QVariant & value ) -> bool
    {
      const auto longVal { value.toLongLong( ) };
      return longVal >= 0 ;
    } );
  }

  params.push_back( offset );

// BBOX
  QgsServerQueryStringParameter bbox { QStringLiteral( "bbox" ), false,
                                       QgsServerQueryStringParameter::Type::String,
                                       QStringLiteral( "BBOX filter for the features to retrieve" ) };
  params.push_back( bbox );

// BBOX CRS
  QgsServerQueryStringParameter bboxCrs { QStringLiteral( "bbox-crs" ), false,
                                          QgsServerQueryStringParameter::Type::String,
                                          QStringLiteral( "CRS for the BBOX filter" ),
                                          QStringLiteral( "http://www.opengis.net/def/crs/OGC/1.3/CRS84" ) };
  params.push_back( bboxCrs );

// Result type
  QgsServerQueryStringParameter resultType { QStringLiteral( "resultType" ), false,
      QgsServerQueryStringParameter::Type::String,
      QStringLiteral( "Type of returned result: 'results' (default) or 'hits'" ),
      QStringLiteral( "results" ) };
  params.push_back( resultType );
  return params;
}

void Wfs3CollectionsItemsHandler::handleRequest( const QgsServerApiContext &context ) const
{
  if ( ! context.project() )
  {
    throw QgsServerApiImproperlyConfiguredException( QStringLiteral( "Project is invalid or undefined" ) );
  }
  const auto mapLayer { layerFromContext( context ) };
  Q_ASSERT( mapLayer );
  const auto title { mapLayer->title().isEmpty() ? mapLayer->name().toStdString() : mapLayer->title().toStdString() };
  const auto shortName { mapLayer->shortName().isEmpty() ? mapLayer->name() : mapLayer->shortName() };

  // Get parameters
  QVariantMap params { validate( context )};

  if ( context.request()->method() == QgsServerRequest::Method::GetMethod )
  {

    // Validate inputs
    bool ok { false };

    // BBOX
    const QString bbox { params[ QStringLiteral( "bbox" )].toString()  };
    const QgsRectangle filterRect { QgsServerApiUtils::parseBbox( bbox ) };
    if ( ! bbox.isEmpty() && filterRect.isNull() )
    {
      throw QgsServerApiBadRequestException( QStringLiteral( "bbox is not valid" ) );
    }

    // CRS
    const QgsCoordinateReferenceSystem crs { QgsServerApiUtils::parseCrs( params[ QStringLiteral( "bbox-crs" ) ].toString() ) };
    if ( ! crs.isValid() )
    {
      throw QgsServerApiBadRequestException( QStringLiteral( "BBOX CRS is not valid" ) );
    }

    // resultType
    const QString resultType { params[ QStringLiteral( "resultType" ) ].toString() };
    static const QStringList availableResultTypes { QStringLiteral( "results" ), QStringLiteral( "hits" )};
    if ( ! availableResultTypes.contains( resultType ) )
    {
      throw QgsServerApiBadRequestException( QStringLiteral( "resultType is not valid [results, hits]" ) );
    }

    // Attribute filters
    QgsStringMap attrFilters;
    const auto constField { QgsServerApiUtils::publishedFields( mapLayer ) };
    for ( const QgsField &f : constField )
    {
      const QString val = params.value( f.name() ).toString() ;
      if ( ! val.isEmpty() )
      {
        QString sanitized { QgsServerApiUtils::sanitizedFieldValue( val ) };
        if ( sanitized.isEmpty() )
        {
          throw QgsServerApiBadRequestException( QStringLiteral( "Invalid filter field value [%1=%2]" ).arg( f.name() ).arg( val ) );
        }
        attrFilters[f.name()] = sanitized;
      }
    }

    // limit & offset
    // Apparently the standard set limits 0-10000 (and does not implement paging,
    // so we do our own paging with "offset")
    const long offset { params.value( QStringLiteral( "offset" ) ).toLongLong( &ok ) };

    // TODO: make the max limit configurable
    const long limit {  params.value( QStringLiteral( "limit" ) ).toLongLong( &ok ) };

    // TODO: implement time
    const QString time { context.request()->queryParameter( QStringLiteral( "time" ) ) };
    if ( ! time.isEmpty() )
    {
      throw QgsServerApiNotImplementedException( QStringLiteral( "Time filter is not implemented" ) ) ;
    }

    // Inputs are valid, process request
    QgsFeatureRequest req;
    if ( ! filterRect.isNull() )
    {
      QgsCoordinateTransform ct( crs, mapLayer->crs(), context.project()->transformContext() );
      ct.transform( filterRect );
      req.setFilterRect( ct.transform( filterRect ) );
    }

    QString filterExpression;
    if ( ! attrFilters.isEmpty() )
    {
      QStringList expressions;
      for ( auto it = attrFilters.constBegin(); it != attrFilters.constEnd(); it++ )
      {
        // Handle star
        static const QRegularExpression re2( R"raw([^\\]\*)raw" );
        if ( re2.match( it.value() ).hasMatch() )
        {
          QString val { it.value() };
          expressions.push_back( QStringLiteral( "\"%1\" LIKE '%2'" ).arg( it.key() ).arg( val.replace( '%', QStringLiteral( "%%" ) ).replace( '*', '%' ) ) );
        }
        else
        {
          expressions.push_back( QStringLiteral( "\"%1\" = '%2'" ).arg( it.key() ).arg( it.value() ) );
        }
      }
      filterExpression = expressions.join( QStringLiteral( " AND " ) );
      req.setFilterExpression( filterExpression );
    }

    // WFS3 core specs only serves 4326
    // TODO: handle custom CRSs
    req.setDestinationCrs( QgsCoordinateReferenceSystem::fromEpsgId( 4326 ), context.project()->transformContext() );
    // Add offset to limit because paging is not supported from QgsFeatureRequest
    req.setLimit( limit + offset );
    QgsJsonExporter exporter { mapLayer };
    exporter.setSourceCrs( QgsCoordinateReferenceSystem::fromEpsgId( 4326 ) );
    QgsFeatureList featureList;
    auto features { mapLayer->getFeatures( req ) };
    QgsFeature feat;
    long i { 0 };
    while ( features.nextFeature( feat ) )
    {
      // Ignore records before offset
      if ( i >= offset )
        featureList << feat;
      i++;
    }

    // Count features
    long matchedFeaturesCount = 0;
    if ( attrFilters.isEmpty() && filterRect.isNull() )
    {
      matchedFeaturesCount = mapLayer->featureCount();
    }
    else
    {
      if ( filterExpression.isEmpty() )
      {
        req.setNoAttributes();
      }
      req.setFlags( QgsFeatureRequest::Flag::NoGeometry );
      req.setLimit( -1 );
      features = mapLayer->getFeatures( req );
      while ( features.nextFeature( feat ) )
      {
        matchedFeaturesCount++;
      }
    }

    json data { exporter.exportFeaturesToJsonObject( featureList ) };

    // Add some metadata
    data["numberMatched"] = matchedFeaturesCount;
    data["numberReturned"] = featureList.count();
    data["links"] = links( context );

    // Current url
    const QUrl url { context.request()->url() };
    // Url without offset and limit
    QString cleanedUrl { url.toString().replace( QRegularExpression( R"raw(&?(offset|limit)(=\d+)*)raw" ), QString() ) };
    if ( ! url.hasQuery() )
    {
      cleanedUrl += '?';
    }

    // Get the self link
    json selfLink;
    for ( const auto &l : data["links"] )
    {
      if ( l["rel"] == "self" )
      {
        selfLink = l;
        break;
      }
    }
    // This should never happen!
    Q_ASSERT( !selfLink.is_null() );

    // Add prev - next links
    if ( offset != 0 )
    {
      auto prevLink { selfLink };
      prevLink["href"] = QStringLiteral( "%1&offset=%2&limit=%3" ).arg( cleanedUrl ).arg( std::max<long>( 0, limit - offset ) ).arg( limit ).toStdString();
      prevLink["rel"] = "prev";
      prevLink["name"] = "Previous page";
      data["links"].push_back( prevLink );
    }
    if ( limit + offset < matchedFeaturesCount )
    {
      auto nextLink { selfLink };
      nextLink["href"] = QStringLiteral( "%1&offset=%2&limit=%3" ).arg( cleanedUrl ).arg( std::min<long>( matchedFeaturesCount, limit + offset ) ).arg( limit ).toStdString();
      nextLink["rel"] = "next";
      nextLink["name"] = "Next page";
      data["links"].push_back( nextLink );
    }

    json navigation = json::array();
    navigation.push_back( {{ "title",  "Landing page" }, { "href", parentLink( url, 3 ) }} ) ;
    navigation.push_back( {{ "title",  "Collections" }, { "href", parentLink( url, 2 ) }} ) ;
    navigation.push_back( {{ "title",   title }, { "href", parentLink( url, 1 )  }} ) ;
    json htmlMetadata
    {
      { "pageTitle", "Features in layer " + title },
      { "layerTitle", title },
      {
        "geojsonUrl", href( context, "/",
                            QgsServerOgcApi::contentTypeToExtension( QgsServerOgcApi::ContentType::GEOJSON ) )
      },
      { "navigation", navigation }
    };
    write( data, context, htmlMetadata );
  }
  else
  {
    throw QgsServerApiNotImplementedException( QStringLiteral( "Only GET method is implemented." ) );
  }

}

Wfs3CollectionsFeatureHandler::Wfs3CollectionsFeatureHandler()
{
}

void Wfs3CollectionsFeatureHandler::handleRequest( const QgsServerApiContext &context ) const
{
  if ( ! context.project() )
  {
    throw QgsServerApiImproperlyConfiguredException( QStringLiteral( "Project is invalid or undefined" ) );
  }
  // Check collectionId
  const auto match { path().match( context.request()->url().path( ) ) };
  if ( ! match.hasMatch() )
  {
    throw QgsServerApiNotFoundError( QStringLiteral( "Collection was not found" ) );
  }
  const auto collectionId { match.captured( QStringLiteral( "collectionId" ) ) };
  // May throw if not found
  const auto mapLayer { layerFromCollection( context, collectionId ) };
  Q_ASSERT( mapLayer );
  const auto title { mapLayer->title().isEmpty() ? mapLayer->name().toStdString() : mapLayer->title().toStdString() };

  if ( context.request()->method() == QgsServerRequest::Method::GetMethod )
  {
    const auto featureId { match.captured( QStringLiteral( "featureId" ) ) };
    QgsJsonExporter exporter { mapLayer };
    auto feature { mapLayer->getFeature( featureId.toLongLong() ) };
    if ( ! feature.isValid() )
    {
      QgsServerApiInternalServerError( QStringLiteral( "Invalid feature [%1]" ).arg( featureId ) );
    }
    json data { exporter.exportFeatureToJsonObject( feature ) };
    data["links"] = links( context );
    json navigation = json::array();
    const auto url { context.request()->url() };
    navigation.push_back( {{ "title", "Landing page" }, { "href", parentLink( url, 4 ) }} ) ;
    navigation.push_back( {{ "title", "Collections" }, { "href", parentLink( url, 3 ) }} ) ;
    navigation.push_back( {{ "title", title }, { "href", parentLink( url, 2 )  }} ) ;
    navigation.push_back( {{ "title", "Items of " + title }, { "href", parentLink( url ) }} ) ;
    json htmlMetadata
    {
      { "pageTitle", title + " - feature " + featureId.toStdString() },
      {
        "geojsonUrl", href( context, "",
                            QgsServerOgcApi::contentTypeToExtension( QgsServerOgcApi::ContentType::GEOJSON ) )
      },
      { "navigation", navigation }
    };
    write( data, context, htmlMetadata );
  }
  else
  {
    throw QgsServerApiNotImplementedException( QStringLiteral( "Only GET method is implemented." ) );
  }
}

Wfs3StaticHandler::Wfs3StaticHandler()
{
}

void Wfs3StaticHandler::handleRequest( const QgsServerApiContext &context ) const
{

  const auto match { path().match( context.request()->url().path( ) ) };
  if ( ! match.hasMatch() )
  {
    throw QgsServerApiNotFoundError( QStringLiteral( "Static file was not found" ) );
  }

  const auto staticFilePath { match.captured( QStringLiteral( "staticFilePath" ) ) };
  // Calculate real path
  const auto filePath { staticPath() + '/' + staticFilePath };
  if ( ! QFile::exists( filePath ) )
  {
    QgsMessageLog::logMessage( QStringLiteral( "Static file was not found: %1" ).arg( filePath ), QStringLiteral( "Server" ), Qgis::Info );
    throw QgsServerApiNotFoundError( QStringLiteral( "Static file %1 was not found" ).arg( staticFilePath ) );
  }

  QFile f( filePath );
  if ( ! f.open( QIODevice::ReadOnly ) )
  {
    throw QgsServerApiInternalServerError( QStringLiteral( "Could not open static file %1" ).arg( staticFilePath ) );
  }

  const qint64 size { f.size() };
  const QByteArray content { f.readAll() };
  const QMimeType mimeType { QMimeDatabase().mimeTypeForFile( filePath )};
  context.response()->setHeader( QStringLiteral( "Content-Type" ), mimeType.name() );
  context.response()->setHeader( QStringLiteral( "Content-Length" ), QString::number( size ) );
  context.response()->write( content );
}
