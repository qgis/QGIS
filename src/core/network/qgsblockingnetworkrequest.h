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
 * \brief A thread safe class for performing blocking (sync) network requests, with full support for QGIS proxy
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

    /**
     * Request flags
     *
     * \since QGIS 3.40
     */
    enum class RequestFlag : int SIP_ENUM_BASETYPE( IntFlag )
    {
      EmptyResponseIsValid = 1 << 0, //!< Do not generate an error if getting an empty response (e.g. HTTP 204)
    };
    Q_ENUM( RequestFlag )
    Q_DECLARE_FLAGS( RequestFlags, RequestFlag )
    Q_FLAG( RequestFlags )

    //! Constructor for QgsBlockingNetworkRequest
    explicit QgsBlockingNetworkRequest();

    ~QgsBlockingNetworkRequest() override;

    /**
     * Performs a "get" operation on the specified \a request.
     *
     * If \a forceRefresh is FALSE then previously cached replies may be used for the request. If
     * it is set to TRUE then a new query is always performed.
     *
     * If an authCfg() has been set, then any authentication configuration required will automatically be applied to
     * \a request. There is no need to manually apply the authentication to the request prior to calling
     * this method.
     *
     * The optional \a feedback argument can be used to abort ongoing requests.
     *
     * The optional \a requestFlags argument can be used to modify the behavior (added in QGIS 3.40).
     *
     * The method will return NoError if the get operation was successful. The contents of the reply can be retrieved
     * by calling reply().
     *
     * If an error was encountered then a specific ErrorCode will be returned, and a detailed error message
     * can be retrieved by calling errorMessage().
     *
     * \see post()
     */
    ErrorCode get( QNetworkRequest &request, bool forceRefresh = false, QgsFeedback *feedback = nullptr, RequestFlags requestFlags = QgsBlockingNetworkRequest::RequestFlags() );

    /**
     * Performs a "post" operation on the specified \a request, using the given \a data.
     *
     * If \a forceRefresh is FALSE then previously cached replies may be used for the request. If
     * it is set to TRUE then a new query is always performed.
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
     * \since QGIS 3.22
     */
    ErrorCode post( QNetworkRequest &request, QIODevice *data, bool forceRefresh = false, QgsFeedback *feedback = nullptr );

    /**
     * This is an overloaded function.
     *
     * Performs a "post" operation on the specified \a request, using the given \a data.
     */
    ErrorCode post( QNetworkRequest &request, const QByteArray &data, bool forceRefresh = false, QgsFeedback *feedback = nullptr );

    /**
     * Performs a "head" operation on the specified \a request.
     *
     * If \a forceRefresh is FALSE then previously cached replies may be used for the request. If
     * it is set to TRUE then a new query is always performed.
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
     * \since QGIS 3.18
     */
    ErrorCode head( QNetworkRequest &request, bool forceRefresh = false, QgsFeedback *feedback = nullptr );

    /**
     * Performs a "put" operation on the specified \a request, using the given \a data.
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
     * \since QGIS 3.22
     */
    ErrorCode put( QNetworkRequest &request, QIODevice *data, QgsFeedback *feedback = nullptr );

    /**
     * This is an overloaded function.
     *
     * Performs a "put" operation on the specified \a request, using the given \a data.
     * \since QGIS 3.18
     */
    ErrorCode put( QNetworkRequest &request, const QByteArray &data, QgsFeedback *feedback = nullptr );

    /**
     * Performs a "delete" operation on the specified \a request.
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
     * \since QGIS 3.18
     */
    ErrorCode deleteResource( QNetworkRequest &request, QgsFeedback *feedback = nullptr );

    /**
     * Returns the error message string, after a get(), post(), head() or put() request has been made.
     */
    QString errorMessage() const { return mErrorMessage; }

    /**
     * Returns the content of the network reply, after a get(), post(), head() or put() request has been made.
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
    void downloadProgress( qint64 bytesReceived, qint64 bytesTotal );

    /**
     * Emitted once a request has finished downloading.
     * \deprecated QGIS 3.40. Use the finished() signal instead.
     */
    Q_DECL_DEPRECATED void downloadFinished() SIP_DEPRECATED;

    /**
     * Emitted when when data are sent during a request.
     * \since QGIS 3.22
     */
    void uploadProgress( qint64 bytesReceived, qint64 bytesTotal );

    /**
     * Emitted once a request has finished.
     */
    void finished();

  private slots:
    void replyProgress( qint64, qint64 );
    void replyFinished();
    void requestTimedOut( QNetworkReply *reply );

  private :

    enum Method
    {
      Get,
      Post,
      Head,
      Put,
      Delete
    };

    //! The reply to the request
    QNetworkReply *mReply = nullptr;

    Method mMethod = Get;

    //! payload data used in PUT/POST request
    QIODevice *mPayloadData;

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

    //! Request flags
    RequestFlags mRequestFlags;

    int mExpirationSec = 30;

    QPointer< QgsFeedback > mFeedback;

    ErrorCode doRequest( Method method, QNetworkRequest &request, bool forceRefresh, QgsFeedback *feedback = nullptr, RequestFlags requestFlags = RequestFlags() );

    QString errorMessageFailedAuth();

    void sendRequestToNetworkAccessManager( const QNetworkRequest &request );

    void abortIfNotPartialContentReturned();
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
