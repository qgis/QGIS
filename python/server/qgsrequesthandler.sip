/***************************************************************************
                          qgsrequesthandler.sip

  This class is an interface hiding the details of reading input and
  writing output from/to a wms request mechanism
  -------------------
  begin                : 2014-09-10
  copyright            : (C) 2014 by Alessandro Pasotti
  email                : a dot pasotti at itopen dot it
 ***************************************************************************/


/**
 * \class  QgsRequestHandler
 * \brief This class is an interface hiding the details of reading input and
 * writing output from/to a wms request mechanism.
 *
 * Examples of possible mechanisms are cgi Get, cgi Post, SOAP or the usage
 * as a standalone command line executable
 */

class QgsRequestHandler
{
%TypeHeaderCode
#include "qgsmapserviceexception.h"
#include "qgsrequesthandler.h"

%End

  public:

    /**Set an HTTP header*/
    virtual void setHeader( const QString &name, const QString &value ) = 0;
    /**Remove an HTTP header*/
    virtual int removeHeader( const QString &name ) = 0;
    /**Delete all HTTP headers*/
    virtual void clearHeaders( ) = 0;
    /**Returns the response body*/
    virtual QByteArray body();
    /**Append the bytestream to response body*/
    virtual void appendBody( const QByteArray &body) = 0;
    /**Clears the response body*/
    virtual void clearBody( ) = 0;
    /**Set the info format string such as "text/xml"*/
    virtual void setInfoFormat( const QString &format ) = 0;
    /**Check wether there is any header set or the body is not empty*/
    virtual bool responseReady() const = 0;
    /**Pointer to last raised exception*/
    virtual bool exceptionRaised() const = 0;
    /**Return a copy of the parsed parameters as a key-value pair, to modify
     * a parameter setParameter( const QString &key, const QString &value)
     * and removeParameter(const QString &key) must be used
     */
    QMap<QString, QString> parameterMap( );
    /**Set a request parameter*/
    virtual void setParameter(const QString &key, const QString &value) = 0;
    /**Remove a request parameter*/
    virtual int removeParameter(const QString &key) = 0;
    /**Return a request parameter*/
    virtual QString parameter(const QString &key) const = 0;
    /**Return the image format*/
    QString format() const;
    /**Return the format string as requested by the client*/
    QString infoFormat() const;
    /**Allow plugins to return a QgsMapServiceException*/
    virtual void setServiceException( QgsMapServiceException ex /Transfer/) = 0;

private:

    /* Not yet part of the API */

    virtual void sendHeaders( ) const = 0;
    virtual void sendBody( ) const = 0;

    /**Send out the response writing to FCGI stdout*/
    virtual void sendResponse( ) const = 0;

    virtual void parseInput() = 0;
    virtual void setGetMapResponse( const QString& service, QImage* img, int imageQuality ) = 0;
    virtual void setGetCapabilitiesResponse( const QDomDocument& doc ) = 0;
    virtual void setGetFeatureInfoResponse( const QDomDocument& infoDoc, const QString& infoFormat ) = 0;
    virtual void setGetStyleResponse( const QDomDocument& doc ) = 0;
    virtual void setGetPrintResponse( QByteArray* ba ) = 0;
    virtual bool startGetFeatureResponse( QByteArray* ba, const QString& infoFormat ) = 0;
    virtual void setGetFeatureResponse( QByteArray* ba ) = 0;
    virtual void endGetFeatureResponse( QByteArray* ba ) = 0;
    virtual void setGetCoverageResponse( QByteArray* ba ) = 0;


};
