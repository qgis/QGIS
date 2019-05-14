/***************************************************************************
                              qgswfs3api.h
                              -------------------------
  begin                : April 15, 2019
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

#ifndef QGSWFS3API_H
#define QGSWFS3API_H

#include "qgsmodule.h"
#include "qgsproject.h"
#include "qgsserverexception.h"

#include "nlohmann/json_fwd.hpp"
#include "inja/inja.hpp"

using json = nlohmann::json;

class QgsServerApiContext;

namespace QgsWfs3
{

  class Api;

  Q_NAMESPACE



  //! Rel link types
  enum rel
  {
    alternate,
    conformance,
    data,
    describedBy,
    item,
    self,
    service
  };
  Q_ENUM_NS( rel )

  //! Content types used for content negotiation based on requested file extension
  enum contentType
  {
    JSON,
    HTML,
    XML,
    GML
  };
  Q_ENUM_NS( contentType )


  //! Generic endpoint handler for API, this class could be eventually factored out to QgsServerApi
  struct Handler
  {

    virtual ~Handler() = default;
    virtual void handleRequest( const Api *api, QgsServerApiContext *context ) const;
    std::string href( const Api *api, const QgsServerRequest &request, const QString &extraPath = QString(), const QString &extension = QString() ) const;

    QRegularExpression path;
    std::string operationId;
    std::string summary;
    std::string description;
    std::string linkTitle;
    rel linkType;
    std::string mimeType;

    /**
     * Writes \a data to the \a response stream as JSON or HTML with an optional \a contentType (by default it is infered from the \a request)
     * \note use xmlDump for XML output
     * \see xmlDump()
     */
    void write( const json &data, const QgsServerRequest &request, QgsServerResponse &response, const QString &contentType = "" ) const;

    /**
     * Utility that writes JSON output to the response stream (indented if debug is active)
     * \param data json data
     * \param response
     */
    void jsonDump( const json &data, QgsServerResponse &response, const QString &contentType = QStringLiteral( "application/json" ) ) const;

    /**
     * Utility that writes HTML output to the response stream using the default template
     * \param data json data
     * \param response
     */
    void htmlDump( const json &data, QgsServerResponse &response ) const;

    /**
     * Utility that writes JSON output to the response stream (indented if debug is active)
     * \param data json data
     * \param response
     */
    void xmlDump( const json &data, QgsServerResponse &response ) const;

    /**
     * Returns the HTML template path for the handler
     */
    const QString templatePath() const;

  };

  /**
   * \ingroup server
   * \class QgsWfs3::Api
   * \brief OGC web service specialized for WFS3
   * \since QGIS 3.10
   */
  class Api: public QgsServerApi
  {
    public:

      /**
       * Constructor for WFS service.
       * \param serverIface Interface for plugins.
       */
      Api( QgsServerInterface *serverIface );

      const QString name()    const override { return QStringLiteral( "WFS3" ); }
      const QString version() const override { return QStringLiteral( "1.0.0" ); }
      const QString rootPath() const override { return QStringLiteral( "/wfs3" ); }

      // Utilities
#ifndef SIP_RUN
      template<class T>
      void registerHandler();
#endif

      /**
       * Returns a copy of the \a url with the path without an ending slash
       */
      QUrl normalizedUrl( QUrl url ) const;

      /**
       * Returns the \a url path extension to upper case (if any)
       */
      static std::string extension( QUrl url );

      const std::vector<std::unique_ptr<Handler>> &handlers() const;

      static std::string relToString( const QgsWfs3::rel &rel );

      static QString contentTypeToString( const contentType &ct );

      static std::string contentTypeToStdString( const contentType &ct );

      static QString contentTypeToExtension( const contentType &ct );

    private:

      QgsServerInterface *mServerIface = nullptr;

      std::vector<std::unique_ptr<Handler>> mHandlers;

      // QgsServerApi interface
    public:

      void executeRequest( QgsServerApiContext *context ) const override;

  };


} // namespace QgsWfs3

#endif // QGSWFS3API_H
