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

#include "qgsfields.h"
#include "qgsserverogcapihandler.h"
#include "qgsserverrequest.h"
#include "qgsserversettings.h"

class QgsFeatureRequest;
class QgsServerOgcApi;
class QgsFeature;

/**
 * The QgsLandingPageHandler implements the landing page handler.
 */
class QgsLandingPageHandler : public QgsServerOgcApiHandler
{
  public:
    QgsLandingPageHandler( const QgsServerSettings *settings );

    void handleRequest( const QgsServerApiContext &context ) const override;

    // QgsServerOgcApiHandler interface
    [[nodiscard]] QRegularExpression path() const override;
    [[nodiscard]] std::string operationId() const override { return "getLandingPage"; }
    [[nodiscard]] QStringList tags() const override { return { QStringLiteral( "Catalog" ) }; }
    [[nodiscard]] std::string summary() const override
    {
      return "Server Landing Page";
    }
    [[nodiscard]] std::string description() const override
    {
      return "The landing page provides information about available projects and services.";
    }
    [[nodiscard]] std::string linkTitle() const override { return "Landing page"; }
    [[nodiscard]] QgsServerOgcApi::Rel linkType() const override { return QgsServerOgcApi::Rel::self; }
    [[nodiscard]] const QString templatePath( const QgsServerApiContext &context ) const override;

    /**
     *  Returns the path prefix, default is empty. Also makes sure that not-empty
     *  prefix starts with "/" (ex: "/mylandingprefix")
     */
    static QString prefix( const QgsServerSettings *settings );


  private:
    [[nodiscard]] json projectsData( const QgsServerRequest &request ) const;

    const QgsServerSettings *mSettings = nullptr;
};


/**
 * The QgsLandingPageMapHandler implements the landing page map handler (JSON only).
 */
class QgsLandingPageMapHandler : public QgsServerOgcApiHandler
{
  public:
    QgsLandingPageMapHandler( const QgsServerSettings *settings );

    void handleRequest( const QgsServerApiContext &context ) const override;

    // QgsServerOgcApiHandler interface
    [[nodiscard]] QRegularExpression path() const override;
    [[nodiscard]] std::string operationId() const override { return "getMap"; }
    [[nodiscard]] QStringList tags() const override { return { QStringLiteral( "Catalog" ), QStringLiteral( "Map Viewer" ) }; }
    [[nodiscard]] std::string summary() const override
    {
      return "Server Map Viewer";
    }
    [[nodiscard]] std::string description() const override
    {
      return "Shows a map";
    }
    [[nodiscard]] std::string linkTitle() const override { return "Map Viewer"; }
    [[nodiscard]] QgsServerOgcApi::Rel linkType() const override { return QgsServerOgcApi::Rel::self; }

  private:
    const QgsServerSettings *mSettings = nullptr;
};

#endif // QGS_LANDINGPAGE_HANDLERS_H
