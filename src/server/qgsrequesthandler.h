
/***************************************************************************
                              qgsrequesthandler.h
                 abstraction for reading from/ writing to a request datasource
                              -------------------
  begin                : May 16, 2006
  copyright            : (C) 2006 by Marco Hugentobler
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

#include <QMap>
#include <QString>
#include <QStringList>

#ifdef HAVE_SERVER_PYTHON_PLUGINS
#include "qgsserverfilter.h"
#endif

class QDomDocument;
class QImage;
class QgsMapServiceException;

/**This class is an interface hiding the details of reading input and writing output from/to a wms request mechanism.
Examples of possible mechanisms are cgi Get, cgi Post, SOAP or the usage as a standalone command line executable*/
class QgsRequestHandler
{

  public:

    QgsRequestHandler() {}
    virtual ~QgsRequestHandler() {}
    /**Parses the input and creates a request neutral Parameter/Value map*/
    virtual void parseInput() = 0;
    /**Sends the map image back to the client*/
    virtual void setGetMapResponse( const QString& service, QImage* img, int imageQuality ) = 0;
    virtual void setGetCapabilitiesResponse( const QDomDocument& doc ) = 0;
    virtual void setGetFeatureInfoResponse( const QDomDocument& infoDoc, const QString& infoFormat ) = 0;
    virtual void setServiceException( QgsMapServiceException ex ) = 0;
    virtual void setXmlResponse( const QDomDocument& doc ) = 0;
    virtual void setXmlResponse( const QDomDocument& doc, const QString& mimeType ) = 0;
    virtual void setGetPrintResponse( QByteArray* b ) = 0;
    virtual bool startGetFeatureResponse( QByteArray* ba, const QString& infoFormat ) = 0;
    virtual void setGetFeatureResponse( QByteArray* ba ) = 0;
    virtual void endGetFeatureResponse( QByteArray* ba ) = 0;
    virtual void setGetCoverageResponse( QByteArray* ba ) = 0;
    /**Set an HTTP header*/
    virtual void setHeader( const QString &name, const QString &value ) = 0;
    /**Remove an HTTP header*/
    virtual int removeHeader( const QString &name ) = 0;
    /**Delete all HTTP headers*/
    virtual void clearHeaders( ) = 0;
    /**Append the bytestream to response body*/
    virtual void appendBody( const QByteArray &body ) = 0;
    /**Clears the response body*/
    virtual void clearBody( ) = 0;
    /**Return the response body*/
    virtual QByteArray body( ) { return mBody; }
    virtual void setInfoFormat( const QString &format ) = 0;
    /**Check if the response headers or the response body are not empty*/
    virtual bool responseReady() const = 0;
    /**Send out HTTP headers and flush output buffer*/
    virtual void sendResponse( ) = 0;
    /**Pointer to last raised exception*/
    virtual bool exceptionRaised() const = 0;
    QMap<QString, QString> parameterMap( ) { return mParameterMap; }
    /**Set a request parameter*/
    virtual void setParameter( const QString &key, const QString &value ) = 0;
    /**Remove a request parameter*/
    virtual int removeParameter( const QString &key ) = 0;
    /**Return a request parameter*/
    virtual QString parameter( const QString &key ) const = 0;
    /**Return the requested format string*/
    QString format() const { return mFormat; }
    /**Return the mime type for the response*/
    QString infoFormat() const { return mInfoFormat; }
    /**Return true if the HTTP headers were already sent to the client*/
    bool headersSent() { return mHeadersSent; }
#ifdef HAVE_SERVER_PYTHON_PLUGINS
    /**Allow core services to call plugin hooks through sendResponse() */
    virtual void setPluginFilters( QgsServerFiltersMap pluginFilters ) = 0;
#endif
  protected:
    virtual void sendHeaders( ) = 0;
    virtual void sendBody( ) const = 0;
#ifdef HAVE_SERVER_PYTHON_PLUGINS
    QgsServerFiltersMap mPluginFilters;
#endif
    QByteArray mBody; // The response payload
    /**This is set by the parseInput methods of the subclasses (parameter FORMAT, e.g. 'FORMAT=PNG')*/
    QString mFormat;
    QString mFormatString; //format string as it is passed in the request (with base)
    bool mHeadersSent;
    QString mService;
    QString mInfoFormat;
    QgsMapServiceException* mException; // Stores the exception
    QMap<QString, QString> mParameterMap;
    /** Response headers. They can be empty, in this case headers are
        automatically generated from the content mFormat */
    QMap<QString, QString> mHeaders;

};

#endif
