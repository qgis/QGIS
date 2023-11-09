/***************************************************************************
                          qgsserverrequest.h

  Define request class for getting request contents
  -------------------
  begin                : 2016-12-05
  copyright            : (C) 2016 by David Marteau
  email                : david dot marteau at 3liz dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSSERVERREQUEST_H
#define QGSSERVERREQUEST_H

#include <QUrl>
#include <QMap>
#include "qgis_server.h"
#include "qgsserverparameters.h"

/**
 * \ingroup server
 * \brief QgsServerRequest
 * Class defining request interface passed to services QgsService::executeRequest() method
 *
 * \since QGIS 3.0
 */

// Note about design: this interface must be passed along to Python and thus signatures methods must be
// compatible with pyQGIS/pyQT api and rules.

class SERVER_EXPORT QgsServerRequest
{
    Q_GADGET

  public:

    typedef QMap<QString, QString> Parameters;
    typedef QMap<QString, QString> Headers;

    /**
     * HTTP Method (or equivalent) used for the request
     */
    enum Method
    {
      HeadMethod,
      PutMethod,
      GetMethod,
      PostMethod,
      DeleteMethod,
      PatchMethod
    };
    Q_ENUM( Method )

    /**
     * The internal HTTP Header used for the request as enum
     */
    enum RequestHeader
    {
      // https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/Host
      HOST,
      // https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/Forwarded
      // https://tools.ietf.org/html/rfc7239
      FORWARDED,
      // https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/X-Forwarded-For
      X_FORWARDED_FOR,
      // https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/X-Forwarded-Host
      X_FORWARDED_HOST,
      // https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/X-Forwarded-Proto
      X_FORWARDED_PROTO,
      // The QGIS service URL
      X_QGIS_SERVICE_URL,
      // The QGIS WMS service URL
      X_QGIS_WMS_SERVICE_URL,
      // The QGIS WFS service URL
      X_QGIS_WFS_SERVICE_URL,
      // The QGIS WCS service URL
      X_QGIS_WCS_SERVICE_URL,
      // The QGIS WMTS service URL
      X_QGIS_WMTS_SERVICE_URL,
      // https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/Accept
      ACCEPT,
      // https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/User-Agent
      USER_AGENT,
      // https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/Authorization
      AUTHORIZATION,
    };
    Q_ENUM( RequestHeader )

    /**
     * Constructor
     */
    QgsServerRequest() = default;

    /**
     * Constructor
     *
     * \param url the url string
     * \param method the request method
     * \param headers
     */
    QgsServerRequest( const QString &url, QgsServerRequest::Method method = QgsServerRequest::GetMethod, const QgsServerRequest::Headers &headers = QgsServerRequest::Headers() );

    /**
     * Constructor
     *
     * \param url QUrl
     * \param method the request method
     * \param headers
     */
    QgsServerRequest( const QUrl &url, QgsServerRequest::Method method = QgsServerRequest::GetMethod, const QgsServerRequest::Headers &headers = QgsServerRequest::Headers() );

    /**
     * Copy constructor.
     */
    QgsServerRequest( const QgsServerRequest &other );

    QgsServerRequest &operator=( const QgsServerRequest & ) = default;

    //! destructor
    virtual ~QgsServerRequest() = default;

    /**
     * Returns a string representation of an HTTP request \a method.
     * \since QGIS 3.12
     */
    static QString methodToString( const Method &method );


    /**
     * \returns  the request url as seen by QGIS server
     *
     * \see originalUrl for the unrewritten url as seen by the web
     *      server, by default the two are equal
     */
    QUrl url() const;

    /**
     * \returns the request method
      */
    QgsServerRequest::Method method() const;

    /**
     * Returns a map of query parameters with keys converted
     * to uppercase
     */
    QgsServerRequest::Parameters parameters() const;

    /**
     * Returns parameters
     */
    QgsServerParameters serverParameters() const;

    /**
     * Set a parameter
     */
    virtual void setParameter( const QString &key, const QString &value );

    /**
     * Gets a parameter value
     */
    QString parameter( const QString &key, const QString &defaultValue = QString() ) const;

    /**
     * Remove a parameter
     */
    virtual void removeParameter( const QString &key );

    /**
     * Returns the header value
     * \param name of the header
     * \return the header value or an empty string
     */
    virtual QString header( const QString &name ) const;

    /**
     * Returns the header value
     * \param headerEnum of the header
     * \return the header value or an empty string
     */
    virtual QString header( const RequestHeader &headerEnum ) const;

    /**
     * Set an header
     * \param name
     * \param value
     */
    void setHeader( const QString &name, const QString &value );

    /**
     * Returns the header map
     * \return the headers map
     */
    QMap<QString, QString> headers() const;

    /**
     * Remove an header
     * \param name
     * \since QGIS 3.20
     */
    void removeHeader( const QString &name );

    /**
     * Returns post/put data
     * Check for QByteArray::isNull() to check if data
     * is available.
     */
    virtual QByteArray data() const;

    /**
     * Set the request url
     */
    virtual void setUrl( const QUrl &url );

    /**
     * Returns the request url as seen by the web server,
     * by default this is equal to the url seen by QGIS server
     *
     * \see url() for the rewritten url
     * \since QGIS 3.6
     */
    QUrl originalUrl() const;

    /**
     * Returns the base URL of QGIS server
     *
     * E.g. if we call QGIS server with 'http://example.com/folder?REQUEST=WMS&...'
     * the base URL will be 'http://example.com/folder'
     *
     * \since QGIS 3.20
     */
    QUrl baseUrl() const;

    /**
     * Set the request method
     */
    void setMethod( QgsServerRequest::Method method );

    /**
     * Returns the query string parameter with the given \a name from the request URL, a \a defaultValue can be specified.
     * \since QGIS 3.10
     */
    const QString queryParameter( const QString &name, const QString &defaultValue = QString( ) ) const;

  protected:

    /**
     * Set the request original \a url (the request url as seen by the web server)
     *
     * \see setUrl() for the rewritten url
     * \since QGIS 3.6
     */
    void setOriginalUrl( const QUrl &url );

    /**
     * Set the base URL of QGIS server
     *
     * \since QGIS 3.20
     */
    void setBaseUrl( const QUrl &url );

  private:
    // Url as seen by QGIS server after web server rewrite
    QUrl       mUrl;
    // Unrewritten url as seen by the web server
    QUrl       mOriginalUrl;
    QUrl       mBaseUrl;
    Method     mMethod = GetMethod;
    // We mark as mutable in order
    // to support lazy initialization
    mutable Headers mHeaders;
    QgsServerParameters mParams;
};

#endif
