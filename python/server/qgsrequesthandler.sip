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
class QgsRequestHandler  /Abstract/
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
    void setServiceException( const QgsMapServiceException& ex /Transfer/ );

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
    void endGetFeatureResponse( QByteArray* ba );

    //! @note not available in Python bindings
    void setGetCoverageResponse( QByteArray* ba );

    /** Set an HTTP header*/
    void setHeader( const QString &name, const QString &value );

    /** Remove an HTTP header*/
    void removeHeader( const QString &name );

    /** Delete all HTTP headers*/
    void clear();

    /** Append the bytestream to response body*/
    void appendBody( const QByteArray &body );

    /** Set the info format string such as "text/xml"*/
    void setInfoFormat( const QString &format );

    /** Send out HTTP headers and flush output buffer*/
    void sendResponse();

    /** Pointer to last raised exception*/
    bool exceptionRaised() const;

    /** Return a copy of the parsed parameters as a key-value pair, to modify
     * a parameter setParameter( const QString &key, const QString &value)
     * and removeParameter(const QString &key) must be used
     */
    QMap<QString, QString> parameterMap();

    /** Set a request parameter*/
    void setParameter( const QString &key, const QString &value );

    /** Remove a request parameter*/
    int removeParameter( const QString &key );

    /** Return a request parameter*/
    QString parameter( const QString &key ) const;

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
    void parseInput();
};
