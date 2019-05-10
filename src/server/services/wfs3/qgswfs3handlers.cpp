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
#include "qgsserverrequest.h"
#include "qgsserverresponse.h"
#include "qgsserverapiutils.h"
#include "qgsfeaturerequest.h"
#include "qgsjsonutils.h"
#include "qgsvectorlayer.h"

APIHandler::APIHandler()
{
  path = "/api";
  operationId = "api";
  summary = "The API definition";
  description = "The API definition";
  linkTitle = "API definition";
  linkType = QgsWfs3::rel::service;
  mimeType = "application/openapi+json;version=3.0";
}

void APIHandler::handleRequest( const QgsWfs3::Api *, const QgsServerRequest &request, QgsServerResponse &response, const QgsProject * ) const
{
  json data
  {
    { "api", "TODO" }
  };
  write( data, request, response );
}

LandingPageHandler::LandingPageHandler()
{
  path = "/";
  operationId = "getLandingPage";
  summary = "Landing page of this API";
  description = "The landing page provides links to the API definition, the Conformance "
                "statements and the metadata about the feature data in this dataset.";
  linkTitle = "Landing page";
  linkType = QgsWfs3::rel::self;
  mimeType = "application/json";
}

void LandingPageHandler::handleRequest( const QgsWfs3::Api *api, const QgsServerRequest &request, QgsServerResponse &response, const QgsProject *project ) const
{
  json data
  {
    { "links", json::array() }
  };
  for ( const auto &h : api->handlers() )
  {
    data["links"].push_back(
    {
      { "href", h->href( api, request )},
      { "rel", QgsWfs3::Api::relToString( linkType ) },
      { "type", h->mimeType },
      { "title", h->linkTitle },
    } );
  }
  write( data, request, response );
}

ConformanceHandler::ConformanceHandler()
{
  path = "/conformance";
  operationId = "getRequirementClasses";
  linkTitle = "WFS 3.0 conformance classes";
  summary = "Information about standards that this API conforms to";
  description = "List all requirements classes specified in a standard (e.g., WFS 3.0 "
                "Part 1: Core) that the server conforms to";
  linkType = QgsWfs3::rel::conformance;
  mimeType = "application/json";
}

void ConformanceHandler::handleRequest( const QgsWfs3::Api *api, const QgsServerRequest &request, QgsServerResponse &response, const QgsProject *project ) const
{
  Handler::handleRequest( api, request, response, project );
}

CollectionsHandler::CollectionsHandler()
{
  path = "/collections";
  operationId = "describeCollections";
  linkTitle = "Metadata about the feature collections";
  summary = "describe the feature collections in the dataset";
  description = "Metadata about the feature collections shared by this API.";
  linkType = QgsWfs3::rel::data;
  mimeType = "application/json";
}

void CollectionsHandler::handleRequest( const QgsWfs3::Api *api, const QgsServerRequest &request, QgsServerResponse &response, const QgsProject *project ) const
{
  qDebug() << "Checking URL " << api->normalizedUrl( request.url() ).path();
  static const QRegularExpression collectionsRe { QStringLiteral( R"re(/collections(\.json|\.html)?$)re" ) };
  static const QRegularExpression itemsRe { QStringLiteral( R"re(/collections/(?<collectionid>[^/]+)/items)re" ) };
  json data;
  const auto path { api->normalizedUrl( request.url() ).path() };
  if ( itemsRe.match( path ).hasMatch() )
  {
    const auto match { itemsRe.match( path ) };
    data = items( api, request, response, project, match.capturedTexts()[1] );
  }
  else if ( collectionsRe.match( path ).hasMatch() )
  {
    data = collections( api, request, response, project );
  }
  else
  {
    throw QgsServerApiBadRequestError( QStringLiteral( "Invalid method called" ) );
  }
  write( data, request, response );
}

json CollectionsHandler::collections( const QgsWfs3::Api *api, const QgsServerRequest &request, QgsServerResponse &response, const QgsProject *project ) const
{
  Q_UNUSED( response );
  json data
  {
    {
      "links", {
        {
          { "href", href( api, request ) },
          { "rel", QgsWfs3::Api::relToString( linkType ) },
          { "title", "this document as JSON" }
        },
        {
          { "href", href( api, request ) + "." + QgsWfs3::Api::contentTypeToExtension( QgsWfs3::contentType::HTML ) },
          { "rel", QgsWfs3::Api::relToString( QgsWfs3::rel::alternate ) },
          { "title", "this document as HTML" }
        },
      }
    },  // TODO: add XSD: mandatory?
    { "collections", json::array() },
  };

  if ( project )
  {
    // TODO: inclue meshes?
    for ( const auto &l : project->layers<QgsVectorLayer *>( ) )
    {
      data["collections"].push_back(
      {
        // identifier of the collection used, for example, in URIs
        { "name", l->name().toStdString() },
        // human readable title of the collection
        { "title", l->title().toStdString() },
        // a description of the features in the collection
        { "description", l->abstract().toStdString() },
        { "extent", "TODO" },
        { "CRS", "TODO" },
        {
          "links", {
            {
              { "href", href( api, request ) + "/" + l->name().toStdString() + "/items"  },
              { "rel", QgsWfs3::Api::relToString( QgsWfs3::rel::item ) },
              { "type", " application/geo+json" },
              { "title", l->title().toStdString() }
            }
          }
        },
      } );
    }
  }
  return data;
}

json CollectionsHandler::items( const QgsWfs3::Api *api, const QgsServerRequest &request, QgsServerResponse &response, const QgsProject *project, const QString &collectionId ) const
{
  if ( ! project )
  {
    throw QgsServerApiImproperlyConfiguredError( QStringLiteral( "Project is invalid or undefined" ) );
  }
  // Check collectionId
  const auto mapLayers { project->mapLayersByShortName<QgsVectorLayer *>( collectionId ) };
  if ( mapLayers.count() != 1 )
  {
    throw QgsServerApiImproperlyConfiguredError( QStringLiteral( "Collection with given id was not found or multiple matches were found" ) );
  }

  // Get parameters
  json data;
  if ( request.method() == QgsServerRequest::Method::GetMethod )
  {

    // Validate inputs
    auto ok { false };
    const auto bbox { request.parameter( QStringLiteral( "bbox" ) ) };
    const auto bboxCrs { request.parameter( QStringLiteral( "bbox-crs" ), QStringLiteral( "http://www.opengis.net/def/crs/OGC/1.3/CRS84" ) ) };
    const auto filterRect { QgsServerApiUtils::parseBbox( bbox ) };
    const auto crs { QgsServerApiUtils::parseCrs( bboxCrs ) };
    if ( crs.isValid() )
    {
      throw QgsServerApiBadRequestError( QStringLiteral( "CRS not valid" ) );
    }
    auto limit { request.parameter( QStringLiteral( "limit" ), QStringLiteral( "10" ) ).toInt( &ok ) };
    if ( 0 >= limit || limit > 10000 || !ok )
    {
      throw QgsServerApiBadRequestError( QStringLiteral( "Limit is not valid (0-10000)" ) );
    }
    // TODO: implement time
    const auto time { request.parameter( QStringLiteral( "time" ) ) };
    if ( ! time.isEmpty() )
    {
      throw QgsServerApiNotImplementedError( QStringLiteral( "Time is not implemented" ) ) ;
    }

    // Inputs are valid, process request
    const auto &mapLayer { mapLayers.first() };
    QgsFeatureRequest req;
    if ( ! filterRect.isNull() )
    {
      QgsCoordinateTransform ct( crs, mapLayer->crs(), project->transformContext() );
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
    data = exporter.exportFeaturesToJsonObject( featureList );
  }
  else
  {
    throw QgsServerApiNotImplementedError( QStringLiteral( "Only GET method is implemented." ) );
  }
  return data;

}
