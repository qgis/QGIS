/***************************************************************************
    qgsbasenetworkrequest.h
    ---------------------
    begin                : February 2016
    copyright            : (C) 2016 by Even Rouault
    email                : even.rouault at spatialys.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSBASENETWORKREQUEST_H
#define QGSBASENETWORKREQUEST_H

#include <functional>

#include <QObject>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QAuthenticator>

#include "qgsauthorizationsettings.h"

//! Abstract base class for a WFS request.
class QgsBaseNetworkRequest : public QObject
{
    Q_OBJECT
  public:
    explicit QgsBaseNetworkRequest( const QgsAuthorizationSettings &auth, const QString &translatedComponent );

    ~QgsBaseNetworkRequest() override;

    //! \brief proceed to sending a GET request
    bool sendGET( const QUrl &url, const QString &acceptHeader, bool synchronous, bool forceRefresh = false, bool cache = true );

    //! \brief proceed to sending a synchronous POST request
    bool sendPOST( const QUrl &url, const QString &contentTypeHeader, const QByteArray &data );

    //! Set whether to log error messages.
    void setLogErrors( bool enabled ) { mLogErrors = enabled; }

    enum ErrorCode { NoError,
                     NetworkError,
                     TimeoutError,
                     ServerExceptionError,
                     ApplicationLevelError
                   };

    //! Returns the error code (after download/post)
    ErrorCode errorCode() const { return mErrorCode; }

    //! Returns the error message (after download/post)
    QString errorMessage() const { return mErrorMessage; }

    //! Returns the server response (after download/post)
    QByteArray response() const { return mResponse; }

  public slots:
    //! Abort network request immediately
    void abort();

  signals:
    //! \brief emit a signal when data arrives
    void downloadProgress( qint64, qint64 );

    //! \brief emit a signal once the download is finished
    void downloadFinished();

  protected slots:
    void replyProgress( qint64, qint64 );
    void replyFinished();
    void replyReadyRead();
    void requestTimedOut( QNetworkReply *reply );

  protected:
    //! Authorization
    QgsAuthorizationSettings mAuth;

    //! Translated name of the component (for error messages)
    QString mTranslatedComponent;

    //! The reply to the request
    QNetworkReply *mReply = nullptr;

    //! The error message associated with the last error.
    QString mErrorMessage;

    //! Error code
    ErrorCode mErrorCode = QgsBaseNetworkRequest::NoError;

    //! Raw response
    QByteArray mResponse;

    //! Whether the request is aborted.
    bool mIsAborted = false;

    //! Whether to force refresh (i.e. issue a network request and not use cache)
    bool mForceRefresh = false;

    //! Whether the request has timed-out
    bool mTimedout = false;

    //! Whether we already received bytes
    bool mGotNonEmptyResponse = false;

    //! Whether to log error messages
    bool mLogErrors = true;

  protected:

    /**
     * Returns (translated) error message, composed with a
     * (possibly translated, but sometimes coming from server) reason
     */
    virtual QString errorMessageWithReason( const QString &reason ) = 0;

    //! Returns experiation delay in second
    virtual int defaultExpirationInSec() { return 0; }

  private:
    QString errorMessageFailedAuth();

    void logMessageIfEnabled();
};


class DownloaderThread : public QThread
{
    Q_OBJECT

  public:
    DownloaderThread( std::function<void()> function, QObject *parent = nullptr )
      : QThread( parent )
      , mFunction( function )
    {
    }

    void run() override
    {
      mFunction();
    }

  private:
    std::function<void()> mFunction;
};

#endif // QGSBASENETWORKREQUEST_H
