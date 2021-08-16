/***************************************************************************
                       qgsnetworkcontentfetcher.h
                             -------------------
    begin                : July, 2014
    copyright            : (C) 2014 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef QGSNETWORKCONTENTFETCHER_H
#define QGSNETWORKCONTENTFETCHER_H

#include <QNetworkReply>
#include <QUrl>

#include "qgis_core.h"

class QTextCodec;

/**
 * \class QgsNetworkContentFetcher
 * \ingroup core
 * \brief HTTP network content fetcher. A simple method for fetching remote HTTP content
 * and converting the content to standard formats. Url redirects are automatically
 * handled.
 * \see QgsNetworkContentFetcherTask
 * \since QGIS 2.5
*/
class CORE_EXPORT QgsNetworkContentFetcher : public QObject
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsNetworkContentFetcher.
     */
    QgsNetworkContentFetcher() = default;

    ~QgsNetworkContentFetcher() override;

    /**
     * Fetches content from a remote URL and handles redirects. The finished()
     * signal will be emitted when content has been fetched.
     * \param url URL to fetch
     * \param authcfg optional authentication configuration
     */
    void fetchContent( const QUrl &url, const QString &authcfg = QString() );

    /**
     * Fetches content using a network \a request and handles redirects. The finished()
     * signal will be emitted when content has been fetched.
     *
     * Optionally, authentication configuration can be set via the \a authcfg argument.
     *
     * \since QGIS 3.2
     */
    void fetchContent( const QNetworkRequest &request, const QString &authcfg = QString() );

    /**
     * Returns a reference to the network reply
     * \returns QNetworkReply for fetched URL content
     */
    QNetworkReply *reply();

    /**
     * Returns the fetched content as a string
     * \returns string containing network content
     */
    QString contentAsString() const;

    /**
     * Cancels any ongoing request.
     * \since QGIS 3.2
     */
    void cancel();

    /**
     * Returns TRUE if the fetching was canceled.
     *
     * \since QGIS 3.10
     */
    bool wasCanceled() const;

  signals:

    /**
     * Emitted when content has loaded
     */
    void finished();

    /**
     * Emitted when data is received.
     * \since QGIS 3.2
     */
    void downloadProgress( qint64 bytesReceived, qint64 bytesTotal );

    /**
     * Emitted when an error with \a code error occurred while processing the request
     * \a errorMsg is a textual description of the error
     * \since QGIS 3.22
     */
    void errorOccurred( QNetworkReply::NetworkError code, const QString &errorMsg );

  private:

    QString mAuthCfg;
    QNetworkReply *mReply = nullptr;

    bool mContentLoaded = false;

    bool mIsCanceled = false;

    /**
     * Tries to create a text codec for decoding html content. Works around bugs in Qt's built in method.
     * \param array input html byte array
     * \returns QTextCodec for html content, if detected
     */
    QTextCodec *codecForHtml( QByteArray &array ) const;

  private slots:

    /**
     * Called when fetchUrlContent has finished loading a url. If
     * result is a redirect then the redirect is fetched automatically.
     */
    void contentLoaded( bool ok = true );

};

#endif
