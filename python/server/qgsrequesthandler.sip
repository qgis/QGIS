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
 * \ingroup server
 * This class is an interface hiding the details of reading input and writing
 * output from/to a wms request mechanism.
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

    /** Parses the input and creates a request neutral Parameter/Value map
     * @note not available in Python bindings
     */
    // virtual void parseInput() = 0;

    /** Sends the map image back to the client
     * @note not available in Python bindings
     */
    // virtual void setGetMapResponse( const QString& service, QImage* img, int imageQuality ) = 0;

    //! @note not available in Python bindings
    // virtual void setGetCapabilitiesResponse( const QDomDocument& doc ) = 0;

    //! @note not available in Python bindings
    // virtual void setGetFeatureInfoResponse( const QDomDocument& infoDoc, const QString& infoFormat ) = 0;

    /** Allow plugins to return a QgsMapServiceException*/
    virtual void setServiceException( QgsMapServiceException ex /Transfer/ ) = 0;

    //! @note not available in Python bindings
    // virtual void setXmlResponse( const QDomDocument& doc ) = 0;

    //! @note not available in Python bindings
    // virtual void setXmlResponse( const QDomDocument& doc, const QString& mimeType ) = 0;

    //! @note not available in Python bindings
    // virtual void setGetPrintResponse( QByteArray* b ) = 0;

    //! @note not available in Python bindings
    // virtual bool startGetFeatureResponse( QByteArray* ba, const QString& infoFormat ) = 0;

    //! @note not available in Python bindings
    // virtual void setGetFeatureResponse( QByteArray* ba ) = 0;

    //! @note not available in Python bindings
    virtual void endGetFeatureResponse( QByteArray* ba ) = 0;

    //! @note not available in Python bindings
    virtual void setGetCoverageResponse( QByteArray* ba ) = 0;

    virtual void setDefaultHeaders();

    /** Set an HTTP header*/
    virtual void setHeader( const QString &name, const QString &value ) = 0;

    /** Remove an HTTP header*/
    virtual int removeHeader( const QString &name ) = 0;

    /** Delete all HTTP headers*/
    virtual void clearHeaders() = 0;

    /** Append the bytestream to response body*/
    virtual void appendBody( const QByteArray &body ) = 0;

    /** Clears the response body*/
    virtual void clearBody() = 0;

    /** Return the response body*/
    virtual QByteArray body();

    /** Set the info format string such as "text/xml"*/
    virtual void setInfoFormat( const QString &format ) = 0;

    /** Check whether there is any header set or the body is not empty*/
    virtual bool responseReady() const = 0;

    /** Send out HTTP headers and flush output buffer*/
    virtual void sendResponse() = 0;

    /** Pointer to last raised exception*/
    virtual bool exceptionRaised() const = 0;

    /** Return a copy of the parsed parameters as a key-value pair, to modify
     * a parameter setParameter( const QString &key, const QString &value)
     * and removeParameter(const QString &key) must be used
     */
    QMap<QString, QString> parameterMap();

    /** Set a request parameter*/
    virtual void setParameter( const QString &key, const QString &value ) = 0;

    /** Remove a request parameter*/
    virtual int removeParameter( const QString &key ) = 0;

    /** Return a request parameter*/
    virtual QString parameter( const QString &key ) const = 0;

    /** Return the requested format string*/
    QString format() const;

    /** Return the mime type for the response*/
    QString infoFormat() const;

    /** Return true if the HTTP headers were already sent to the client*/
    bool headersSent();


    //! @note not available in Python bindings
    // virtual QPair<QByteArray, QByteArray> getResponse() = 0;

private:
    /** Parses the input and creates a request neutral Parameter/Value map*/
    virtual void parseInput() = 0;
};
