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
#include "qgsserverexception.h"
#include "qgsrequesthandler.h"
%End

  public:

    /** Allow plugins to return a QgsServerException*/
    void setServiceException( const QgsServerException &ex );

    //! Set an HTTP response header
    void setResponseHeader( const QString &name, const QString &value );

    //! Remove an HTTP response header
    void removeResponseHeader( const QString &name );

    //! Retrieve response header value
    QString responseHeader( const QString &name ) const;

    //! Return the response headers
    QMap<QString, QString> responseHeaders() const;

    //! Set an HTTP request header
    void setRequestHeader( const QString &name, const QString &value );

    //! Remove an HTTP request header
    void removeRequestHeader( const QString &name );

    //! Retrieve request header value
    QString requestHeader( const QString &name ) const;

    //! Return the Request headers
    QMap<QString, QString> requestHeaders() const;

    /** Append the bytestream to response body*/
    void appendBody( const QByteArray &body );

    /** Send out HTTP headers and flush output buffer*/
    void sendResponse();

    //! Set response http status code
    void setStatusCode( int code );

    //! Return response http status code
    int statusCode(  ) const;

    /** Pointer to last raised exception*/
    bool exceptionRaised() const;

    /** Clear response buffer */
    void clearBody();

    //! Clears the response body and headers
    void clear();

    /** Return body data */
    QByteArray body() const;

    /** Return a copy of the parsed parameters as a key-value pair, to modify
     * a parameter setParameter( const QString &key, const QString &value)
     * and removeParameter(const QString &key) must be used
     */
    QMap<QString, QString> parameterMap() const;

    /** Set a request parameter*/
    void setParameter( const QString &key, const QString &value );

    /** Remove a request parameter*/
    void removeParameter( const QString &key );

    /** Return a request parameter*/
    QString parameter( const QString &key ) const;

    /** Return the requested format string*/
    QString format() const;

    /** Return true if the HTTP headers were already sent to the client*/
    bool headersSent() const;
};
