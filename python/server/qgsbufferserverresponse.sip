/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/server/qgsbufferserverresponse.h                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsBufferServerResponse: QgsServerResponse
{
%Docstring
 Class defining buffered response
%End

%TypeHeaderCode
#include "qgsbufferserverresponse.h"
%End
  public:

    QgsBufferServerResponse();


    virtual void setHeader( const QString &key, const QString &value );

%Docstring
  Set Header entry
  Add Header entry to the response
  Note that it is usually an error to set Header after data have been sent through the wire
%End

    virtual void removeHeader( const QString &key );

%Docstring
 Clear header
 Undo a previous 'setHeader' call
%End

    virtual QString header( const QString &key ) const;

%Docstring
 Return the header value
 :rtype: str
%End

    virtual QMap<QString, QString> headers() const;
%Docstring
 Return all the headers
 :rtype: QMap<str, QString>
%End

    virtual bool headersSent() const;

%Docstring
 Return true if the headers have alredy been sent
 :rtype: bool
%End

    virtual void setStatusCode( int code );

%Docstring
 Set the http status code
 \param code HTTP status code value
%End

    virtual int statusCode() const;
%Docstring
 Return the http status code
 :rtype: int
%End

    virtual void sendError( int code,  const QString &message );

%Docstring
 Send error
 This method delegates error handling at the server level. This is different
 from calling setReturnCode() which let you return a specific response body.
 Calling sendError() will end the transaction and any attempt to write data
 or set headers will be an error.
 \param code HHTP return code value
 \param message An informative error message
%End

    virtual QIODevice *io();

%Docstring
 Return the underlying QIODevice
 :rtype: QIODevice
%End

    virtual void finish();

%Docstring
 Finish the response,  ending the transaction
%End

    virtual void flush();

%Docstring
 Flushes the current output buffer to the network

 'flush()' may be called multiple times. For HTTP transactions
 headers will be written on the first call to 'flush()'.
%End

    virtual void clear();

%Docstring
 Reset all headers and content for this response
%End

    virtual QByteArray data() const;

%Docstring
 Get the data written so far

 This is implementation dependent: some implementations may not
 give access to the underlying and return an empty array.

 Note that each call to 'flush' may empty the buffer and in case
 of streaming process you may get partial content
 :rtype: QByteArray
%End

    virtual void truncate();

%Docstring
 Truncate data

 Clear internal buffer
%End

    QByteArray body() const;
%Docstring
 Return body
 :rtype: QByteArray
%End


  private:
    QgsBufferServerResponse( const QgsBufferServerResponse & ) ;
};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/server/qgsbufferserverresponse.h                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
