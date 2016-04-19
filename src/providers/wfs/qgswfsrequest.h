/***************************************************************************
    qgswfsrequest.h
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
#ifndef QGSWFSREQUEST_H
#define QGSWFSREQUEST_H

#include <QObject>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>

#include "qgswfsdatasourceuri.h"

/** Abstract base class for a WFS request. */
class QgsWFSRequest : public QObject
{
    Q_OBJECT
  public:
    explicit QgsWFSRequest( const QString& theUri );

    virtual ~QgsWFSRequest();

    /** \brief proceed to sending a GET request */
    bool sendGET( const QUrl& url, bool synchronous, bool forceRefresh = false, bool cache = true );

    /** \brief proceed to sending a synchronous POST request */
    bool sendPOST( const QUrl& url, const QString& contentTypeHeader, const QByteArray& data );

    enum ErrorCode { NoError,
                     NetworkError,
                     TimeoutError,
                     XmlError,
                     ServerExceptionError,
                     WFSVersionNotSupported
                 };

    /** \brief Return error code (after download/post) */
    ErrorCode errorCode() const { return mErrorCode; }

    /** \brief Return error message (after download/post) */
    const QString& errorMessage() const { return mErrorMessage; }

    /** \brief Return server response (after download/post) */
    const QByteArray& response() const { return mResponse; }

  public slots:
    /** Abort network request immediately */
    void abort();

  signals:
    /** \brief emit a signal when data arrives */
    void downloadProgress( qint64, qint64 );

    /** \brief emit a signal once the download is finished */
    void downloadFinished();

  protected slots:
    void replyProgress( qint64, qint64 );
    void replyFinished();
    void requestTimedOut( QNetworkReply* reply );

  protected:
    /** URI */
    QgsWFSDataSourceURI mUri;

    /** The reply to the request */
    QNetworkReply *mReply;

    /** The error message associated with the last error. */
    QString mErrorMessage;

    /** Error code */
    ErrorCode mErrorCode;

    /** Raw response */
    QByteArray mResponse;

    /** Whether the request is aborted. */
    bool mIsAborted;

    /** Whether to force refresh (i.e. issue a network request and not use cache) */
    bool mForceRefresh;

    /** Whether the request has timed-out */
    bool mTimedout;

    /** Whether we already received bytes */
    bool mGotNonEmptyResponse;

  protected:

    //! base service URL
    QUrl baseURL() const { return mUri.baseURL(); }

    /** Return (translated) error message, composed with a
        (possibly translated, but sometimes coming from server) reason  */
    virtual QString errorMessageWithReason( const QString& reason ) = 0;

    /** Return experiation delay in second */
    virtual int defaultExpirationInSec() { return 0; }

  private:
    QString errorMessageFailedAuth();

};

#endif // QGSWFSREQUEST_H
