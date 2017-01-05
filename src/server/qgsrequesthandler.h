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

//Needed for HAVE_SERVER_PYTHON_PLUGINS
#include "qgsconfig.h"
#ifdef HAVE_SERVER_PYTHON_PLUGINS
#include "qgsserverfilter.h"
#endif

#include <QMap>
#include <QString>
#include <QStringList>
#include <QPair>
#include <QColor>
#include <QPair>
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
    // QgsServerRequest and QgsServerResponse MUST live in the same scope
    explicit QgsRequestHandler( const QgsServerRequest& request, QgsServerResponse& response );
    ~QgsRequestHandler();

    /** Sends the map image back to the client
     * @note not available in Python bindings
     */
    void setGetMapResponse( const QString& service, QImage* img, int imageQuality );
 
    //! @note not available in Python bindings
    void setGetCapabilitiesResponse( const QDomDocument& doc );

    //! @note not available in Python bindings
    void setGetFeatureInfoResponse( const QDomDocument& infoDoc, const QString& infoFormat );

    //! Allow plugins to return a QgsMapServiceException
    void setServiceException( const QgsMapServiceException &ex );

    //! @note not available in Python bindings
    void setXmlResponse( const QDomDocument& doc );

    //! @note not available in Python bindings
    void setXmlResponse( const QDomDocument& doc, const QString& mimeType );

    //! @note not available in Python bindings
    void setGetPrintResponse( QByteArray* ba );

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
    QMap<QString, QString> parameterMap() { return mParameterMap; }

    //! Set a request parameter
    void setParameter( const QString &key, const QString &value );

    //! Return a request parameter
    QString parameter( const QString &key ) const;

    //! Remove a request parameter
    int removeParameter( const QString &key );

#ifdef HAVE_SERVER_PYTHON_PLUGINS
     /** Allow core services to call plugin hooks through sendResponse()
     * @note not available in Python bindings
     */
    void setPluginFilters( const QgsServerFiltersMap &pluginFilters );
#endif

    /** Parses the input and creates a request neutral Parameter/Value map
     * @note not available in Python bindings
     */
    void parseInput();

    //! Return the requested format string
    QString format() const { return mFormat; }

    //! Return the mime type for the response
    QString infoFormat() const { return mInfoFormat; }

    //! Return true if the HTTP headers were already sent to the client
    bool headersSent() { return mHeadersSent; }

  private:
    void setHttpResponse( const QByteArray& ba, const QString &format );

    /** Converts format to official mimetype (e.g. 'jpg' to 'image/jpeg')
      @return mime string (or the entered string if not found)*/
    QString formatToMimeType( const QString& format ) const;

    void requestStringToParameterMap( QMap<QString, QString>& parameters );

  private:
    static void medianCut( QVector<QRgb>& colorTable, int nColors, const QImage& inputImage );
    static void imageColors( QHash<QRgb, int>& colors, const QImage& image );
    static void splitColorBox( QgsColorBox& colorBox, QgsColorBoxMap& colorBoxMap,
                               QMap<int, QgsColorBox>::iterator colorBoxMapIt );
    static bool minMaxRange( const QgsColorBox& colorBox, int& redRange, int& greenRange, int& blueRange, int& alphaRange );
    static bool redCompare( QPair<QRgb, int> c1, QPair<QRgb, int> c2 );
    static bool greenCompare( QPair<QRgb, int> c1, QPair<QRgb, int> c2 );
    static bool blueCompare( QPair<QRgb, int> c1, QPair<QRgb, int> c2 );
    static bool alphaCompare( QPair<QRgb, int> c1, QPair<QRgb, int> c2 );
    //! Calculates a representative color for a box (pixel weighted average)
    static QRgb boxColor( const QgsColorBox& box, int boxPixels );
    // TODO: if HAVE_SERVER_PYTHON

#ifdef HAVE_SERVER_PYTHON_PLUGINS
    QgsServerFiltersMap mPluginFilters;
#endif
    //! This is set by the parseInput methods of the subclasses (parameter FORMAT, e.g. 'FORMAT=PNG')
    QString mFormat;
    QString mFormatString; //format string as it is passed in the request (with base)
    bool mHeadersSent;
    QString mService;
    QString mInfoFormat;
    QMap<QString, QString> mParameterMap;
    QgsMapServiceException* mException; // Stores the exception

    const QgsServerRequest& mRequest;
    QgsServerResponse&      mResponse;

};

#endif
