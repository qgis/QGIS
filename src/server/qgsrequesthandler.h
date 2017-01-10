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
#include <QString>
#include <QStringList>
#include <QPair>
#include <QColor>
#include <QHash>

#include "qgis_server.h"

class QDomDocument;
class QImage;
class QgsMapServiceException;
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

    /** Constructor
     *
     * Note that QgsServerRequest and QgsServerResponse MUST live in the same scope
     */
    explicit QgsRequestHandler( QgsServerRequest& request, QgsServerResponse& response );
    ~QgsRequestHandler();

    //! @note not available in Python bindings
    void setGetCapabilitiesResponse( const QDomDocument& doc );

    //! Allow plugins to return a QgsMapServiceException
    void setServiceException( const QgsMapServiceException &ex );

    //! @note not available in Python bindings
    void setXmlResponse( const QDomDocument& doc );

    //! @note not available in Python bindings
    void setXmlResponse( const QDomDocument& doc, const QString& mimeType );

    //! @note not available in Python bindings
    bool startGetFeatureResponse( QByteArray* ba, const QString& infoFormat );

    //! @note not available in Python bindings
    void setGetFeatureResponse( QByteArray* ba );

    //! @note not available in Python bindings
    void endGetFeatureResponse( QByteArray* ba );

    //! @note not available in Python bindings
    void setGetCoverageResponse( QByteArray* ba );

    /** Send out HTTP headers and flush output buffer
     *
     *  This method is intended only for streaming
     *  partial content.
     */
    void sendResponse();

    //! Set an HTTP header
    void setHeader( const QString &name, const QString &value );

    //! Remove an HTTP header
    void removeHeader( const QString &name );

    //! Retrieve header value
    QString getHeader( const QString& name ) const;

    //! Return the list of all header keys
    QList<QString> headerKeys() const;

    //! Clears the response body and headers
    void clear();

    //! Set the info format string such as "text/xml"
    void appendBody( const QByteArray &body );

    //! Set the info format string such as "text/xml"
    void setInfoFormat( const QString &format );

    //! Pointer to last raised exception
    bool exceptionRaised() const;

    /** Return the parsed parameters as a key-value pair, to modify
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

    /** Parses the input and creates a request neutral Parameter/Value map
     * @note not available in Python bindings
     */
    void parseInput();

    //! Return the requested format string
    QString format() const { return mFormat; }

    //! Return the mime type for the response
    QString infoFormat() const { return mInfoFormat; }

    //! Return true if the HTTP headers were already sent to the client
    bool headersSent() const;

  private:
    void setHttpResponse( const QByteArray& ba, const QString &format );

    /** Converts format to official mimetype (e.g. 'jpg' to 'image/jpeg')
      @return mime string (or the entered string if not found)*/
    QString formatToMimeType( const QString& format ) const;

  private:

    void setupParameters();

    //! This is set by the parseInput methods of the subclasses (parameter FORMAT, e.g. 'FORMAT=PNG')
    QString mFormat;
    QString mFormatString; //format string as it is passed in the request (with base)
    QString mService;
    QString mInfoFormat;
    QgsMapServiceException* mException; // Stores the exception

    QgsServerRequest&   mRequest;
    QgsServerResponse&  mResponse;

};

#endif
