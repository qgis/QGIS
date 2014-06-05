/***************************************************************************
                              qgsrequesthandler.h
                 abstraction for reading from/ writing to a request datasource
                              -------------------
  begin                : May 16, 2006
  copyright            : (C) 2006 by Marco Hugentobler
  email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSWMSREQUESTHANDLER
#define QGSWMSREQUESTHANDLER

#include <QMap>
#include <QString>
class QDomDocument;
class QgsMapServiceException;
class QImage;

/**This class is an interface hiding the details of reading input and writing output from/to a wms request mechanism.
Examples of possible mechanisms are cgi Get, cgi Post, SOAP or the usage as a standalone command line executable*/
class QgsRequestHandler
{
  public:
    QgsRequestHandler() {}
    virtual ~QgsRequestHandler() {}
    /**Parses the input and creates a request neutral Parameter/Value map*/
    virtual QMap<QString, QString> parseInput() = 0;
    /**Sends the map image back to the client*/
    virtual void sendGetMapResponse( const QString& service, QImage* img, int imageQuality ) const = 0;
    virtual void sendGetCapabilitiesResponse( const QDomDocument& doc ) const = 0;
    virtual void sendGetFeatureInfoResponse( const QDomDocument& infoDoc, const QString& infoFormat ) const = 0;
    virtual void sendServiceException( const QgsMapServiceException& ex ) const = 0;
    virtual void sendGetStyleResponse( const QDomDocument& doc ) const = 0;
    virtual void sendGetPrintResponse( QByteArray* ba ) const = 0;
    virtual bool startGetFeatureResponse( QByteArray* ba, const QString& infoFormat ) const = 0;
    virtual void sendGetFeatureResponse( QByteArray* ba ) const = 0;
    virtual void endGetFeatureResponse( QByteArray* ba ) const = 0;
    virtual void sendGetCoverageResponse( QByteArray* ba ) const = 0;
    QString format() const { return mFormat; }
  protected:
    /**This is set by the parseInput methods of the subclasses (parameter FORMAT, e.g. 'FORMAT=PNG')*/
    QString mFormat;
    QString mFormatString; //format string as it is passed in the request (with base
    QString mService;
};

#endif
