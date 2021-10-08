/***************************************************************************
  qgsserverogcapi.h - QgsServerOgcApi

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
#ifndef QGSSERVEROGCAPI_H
#define QGSSERVEROGCAPI_H

#include "qgsserverapi.h"
#include "qgis_server.h"


class QgsServerOgcApiHandler;

/**
 * \ingroup server
 * \brief QGIS Server OGC API endpoint. QgsServerOgcApi provides the foundation for
 * the new generation of REST-API based OGC services (e.g. WFS3).
 *
 * This class can be used directly and configured by registering handlers
 * as instances of QgsServerOgcApiHandler.
 *
 * \code{.py}
 *
 * class Handler1(QgsServerOgcApiHandler):
 *   """A handler, see QgsServerOgcApiHandler for an example"""
 *   ...
 *
 * h = Handler1()
 * api = QgsServerOgcApi(serverInterface(), "/api1", "apione", "A firs API", "1.0")
 * api.registerHandler(h)
 * server.serverInterface().serviceRegistry().registerApi(api)
 *
 * \endcode
 *
 * \since QGIS 3.10
 */
class SERVER_EXPORT QgsServerOgcApi : public QgsServerApi
{

    Q_GADGET

  public:

    // Note: non a scoped enum or qHash fails
    //! Rel link types
    enum Rel
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
      data //! The target IRI points to resource data
    };
    Q_ENUM( Rel )

    // Note: cannot be a scoped enum because qHash does not support them
    //! Media types used for content negotiation, insert more specific first
    enum ContentType
    {
      GEOJSON,
      OPENAPI3, //! "application/openapi+json;version=3.0"
      JSON,
      HTML,
      XML
    };
    Q_ENUM( ContentType )

    /**
     * QgsServerOgcApi constructor
     * \param serverIface pointer to the server interface
     * \param rootPath root path for this API (usually starts with a "/", e.g. "/wfs3")
     * \param name API name
     * \param description API description
     * \param version API version
     */
    QgsServerOgcApi( QgsServerInterface *serverIface,
                     const QString &rootPath,
                     const QString &name,
                     const QString &description = QString(),
                     const QString &version = QString() );

    // QgsServerApi interface
    const QString name() const override { return mName; }
    const QString description() const override { return mDescription; }
    const QString version() const override { return mVersion; }
    const QString rootPath() const override { return mRootPath ; }

    ~QgsServerOgcApi() override;

    /**
     * Executes a request by passing the given \a context to the API handlers.
     */
    virtual void executeRequest( const QgsServerApiContext &context ) const override SIP_THROW( QgsServerApiBadRequestException ) SIP_VIRTUALERRORHANDLER( serverapi_badrequest_exception_handler );

    /**
     * Returns a map of contentType => list of mime types
     * \note not available in Python bindings
     */
    static const QMap<QgsServerOgcApi::ContentType, QStringList> contentTypeMimes() SIP_SKIP;

    /**
     * Returns contentType specializations (e.g. JSON => [GEOJSON, OPENAPI3], XML => [GML])
     * \note not available in Python bindings
     */
    static const QHash<QgsServerOgcApi::ContentType, QList<QgsServerOgcApi::ContentType> > contentTypeAliases() SIP_SKIP;

    // Utilities
#ifndef SIP_RUN

    /**
     * Registers an OGC API handler passing \a Args to the constructor
     * \note not available in Python bindings
     */
    template<class T, typename... Args>
    void registerHandler( Args... args )
    {
      mHandlers.emplace_back( std::make_shared<T>( args... ) );
    }
#endif

    /**
     * Registers an OGC API \a handler, ownership of the handler is transferred to the API
     */
    void registerHandler( QgsServerOgcApiHandler *handler SIP_TRANSFER );

    /**
     * Returns a sanitized \a url with extra slashes removed and the path URL component that
     * always starts with a slash.
     */
    static QUrl sanitizeUrl( const QUrl &url );

    /**
     * Returns the string representation of \a rel attribute.
     */
    static std::string relToString( const QgsServerOgcApi::Rel &rel );

    /**
     * Returns the string representation of a \a ct (Content-Type) attribute.
     */
    static QString contentTypeToString( const QgsServerOgcApi::ContentType &ct );

    /**
     * Returns the string representation of a \a ct (Content-Type) attribute.
     */
    static std::string contentTypeToStdString( const QgsServerOgcApi::ContentType &ct );

    /**
     * Returns the file extension for a \a ct (Content-Type).
     */
    static QString contentTypeToExtension( const QgsServerOgcApi::ContentType &ct );

    /**
     * Returns the Content-Type value corresponding to \a extension.
     */
    static QgsServerOgcApi::ContentType contenTypeFromExtension( const std::string &extension );

    /**
     * Returns the mime-type for the \a contentType or an empty string if not found
     */
    static std::string mimeType( const QgsServerOgcApi::ContentType &contentType );

    /**
     * Returns registered handlers
     */
    const std::vector<std::shared_ptr<QgsServerOgcApiHandler> > handlers() const SIP_SKIP;

  private:

    QString mRootPath;
    QString mName;
    QString mDescription;
    QString mVersion;

    //Note: this cannot be unique because of SIP bindings
    std::vector<std::shared_ptr<QgsServerOgcApiHandler>> mHandlers;

    //! For each content type, stores a list of at least one content type mime strings aliases
    static QMap<QgsServerOgcApi::ContentType, QStringList> sContentTypeMime;

    /**
     * Stores content type generalization aliases (e.g. JSON->[GEOJSON,OPENAPI3], XML->[GML] )
     * We are good citizen and we accept JSON from the client (for example) if the actual type
     * is a JSON-based format (OPENAPI3 or GEOJSON).
     */
    static QHash<QgsServerOgcApi::ContentType, QList<QgsServerOgcApi::ContentType>> sContentTypeAliases;

};

#endif // QGSSERVEROGCAPI_H
