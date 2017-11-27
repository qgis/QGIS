/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/server/qgsrequesthandler.h                                       *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






typedef QList< QPair<QRgb, int> > QgsColorBox; //Color / number of pixels
typedef QMultiMap< int, QgsColorBox > QgsColorBoxMap; // sum of pixels / color box

class QgsRequestHandler
{
%Docstring
 This class is an interface hiding the details of reading input and writing
 output from/to a wms request mechanism.
%End

%TypeHeaderCode
#include "qgsrequesthandler.h"
%End
  public:

    explicit QgsRequestHandler( QgsServerRequest &request, QgsServerResponse &response );
%Docstring
 Constructor

 Note that QgsServerRequest and QgsServerResponse MUST live in the same scope
%End

    void setServiceException( const QgsServerException &ex );
%Docstring
Allow plugins to return a QgsMapServiceException
%End

    void sendResponse();
%Docstring
 Send out HTTP headers and flush output buffer

  This method is intended only for streaming
  partial content.
%End

    void setResponseHeader( const QString &name, const QString &value );
%Docstring
Set an HTTP response header
%End

    void removeResponseHeader( const QString &name );
%Docstring
Remove an HTTP response header
%End

    QString responseHeader( const QString &name ) const;
%Docstring
Retrieve response header value
 :rtype: str
%End

    QMap<QString, QString> responseHeaders() const;
%Docstring
Return the response headers
 :rtype: QMap<str, QString>
%End

    void setRequestHeader( const QString &name, const QString &value );
%Docstring
Set an HTTP request header
%End

    void removeRequestHeader( const QString &name );
%Docstring
Remove an HTTP request header
%End

    QString requestHeader( const QString &name ) const;
%Docstring
Retrieve request header value
 :rtype: str
%End

    QMap<QString, QString> requestHeaders() const;
%Docstring
Return the Request headers
 :rtype: QMap<str, QString>
%End

    void clear();
%Docstring
Clears the response body and headers
%End

    void appendBody( const QByteArray &body );
%Docstring
Set the info format string such as "text/xml"
%End

    bool exceptionRaised() const;
%Docstring
Pointer to last raised exception
 :rtype: bool
%End

    void clearBody();
%Docstring
Clear response buffer
%End

    QByteArray body() const;
%Docstring
Return response body data
 :rtype: QByteArray
%End

    QByteArray data() const;
%Docstring
Return request POST data (can be null)
 :rtype: QByteArray
%End

    QString url() const;
%Docstring
Return request url
 :rtype: str
%End

    void setStatusCode( int code );
%Docstring
Set response http status code
%End

    int statusCode() const;
%Docstring
Return response http status code
 :rtype: int
%End

    QMap<QString, QString> parameterMap() const;
%Docstring
 Return the parsed parameters as a key-value pair, to modify
 a parameter setParameter( const QString &key, const QString &value)
 and removeParameter(const QString &key) must be used
 :rtype: QMap<str, QString>
%End

    void setParameter( const QString &key, const QString &value );
%Docstring
Set a request parameter
%End

    QString parameter( const QString &key ) const;
%Docstring
Return a request parameter
 :rtype: str
%End

    void removeParameter( const QString &key );
%Docstring
Remove a request parameter
%End


    QString format() const;
%Docstring
Return the requested format string
 :rtype: str
%End

    bool headersSent() const;
%Docstring
Return true if the HTTP headers were already sent to the client
 :rtype: bool
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/server/qgsrequesthandler.h                                       *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
