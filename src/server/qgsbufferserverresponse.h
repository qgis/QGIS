/***************************************************************************
                          qgsfcgiserverresponse.h

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

#include "qgsserverresponse.h"
#include "qgsserverrequest.h"

#include <QBuffer>
#include <QByteArray>
#include <QMap>

/**
 * \ingroup server
 * \class QgsBufferServerResponse
 * Class defining buffered response
 */
class QgsBufferServerResponse: public QgsServerResponse
{
  public:

    QgsBufferServerResponse();
    ~QgsBufferServerResponse();

    virtual void setHeader( const QString& key, const QString& value ) override;

    virtual void clearHeader( const QString& key ) override;

    virtual QString getHeader( const QString& key ) const override;

    virtual QList<QString> headerKeys() const override;

    virtual bool headersSent() const override;

    virtual void setReturnCode( int code ) override;

    virtual void sendError( int code,  const QString& message ) override;

    virtual QIODevice* io() override;

    virtual void finish() override;

    virtual void flush() override;

    virtual void clear() override;

    /**
     * Return body
     */
    QByteArray body() const { return mBody; }

    /**
     * Return header's map
     */
    QMap<QString, QString> headers() const { return mHeaders; }

    /**
     * Return the status code
     */
    int returnCode() const { return mReturnCode; }

  private:
    QMap<QString, QString> mHeaders;
    QBuffer                mBuffer;
    QByteArray             mBody;
    bool                   mFinished = false;
    bool                   mHeadersSent = false;
    int                    mReturnCode = 200;
};

/**
 * \ingroup server
 * QgsBufferServerRequest
 * Class defining request with  data
 */
class QgsBufferServerRequest : public QgsServerRequest
{
  public:

    /**
    * Constructor
    *
    * @param url the url string
    * @param method the request method
    */
    QgsBufferServerRequest( const QString& url, Method method = GetMethod, QByteArray* data = nullptr );

    /**
     * Constructor
     *
     * @param url QUrl
     * @param method the request method
     */
    QgsBufferServerRequest( const QUrl& url, Method method = GetMethod, QByteArray* data = nullptr );

    ~QgsBufferServerRequest();

    virtual QByteArray data() const { return mData; }

  private:
    QByteArray mData;
};

#endif





