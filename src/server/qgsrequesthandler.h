/***************************************************************************
                              qgshttprequesthandler.h
                              -----------------------
  begin                : June 29, 2007
  copyright            : (C) 2007 by Marco Hugentobler
                         (C) 2014 by Alessandro Pasotti
  email                : marco dot hugentobler at karto dot baug dot ethz dot ch
                         a dot pasotti at itopen dot it
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSREQUESTHANDLER_H
#define QGSREQUESTHANDLER_H

#include <QMap>
#include "qgis_sip.h"
#include <QString>

#include "qgis_server.h"

class QgsServerException;
class QgsServerRequest;
class QgsServerResponse;

/**
 * \ingroup server
 * \brief This class is an interface hiding the details of reading input and writing
 * output from/to a wms request mechanism.
 */
class SERVER_EXPORT QgsRequestHandler
{
  public:
    /**
     * Constructor
     *
     * Note that QgsServerRequest and QgsServerResponse MUST live in the same scope
     */
    explicit QgsRequestHandler( QgsServerRequest &request, QgsServerResponse &response );

    //! Allow plugins to return a QgsMapServiceException
    void setServiceException( const QgsServerException &ex );

    /**
     * Send out HTTP headers and flush output buffer
     *
     *  This method is intended only for streaming
     *  partial content.
     */
    void sendResponse();

    //! Sets an HTTP response header
    void setResponseHeader( const QString &name, const QString &value );

    //! Remove an HTTP response header
    void removeResponseHeader( const QString &name );

    //! Retrieve response header value
    QString responseHeader( const QString &name ) const;

    //! Returns the response headers
    QMap<QString, QString> responseHeaders() const;

    //! Sets an HTTP request header
    void setRequestHeader( const QString &name, const QString &value );

    //! Remove an HTTP request header
    void removeRequestHeader( const QString &name );

    //! Retrieve request header value
    QString requestHeader( const QString &name ) const;

    //! Returns the the Request headers
    QMap<QString, QString> requestHeaders() const;

    //! Clears the response body and headers
    void clear();

    //! Sets the info format string such as "text/xml"
    void appendBody( const QByteArray &body );

    //! Pointer to last raised exception
    bool exceptionRaised() const;

    //! Clear response buffer
    void clearBody();

    //! Returns the response body data
    QByteArray body() const;

    //! Returns the request POST data (can be null)
    QByteArray data() const;

    //! Returns the request url
    QString url() const;

    /**
     * Returns the path component of the request URL
     * \since QGIS 3.16
     */
    QString path() const;

    //! Sets response http status code
    void setStatusCode( int code );

    //! Returns the response http status code
    int statusCode() const;

    /**
     * Returns the parsed parameters as a key-value pair, to modify
     * a parameter setParameter( const QString &key, const QString &value)
     * and removeParameter(const QString &key) must be used
     */
    QMap<QString, QString> parameterMap() const;

    //! Sets a request parameter
    void setParameter( const QString &key, const QString &value );

    //! Returns a request parameter
    QString parameter( const QString &key ) const;

    //! Remove a request parameter
    void removeParameter( const QString &key );

    /**
     * Parses the input and creates a request neutral Parameter/Value map
     * \note not available in Python bindings
     */
    void parseInput() SIP_SKIP;

    //! Returns the requested format string
    QString format() const { return mFormat; }

    //! Returns TRUE if the HTTP headers were already sent to the client
    bool headersSent() const;

  private:
#ifdef SIP_RUN
    QgsRequestHandler &operator=( const QgsRequestHandler & );
#endif

    void setupParameters();

    //! This is set by the parseInput methods of the subclasses (parameter FORMAT, e.g. 'FORMAT=PNG')
    QString mFormat;
    QString mFormatString; //format string as it is passed in the request (with base)
    QString mService;
    bool mExceptionRaised;

    QgsServerRequest &mRequest;
    QgsServerResponse &mResponse;
};

#endif
