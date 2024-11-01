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

#define SIP_NO_FILE


#include "qgsserverrequest.h"
#include "qgsserverresponse.h"

#include <QBuffer>
#include <QThread>

/**
 * \ingroup server
 * \class QgsSocketMonitoringThread
 * \brief Thread used to monitor the fcgi socket
 * \since QGIS 3.36
 */
class QgsSocketMonitoringThread : public QThread
{
    Q_OBJECT

  public:
    /**
     * \brief QgsSocketMonitoringThread
     * \param  isResponseFinished
     * \param  feedback
     */
    QgsSocketMonitoringThread( bool *isResponseFinished, QgsFeedback *feedback );
    void run();

  private:
    bool *mIsResponseFinished = nullptr;
    QgsFeedback *mFeedback = nullptr;
    int mIpcFd = -1;
};

/**
 * \ingroup server
 * \class QgsFcgiServerResponse
 * \brief Class defining fcgi response
 */
class SERVER_EXPORT QgsFcgiServerResponse : public QgsServerResponse
{
  public:
    /**
     * Constructor for QgsFcgiServerResponse.
     * \param method The HTTP method (Get by default)
     */
    QgsFcgiServerResponse( QgsServerRequest::Method method = QgsServerRequest::GetMethod );
    virtual ~QgsFcgiServerResponse();

    void setHeader( const QString &key, const QString &value ) override;

    void removeHeader( const QString &key ) override;

    QString header( const QString &key ) const override;

    QMap<QString, QString> headers() const override { return mHeaders; }

    bool headersSent() const override;

    void setStatusCode( int code ) override;

    int statusCode() const override { return mStatusCode; }

    void sendError( int code, const QString &message ) override;

    QIODevice *io() override;

    void finish() override;

    void flush() override;

    void clear() override;

    QByteArray data() const override;

    void truncate() override;

    /**
     * Set the default headers
     */
    void setDefaultHeaders();

    /**
     * Returns socket feedback if any
     * \since QGIS 3.36
     */
    QgsFeedback *feedback() const override { return mFeedback.get(); }

  private:
    QMap<QString, QString> mHeaders;
    QBuffer mBuffer;
    bool mFinished = false;
    bool mHeadersSent = false;
    QgsServerRequest::Method mMethod;
    int mStatusCode = 0;

    std::unique_ptr<QgsSocketMonitoringThread> mSocketMonitoringThread;
    std::unique_ptr<QgsFeedback> mFeedback;
};

#endif
