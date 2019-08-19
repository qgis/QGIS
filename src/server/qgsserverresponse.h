/***************************************************************************
                          qgsserverresponse.h

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
#ifndef QGSSERVERRESPONSE_H
#define QGSSERVERRESPONSE_H

#include "qgis_server.h"
#include "qgis_sip.h"
#include "qgsserverexception.h"

#include <QString>
#include <QIODevice>

class QgsServerException;

/**
 * \ingroup server
 * QgsServerResponse
 * Class defining response interface passed to services QgsService::executeRequest() method
 *
 * \since QGIS 3.0
 */

// Note:
// This class is intended to be used from Python code: method signatures and return types should be
// compatible with pyQGIS/pyQT types and rules.

class SERVER_EXPORT QgsServerResponse
{
  public:

    //!constructor
    QgsServerResponse() = default;

    //! destructor
    virtual ~QgsServerResponse() = default;

    /**
     *  Set Header entry
     *  Add Header entry to the response
     *  Note that it is usually an error to set Header after data have been sent through the wire
     */
    virtual void setHeader( const QString &key, const QString &value ) = 0;

    /**
     * Clear header
     * Undo a previous 'setHeader' call
     */
    virtual void removeHeader( const QString &key ) = 0;

    /**
     * Returns the header value
     */
    virtual QString header( const QString &key ) const = 0;

    /**
     * Returns the header value
     */
    virtual QMap<QString, QString> headers() const = 0;

    /**
     * Returns TRUE if the headers have already been sent
     */
    virtual bool headersSent() const = 0;


    /**
     * Set the http status code
     * \param code HTTP status code value
     */
    virtual void setStatusCode( int code ) = 0;

    /**
     * Returns the http status code
     */
    virtual int statusCode() const = 0;

    /**
     * Send error
     * This method delegates error handling at the server level. This is different
     * from calling setReturnCode() which let you return a specific response body.
     * Calling sendError() will end the transaction and any attempt to write data
     * or set headers will be an error.
     * \param code HHTP return code value
     * \param message An informative error message
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
     * This is a convenient method that will write directly
     * to the underlying I/O device
     * \returns the number of bytes that were actually written
     */
    virtual qint64 write( const QByteArray &byteArray );

    /**
     * Writes at most maxSize bytes of data
     *
     * This is a convenient method that will write directly
     * to the underlying I/O device
     * \returns the number of bytes written
     *
     *  \note not available in Python bindings
     */
    virtual qint64 write( const char *data, qint64 maxsize ) SIP_SKIP;

    /**
     * Writes at most maxSize bytes of data
     *
     * This is a convenient method that will write directly
     * to the underlying I/O device
     * \returns the number of bytes written
     *
     * \note not available in Python bindings
     */
    virtual qint64 write( const char *data ) SIP_SKIP;

    /**
     * Writes at most maxSize bytes of data
     *
     * This is a convenient method that will write directly
     * to the underlying I/O device
     * \returns the number of bytes written
     * \note not available in Python bindings
     * \since QGIS 3.10
     */
    virtual qint64 write( std::string data ) SIP_SKIP;

    /**
     * Write server exception
     */
    virtual void write( const QgsServerException &ex );

    /**
     * Returns the underlying QIODevice
     */
    virtual QIODevice *io() = 0;

    /**
     * Finish the response, ending the transaction. The default implementation does nothing.
     */
    virtual void finish() SIP_THROW( QgsServerException ) SIP_VIRTUALERRORHANDLER( server_exception_handler );

    /**
     * Flushes the current output buffer to the network
     *
     * 'flush()' may be called multiple times. For HTTP transactions
     * headers will be written on the first call to 'flush()'.
     * The default implementation does nothing.
     */
    virtual void flush() SIP_THROW( QgsServerException ) SIP_VIRTUALERRORHANDLER( server_exception_handler );

    /**
     * Reset all headers and content for this response
     */
    virtual void clear() = 0;

    /**
     * Gets the data written so far
     *
     * This is implementation dependent: some implementations may not
     * give access to the underlying and return an empty array.
     *
     * Note that each call to 'flush' may empty the buffer and in case
     * of streaming process you may get partial content
     */
    virtual QByteArray data() const = 0;

    /**
     * Truncate data
     *
     * Clear internal buffer
     */
    virtual void truncate() = 0;
};

#endif
