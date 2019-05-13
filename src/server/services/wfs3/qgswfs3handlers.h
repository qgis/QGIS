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

struct APIHandler: public QgsWfs3::Handler
{
  APIHandler( );
  void handleRequest( const QgsWfs3::Api *api, QgsServerApiContext *context ) const override;
};


struct LandingPageHandler: public QgsWfs3::Handler
{
  LandingPageHandler( );
  void handleRequest( const QgsWfs3::Api *api, QgsServerApiContext *context ) const override;
};


struct ConformanceHandler: public QgsWfs3::Handler
{
  ConformanceHandler( );
  void handleRequest( const QgsWfs3::Api *api, QgsServerApiContext *context ) const override;
};



struct CollectionsHandler: public QgsWfs3::Handler
{
    CollectionsHandler( );
    void handleRequest( const QgsWfs3::Api *api, QgsServerApiContext *context ) const override;

  private:

    json collections( const QgsWfs3::Api *api, QgsServerApiContext *context ) const;

    json items( const QgsWfs3::Api *api, QgsServerApiContext *context, const QString &collectionId ) const;

};

#endif // QGS_WFS3_HANDLERS_H
