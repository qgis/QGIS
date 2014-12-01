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

#ifndef QGSHTTPREQUESTHANDLER_H
#define QGSHTTPREQUESTHANDLER_H

#include "qgsrequesthandler.h"
#include <QColor>
#include <QPair>

typedef QList< QPair<QRgb, int> > QgsColorBox; //Color / number of pixels
typedef QMultiMap< int, QgsColorBox > QgsColorBoxMap; // sum of pixels / color box

/**Base class for request handler using HTTP.
It provides a method to set data to the client*/
class QgsHttpRequestHandler: public QgsRequestHandler
{
  public:
    QgsHttpRequestHandler();
    ~QgsHttpRequestHandler();

    virtual void setGetMapResponse( const QString& service, QImage* img, int imageQuality );
    virtual void setGetCapabilitiesResponse( const QDomDocument& doc );
    virtual void setGetFeatureInfoResponse( const QDomDocument& infoDoc, const QString& infoFormat );
    virtual void setServiceException( QgsMapServiceException ex );
    virtual void setXmlResponse( const QDomDocument& doc );
    virtual void setXmlResponse( const QDomDocument& doc, const QString& mimeType );
    virtual void setGetPrintResponse( QByteArray* ba );
    virtual bool startGetFeatureResponse( QByteArray* ba, const QString& infoFormat );
    virtual void setGetFeatureResponse( QByteArray* ba );
    virtual void endGetFeatureResponse( QByteArray* ba );
    virtual void setGetCoverageResponse( QByteArray* ba );
    /**Send out HTTP headers and flush output buffer*/
    virtual void sendResponse();
    virtual void setHeader( const QString &name, const QString &value );
    virtual int removeHeader( const QString &name );
    virtual void clearHeaders( );
    virtual void appendBody( const QByteArray &body );
    virtual void clearBody( );
    virtual void setInfoFormat( const QString &format );
    virtual bool responseReady() const;
    virtual bool exceptionRaised() const;
    virtual void setParameter( const QString &key, const QString &value );
    virtual QString parameter( const QString &key ) const;
    virtual int removeParameter( const QString &key );
#ifdef HAVE_SERVER_PYTHON_PLUGINS
    virtual void setPluginFilters( QgsServerFiltersMap pluginFilters );
#endif
  protected:
    virtual void sendHeaders( );
    virtual void sendBody( ) const;
    void setHttpResponse( QByteArray *ba, const QString &format );
    /**Converts format to official mimetype (e.g. 'jpg' to 'image/jpeg')
      @return mime string (or the entered string if not found)*/
    QString formatToMimeType( const QString& format ) const;

    void requestStringToParameterMap( const QString& request, QMap<QString, QString>& parameters );
    /**Read CONTENT_LENGTH characters from stdin*/
    QString readPostBody() const;

  private:
    static void medianCut( QVector<QRgb>& colorTable, int nColors, const QImage& inputImage );
    static void imageColors( QHash<QRgb, int>& colors, const QImage& image );
    static void splitColorBox( QgsColorBox& colorBox, QgsColorBoxMap& colorBoxMap,
                               QMap<int, QgsColorBox>::iterator colorBoxMapIt );
    static bool minMaxRange( const QgsColorBox& colorBox, int& redRange, int& greenRange, int& blueRange, int& alphaRange );
    static bool redCompare( const QPair<QRgb, int>& c1, const QPair<QRgb, int>& c2 );
    static bool greenCompare( const QPair<QRgb, int>& c1, const QPair<QRgb, int>& c2 );
    static bool blueCompare( const QPair<QRgb, int>& c1, const QPair<QRgb, int>& c2 );
    static bool alphaCompare( const QPair<QRgb, int>& c1, const QPair<QRgb, int>& c2 );
    /**Calculates a representative color for a box (pixel weighted average)*/
    static QRgb boxColor( const QgsColorBox& box, int boxPixels );
};

#endif
