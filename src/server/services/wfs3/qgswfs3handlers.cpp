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
  mimeType = "application/openapi+json;version=3.0";
}

void APIHandler::handleRequest( const QgsWfs3::Api *, QgsServerApiContext *context ) const
{
  json data
  {
    { "api", "TODO" }
  };
  write( data, *context->request(), *context->response() );
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
  mimeType = "application/json";
}

void LandingPageHandler::handleRequest( const QgsWfs3::Api *api, QgsServerApiContext *context ) const
{
  json data
  {
    { "links", json::array() }
  };
  for ( const auto &h : api->handlers() )
  {
    data["links"].push_back(
    {
      { "href", h->href( api, *context->request() )},
      { "rel", QgsWfs3::Api::relToString( linkType ) },
      { "type", h->mimeType },
      { "title", h->linkTitle },
    } );
  }
  write( data, *context->request(), *context->response() );
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
  mimeType = "application/json";
}

void ConformanceHandler::handleRequest( const QgsWfs3::Api *api, QgsServerApiContext *context ) const
{
  // TODO
  Handler::handleRequest( api, context );
}

CollectionsHandler::CollectionsHandler()
{
  path.setPattern( QStringLiteral( R"re(/collections$)re" ) );
  operationId = "describeCollections";
  linkTitle = "Metadata about the feature collections";
  summary = "describe the feature collections in the dataset";
  description = "Metadata about the feature collections shared by this API.";
  linkType = QgsWfs3::rel::data;
  mimeType = "application/json";
}

void CollectionsHandler::handleRequest( const QgsWfs3::Api *api, QgsServerApiContext *context ) const
{
  json data
  {
    {
      "links", {
        {
          { "href", href( api, *context->request() ) },
          { "rel", QgsWfs3::Api::relToString( linkType ) },
          { "title", "this document as JSON" }
        },
        {
          { "href", href( api, *context->request(), QString(), QgsWfs3::Api::contentTypeToExtension( QgsWfs3::contentType::HTML ) ) },
          { "rel", QgsWfs3::Api::relToString( QgsWfs3::rel::alternate ) },
          { "title", "this document as HTML" }
        },
      }
    },  // TODO: add XSD: mandatory?
    { "collections", json::array() },
  };

  if ( context->project() )
  {
    // TODO: inclue meshes?
    for ( const auto &l : context->project()->layers<QgsVectorLayer *>( ) )
    {
      const auto title { l->title().isEmpty() ? l->name().toStdString() : l->title().toStdString() };
      const auto shortName { l->shortName().isEmpty() ? l->name() : l->shortName() };
      data["collections"].push_back(
      {
        // identifier of the collection used, for example, in URIs
        { "name", l->name().toStdString() },
        // human readable title of the collection
        { "title", title },
        // a description of the features in the collection
        { "description", l->abstract().toStdString() },
        { "extent", "TODO" },
        { "CRS", "TODO" },
        {
          "links", {
            {
              { "href", href( api, *context->request(), QStringLiteral( "/%1/items" ).arg( shortName ) )  },
              { "rel", QgsWfs3::Api::relToString( QgsWfs3::rel::item ) },
              { "type", "application/geo+json" },
              { "title", title }
            },
            {
              { "href", href( api, *context->request(), QStringLiteral( "/%1/concepts" ).arg( shortName ) )  },
              { "rel", QgsWfs3::Api::relToString( QgsWfs3::rel::item ) },
              { "type", "application/geo+json" },
              { "title", "Describe " + title }
            }
          }
        },
      } );
    }
  }
  write( data, *context->request(), *context->response() );
}

DescribeCollectionHandler::DescribeCollectionHandler()
{
  path.setPattern( QStringLiteral( R"re(/collections/(?<collectionId>[^/]+)(\.json|\.html)?$)re" ) );
  operationId = "describeCollection";
  linkTitle = "Metadata about the feature collections";
  summary = "describe the feature collections in the dataset";
  description = "Metadata about the feature collections shared by this API.";
  linkType = QgsWfs3::rel::data;
  mimeType = "application/json";
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
  const auto mapLayers { context->project()->mapLayersByShortName<QgsVectorLayer *>( collectionId ) };
  if ( mapLayers.count() != 1 )
  {
    throw QgsServerApiImproperlyConfiguredError( QStringLiteral( "Collection with given id was not found or multiple matches were found" ) );
  }
  const auto layer { mapLayers.first() };
  const auto title { layer->title().isEmpty() ? layer->name().toStdString() : layer->title().toStdString() };
  const auto shortName { layer->shortName().isEmpty() ? layer->name() : layer->shortName() };
  json data
  {
    { "name", layer->name().toStdString() },
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
        { "spatial", QgsServerApiUtils::layerExtent( layer ) }
      }
    },
    {
      "links", {
        {
          { "href", href( api, *context->request(), QStringLiteral( "/%1/items" ).arg( shortName ) )  },
          { "rel", QgsWfs3::Api::relToString( QgsWfs3::rel::item ) },
          { "type", "application/geo+json" },
          { "title", title }
        }
        /* TODO: not sure what these "concepts" are about, or if they are mandatory
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
  write( data, *context->request(), *context->response() );
}


CollectionsItemsHandler::CollectionsItemsHandler()
{
  path.setPattern( QStringLiteral( R"re(/collections/(?<collectionId>[^/]+)/items(\.json|\.html)?$)re" ) );
  operationId = "describeCollection";
  linkTitle = "Retrieve the features of the collection";
  summary = "retrieve features of feature collection collectionId";
  description = "Every feature in a dataset belongs to a collection. A dataset may "
                "consist of multiple feature collections. A feature collection is often a "
                "collection of features of a similar type, based on a common schema. "
                "Use content negotiation to request HTML or GeoJSON.";
  linkType = QgsWfs3::rel::data;
  mimeType = "application/json";
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
  const auto mapLayers { context->project()->mapLayersByShortName<QgsVectorLayer *>( collectionId ) };
  if ( mapLayers.count() != 1 )
  {
    throw QgsServerApiImproperlyConfiguredError( QStringLiteral( "Collection with given id was not found or multiple matches were found" ) );
  }
  const auto &mapLayer { mapLayers.first() };

  // Get parameters
  if ( context->request()->method() == QgsServerRequest::Method::GetMethod )
  {

    // Validate inputs
    auto ok { false };
    const auto bbox { context->request()->queryParameter( QStringLiteral( "bbox" ) ) };
    const auto offset { context->request()->queryParameter( QStringLiteral( "offset" ) ) };
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
    req.setLimit( limit );
    QgsJsonExporter exporter { mapLayer };
    QgsFeatureList featureList;
    auto features { mapLayer->getFeatures( req ) };
    QgsFeature feat;
    while ( features.nextFeature( feat ) )
    {
      featureList << feat;
    }
    write( exporter.exportFeaturesToJsonObject( featureList ), *context->request(), *context->response() );
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
  mimeType = "application/json";
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
  const auto mapLayers { context->project()->mapLayersByShortName<QgsVectorLayer *>( collectionId ) };
  if ( mapLayers.count() != 1 )
  {
    throw QgsServerApiImproperlyConfiguredError( QStringLiteral( "Collection with given id was not found or multiple matches were found" ) );
  }
  if ( context->request()->method() == QgsServerRequest::Method::GetMethod )
  {
    const auto featureId { match.captured( QStringLiteral( "featureId" ) ) };
    const auto &mapLayer { mapLayers.first() };
    QgsJsonExporter exporter { mapLayer };
    QgsFeatureRequest req { QgsExpression { QStringLiteral( "$id = '%1'" ).arg( featureId ) } };
    auto features { mapLayer->getFeatures( req ) };
    QgsFeature feat;
    QgsFeatureList featureList;
    while ( features.nextFeature( feat ) )
    {
      featureList << feat;
    }
    write( exporter.exportFeaturesToJsonObject( featureList ), *context->request(), *context->response() );
  }
  else
  {
    throw QgsServerApiNotImplementedError( QStringLiteral( "Only GET method is implemented." ) );
  }
}
