/***************************************************************************
                          qgsbufferserverresponse.h

  Define response wrapper for storing responsea in buffer
  -------------------
  begin                : 2017-01-03
  copyright            : (C) 2017 by David Marteau
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
#ifndef QGSBUFFERSERVERRESPONSE_H
#define QGSBUFFERSERVERRESPONSE_H

#include "qgis_server.h"
#include "qgis_sip.h"
#include "qgsserverresponse.h"

#include <QBuffer>
#include <QByteArray>
#include <QMap>
#include <QString>

/**
 * \ingroup server
 * \class QgsBufferServerResponse
 * \brief Defines a buffered server response.
 */
class SERVER_EXPORT QgsBufferServerResponse : public QgsServerResponse
{
  public:
    QgsBufferServerResponse();
    QgsBufferServerResponse( const QgsBufferServerResponse & ) = delete;

    /**
     *  Set Header entry
     *  Add Header entry to the response
     *  Note that it is usually an error to set Header after data have been sent through the wire
     */
    void setHeader( const QString &key, const QString &value ) override;

    /**
     *  Add a header \a value for the given \a key, without replacing any existing value for the same \a key
     *  Add Header entry to the response
     *  \note that it is usually an error to set Header after data have been sent through the wire
     *  \since QGIS 4.2
     */
    void addHeader( const QString &key, const QString &value ) override;

    /**
     * Clear header
     * Undo a previous 'setHeader' call
     */
    void removeHeader( const QString &key ) override;

    /**
     * Returns a single header value, or an empty string if the header is not set.
     * If multiple values are set for the same header, only the last value is returned.
     * \deprecated QGIS 4.2. Use fullHeader() instead.
     */
    Q_DECL_DEPRECATED QString header( const QString &key ) const override SIP_DEPRECATED;

    /**
     * Returns all the headers
     */
    QMap<QString, QList<QString>> fullHeaders() const override { return mHeaders; }

    /**
     * Returns all the values for a header \a key
     */
    virtual QList<QString> fullHeader( const QString &key ) const override;

    /**
     * Returns all the headers as a map: only the last value is returned if multiple values are set for the same header name.
     * \deprecated QGIS 4.2. Use fullHeaders() instead.
     */
    Q_DECL_DEPRECATED QMap<QString, QString> headers() const override SIP_DEPRECATED;

    /**
     * Returns TRUE if the headers have already been sent
     */
    bool headersSent() const override;

    /**
     * Set the http status code
     * \param code HTTP status code value
     */
    void setStatusCode( int code ) override;

    /**
     * Returns the http status code
     */
    int statusCode() const override { return mStatusCode; }

    /**
     * Send error
     * This method delegates error handling at the server level. This is different
     * from calling setReturnCode() which let you return a specific response body.
     * Calling sendError() will end the transaction and any attempt to write data
     * or set headers will be an error.
     * \param code HHTP return code value
     * \param message An informative error message
     */
    void sendError( int code, const QString &message ) override;

    /**
     * Returns the underlying QIODevice
     */
    QIODevice *io() override;

    /**
     * Finish the response,  ending the transaction
     */
    void finish() override;

    /**
     * Flushes the current output buffer to the network
     *
     * 'flush()' may be called multiple times. For HTTP transactions
     * headers will be written on the first call to 'flush()'.
     */
    void flush() override;

    /**
     * Reset all headers and content for this response
     */
    void clear() override;

    /**
     * Gets the data written so far
     *
     * This is implementation dependent: some implementations may not
     * give access to the underlying and return an empty array.
     *
     * Note that each call to 'flush' may empty the buffer and in case
     * of streaming process you may get partial content
     */
    QByteArray data() const override;

    /**
     * Truncate data
     *
     * Clear internal buffer
     */
    void truncate() override;

    /**
     * Returns body
     */
    QByteArray body() const { return mBody; }


  private:
#ifdef SIP_RUN
    QgsBufferServerResponse( const QgsBufferServerResponse & ) SIP_FORCE;
#endif

    QMap<QString, QList<QString>> mHeaders;
    QBuffer mBuffer;
    QByteArray mBody;
    bool mFinished = false;
    bool mHeadersSent = false;
    int mStatusCode = 200;
};

#endif
