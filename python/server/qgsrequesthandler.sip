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
    void setServiceException( const QgsServerException& ex );

    /** Set an HTTP header*/
    void setHeader( const QString &name, const QString &value );

    //! Retrieve header value
    QString getHeader( const QString& name ) const;

    //! Return the list of all header keys
    QList<QString> headerKeys() const;

   /** Remove an HTTP header*/
    void removeHeader( const QString &name );

    /** Delete all HTTP headers*/
    void clear();

    /** Append the bytestream to response body*/
    void appendBody( const QByteArray &body );

    /** Send out HTTP headers and flush output buffer*/
    void sendResponse();

    /** Pointer to last raised exception*/
    bool exceptionRaised() const;

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
