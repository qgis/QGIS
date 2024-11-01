/***************************************************************************
  qgsserverstatichandler.h - QgsServerStaticHandler

 ---------------------
 begin                : 30.7.2020
 copyright            : (C) 2020 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSSERVERSTATICHANDLER_H
#define QGSSERVERSTATICHANDLER_H

#include "qgsserverogcapihandler.h"
#include "qgis_server.h"

/**
 * \ingroup server
 * \brief The QgsServerStaticHandler class serves static files from the static path (resources/server/api/wfs3/static)
 * \see QgsServerOgcApiHandler::staticPath()
 * \since QGIS 3.16
 */
class SERVER_EXPORT QgsServerStaticHandler : public QgsServerOgcApiHandler
{
  public:
    /**
     * Creates QgsServerStaticHandler
     * \param pathRegExp optional regular expression to capture static file names, defaults to "/static/(?<staticFilePath>.*)$",
     *        note that the file path is captured in a named group "staticFilePath"
     * \param staticPathSuffix optional path suffix to use when static files are stored in a subdirectory of the default staticPath()
     * \see QgsServerOgcApiHandler::staticPath()
     */
    QgsServerStaticHandler( const QString &pathRegExp = QStringLiteral( "/static/(?<staticFilePath>.*)$" ), const QString &staticPathSuffix = QString() );

    void handleRequest( const QgsServerApiContext &context ) const override;

    // QgsServerOgcApiHandler interface
    QRegularExpression path() const override { return mPathRegExp; }
    std::string operationId() const override { return "static"; }
    std::string summary() const override { return "Serves static files"; }
    std::string description() const override { return "Serves static files"; }
    std::string linkTitle() const override { return "Serves static files"; }
    QgsServerOgcApi::Rel linkType() const override { return QgsServerOgcApi::Rel::data; }

  private:
    QRegularExpression mPathRegExp;
    QString mStaticPathSuffix;
};


#endif // QGSSERVERSTATICHANDLER_H
