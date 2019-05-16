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

  //! Media types used for content negotiation, insert more specific first
  enum contentTypes
  {
    GEOJSON,
    OPENAPI3, //! "application/openapi+json;version=3.0"
    JSON,
    HTML,
    GML,
    XML
  };
  Q_ENUM_NS( contentTypes )


  //! Stores content type strings
  extern QMap<contentTypes, QString> sContentTypeMime;

  /**
   * Looks for the first contentType match in accept and returns it's mime type,
   * returns an empty string if there are not matches.
   */
  QString contentTypeForAccept( const QString &accept );

  //! Generic endpoint handler for API, this class could be eventually factored out to QgsServerApi
  struct Handler
  {

    virtual ~Handler() = default;
    virtual void handleRequest( const Api *api, QgsServerApiContext *context ) const;

    /**
     * Returns an URL to be used for links in the output.
     * \param api the parent API instance
     * \param request the current request object
     * \param extraPath an optional extra path that will be appended to the calculated URL
     * \param extension optional file extension to add (the dot will be added automatically), note that JSON extension will not be added because it's the default.
     */
    std::string href( const Api *api, const QgsServerRequest *request, const QString &extraPath = QString(), const QString &extension = QString() ) const;

    QRegularExpression path;
    std::string operationId;
    std::string summary;
    std::string description;
    std::string linkTitle;
    rel linkType;

    //! Defines a root link for the landing page (e.g. "collections" or "api"), if it is empty the link will not be included in the landing page
    QString landingPageRootLink = "";

    //! Default response content type in case the client did not specify it
    contentTypes mimeType = contentTypes::JSON;

    /**
     * Writes \a data to the \a response stream, contentType it is calculated from the \a request
     * \note use xmlDump for XML output
     * \see xmlDump()
     */
    void write( const json &data, const QgsServerRequest *request, QgsServerResponse *response ) const;

    /**
     * Writes \a data to the \a response stream as JSON (indented if debug is active), an optional \a contentType can be specified.
     */
    void jsonDump( const json &data, QgsServerResponse *response, const QString &contentType = QStringLiteral( "application/json" ) ) const;

    /**
     * Writes \a data to the \a response stream as HTML (indented if debug is active) using a template
     * \see templatePath()
     */
    void htmlDump( const json &data, QgsServerResponse *response ) const;

    /**
     * Writes \a data to the \a response stream as XML (indented if debug is active)
     */
    void xmlDump( const json &data, QgsServerResponse *response ) const;

    /**
     * Returns the HTML template path for the handler
     */
    const QString templatePath() const;

    /**
     * Returns the absolute path to base directory where resources for this handler are stored
     * TODO: make this path configurable by env and/or settings
     */
    const QString resourcesPath() const;

    /**
     * Returns the absolute path to the directory where static resources for this handler are stored
     * TODO: make this path configurable by env and/or settings
     */
    const QString staticPath() const;

    /**
     * Returns the requested content type from the \a request
     *
     * "Accept" header is examined first, if it's not set the path extension
     * will be checked for known mime types, fallback to the default content type
     * of the handler if none of the above matches.
     */
    contentTypes contentTypeFromRequest( const QgsServerRequest *request ) const;

    /**
     * Returns the layer identified by \a collectionId from the given \a context
     *
     * \throws QgsServerApiImproperlyConfiguredError if not found or if more than one layer with same \a collectionId was found
     */
    QgsVectorLayer *layerFromCollection( QgsServerApiContext *context, const QString &collectionId ) const;

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

      const QString name() const override { return QStringLiteral( "WFS3" ); }
      const QString version() const override { return QStringLiteral( "1.0.0" ); }
      const QString rootPath() const override { return QStringLiteral( "/wfs3" ); }

      // Utilities
#ifndef SIP_RUN
      template<class T>
      void registerHandler();
#endif

      /**
       * Returns a copy of the \a url normalized (trailing and double slashes removed)
       */
      static QUrl normalizedUrl( QUrl url );

      const std::vector<std::unique_ptr<Handler>> &handlers() const;

      static std::string relToString( const QgsWfs3::rel &rel );

      static QString contentTypeToString( const contentTypes &ct );

      static std::string contentTypeToStdString( const contentTypes &ct );

      static QString contentTypeToExtension( const contentTypes &ct );


    private:

      QgsServerInterface *mServerIface = nullptr;

      std::vector<std::unique_ptr<Handler>> mHandlers;

      // QgsServerApi interface
    public:

      void executeRequest( QgsServerApiContext *context ) const override;

  };


} // namespace QgsWfs3

#endif // QGSWFS3API_H
