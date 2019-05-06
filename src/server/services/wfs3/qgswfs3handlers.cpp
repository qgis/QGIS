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
  json data
  {
    { "links", json::array() },
    { "collections", json::array() },
  };
  data["links"].push_back(
  {
    { "href", href( api, request ) },
    { "rel", QgsWfs3::Api::relToString( linkType ) },
    { "title", linkTitle },
  } );
  if ( project )
  {
    for ( const auto &l : project->mapLayers( ) )
    {
      data["collections"].push_back(
      {
        { "name", l->name().toStdString() }
      } );
    }
  }
  write( data, request, response );
}
