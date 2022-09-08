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
using namespace nlohmann;
#endif

class QgsServerApiContext;

/**
 * \ingroup server
 * \brief The QgsServerOgcApiHandler abstract class represents a OGC API handler to be registered
 * in QgsServerOgcApi class.
 *
 * Subclasses must override operational and informative methods and define
 * the core functionality in handleRequest() method.
 *
 * The following methods MUST be implemented:
 *
 * - path
 * - operationId
 * - summary  (shorter text)
 * - description (longer text)
 * - linkTitle
 * - linkType
 * - schema
 *
 * Optionally, override:
 *
 * - tags
 * - parameters
 * - contentTypes
 * - defaultContentType
 *
 * \code{.py}
 *
 * class Handler1(QgsServerOgcApiHandler):
 *   """Example handler"""
 *
 *   def path(self):
 *       return QtCore.QRegularExpression("/handlerone")
 *
 *   def operationId(self):
 *       return "handlerOne"
 *
 *   def summary(self):
 *       return "First of its name"
 *
 *   def description(self):
 *       return "The first handler ever"
 *
 *   def linkTitle(self):
 *       return "Handler One Link Title"
 *
 *   def linkType(self):
 *       return QgsServerOgcApi.data
 *
 *   def handleRequest(self, context):
 *       """Simple mirror: returns the parameters"""
 *
 *       params = self.values(context)
 *       self.write(params, context)
 *
 *   def parameters(self, context):
 *       return [QgsServerQueryStringParameter("value1", True, QgsServerQueryStringParameter.Type.Double, "a double value")]
 *
 *
 * \endcode
 *
 * \since QGIS 3.10
 */
class SERVER_EXPORT QgsServerOgcApiHandler
{

  public:

    virtual ~QgsServerOgcApiHandler();

    // /////////////////////////////////////////////
    // MAIN Section (operational)

    /**
     * URL pattern for this handler, named capture group are automatically
     * extracted and returned by values()
     *
     * Example: "/handlername/(?P<code1>\d{2})/items" will capture "code1" as a
     * named parameter.
     *
     * \see values()
     */
    virtual QRegularExpression path() const = 0;

    //! Returns the operation id for template file names and other internal references
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

    //! Tags
    virtual QStringList tags() const { return {}; }

    /**
     * Returns the default response content type in case the client did not specifically
     * ask for any particular content type.
     * The default implementation returns the first content type returned by
     * contentTypes() or JSON if that list is empty.
     */
    virtual QgsServerOgcApi::ContentType defaultContentType() const;

    /**
     * Returns the list of content types this handler can serve, default to JSON and HTML.
     * In case a specialized type (such as GEOJSON) is supported,
     * the generic type (such as JSON) should not be listed.
     * \note not available in Python bindings
     */
    QList<QgsServerOgcApi::ContentType> contentTypes() const SIP_SKIP;

    /**
     * Handles the request within its \a context
     *
     * Subclasses must implement this methods, and call validate() to
     * extract validated parameters from the request.
     *
     * \throws QgsServerApiBadRequestError if the method encounters any error
     */
    virtual void handleRequest( const QgsServerApiContext &context ) const SIP_THROW( QgsServerApiBadRequestException ) SIP_VIRTUALERRORHANDLER( serverapi_badrequest_exception_handler );

    /**
     * Analyzes the incoming request \a context and returns the validated
     * parameter map, throws QgsServerApiBadRequestError in case of errors.
     *
     * Path fragments from the named groups in the path() regular expression
     * are also added to the map.
     *
     * Your handleRequest method should call this function to retrieve
     * the parameters map.
     *
     * \returns the validated parameters map by extracting captured
     *          named parameters from the path (no validation is performed on
     *          the type because the regular expression can do it),
     *          and the query string parameters.
     *
     * \see path()
     * \see parameters()
     * \throws QgsServerApiBadRequestError if validation fails
     */
    virtual QVariantMap values( const QgsServerApiContext &context ) const SIP_THROW( QgsServerApiBadRequestException );

    /**
     * Looks for the first ContentType match in the accept header and returns its mime type,
     * returns an empty string if there are not matches.
     */
    QString contentTypeForAccept( const QString &accept ) const;

    // /////////////////////////////////////////////////////
    // Utility methods: override should not be required

#ifndef SIP_RUN  // Skip SIP

    /**
     * Writes \a data to the \a context response stream, content-type is calculated from the \a context request,
     * optional \a htmlMetadata for the HTML templates can be specified and will be added as "metadata" to
     * the HTML template variables.
     *
     * HTML output uses a template engine.
     *
     * Available template functions:
     * See: https://github.com/pantor/inja#tutorial
     *
     * Available custom template functions:
     *
     * - path_append( path ): appends a directory path to the current url
     * - path_chomp( n ): removes the specified number "n" of directory components from the current url path
     * - json_dump( ): prints current JSON data passed to the template
     * - static( path ): returns the full URL to the specified static path, for example:
     *   static( "/style/black.css" ) will return something like "/wfs3/static/style/black.css"
     * - links_filter( links, key, value ): returns filtered links from a link list
     * - content_type_name( content_type ): returns a short name from a content type for example "text/html" will return "HTML"
     * - nl2br( text ): returns the input text with all newlines replaced by "<br>" tags
     * - starts_with( string, prefix ): returns true if a string begins with the provided string prefix, false otherwise
     *
     * \note not available in Python bindings
     */
    void write( json &data, const QgsServerApiContext &context, const json &htmlMetadata = nullptr ) const;

    /**
     * Writes \a data to the \a context response stream as JSON
     * (indented if debug is active), an optional \a contentType can be specified.
     *
     * \note not available in Python bindings
     */
    void jsonDump( json &data, const QgsServerApiContext &context, const QString &contentType = QStringLiteral( "application/json" ) ) const;

    /**
     * Writes \a data as HTML to the response stream in \a context using a template.
     *
     * \see templatePath()
     * \note not available in Python bindings
     */
    void htmlDump( const json &data, const QgsServerApiContext &context ) const;

    /**
     * Returns handler information from the \a context for the OPENAPI description (id, description and other metadata) as JSON.
     * It may return a NULL JSON object in case the handler does not need to be included in the API.
     *
     * \note requires a valid project to be present in the context
     * \note not available in Python bindings
     */
    virtual json schema( const QgsServerApiContext &context ) const;

    /**
     * Builds and returns a link to the resource.
     *
     * \param context request context
     * \param linkType type of the link (rel attribute), default to SELF
     * \param contentType content type of the link (default to JSON)
     * \param title title of the link
     * \note not available in Python bindings
     */
    json link( const QgsServerApiContext &context,
               const QgsServerOgcApi::Rel &linkType = QgsServerOgcApi::Rel::self,
               const QgsServerOgcApi::ContentType contentType = QgsServerOgcApi::ContentType::JSON,
               const std::string &title = "" ) const;

    /**
     * Returns all the links for the given request \a context.
     *
     * The base implementation returns the alternate and self links, subclasses may
     * add other links.
     *
     * \note not available in Python bindings
     */
    json links( const QgsServerApiContext &context ) const;


    /**
     * Returns a vector layer instance from the "collectionId" parameter of the path in the given \a context,
     * requires a valid project instance in the context.
     *
     * \note not available in Python bindings
     *
     * \throws QgsServerApiNotFoundError if the layer could not be found
     * \throws QgsServerApiImproperlyConfiguredException if project is not set
     */
    QgsVectorLayer *layerFromContext( const QgsServerApiContext &context ) const;

#endif  // SIP skipped

    /**
     * Writes \a data to the \a context response stream, content-type is calculated from the \a context request,
     * optional \a htmlMetadata for the HTML templates can be specified and will be added as "metadata" to
     * the HTML template variables.
     *
     * HTML output uses a template engine.
     *
     * Available template functions:
     * See: https://github.com/pantor/inja#tutorial
     *
     * Available custom template functions:
     *
     * - path_append( path ): appends a directory path to the current url
     * - path_chomp( n ): removes the specified number "n" of directory components from the current url path
     * - json_dump( ): prints current JSON data passed to the template
     * - static( path ): returns the full URL to the specified static path, for example:
     *   static( "/style/black.css" ) will return something like "/wfs3/static/style/black.css"
     * - links_filter( links, key, value ): returns filtered links from a link list
     * - content_type_name( content_type ): returns a short name from a content type for example "text/html" will return "HTML"
     * - nl2br( text ): returns the input text with all newlines replaced by "<br>" tags
     * - starts_with( string, prefix ): returns true if a string begins with the provided string prefix, false otherwise
     *
     */
    void write( QVariant &data, const QgsServerApiContext &context, const QVariantMap &htmlMetadata = QVariantMap() ) const SIP_THROW( QgsServerApiBadRequestException );

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
     * The template path is calculated from QgsServerSettings's apiResourcesDirectory() as follow:
     * apiResourcesDirectory() + "/ogc/templates/" + context.apiRootPath + operationId + ".html"
     * e.g. for an API with root path "/wfs3" and an handler with operationId "collectionItems", the path
     * will be apiResourcesDirectory() + "/ogc/templates/wfs3/collectionItems.html"
     */
    virtual const QString templatePath( const QgsServerApiContext &context ) const;

    /**
     * Returns the absolute path to the base directory where static resources for
     * this handler are stored in the given \a context.
     *
     */
    virtual const QString staticPath( const QgsServerApiContext &context ) const;

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
    static QString parentLink( const QUrl &url, int levels = 1 );

    /**
     * Returns a vector layer from the \a collectionId in the given \a context.
     * \throws QgsServerApiNotFoundError if the layer could not be found.
     */
    static QgsVectorLayer *layerFromCollectionId( const QgsServerApiContext &context, const QString &collectionId );

    /**
     * Returns the defaultResponse as JSON
     *
     * \note not available in Python bindings
     */
    static json defaultResponse() SIP_SKIP;

    /**
     * Returns tags as JSON
     *
     * \see tags()
     *
     * \note not available in Python bindings
     */
    json jsonTags( ) const SIP_SKIP;

  protected:

    /**
     * Set the content types to \a contentTypes
     */
    void setContentTypesInt( const QList<int> &contentTypes ) SIP_PYNAME( setContentTypes );

    /**
     * Set the content types to \a contentTypes
     * \note not available in Python bindings
     */
    void setContentTypes( const QList<QgsServerOgcApi::ContentType> &contentTypes ) SIP_SKIP;

  private:

    //! List of content types this handler can serve, first is the default
    QList<QgsServerOgcApi::ContentType> mContentTypes = { QgsServerOgcApi::ContentType::JSON, QgsServerOgcApi::ContentType::HTML };


};

#endif // QGSSERVEROGCAPIHANDLER_H
