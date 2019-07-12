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

#include <QRegularExpression>
#include "qgis_server.h"
#include "qgsserverquerystringparameter.h"
#include "qgsserverogcapi.h"
#include "nlohmann/json_fwd.hpp"
#include "inja/inja.hpp"

#ifndef SIP_RUN
using json = nlohmann::json;
#endif

class QgsServerApiContext;

/**
 * \ingroup server
 * The QgsServerOgcApiHandler abstract class represents a OGC API handler to be used in QgsServerOgcApi class.
 *
 * Subclasses must override operational and informative methods and define
 * the core functionality in handleRequest() method.
 *
 * The following methods also MUST be implemented:
 * - path
 * - operationId
 * - summary
 * - description
 * - linkTitle
 * - linkType
 *
 * Optionally, override:
 * - parameters
 * - contentTypes
 * - defaultContentType
 *
 *
 * \note not available in Python bindings
 * \since QGIS 3.10
 */
class SERVER_EXPORT QgsServerOgcApiHandler
{

  public:

    QgsServerOgcApiHandler() = default;

    virtual ~QgsServerOgcApiHandler();

    // /////////////////////////////////////////////
    // MAIN Section (operational)

    /**
     * URL pattern for this handler, named capture group are supported as
     * well as positional capture groups.
     *
     * Example: "/handlername/(?P<code1>\d{2})/(\d{3})" will capture "code1" as a
     * named parameter and the second group as a positional parameter with index 0.
     *
     * \see validate()
     */
    virtual QRegularExpression path() const = 0;

    //! Operation id for template file names and other internal references
    virtual std::string operationId() const = 0;

    /**
     * Returns a list of query string parameters.
     *
     * Depending on the handler, it may be dynamic (per-request) or static.
     * \param context the request context
     */
    virtual QList<QgsServerQueryStringParameter> parameters( const QgsServerApiContext &context ) const  { Q_UNUSED( context ); return { }; }

    // /////////////////////////////////////////////
    // METADATA Sections (informative)

    //! Summary
    virtual std::string summary() const = 0;

    //! Description
    virtual std::string description() const = 0;

    //! Title for the handler link
    virtual std::string linkTitle() const = 0;

    //! Main role for the resource link
    virtual QgsServerOgcApi::Rel linkType() const = 0;

    /**
     * Default response content type in case the client did not specifically
     * ask for any particular content type.
     */
    virtual QgsServerOgcApi::ContentType defaultContentType() const  { return QgsServerOgcApi::ContentType::JSON; }

    /**
     * List of content types this handler can serve, default to JSON and HTML.
     * In case a specialized type (such as GEOJSON) is supported,
     * the generic type (such as JSON) should not be listed.
     */
    virtual QList<QgsServerOgcApi::ContentType> contentTypes() const { return  { QgsServerOgcApi::ContentType::JSON, QgsServerOgcApi::ContentType::HTML }; }

    /**
     * Handles the request within its \a context
     *
     * Subclasses must implement this methods, and call validate() to
     * extract validated parameters from the request.
     *
     * \throws QgsServerApiBadRequestError if the method encounters any error
     */
    virtual void handleRequest( const QgsServerApiContext &context ) const = 0;

    /**
     * Validates the request within its \a context and returns the
     * parameter map.
     *
     * Your handleRequest method should call this function to retrieve
     * the parameters map.
     *
     * Path fragments are returned in the string list "path_arguments", if the
     * path() regular expression contains named capture groups, the
     * corresponding parameters are also added to the map.
     *
     * \returns the validated parameters map by extracting captured
     *          parameters from the path (no validation is performed on
     *          the type because the regurlar expression can do it),
     *          and the query string parameters.
     *
     * \see path()
     * \see parameters()
     * \throws QgsServerApiBadRequestError if validation fails
     */
    virtual QVariantMap validate( const QgsServerApiContext &context ) const SIP_THROW( QgsServerApiBadRequestException );

    /**
     * Looks for the first contentType match in accept and returns its mime type,
     * returns an empty string if there are not matches.
     */
    QString contentTypeForAccept( const QString &accept ) const;

    // /////////////////////////////////////////////////////
    // Utility methods: override should not be required

#ifndef SIP_RUN

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
     * \note not available in Python bindings
     */
    void write( json &data, const QgsServerApiContext &context, const json &htmlMetadata = nullptr ) const;

    /**
     * Writes \a data to the \a context response stream as JSON
     * (indented if debug is active), an optional \a contentType can be specified.
     * \note not available in Python bindings
     */
    void jsonDump( json &data, const QgsServerApiContext &context, const QString &contentType = QStringLiteral( "application/json" ) ) const;

    /**
     * Writes \a data to the \a response stream as HTML (indented if debug is active) using a template.
     *
     * \param api parent Api instance
     * \param context the API request context object
     * \see templatePath()
     * \note not available in Python bindings
     */
    void htmlDump( const json &data, const QgsServerApiContext &context ) const;

    /**
     * Returns handler information (id, description and other metadata) as JSON.
     * \note not available in Python bindings
     */
    json handlerData( ) const;

    /**
     * Writes \a data to the \a response stream as XML (indented if debug is active).
     * \note not available in Python bindings
     */
    void xmlDump( const json &data, QgsServerResponse *response ) const;

    /**
     * Utility method that builds and returns a link to the resource.
     *
     * \param context request context
     * \param linkType type of the link (rel attribute), default to "self"
     * \param contentType, default to objects's linkType
     * \param title default to "This documents as <content type>"
     * \note not available in Python bindings
     */
    json link( const QgsServerApiContext &context,
               const QgsServerOgcApi::Rel &linkType = QgsServerOgcApi::Rel::self,
               const QgsServerOgcApi::ContentType contentType = QgsServerOgcApi::ContentType::JSON,
               const std::string &title = "" ) const;

    /**
     * Returns all the links for the given request \a api and \a context.
     *
     * The base implementation returns the alternate and self links, subclasses may
     * add other links.
     */
    json links( const QgsServerApiContext &context ) const;


    /**
     * Returns a vector layer instance from the "collectionId" parameter of the path in the given \a context,
     * requires a valid project instance in the context.
     *
     * \throws QgsServerApiNotFoundError if the layer could not be found
     * \throws QgsServerApiImproperlyConfiguredException if project is not set
     */
    QgsVectorLayer *layerFromContext( const QgsServerApiContext &context ) const;


#endif

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
    void write( QVariant &data, const QgsServerApiContext &context, const QVariantMap &htmlMetadata = QVariantMap() ) const;

    /**
     * Returns an URL to self, to be used for links to the current resources and as a base for constructing links to sub-resources
     *
     * \param context the current request context
     * \param extraPath an optional extra path that will be appended to the calculated URL
     * \param extension optional file extension to add (the dot will be added automatically).
     */
    std::string href( const QgsServerApiContext &context, const QString &extraPath = QString(), const QString &extension = QString() ) const;

    /**
     * Returns the HTML template path for the handler in the given \a context
     *
     * The template path is calculated as follow:
     * QgsServerOgcApi::resourcesPath() + "/ogc/templates/" + context.apiRootPath + operationId + ".html"
     * e.g. for an API with root path "/wfs3" and an handler with operationId "collectionItems", the path
     * will be QgsServerOgcApi::resourcesPath() + "/ogc/templates/wfs3/collectionItems.html"
     * \see QgsServerOgcApi::resourcesPath()
     */
    const QString templatePath( const QgsServerApiContext &context ) const;

    /**
     * Returns the absolute path to the base directory where static resources for
     * this handler are stored.
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
    QgsServerOgcApi::ContentType contentTypeFromRequest( const QgsServerRequest *request ) const;

    /**
     * Returns a link to the parent page up to \a levels in the HTML hierarchy from the given \a url, MAP query argument is preserved
     */
    static std::string parentLink( const QUrl &url, int levels = 1 );

    static QgsVectorLayer *layerFromCollection( const QgsServerApiContext &context, const QString &collectionId );


  private:


};

#endif // QGSSERVEROGCAPIHANDLER_H
