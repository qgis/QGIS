/***************************************************************************
                          qgsfcgiserverresponse.h

  Define response wrapper for fcgi response
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
#ifndef QGSFCGISERVERRESPONSE_H
#define QGSFCGISERVERRESPONSE_H

#include "qgsserverrequest.h"
#include "qgsserverresponse.h"

#include <QBuffer>

/**
 * \ingroup server
 * QgsFcgiServerResponse
 * Class defining fcgi response
 */
class QgsFcgiServerResponse: public QgsServerResponse
{
  public:

    QgsFcgiServerResponse( QgsServerRequest::Method method = QgsServerRequest::GetMethod );
    ~QgsFcgiServerResponse();

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
     * Set the default headers
     */
    void setDefaultHeaders();

  private:
    QMap<QString, QString> mHeaders;
    QBuffer mBuffer;
    bool mFinished    = false;
    bool mHeadersSent = false;
    QgsServerRequest::Method mMethod;
};

/**
 * \ingroup server
 * QgsFcgiServerResquest
 * Class defining fcgi request
 */
class QgsFcgiServerRequest: public QgsServerRequest
{
  public:
    QgsFcgiServerRequest();
    ~QgsFcgiServerRequest();

    virtual QByteArray data() const override;

    /**
     * Return true if an error occurred during initialization
     */
    bool hasError() const { return mHasError; }

  private:
    void readData();

    QByteArray mData;
    bool       mHasError;
};

#endif





