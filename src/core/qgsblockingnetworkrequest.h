/***************************************************************************
    qgsblockingnetworkrequest.h
    ---------------------------
    begin                : November 2018
    copyright            : (C) 2018 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSBLOCKINGNETWORKREQUEST_H
#define QGSBLOCKINGNETWORKREQUEST_H

#include "qgis_core.h"
#include "qgsnetworkreply.h"
#include "qgsfeedback.h"
#include <QThread>
#include <QObject>
#include <functional>
#include <QPointer>

class QNetworkRequest;
class QNetworkReply;

/**
 * A thread safe class for performing blocking (sync) network requests, with full support for QGIS proxy
 * and authentication settings.
 *
 * This class should be used whenever a blocking network request is required. Unlike implementations
 * which rely on QApplication::processEvents() or creation of a QEventLoop, this class is completely
 * thread safe and can be used on either the main thread or background threads without issue.
 *
 * Redirects are automatically handled by the class.
 *
 * After completion of a request, the reply content should be retrieved by calling getReplyContent().
 * This method returns a QgsNetworkReplyContent container, which is safe and cheap to copy and pass
 * between threads without issue.
 *
 * \ingroup core
 * \since QGIS 3.6
 */
class CORE_EXPORT QgsBlockingNetworkRequest : public QObject
{
    Q_OBJECT
  public:

    //! Error codes
    enum ErrorCode
    {
      NoError, //!< No error was encountered
      NetworkError, //!< A network error occurred
      TimeoutError, //!< Timeout was reached before a reply was received
      ServerExceptionError, //!< An exception was raised by the server
    };

    //! Constructor for QgsBlockingNetworkRequest
    explicit QgsBlockingNetworkRequest();

    ~QgsBlockingNetworkRequest() override;

    /**
     * Performs a "get" operation on the specified \a request.
     *
     * If \a forceRefresh is false then previously cached replies may be used for the request. If
     * it is set to true then a new query is always performed.
     *
     * If an authCfg() has been set, then any authentication configuration required will automatically be applied to
     * \a request. There is no need to manually apply the authentication to the request prior to calling
     * this method.
     *
     * The optional \a feedback argument can be used to abort ongoing requests.
     *
     * The method will return NoError if the get operation was successful. The contents of the reply can be retrieved
     * by calling reply().
     *
     * If an error was encountered then a specific ErrorCode will be returned, and a detailed error message
     * can be retrieved by calling errorMessage().
     *
     * \see post()
     */
    ErrorCode get( QNetworkRequest &request, bool forceRefresh = false, QgsFeedback *feedback = nullptr );

    /**
     * Performs a "post" operation on the specified \a request, using the given \a data.
     *
     * If \a forceRefresh is false then previously cached replies may be used for the request. If
     * it is set to true then a new query is always performed.
     *
     * If an authCfg() has been set, then any authentication configuration required will automatically be applied to
     * \a request. There is no need to manually apply the authentication to the request prior to calling
     * this method.
     *
     * The optional \a feedback argument can be used to abort ongoing requests.
     *
     * The method will return NoError if the get operation was successful. The contents of the reply can be retrieved
     * by calling reply().
     *
     * If an error was encountered then a specific ErrorCode will be returned, and a detailed error message
     * can be retrieved by calling errorMessage().
     *
     * \see get()
     */
    ErrorCode post( QNetworkRequest &request, const QByteArray &data, bool forceRefresh = false, QgsFeedback *feedback = nullptr );

    /**
     * Returns the error message string, after a get() or post() request has been made.\
     */
    QString errorMessage() const { return mErrorMessage; }

    /**
     * Returns the content of the network reply, after a get() or post() request has been made.
     */
    QgsNetworkReplyContent reply() const { return mReplyContent; }

    /**
     * Returns the authentication config id which will be used during the request.
     * \see setAuthCfg()
     */
    QString authCfg() const;

    /**
     * Sets the authentication config id which should be used during the request.
     * \see authCfg()
     */
    void setAuthCfg( const QString &authCfg );

  public slots:

    /**
     * Aborts the network request immediately.
     */
    void abort();

  signals:

    /**
     * Emitted when when data arrives during a request.
     */
    void downloadProgress( qint64, qint64 );

    /**
     * Emitted once a request has finished downloading.
     */
    void downloadFinished();

  private slots:
    void replyProgress( qint64, qint64 );
    void replyFinished();
    void requestTimedOut( QNetworkReply *reply );

  private :

    enum Method
    {
      Get,
      Post
    };

    //! The reply to the request
    QNetworkReply *mReply = nullptr;

    Method mMethod = Get;
    QByteArray mPostData;

    //! Authentication configuration ID
    QString mAuthCfg;

    //! The error message associated with the last error.
    QString mErrorMessage;

    //! Error code
    ErrorCode mErrorCode = NoError;

    QgsNetworkReplyContent mReplyContent;

    //! Whether the request is aborted.
    bool mIsAborted = false;

    //! Whether to force refresh (i.e. issue a network request and not use cache)
    bool mForceRefresh = false;

    //! Whether the request has timed-out
    bool mTimedout = false;

    //! Whether we already received bytes
    bool mGotNonEmptyResponse = false;

    int mExpirationSec = 30;

    QPointer< QgsFeedback > mFeedback;

    ErrorCode doRequest( Method method, QNetworkRequest &request, bool forceRefresh, QgsFeedback *feedback = nullptr );

    QString errorMessageFailedAuth();

};

///@cond PRIVATE
#ifndef SIP_RUN

class DownloaderThread : public QThread
{
    Q_OBJECT

  public:
    DownloaderThread( const std::function<void()> &function, QObject *parent = nullptr )
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

#endif
///@endcond

#endif // QGSBLOCKINGNETWORKREQUEST_H
