/**This class is an interface hiding the details of reading input and writing output from/to a wms request mechanism.
Examples of possible mechanisms are cgi Get, cgi Post, SOAP or the usage as a standalone command line executable*/
class QgsRequestHandler
{
%TypeHeaderCode
#include "qgsmapserviceexception.h"
#include "qgsrequesthandler.h"

%End

  public:

    virtual void parseInput() = 0;
    virtual void setGetMapResponse( const QString& service, QImage* img, int imageQuality ) = 0;
    virtual void setGetCapabilitiesResponse( const QDomDocument& doc ) = 0;
    virtual void setGetFeatureInfoResponse( const QDomDocument& infoDoc, const QString& infoFormat ) = 0;
    virtual void setServiceException( QgsMapServiceException ex ) = 0;
    virtual void setGetStyleResponse( const QDomDocument& doc ) = 0;
    virtual void setGetPrintResponse( QByteArray* ba ) = 0;
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
    virtual void appendBody( const QByteArray &body) = 0;
    /**Clears the response body*/
    virtual void clearBody( ) = 0;
    virtual void setInfoFormat( const QString &format ) = 0;
    virtual void sendResponse( ) const = 0;
    virtual bool responseReady() const = 0;
    /**Pointer to last raised exception*/
    virtual bool exceptionRaised() const = 0;
    QMap<QString, QString> parameterMap( );
    /**Set a request parameter*/
    virtual void setParameter(const QString &key, const QString &value) = 0;
    /**Remove a request parameter*/
    virtual int removeParameter(const QString &key) = 0;
    /**Return a request parameter*/
    virtual QString parameter(const QString &key) const = 0;
    QString format() const;

protected:

    virtual void sendHeaders( ) const = 0;
    virtual void sendBody( ) const = 0;


};
