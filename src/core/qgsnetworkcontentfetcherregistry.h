/***************************************************************************
                       qgsnetworkcontentfetcherregistry.h
                             -------------------
    begin                : April, 2018
    copyright            : (C) 2018 by Denis Rouzaud
    email                : denis.rouzaud@gmail.com

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSNETWORKCONTENTFETCHERREGISTRY_H
#define QGSNETWORKCONTENTFETCHERREGISTRY_H

#include <QObject>
#include <QMap>
#include <QNetworkReply>
#include <QTemporaryFile>

class QTemporaryFile;

class QgsNetworkContentFetcherTask;


/**
 * \class QgsNetworkContentFetcherRegistry
 * \ingroup core
 * \brief Registry for temporary fetched files
 *
 * This provides a simple way of downloading and accessing
 * remote files during QGIS application running.
 *
 * \see QgsNetworkContentFetcher
 * \see QgsNetworkContentFetcherTask
 *
 * \since QGIS 3.2
*/
class QgsNetworkContentFetcherRegistry : public QObject
{
    Q_OBJECT
  public:
    //! Status of fetched content
    enum ContentStatus
    {
      UnknownUrl, //!< No download started for such URL
      Downloading, //!< Currently downloading
      Finished, //!< Download finished and successful
      Failed //!< Download failed
    };

    /**
     * FetchedContent contains a pointer to the file and the status of the download
     */
    class FetchedContent
    {
      public:
        //! Constructs a FetchedContent with pointer to the downloaded file and status of the download
        explicit FetchedContent( QTemporaryFile *file = nullptr, ContentStatus status = UnknownUrl )
          : mFile( file ), mStatus( status ) {}

        //! Return a pointer to the local file, a null pointer if the file is not accessible yet.
        const QFile *file() const {return mFile;}

        //! Return the status of the download
        ContentStatus status() const {return mStatus;}

        //! Return the potential error of the download
        QNetworkReply::NetworkError reply() const {return mError;}

      private:
        void setFile( QTemporaryFile *file ) {mFile = file;}
        void setStatus( ContentStatus status ) {mStatus = status;}
        void setError( QNetworkReply::NetworkError error ) {mError = error;}
        QTemporaryFile *mFile;
        QgsNetworkContentFetcherTask *mFetchingTask;
        ContentStatus mStatus;
        QNetworkReply::NetworkError mError = QNetworkReply::NoError;

        // allow modification of task and file from main class
        friend class QgsNetworkContentFetcherRegistry;
    };

    //! Create the registry for temporary downloaded files
    explicit QgsNetworkContentFetcherRegistry();

    ~QgsNetworkContentFetcherRegistry();

    /**
     * @brief fetch
     * @param url
     * @param reload
     * @return
     */
    const QgsNetworkContentFetcherTask *fetch( const QUrl &url, const bool reload = false );

    FetchedContent file( const QUrl &url );

  private:
    QMap<QUrl, FetchedContent> mFileRegistry;

};

#endif // QGSNETWORKCONTENTFETCHERREGISTRY_H
