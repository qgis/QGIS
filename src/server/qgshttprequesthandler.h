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

    virtual void setGetMapResponse( const QString& service, QImage* img, int imageQuality ) override;
    virtual void setGetCapabilitiesResponse( const QDomDocument& doc ) override;
    virtual void setGetFeatureInfoResponse( const QDomDocument& infoDoc, const QString& infoFormat ) override;
    virtual void setServiceException( QgsMapServiceException ex ) override;
    virtual void setXmlResponse( const QDomDocument& doc ) override;
    virtual void setXmlResponse( const QDomDocument& doc, const QString& mimeType ) override;
    virtual void setGetPrintResponse( QByteArray* ba ) override;
    virtual bool startGetFeatureResponse( QByteArray* ba, const QString& infoFormat ) override;
    virtual void setGetFeatureResponse( QByteArray* ba ) override;
    virtual void endGetFeatureResponse( QByteArray* ba ) override;
    virtual void setGetCoverageResponse( QByteArray* ba ) override;
    /**Send out HTTP headers and flush output buffer*/
    virtual void sendResponse() override;
    virtual void setHeader( const QString &name, const QString &value ) override;
    virtual int removeHeader( const QString &name ) override;
    virtual void clearHeaders( ) override;
    virtual void appendBody( const QByteArray &body ) override;
    virtual void clearBody( ) override;
    virtual void setInfoFormat( const QString &format ) override;
    virtual bool responseReady() const override;
    virtual bool exceptionRaised() const override;
    virtual void setParameter( const QString &key, const QString &value ) override;
    virtual QString parameter( const QString &key ) const override;
    virtual int removeParameter( const QString &key ) override;
#ifdef HAVE_SERVER_PYTHON_PLUGINS
    virtual void setPluginFilters( QgsServerFiltersMap pluginFilters ) override;
#endif
  protected:
    virtual void sendHeaders( ) override;
    virtual void sendBody( ) const override;
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
