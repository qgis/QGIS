/***************************************************************************
                              qgslandingpagehandlers.h
                              -------------------------
  begin                : July 30, 2020
  copyright            : (C) 2020 by Alessandro Pasotti
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

#ifndef QGS_LANDINGPAGE_HANDLERS_H
#define QGS_LANDINGPAGE_HANDLERS_H

#include "qgsserversettings.h"
#include "qgsserverogcapihandler.h"
#include "qgsfields.h"
#include "qgsserverrequest.h"

class QgsFeatureRequest;
class QgsServerOgcApi;
class QgsFeature;

/**
 * The QgsLandingPageHandler implements the landing page handler.
 */
class QgsLandingPageHandler: public QgsServerOgcApiHandler
{
  public:

    QgsLandingPageHandler( const QgsServerSettings *settings );

    void handleRequest( const QgsServerApiContext &context ) const override;

    // QgsServerOgcApiHandler interface
    QRegularExpression path() const override { return QRegularExpression( R"re(^/(index.html|index.json)?$)re" ); }
    std::string operationId() const override { return "getLandingPage"; }
    QStringList tags() const override { return { QStringLiteral( "Catalog" ) }; }
    std::string summary() const override
    {
      return "Server Landing Page";
    }
    std::string description() const override
    {
      return "The landing page provides information about available projects and services.";
    }
    std::string linkTitle() const override { return "Landing page"; }
    QgsServerOgcApi::Rel linkType() const override { return QgsServerOgcApi::Rel::self; }
    const QString templatePath( const QgsServerApiContext &context ) const override;

  private:


    json projectsData( const QgsServerRequest &request ) const;

    const QgsServerSettings *mSettings = nullptr;
};


/**
 * The QgsLandingPageMapHandler implements the landing page map handler (JSON only).
 */
class QgsLandingPageMapHandler: public QgsServerOgcApiHandler
{
  public:

    QgsLandingPageMapHandler( const QgsServerSettings *settings );

    void handleRequest( const QgsServerApiContext &context ) const override;

    // QgsServerOgcApiHandler interface
    QRegularExpression path() const override { return QRegularExpression( R"re(^/map/([a-f0-9]{32}).*$)re" ); }
    std::string operationId() const override { return "getMap"; }
    QStringList tags() const override { return { QStringLiteral( "Catalog" ), QStringLiteral( "Map Viewer" ) }; }
    std::string summary() const override
    {
      return "Server Map Viewer";
    }
    std::string description() const override
    {
      return "Shows a map";
    }
    std::string linkTitle() const override { return "Map Viewer"; }
    QgsServerOgcApi::Rel linkType() const override { return QgsServerOgcApi::Rel::self; }

  private:

    const QgsServerSettings *mSettings = nullptr;
};

#endif // QGS_LANDINGPAGE_HANDLERS_H
