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

/**
  \class QgsNetworkContentFetcher
  \ingroup core
  \brief HTTP network content fetcher. A simple method for fetching remote HTTP content
  and converting the content to standard formats. Url redirects are automatically
  handled.
  \since 2.5
*/

class CORE_EXPORT QgsNetworkContentFetcher : public QObject
{
    Q_OBJECT

  public:
    QgsNetworkContentFetcher();

    virtual ~QgsNetworkContentFetcher();

    /**Fetches content from a remote URL and handles redirects. The finished()
     * signal will be emitted when content has been fetched.
     * @param url URL to fetch
    */
    void fetchContent( const QUrl url );

    /**Returns a reference to the network reply
     * @returns QNetworkReply for fetched URL content
    */
    QNetworkReply* reply();

    /**Returns the fetched content as a string
     * @returns string containing network content
    */
    QString contentAsString() const;

  signals:

    /**Emitted when content has loaded
    */
    void finished();

  private:

    QNetworkReply* mReply;

    bool mContentLoaded;

  private slots:

    /**Called when fetchUrlContent has finished loading a url. If
     * result is a redirect then the redirect is fetched automatically.
    */
    void contentLoaded( bool ok = true );

};

#endif
