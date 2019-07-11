/***************************************************************************
  qgsserverogcapihandler.h - QgsServerOgcApiHandler

 ---------------------
 begin                : 10.7.2019
 copyright            : (C) 2019 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSSERVEROGCAPIHANDLER_H
#define QGSSERVEROGCAPIHANDLER_H

#define SIP_NO_FILE

#include <QRegularExpression>
#include "qgis_server.h"
#include "qgsserverquerystringparameter.h"
#include "nlohmann/json_fwd.hpp"
#include "inja/inja.hpp"

using json = nlohmann::json;

class QgsServerApiContext;

/**
 * \ingroup server
 * The QgsServerOgcApiHandler abstract class represents a OGC API handler to be used in QgsServerOgcApi class.
 *
 * Subclasses must override operational and informative methods and define
 * the core functionality in handleRequest() method.
 * \note not available in Python bindings
 * \since QGIS 3.10
 */
class SERVER_EXPORT QgsServerOgcApiHandler
{
    Q_GADGET

  public:

    //! Rel link types
    enum class rel
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
    Q_ENUM( rel )

    //! Media types used for content negotiation, insert more specific first
    enum class contentType
    {
      GEOJSON,
      OPENAPI3, //! "application/openapi+json;version=3.0"
      JSON,
      HTML,
      GML,
      XML
    };
    Q_ENUM( contentType )

    QgsServerOgcApiHandler() = default;

    virtual ~QgsServerOgcApiHandler() = default;

    // /////////////////////////////////////////////
    // MAIN Section (operational)

    //! URL pattern for this handler
    virtual QRegularExpression path() const = 0;

    //! Operation id for template file names and other internal references
    virtual std::string operationId() const = 0;

    /**
     * Returns a list of query string parameters.
     * Depending on the service, it may be dynamic (per-request) or static.
     */
    virtual QList<QgsServerQueryStringParameter> parameters() const  { return { }; }

    // /////////////////////////////////////////////
    // METADATA Sections (informative)

    //! Summary
    virtual std::string summary() const = 0;

    //! Description
    virtual std::string description() const = 0;

    //! Title for the handler link
    virtual std::string linkTitle() const = 0;

    //! Main role for the resource link
    virtual rel linkType() const = 0;

    /**
     * Default response content type in case the client did not specifically
     * ask for any particular content type.
     */
    virtual contentType defaultContentType() const  { return contentType::JSON; }

    /**
     * List of content types this handler can serve, default to JSON and HTML.
     * In case a specialized type (such as GEOJSON) is supported,
     * the generic type (such as JSON) should not be listed.
     */
    virtual QList<contentType> contentTypes() const { return  { contentType::JSON, contentType::HTML }; }

    /**
     * Handles the request within it's \a context
     */
    virtual void handleRequest( const QgsServerApiContext &context ) const = 0;

    /**
     * Validates the request within it's \a context
     *
     * \returns the validated parameters map
     * \throws QgsServerApiBadRequestError if validation fails
     */
    virtual QVariantMap validate( const QgsServerApiContext &context );

    /**
     * Looks for the first contentType match in accept and returns it's mime type,
     * returns an empty string if there are not matches.
     */
    QString contentTypeForAccept( const QString &accept );

    // /////////////////////////////////////////////////////
    // Utility methods: override should not be required

    /**
     * Writes \a data to the \a context response stream, content-type it is calculated from the \a context request,
     * optional \a htmlMetadata for the HTML templates can be specified and will be added as "metadata" to
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
    void write( json &data, const QgsServerApiContext &context, const json &htmlMetadata = nullptr ) const;

    /**
     * Writes \a data to the \a response stream as JSON (indented if debug is active), an optional \a contentType can be specified.
     */
    void jsonDump( json &data, QgsServerResponse *response, const QString &contentType = QStringLiteral( "application/json" ) ) const;

    /**
     * Writes \a data to the \a response stream as HTML (indented if debug is active) using a template.
     *
     * \param api parent Api instance
     * \param request the request object
     * \see templatePath()
     */
    void htmlDump( const json &data, const QgsServerRequest *request, QgsServerResponse *response ) const;

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


  private:

    //! Stores content type mime strings
    static QMap<contentType, QString> sContentTypeMime;

    //! Stores content type aliases (e.g. JSON->[GEOJSON,OPENAPI3], XML->[GML] )
    static QHash<contentType, QList<contentType>> sContentTypeAliases;

};

#endif // QGSSERVEROGCAPIHANDLER_H
