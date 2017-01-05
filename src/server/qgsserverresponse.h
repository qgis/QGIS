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

#include <QString>
#include <QIODevice>

/**
 * \ingroup server
 * QgsServerResponse
 * Class defining response interface passed to services QgsService::executeRequest() method
 */

// Note:
// This class is intended to be used from python code: method signatures and return types should be
// compatible with pyQGIS/pyQT types and rules.

class SERVER_EXPORT QgsServerResponse
{
  public:

    //!constructor
    QgsServerResponse();

    //! destructor
    virtual ~QgsServerResponse();

    /**
     *  Set Header entry
     *  Add Header entry to the response
     *  Note that it is usually an error to set Header after writing data
     */
    virtual void setHeader( const QString& key, const QString& value ) = 0;

    /**
     * Clear header
     * Undo a previous 'set_header' call
     */
    virtual void clearHeader( const QString& key ) = 0;

    /**
     * Return the header value
     */
    virtual QString getHeader( const QString& key ) const = 0;

    /**
     * Return the list of all header keys
     */
    virtual QList<QString> headerKeys() const = 0;

    /**
     * Return true if the headers have alredy beeing written
     */
    virtual bool headersWritten() const = 0;


    /** Set the http return code
     * @param code HTTP return code value
     */
    virtual void setReturnCode( int code ) = 0;

    /**
     * Send error
     * This method delegates error handling at the server level. This is different
     * from calling setReturnCode() which let you return a specific response body.
     * Calling sendError() will end the transaction and any attempt to write data
     * or set headers will be an error.
     * @param code HHTP return code value
     * @param message An informative error message
     */
    virtual void sendError( int code,  const QString& message ) = 0;

    /**
     * Write string
     * This is a convenient method that will write directly
     * to the underlying I/O device
     */
    virtual void write( const QString& data );

    /**
     * Write chunk of data
     * This is a convenient method that will write directly
     * to the underlying I/O device
     * @return the number of bytes that were actually written
     */
    virtual qint64 write( const QByteArray &byteArray );

    /**
     * Writes at most maxSize bytes of data
     *
     * This is a convenient method that will write directly
     * to the underlying I/O device
     * @return the number of bytes written
     *
     *  @note not available in python bindings
     */
    virtual qint64 write( const char* data, qint64 maxsize );

    /**
     * Writes at most maxSize bytes of data
     *
     * This is a convenient method that will write directly
     * to the underlying I/O device
     * @return the number of bytes written
     *
     * @note not available in python bindings
     */
    virtual qint64 write( const char* data );

    /**
     * Return the underlying QIODevice
     */
    virtual QIODevice* io() = 0;

    /**
     * Finish the response,  ending the transaction
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
};

#endif
