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
#include <QStringList>
#include <QPair>
#include <QColor>
#include <QHash>

#include "qgis_server.h"

class QDomDocument;
class QImage;
class QgsServerException;
class QgsServerRequest;
class QgsServerResponse;

typedef QList< QPair<QRgb, int> > QgsColorBox; //Color / number of pixels
typedef QMultiMap< int, QgsColorBox > QgsColorBoxMap; // sum of pixels / color box

/**
 * \ingroup server
 * This class is an interface hiding the details of reading input and writing
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

    //! Set an HTTP response header
    void setResponseHeader( const QString &name, const QString &value );

    //! Remove an HTTP response header
    void removeResponseHeader( const QString &name );

    //! Retrieve response header value
    QString responseHeader( const QString &name ) const;

    //! Return the response headers
    QMap<QString, QString> responseHeaders() const;

    //! Set an HTTP request header
    void setRequestHeader( const QString &name, const QString &value );

    //! Remove an HTTP request header
    void removeRequestHeader( const QString &name );

    //! Retrieve request header value
    QString requestHeader( const QString &name ) const;

    //! Return the Request headers
    QMap<QString, QString> requestHeaders() const;

    //! Clears the response body and headers
    void clear();

    //! Set the info format string such as "text/xml"
    void appendBody( const QByteArray &body );

    //! Pointer to last raised exception
    bool exceptionRaised() const;

    //! Clear response buffer
    void clearBody();

    //! Return response body data
    QByteArray body() const;

    //! Return request POST data (can be null)
    QByteArray data() const;

    //! Return request url
    QString url() const;

    //! Set response http status code
    void setStatusCode( int code );

    //! Return response http status code
    int statusCode() const;

    /**
     * Return the parsed parameters as a key-value pair, to modify
     * a parameter setParameter( const QString &key, const QString &value)
     * and removeParameter(const QString &key) must be used
     */
    QMap<QString, QString> parameterMap() const;

    //! Set a request parameter
    void setParameter( const QString &key, const QString &value );

    //! Return a request parameter
    QString parameter( const QString &key ) const;

    //! Remove a request parameter
    void removeParameter( const QString &key );

    /**
     * Parses the input and creates a request neutral Parameter/Value map
     * \note not available in Python bindings
     */
    void parseInput() SIP_SKIP;

    //! Return the requested format string
    QString format() const { return mFormat; }

    //! Return true if the HTTP headers were already sent to the client
    bool headersSent() const;

  private:

    void setupParameters();

    //! This is set by the parseInput methods of the subclasses (parameter FORMAT, e.g. 'FORMAT=PNG')
    QString mFormat;
    QString mFormatString; //format string as it is passed in the request (with base)
    QString mService;
    bool mExceptionRaised;

    QgsServerRequest   &mRequest;
    QgsServerResponse  &mResponse;

};

#endif
