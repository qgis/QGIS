/***************************************************************************
                          qgsserverresponse.sip

  Define response class for services
  -------------------
  begin                : 2016-12-05
  copyright            : (C) 2016 by David Marteau
  email                : david dot marteau at 3liz dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/**
 * \ingroup server
 * QgsServerResponse
 * Class defining response interface passed to services QgsService::executeRequest() method
 */

class QgsServerResponse
{
%TypeHeaderCode
#include "qgsserverresponse.h"
%End
  public:

    //!constructor
    QgsServerResponse();

    //! destructor
    virtual ~QgsServerResponse();

    /** Set Header entry
     *  Add Header entry to the response
     *  Note that it is usually an error to set Header after writing data
     */
    virtual void setHeader( const QString &key, const QString &value ) = 0;

    /**
     * Clear header
     */
    virtual void removeHeader( const QString &key ) = 0;

    /**
     * Return the header value
     */
    virtual QString header( const QString &key ) const = 0;

    /**
     * Return the response headers
     */
    virtual QMap<QString, QString> headers() const = 0;

    /**
     * Return true if the headers have alredy been sent
     */
    virtual bool headersSent() const = 0;

    /** Return the http status code
     */
    virtual int statusCode( ) const = 0;

    /** Set the http status code
     * @param code HTTP return code value
     */
    virtual void setStatusCode( int code ) = 0;

    /**
     * Send error
     * This method delegates error handling at the server level. This is different
     * from calling setReturnCode() along with and a specific response body.
     * @param code HTTP return code value
     * @param message An informative error message
     */
    virtual void sendError( int code,  const QString &message ) = 0;

    /**
     * Write string
     * This is a convenient method that will write directly
     * to the underlying I/O device
     */
    virtual void write( const QString &data );

    /**
     * Write chunk of data
     * This is a convenient method that will write directly to the
     * underlying I/O device
     * @return the number of bytes that were actually written
     */
    virtual qint64 write( const QByteArray &byteArray );

    /**
     * Write server exception
     */
    virtual void write( const QgsServerException &ex );

    /**
     * Return the underlying QIODevice
     */
    virtual QIODevice *io() = 0;

    /**
     * End the transaction
     */
    virtual void finish() = 0;

    /**
     * Flushes the current output buffer to the network
     *
     * 'flush()' may be called multiple times. For HTTP transactions
     * headers will be written on the first call to 'flush()'.
     */
    virtual void flush() = 0;

    /**
     * Reset all headers and content for this response
     */
    virtual void clear() = 0;

    /**
     * Get the data written so far
     */
    virtual QByteArray data() const = 0;

    /**
     * Truncate internal buffer
     */
    virtual void truncate() = 0;
};

