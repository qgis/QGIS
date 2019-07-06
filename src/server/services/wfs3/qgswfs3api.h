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
    // The following registered link relation types are used
    alternate, //! Refers to a substitute for this context.
    describedBy, //! Refers to a resource providing information about the link’s context.
    collection, //! The target IRI points to a resource that is a member of the collection represented by the context IRI.
    item, //! The target IRI points to a resource that is a member of the collection represented by the context IRI.
    self, //! Conveys an identifier for the link’s context.
    service_desc, //! Identifies service description for the context that is primarily intended for consumption by machines.
    service_doc, //! Identifies service documentation for the context that is primarily intended for human consumption.
    prev, //! Indicates that the link’s context is a part of a series, and that the previous in the series is the link targe
    next, //! Indicates that the link’s context is a part of a series, and that the next in the series is the link target.
    license, //! Refers to a license associated with this context.
    // In addition the following link relation types are used for which no applicable registered link relation type could be identified:
    items, //! Refers to a resource that is comprised of members of the collection represented by the link’s context.
    conformance, //! The target IRI points to a resource which represents the collection resource for the context IRI.
    data,
  };
  Q_ENUM_NS( rel )

  //! Media types used for content negotiation, insert more specific first
  enum contentType
  {
    GEOJSON,
    OPENAPI3, //! "application/openapi+json;version=3.0"
    JSON,
    HTML,
    GML,
    XML
  };
  Q_ENUM_NS( contentType )

  //! Stores content type strings
  extern QMap<contentType, QString> sContentTypeMime;

  //! Stores content type aliases (e.g. JSON->[GEOJSON,OPENAPI3], XML->[GML] )
  extern QHash<contentType, QList<contentType>> sContentTypeAliases;

  /**
   * Looks for the first contentType match in accept and returns it's mime type,
   * returns an empty string if there are not matches.
   */
  QString contentTypeForAccept( const QString &accept );

  //! Generic endpoint handler for API, part of this class could be eventually factored out to QgsServerApi
  struct Handler
  {

    virtual ~Handler() = default;
    virtual void handleRequest( const Api *api, const QgsServerApiContext &context ) const;

    //! URL pattern for this handler
    QRegularExpression path;
    //! Operation id for template file names and other internal references
    std::string operationId;
    //! Summary
    std::string summary;
    //! Description
    std::string description;
    //! Title for the handler link
    std::string linkTitle;
    //! Main role for the resource link
    rel linkType;
    // TODO: add XML?

    /**
     * List of content types this handler can serve, default to JSON and HTML.
     * In case a specialized type (such as GEOJSON) is supported, the generic type (such as JSON) should not be listed.
     */
    QList<contentType> contentTypes { contentType::JSON, contentType::HTML };

    /**
     * Returns an URL to be used for links in the output.
     *
     * \param api the parent API instance
     * \param request the current request object
     * \param extraPath an optional extra path that will be appended to the calculated URL
     * \param extension optional file extension to add (the dot will be added automatically), note that JSON extension will not be added because it's the default.
     */
    std::string href( const Api *api, const QgsServerRequest *request, const QString &extraPath = QString(), const QString &extension = QString() ) const;

    /**
     * Utility method that builds and returns a link to the resource.
     *
     * \param api the parent API instance
     * \param context request context
     * \param linkType type of the link (rel attribute), default to "self"
     * \param contentType, default to objects's linkType
     * \param title default to "This documents as <content type>"
     */
    json link( const Api *api,
               const QgsServerApiContext &context,
               const rel &linkType = rel::self,
               const contentType contentType = contentType::JSON,
               const std::string &title = "" ) const;

    /**
     * Returns all the links for the given request \a api and \a context.
     *
     * The base implementation returns the alternate and self links, subclasses may
     * add other links.
     */
    json links( const Api *api,
                const QgsServerApiContext &context ) const;


    //! Defines a root link for the landing page (e.g. "collections" or "api"), if it is empty the link will not be included in the landing page.
    QString landingPageRootLink;

    //! Default response content type in case the client did not specify it.
    contentType defaultContentType = contentType::JSON;

    /**
     * Writes \a data to the \a context response stream, content-type it is calculated from the \a context request,
     * optional \a metadata for the HTML templates can be specified and will be added as "metadata" to
     * the HTML template variables.
     *
     * HTML output uses a template engine.
     *
     * Available template functions:
     * See: https://github.com/pantor/inja#tutorial
     *
     * Available custom template functions:
     * - path_append(<path>): appends a directory <path> to the current url
     * - path_chomp(n):removes the specified number "n" of directory components from the current url path
     * - json_dump(): prints current JSON data passed to the template
     * - static(<path>): returns the full URL to the specified static <path>, for example:
     *   static("/style/black.css") will return something like "/wfs3/static/style/black.css".
     * - links_filter( <links>, <key>, <value> ): eturns filtered links from a link list
     * - content_type_name( <content_type> ): Returns a short name from a content type for example "text/html" will return "HTML"
     *
     * \note use xmlDump for XML output
     * \see xmlDump()
     */
    void write( json &data, const Api *api, const QgsServerApiContext &context, const json &metadata = nullptr ) const;

    /**
     * Writes \a data to the \a response stream as JSON (indented if debug is active), an optional \a contentType can be specified.
     */
    void jsonDump( json &data, QgsServerResponse *response, const QString &contentType = QStringLiteral( "application/json" ) ) const;

    /**
     * Writes \a data to the \a response stream as HTML (indented if debug is active) using a template.
     *
     * \param api parent Api instance
     * \param response the response object
     * \see templatePath()
     */
    void htmlDump( const json &data, const Api *api, const QgsServerRequest *request, QgsServerResponse *response ) const;

    /**
     * Returns handler information (id, description and other metadata) as JSON.
     */
    json handlerData( ) const;

    /**
     * Writes \a data to the \a response stream as XML (indented if debug is active).
     */
    void xmlDump( const json &data, QgsServerResponse *response ) const;

    /**
     * Returns the HTML template path for the handler
     */
    const QString templatePath() const;

    /**
     * Returns the absolute path to base directory where resources for this handler are stored.
     *
     * TODO: make this path configurable by env and/or settings
     */
    const QString resourcesPath() const;

    /**
     * Returns the absolute path to the directory where static resources for this handler are stored.
     *
     * TODO: make this path configurable by env and/or settings
     */
    const QString staticPath() const;

    /**
     * Returns the content type from the \a request.
     *
     * The path file extension is examined first and checked for known mime types,
     * the "Accept" HTTP header is examined next.
     * Fallback to the default content type of the handler if none of the above matches.
     *
     * \throws QgsServerApiBadRequestError if the content type of the request is not compatible with the handler (\see contentTypes member)
     */
    contentType contentTypeFromRequest( const QgsServerRequest *request ) const;

    /**
     * Returns the layer identified by \a collectionId from the given \a context.
     *
     * \throws QgsServerApiImproperlyConfiguredError if not found or if more than one layer with same \a collectionId was found
     */
    QgsVectorLayer *layerFromCollection( const QgsServerApiContext &context, const QString &collectionId ) const;

  };

  /**
   * \ingroup server
   * \class QgsWfs3::Api
   * \brief OGC web service for WFS3
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
      const QString description() const override { return QStringLiteral( "Implementation of the OGC WFS3 Draft" ); }
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

      /**
       * Returns a link to the parent page up to \a levels in the HTML hierarchy from the given \a url, MAP query argument is preserved
       */
      static std::string parentLink( const QUrl &url, int levels = 1 );

      /**
       * Returns a list of registered handlers.
       */
      const std::vector<std::unique_ptr<Handler>> &handlers() const;

      /**
       * Returns the string representation of \a rel attribute.
       */
      static std::string relToString( const QgsWfs3::rel &rel );

      /**
       * Returns the string representation of a \a ct (Content-Type) attribute.
       */
      static QString contentTypeToString( const contentType &ct );

      /**
       * Returns the string representation of a \a ct (Content-Type) attribute.
       */
      static std::string contentTypeToStdString( const contentType &ct );

      /**
       * Returns the file extension for a \a ct (Content-Type).
       */
      static QString contentTypeToExtension( const contentType &ct );

      /**
       * Returns the Content-Type value corresponding to \a extension.
       */
      static contentType contenTypeFromExtension( const std::string &extension );

      /**
       * Returns the mime-type for the \a contentType or an empty string if not found
       */
      static std::string mimeType( const contentType &contentType );

    private:

      std::vector<std::unique_ptr<Handler>> mHandlers;

      // QgsServerApi interface
    public:

      /**
       * Executes a request in the given \a context
       *
       * \throws QgsServerApiBadRequestError if the handler does not match the request
       */
      void executeRequest( const QgsServerApiContext &context ) const override;

  };


} // namespace QgsWfs3

#endif // QGSWFS3API_H
