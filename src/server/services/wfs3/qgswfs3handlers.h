/***************************************************************************
                              qgswfs3handlers.h
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

#ifndef QGS_WFS3_HANDLERS_H
#define QGS_WFS3_HANDLERS_H

#include "qgsserverapi.h"
#include "qgswfs3api.h"


/**
 * The APIHandler struct handles the API definition
 */
struct APIHandler: public QgsWfs3::Handler
{

  APIHandler( );
  void handleRequest( const QgsWfs3::Api *api, QgsServerApiContext *context ) const override;
};


/**
 * The StaticHandler struct serves static files from resources/server/api/wfs3/static
 */
struct StaticHandler: public QgsWfs3::Handler
{

  StaticHandler( );
  void handleRequest( const QgsWfs3::Api *api, QgsServerApiContext *context ) const override;
};


struct LandingPageHandler: public QgsWfs3::Handler
{

  LandingPageHandler( );

  /**
   * Return links to handlers having showInLandingPage = TRUE;
   */
  void handleRequest( const QgsWfs3::Api *api, QgsServerApiContext *context ) const override;
};


struct ConformanceHandler: public QgsWfs3::Handler
{

  ConformanceHandler( );
  void handleRequest( const QgsWfs3::Api *api, QgsServerApiContext *context ) const override;
};


/**
 * The CollectionsHandler lists all available collections for the current project
 * Path: /collections
 */
struct CollectionsHandler: public QgsWfs3::Handler
{

  CollectionsHandler( );
  void handleRequest( const QgsWfs3::Api *api, QgsServerApiContext *context ) const override;
};

/**
 * The DescribeCollectionHandler describes a single collection
 * Path: /collections/{collectionId}
 */
struct DescribeCollectionHandler: public QgsWfs3::Handler
{

  DescribeCollectionHandler( );
  void handleRequest( const QgsWfs3::Api *api, QgsServerApiContext *context ) const override;

};

/**
 * The CollectionsItemsHandler list all items in the collection
 * Path: /collections/{collectionId}
 */
struct CollectionsItemsHandler: public QgsWfs3::Handler
{

  CollectionsItemsHandler( );
  void handleRequest( const QgsWfs3::Api *api, QgsServerApiContext *context ) const override;
};


struct CollectionsFeatureHandler: public QgsWfs3::Handler
{

  CollectionsFeatureHandler( );
  void handleRequest( const QgsWfs3::Api *api, QgsServerApiContext *context ) const override;
};


#endif // QGS_WFS3_HANDLERS_H
