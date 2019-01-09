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
 * QgsServerRequest
 * Class defining request interface passed to services QgsService::executeRequest() method
 *
 * \since QGIS 3.0
 */

// Note about design: this interface must be passed along to Python and thus signatures methods must be
// compatible with pyQGIS/pyQT api and rules.

class SERVER_EXPORT QgsServerRequest
{
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
      DeleteMethod
    };


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

    //! destructor
    virtual ~QgsServerRequest() = default;

    /**
     * \returns  the request url
     *
     * Subclasses may override in case the original URL (not rewritten, e.g.from
     * a web server rewrite module) needs to be returned instead of the URL
     * seen by QGIS server.
     */
    virtual QUrl url() const;

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
    void setParameter( const QString &key, const QString &value );

    /**
     * Gets a parameter value
     */
    QString parameter( const QString &key ) const;

    /**
     * Remove a parameter
     */
    void removeParameter( const QString &key );

    /**
     * Returns the header value
     * \param name of the header
     * \return the header value or an empty string
     */
    QString header( const QString &name ) const;

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
    void setUrl( const QUrl &url );

    /**
     * Set the request method
     */
    void setMethod( QgsServerRequest::Method method );

  private:
    QUrl       mUrl;
    Method     mMethod = GetMethod;
    // We mark as mutable in order
    // to support lazy initialization
    mutable Headers mHeaders;
    QgsServerParameters mParams;
};

#endif
