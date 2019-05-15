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
#include "qgsserverapicontext.h"
#include "qgsserverrequest.h"
#include "qgsserverresponse.h"
#include "qgsserverapiutils.h"
#include "qgsfeaturerequest.h"
#include "qgsjsonutils.h"
#include "qgsvectorlayer.h"

APIHandler::APIHandler()
{
  path.setPattern( QStringLiteral( R"re(/api)re" ) );
  operationId = "api";
  summary = "The API definition";
  description = "The API definition";
  linkTitle = "API definition";
  linkType = QgsWfs3::rel::service;
  mimeType = QgsWfs3::contentTypes::OPENAPI3;
  landingPageRootLink = QStringLiteral( "api" );
}

void APIHandler::handleRequest( const QgsWfs3::Api *, QgsServerApiContext *context ) const
{
  // TODO
  json data
  {
    { "api", "This page will contain the API documentation for this service." }
  };
  write( data, context->request(), context->response() );
}

LandingPageHandler::LandingPageHandler()
{
  path.setPattern( QStringLiteral( R"re($)re" ) );
  operationId = "getLandingPage";
  summary = "Landing page of this API";
  description = "The landing page provides links to the API definition, the Conformance "
                "statements and the metadata about the feature data in this dataset.";
  linkTitle = "Landing page";
  linkType = QgsWfs3::rel::self;
  mimeType = QgsWfs3::contentTypes::HTML;
}

void LandingPageHandler::handleRequest( const QgsWfs3::Api *api, QgsServerApiContext *context ) const
{
  json data
  {
    { "links", json::array() }
  };
  QList<const Handler *> landingPageHandlers { this };
  for ( const auto &h : api->handlers() )
  {
    if ( ! h->landingPageRootLink.isEmpty() )
    {
      landingPageHandlers << h.get();
    }
  }
  for ( const auto &h : landingPageHandlers )
  {
    data["links"].push_back(
    {
      { "href", h->href( api, context->request(), "/" + h->landingPageRootLink )},
      { "rel", QgsWfs3::Api::relToString( h->linkType ) },
      { "type", QgsWfs3::sContentTypeMime.value( h->mimeType ).toStdString() },
      { "title", h->linkTitle },
    } );
  }
  write( data, context->request(), context->response() );
}


ConformanceHandler::ConformanceHandler()
{
  path.setPattern( QStringLiteral( R"re(/conformance$)re" ) );
  operationId = "getRequirementClasses";
  linkTitle = "WFS 3.0 conformance classes";
  summary = "Information about standards that this API conforms to";
  description = "List all requirements classes specified in a standard (e.g., WFS 3.0 "
                "Part 1: Core) that the server conforms to";
  linkType = QgsWfs3::rel::conformance;
  mimeType = QgsWfs3::contentTypes::JSON;
  landingPageRootLink = QStringLiteral( "conformance" );
}

void ConformanceHandler::handleRequest( const QgsWfs3::Api *api, QgsServerApiContext *context ) const
{
  // TODO
  Handler::handleRequest( api, context );
}

CollectionsHandler::CollectionsHandler()
{
  path.setPattern( QStringLiteral( R"re(/collections(\.json|\.html)?$)re" ) );
  operationId = "describeCollections";
  linkTitle = "Metadata about the feature collections";
  summary = "describe the feature collections in the dataset";
  description = "Metadata about the feature collections shared by this API.";
  linkType = QgsWfs3::rel::data;
  mimeType = QgsWfs3::contentTypes::JSON;
  landingPageRootLink = QStringLiteral( "collections" );
}

void CollectionsHandler::handleRequest( const QgsWfs3::Api *api, QgsServerApiContext *context ) const
{
  json data
  {
    {
      "links", {
        {
          { "href", href( api, context->request() ) },
          { "rel", QgsWfs3::Api::relToString( linkType ) },
          { "title", "this document as JSON" }
        },
        {
          { "href", href( api, context->request(), QString(), QgsWfs3::Api::contentTypeToExtension( QgsWfs3::contentTypes::HTML ) ) },
          { "rel", QgsWfs3::Api::relToString( QgsWfs3::rel::alternate ) },
          { "title", "this document as HTML" }
        },
      }
    },  // TODO: add XSD or other schema?
    { "collections", json::array() },
    {
      "crs", {
        "http://www.opengis.net/def/crs/OGC/1.3/CRS84",
        "http://www.opengis.net/def/crs/EPSG/0/4326"
      }
    }
  };

  if ( context->project() )
  {
    // TODO: inclue meshes?
    for ( const auto &layer : context->project()->layers<QgsVectorLayer *>( ) )
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
              { "href", href( api, context->request(), QStringLiteral( "/%1/items" ).arg( shortName ) )  },
              { "rel", QgsWfs3::Api::relToString( QgsWfs3::rel::item ) },
              { "type", "application/geo+json" },
              { "title", title + " as GeoJSON" }
            }/* TODO: not sure what these "concepts" are about, neither if they are mandatory
            {
              { "href", href( api, context->request(), QStringLiteral( "/%1/concepts" ).arg( shortName ) )  },
              { "rel", QgsWfs3::Api::relToString( QgsWfs3::rel::item ) },
              { "type", "text/html" },
              { "title", "Describe " + title }
            }
            */
          }
        },
      } );
    }
  }
  write( data, context->request(), context->response() );
}

DescribeCollectionHandler::DescribeCollectionHandler()
{
  path.setPattern( QStringLiteral( R"re(/collections/(?<collectionId>[^/]+)(\.json|\.html)?$)re" ) );
  operationId = "describeCollection";
  linkTitle = "Metadata about the feature collections";
  summary = "describe the feature collections in the dataset";
  description = "Metadata about the feature collections shared by this API.";
  linkType = QgsWfs3::rel::data;
  mimeType = QgsWfs3::contentTypes::JSON;
}

void DescribeCollectionHandler::handleRequest( const QgsWfs3::Api *api, QgsServerApiContext *context ) const
{
  if ( ! context->project() )
  {
    throw QgsServerApiImproperlyConfiguredError( QStringLiteral( "Project is invalid or undefined" ) );
  }
  // Check collectionId
  const auto match { path.match( context->request()->url().path( ) ) };
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
      "links", {
        {
          { "href", href( api, context->request(), QStringLiteral( "/%1/items" ).arg( shortName ) )  },
          { "rel", QgsWfs3::Api::relToString( QgsWfs3::rel::item ) },
          { "type", "application/geo+json" },
          { "title", title }
        }
        /* TODO: not sure what these "concepts" are about, neither if they are mandatory
        ,{
          { "href", href( api, *context->request() , QStringLiteral( "/concepts" ), QStringLiteral( "html") )  },
          { "rel", QgsWfs3::Api::relToString( QgsWfs3::rel::item ) },
          { "type", "text/html" },
          { "title", "Describe " + title }
        }
        */
      }
    }
  };
  write( data, context->request(), context->response() );
}


CollectionsItemsHandler::CollectionsItemsHandler()
{
  path.setPattern( QStringLiteral( R"re(/collections/(?<collectionId>[^/]+)/items(\.json|\.html)?$)re" ) );
  operationId = "getFeatures";
  linkTitle = "Retrieve the features of the collection";
  summary = "retrieve features of feature collection collectionId";
  description = "Every feature in a dataset belongs to a collection. A dataset may "
                "consist of multiple feature collections. A feature collection is often a "
                "collection of features of a similar type, based on a common schema. "
                "Use content negotiation to request HTML or GeoJSON.";
  linkType = QgsWfs3::rel::data;
  mimeType =  QgsWfs3::contentTypes::JSON;
}

void CollectionsItemsHandler::handleRequest( const QgsWfs3::Api *api, QgsServerApiContext *context ) const
{
  Q_UNUSED( api );
  if ( ! context->project() )
  {
    throw QgsServerApiImproperlyConfiguredError( QStringLiteral( "Project is invalid or undefined" ) );
  }
  // Check collectionId
  const auto match { path.match( context->request()->url().path( ) ) };
  if ( ! match.hasMatch() )
  {
    throw QgsServerApiNotFoundError( QStringLiteral( "Collection was not found" ) );
  }
  const auto collectionId { match.captured( QStringLiteral( "collectionId" ) ) };
  // May throw if not found
  const auto mapLayer { layerFromCollection( context, collectionId ) };
  Q_ASSERT( mapLayer );

  // Get parameters
  if ( context->request()->method() == QgsServerRequest::Method::GetMethod )
  {

    // Validate inputs
    auto ok { false };
    const auto bbox { context->request()->queryParameter( QStringLiteral( "bbox" ) ) };
    const auto resultType { context->request()->queryParameter( QStringLiteral( "resultType" ), QStringLiteral( "results" ) ) };
    static const QStringList availableResultTypes { QStringLiteral( "results" ), QStringLiteral( "hits" )};
    if ( ! availableResultTypes.contains( resultType ) )
    {
      throw QgsServerApiBadRequestError( QStringLiteral( "resultType is not valid [results, hits]" ) );
    }
    // TODO: attribute filters
    const auto bboxCrs { context->request()->queryParameter( QStringLiteral( "bbox-crs" ), QStringLiteral( "http://www.opengis.net/def/crs/OGC/1.3/CRS84" ) ) };
    const auto filterRect { QgsServerApiUtils::parseBbox( bbox ) };
    const auto crs { QgsServerApiUtils::parseCrs( bboxCrs ) };
    if ( ! crs.isValid() )
    {
      throw QgsServerApiBadRequestError( QStringLiteral( "CRS is not valid" ) );
    }
    auto offset { context->request()->queryParameter( QStringLiteral( "offset" ), QStringLiteral( "0" ) ).toInt( &ok ) };
    if ( offset < 0 || !ok )
    {
      throw QgsServerApiBadRequestError( QStringLiteral( "Offset is not valid" ) );
    }
    auto limit { context->request()->queryParameter( QStringLiteral( "limit" ), QStringLiteral( "10" ) ).toInt( &ok ) };
    if ( 0 >= limit || limit > 10000 || !ok )
    {
      throw QgsServerApiBadRequestError( QStringLiteral( "Limit is not valid (0-10000)" ) );
    }
    // TODO: implement time
    const auto time { context->request()->queryParameter( QStringLiteral( "time" ) ) };
    if ( ! time.isEmpty() )
    {
      throw QgsServerApiNotImplementedError( QStringLiteral( "Time is not implemented" ) ) ;
    }

    // Inputs are valid, process request
    QgsFeatureRequest req;
    if ( ! filterRect.isNull() )
    {
      QgsCoordinateTransform ct( crs, mapLayer->crs(), context->project()->transformContext() );
      ct.transform( filterRect );
      req.setFilterRect( ct.transform( filterRect ) );
    }
    // WFS3 core specs only serves 4326
    // TODO: handle custom CRSs
    req.setDestinationCrs( QgsCoordinateReferenceSystem::fromEpsgId( 4326 ), context->project()->transformContext() );
    // Add offset to limit because paging is not supported from QgsFeatureRequest
    req.setLimit( limit + offset );
    // Offset
    QgsJsonExporter exporter { mapLayer };
    QgsFeatureList featureList;
    auto features { mapLayer->getFeatures( req ) };
    QgsFeature feat;
    auto i { 0 };
    while ( features.nextFeature( feat ) )
    {
      // Ignore records before offset
      if ( i >= offset )
        featureList << feat;
      i++;
    }
    write( exporter.exportFeaturesToJsonObject( featureList ), context->request(), context->response() );
  }
  else
  {
    throw QgsServerApiNotImplementedError( QStringLiteral( "Only GET method is implemented." ) );
  }

}

CollectionsFeatureHandler::CollectionsFeatureHandler()
{
  path.setPattern( QStringLiteral( R"re(/collections/(?<collectionId>[^/]+)/items/(?<featureId>[^/]+)(\.json|\.html)?$)re" ) );
  operationId = "getFeature";
  linkTitle = "Retrieve a feature";
  summary = "retrieve a feature; use content negotiation to request HTML or GeoJSON";
  description = "";
  linkType = QgsWfs3::rel::data;
  mimeType = QgsWfs3::contentTypes::JSON;
}

void CollectionsFeatureHandler::handleRequest( const QgsWfs3::Api *api, QgsServerApiContext *context ) const
{
  Q_UNUSED( api );
  if ( ! context->project() )
  {
    throw QgsServerApiImproperlyConfiguredError( QStringLiteral( "Project is invalid or undefined" ) );
  }
  // Check collectionId
  const auto match { path.match( context->request()->url().path( ) ) };
  if ( ! match.hasMatch() )
  {
    throw QgsServerApiNotFoundError( QStringLiteral( "Collection was not found" ) );
  }
  const auto collectionId { match.captured( QStringLiteral( "collectionId" ) ) };
  // May throw if not found
  const auto mapLayer { layerFromCollection( context, collectionId ) };
  Q_ASSERT( mapLayer );

  if ( context->request()->method() == QgsServerRequest::Method::GetMethod )
  {
    const auto featureId { match.captured( QStringLiteral( "featureId" ) ) };
    QgsJsonExporter exporter { mapLayer };
    QgsFeatureRequest req { QgsExpression { QStringLiteral( "$id = '%1'" ).arg( featureId ) } };
    auto features { mapLayer->getFeatures( req ) };
    QgsFeature feat;
    QgsFeatureList featureList;
    while ( features.nextFeature( feat ) )
    {
      featureList << feat;
    }
    write( exporter.exportFeaturesToJsonObject( featureList ), context->request(), context->response() );
  }
  else
  {
    throw QgsServerApiNotImplementedError( QStringLiteral( "Only GET method is implemented." ) );
  }
}

StaticHandler::StaticHandler()
{
  path.setPattern( QStringLiteral( R"re(/static/(?<staticPath>.*)$)re" ) );
  operationId = "static";
  linkTitle = "Serves static files";
  summary = "Serves static files";
  description = "";
  linkType = QgsWfs3::rel::data;
  mimeType = QgsWfs3::contentTypes::JSON;
}

void StaticHandler::handleRequest( const QgsWfs3::Api *api, QgsServerApiContext *context ) const
{

}
