/***************************************************************************
    qgsnetworkloggernode.h
    -------------------------
    begin                : March 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSNETWORKLOGGERNODE_H
#define QGSNETWORKLOGGERNODE_H

#include "qgsnetworkaccessmanager.h"
#include "devtools/qgsdevtoolsmodelnode.h"
#include <QElapsedTimer>
#include <QVariant>
#include <QColor>
#include <QUrl>
#include <memory>
#include <deque>

class QAction;

/**
 * \ingroup app
 * \class QgsNetworkLoggerRootNode
 * \brief Root node for the network logger model.
 *
 * \since QGIS 3.14
 */
class QgsNetworkLoggerRootNode final : public QgsDevToolsModelGroup
{
  public:

    QgsNetworkLoggerRootNode();
    QVariant data( int role = Qt::DisplayRole ) const override final;

    /**
     * Removes a \a row from the root group.
     */
    void removeRow( int row );

    QVariant toVariant() const override;
};

class QgsNetworkLoggerRequestDetailsGroup;
class QgsNetworkLoggerReplyGroup;
class QgsNetworkLoggerSslErrorGroup;

/**
 * \ingroup app
 * \class QgsNetworkLoggerRequestGroup
 * \brief Parent group for all network requests, showing the request id, type, url,
 * and containing child groups with detailed request and response information.
 *
 * Visually, a QgsNetworkLoggerRequestGroup is structured by:
 *
 * |__ QgsNetworkLoggerRequestGroup (showing id, type (GET etc) url)
 *     |__ QgsNetworkLoggerRequestDetailsGroup (holding Request details)
 *         |__ QgsNetworkLoggerValueNode (key-value pairs with info)
 *             ...
 *         |__ QgsNetworkLoggerRequestQueryGroup (holding query info)
 *               |__ ...
 *         |__ QgsNetworkLoggerRequestHeadersGroup ('Headers')
 *               |__ ...
 *         |__ QgsNetworkLoggerPostContentGroup (showing Data in case of POST)
 *               |__ ...
 *     |__ QgsNetworkLoggerReplyGroup (holding Reply details)
 *         |__ QgsNetworkLoggerReplyHeadersGroup (Reply 'Headers')
 *             |__ ...
 *     |__ QgsNetworkLoggerSslErrorGroup (holding SSL error details, if encountered)
 *         |__ ...
 *
 * \since QGIS 3.14
 */
class QgsNetworkLoggerRequestGroup final : public QgsDevToolsModelGroup
{
  public:

    //! Request statu
    enum class Status
    {
      Pending, //!< Request underway
      Complete, //!< Request was successfully completed
      Error, //!< Request encountered an error
      TimeOut, //!< Request timed out
      Canceled, //!< Request was manually canceled
    };

    /**
     * Constructor for QgsNetworkLoggerRequestGroup, populated from the
     * specified \a request details.
     */
    QgsNetworkLoggerRequestGroup( const QgsNetworkRequestParameters &request );
    QVariant data( int role = Qt::DisplayRole ) const override;
    QList< QAction * > actions( QObject *parent ) override final;
    QVariant toVariant() const override;

    /**
     * Returns the request's status.
     */
    Status status() const { return mStatus; }

    /**
     * Returns the request's URL.
     */
    QUrl url() const { return mUrl; }

    /**
     * Sets the request's URL.
     */
    void setUrl( const QUrl &url );

    /**
     * Returns TRUE if the request was served directly from local cache.
     */
    bool replyFromCache() const { return mReplyFromCache; }

    /**
     * Called to set the \a reply associated with the request.
     *
     * Will automatically create children encapsulating the reply details.
     */
    void setReply( const  QgsNetworkReplyContent &reply );

    /**
     * Flags the reply as having timed out.
     */
    void setTimedOut();

    /**
     * Sets the requests download progress.
     */
    void setProgress( qint64 bytesReceived, qint64 bytesTotal );

    /**
     * Reports any SSL errors encountered while processing the request.
     */
    void setSslErrors( const QList<QSslError> &errors );

    /**
     * Converts a network \a operation to a string value.
     */
    static QString operationToString( QNetworkAccessManager::Operation operation );

    /**
     * Converts a request \a status to a translated string value.
     */
    static QString statusToString( Status status );

    /**
     * Converts a cache load \a control value to a translated string.
     */
    static QString cacheControlToString( QNetworkRequest::CacheLoadControl control );

  private:

    QUrl mUrl;
    int mRequestId = 0;
    QNetworkAccessManager::Operation mOperation;
    QElapsedTimer mTimer;
    qint64 mTotalTime = 0;
    int mHttpStatus = -1;
    QString mContentType;
    qint64 mBytesReceived = 0;
    qint64 mBytesTotal = 0;
    int mReplies = 0;
    QByteArray mData;
    Status mStatus = Status::Pending;
    bool mHasSslErrors = false;
    bool mReplyFromCache = false;
    QList< QPair< QString, QString > > mHeaders;
    QgsNetworkLoggerRequestDetailsGroup *mDetailsGroup = nullptr;
    QgsNetworkLoggerReplyGroup *mReplyGroup = nullptr;
    QgsNetworkLoggerSslErrorGroup *mSslErrorsGroup = nullptr;
};

class QgsNetworkLoggerRequestQueryGroup;
class QgsNetworkLoggerRequestHeadersGroup;
class QgsNetworkLoggerPostContentGroup;

/**
 * \ingroup app
 * \class QgsNetworkLoggerRequestGroup
 * \brief Parent group for all network request details, showing the request parameters
 * and header information.
 *
 * Visually, a QgsNetworkLoggerRequestDetailsGroup is structured by:
 *
 *  |__ QgsNetworkLoggerRequestDetailsGroup (holding Request details)
 *      |__ QgsNetworkLoggerValueNode (key-value pairs with info)
 *          Operation: ...
 *          Thread: ...
 *          Initiator: ...
 *          ID: ...
 *          Cache (control): ...
 *          Cache (save): ...
 *      |__ QgsNetworkLoggerRequestQueryGroup (holding query info)
 *            |__ ...
 *      |__ QgsNetworkLoggerRequestHeadersGroup ('Headers')
 *            |__ ...
 *      |__ QgsNetworkLoggerPostContentGroup (showing Data in case of POST)
 *            |__ ...
 *
 * \since QGIS 3.14
 */
class QgsNetworkLoggerRequestDetailsGroup final : public QgsDevToolsModelGroup
{
  public:

    /**
     * Constructor for QgsNetworkLoggerRequestDetailsGroup, populated from the
     * specified \a request details.
     */
    QgsNetworkLoggerRequestDetailsGroup( const QgsNetworkRequestParameters &request );
    QVariant toVariant() const override;

  private:

    QgsNetworkLoggerRequestQueryGroup *mQueryGroup = nullptr;
    QgsNetworkLoggerRequestHeadersGroup *mRequestHeaders = nullptr;
    QgsNetworkLoggerPostContentGroup *mPostContent = nullptr;
};


/**
 * \ingroup app
 * \class QgsNetworkLoggerRequestHeadersGroup
 * \brief Parent group for all network request header information.
 *
 * Visually, a QgsNetworkLoggerRequestHeadersGroup is structured by:
 *
 *  |__ QgsNetworkLoggerRequestHeadersGroup (holding Request details)
 *      User-Agent: ...
 *      ...
 *
 * \since QGIS 3.14
 */
class QgsNetworkLoggerRequestHeadersGroup final : public QgsDevToolsModelGroup
{
  public:

    /**
     * Constructor for QgsNetworkLoggerRequestHeadersGroup, populated from the
     * specified \a request details.
     */
    QgsNetworkLoggerRequestHeadersGroup( const QgsNetworkRequestParameters &request );

};


/**
 * \ingroup app
 * \class QgsNetworkLoggerRequestQueryGroup
 * \brief Parent group for all network request query information.
 *
 * Visually, a QgsNetworkLoggerRequestQueryGroup is structured by:
 *
 *  |__ QgsNetworkLoggerRequestQueryGroup (holding query info)
 *      query param: value
 *      ...
 *
 * \since QGIS 3.14
 */
class QgsNetworkLoggerRequestQueryGroup final : public QgsDevToolsModelGroup
{
  public:

    /**
     * Constructor for QgsNetworkLoggerRequestQueryGroup, populated from the
     * specified \a url.
     */
    QgsNetworkLoggerRequestQueryGroup( const QUrl &url );

};

/**
 * \ingroup app
 * \class QgsNetworkLoggerPostContentGroup
 * \brief Parent group for all request post data, showing POST data.
 *
 * Visually, a QgsNetworkLoggerPostContentGroup is structured by:
 *
 *  |__ QgsNetworkLoggerPostContentGroup (holding POST data)
 *      |__ Data: POST data
 *
 * \since QGIS 3.14
 */
class QgsNetworkLoggerPostContentGroup final : public QgsDevToolsModelGroup
{
  public:

    /**
     * Constructor for QgsNetworkLoggerPostContentGroup, populated from the
     * specified \a request details.
     */
    QgsNetworkLoggerPostContentGroup( const QgsNetworkRequestParameters &parameters );
};

class QgsNetworkLoggerReplyHeadersGroup;

/**
 * \ingroup app
 * \class QgsNetworkLoggerReplyGroup
 * \brief Parent group for all network replies, showing the reply details.
 *
 * Visually, a QgsNetworkLoggerReplyGroup is structured by:
 *
 * |__ QgsNetworkLoggerReplyGroup (holding Reply details)
 *     |__ Status: reply status (e.g. 'Canceled')
 *     |__ Error code: code (if applicable)
 *     |__ Cache (result): whether reply was taken from cache or network
 *     |__ QgsNetworkLoggerReplyHeadersGroup (Reply 'Headers')
 *         |__ ...
 *
 * \since QGIS 3.14
 */
class QgsNetworkLoggerReplyGroup final : public QgsDevToolsModelGroup
{
  public:

    /**
     * Constructor for QgsNetworkLoggerReplyGroup, populated from the
     * specified \a reply details.
     */
    QgsNetworkLoggerReplyGroup( const QgsNetworkReplyContent &reply );
    QVariant toVariant() const override;

  private:

    QgsNetworkLoggerReplyHeadersGroup *mReplyHeaders = nullptr;

};

/**
 * \ingroup app
 * \class QgsNetworkLoggerReplyHeadersGroup
 * \brief Parent group for network reply headers, showing the reply header details.
 *
 * Visually, a QgsNetworkLoggerReplyHeadersGroup is structured by:
 *
 * |__ QgsNetworkLoggerReplyHeadersGroup (holding reply Header details)
 *      Content-Type: ...
 *      Content-Length: ...
 *      ...
 *
 * \since QGIS 3.14
 */
class QgsNetworkLoggerReplyHeadersGroup final : public QgsDevToolsModelGroup
{
  public:

    /**
     * Constructor for QgsNetworkLoggerReplyHeadersGroup, populated from the
     * specified \a reply details.
     */
    QgsNetworkLoggerReplyHeadersGroup( const QgsNetworkReplyContent &reply );

};

/**
 * \ingroup app
 * \class QgsNetworkLoggerSslErrorNode
 * \brief Parent group for SSQL errors, showing the error details.
 *
 * Visually, a QgsNetworkLoggerSslErrorGroup is structured by:
 *
 * |__ QgsNetworkLoggerSslErrorGroup (holding error details)
 *      Error: ...
 *      Error: ...
 *      ...
 *
 * \since QGIS 3.14
 */
class QgsNetworkLoggerSslErrorGroup final : public QgsDevToolsModelGroup
{
  public:

    /**
     * Constructor for QgsNetworkLoggerSslErrorGroup, populated from the
     * specified \a errors.
     */
    QgsNetworkLoggerSslErrorGroup( const QList<QSslError> &errors );
    QVariant data( int role = Qt::DisplayRole ) const override;
};



#endif // QGSNETWORKLOGGERNODE_H
