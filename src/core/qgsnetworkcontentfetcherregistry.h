/***************************************************************************
                       qgsnetworkcontentfetcherregistry.h
                             -------------------
    begin                : April, 2018
    copyright            : (C) 2018 by Denis Rouzaud
    email                : denis@opengis.ch

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
#include <QMutex>
#include <QNetworkReply>

#include "qgis_core.h"
#include "qgstaskmanager.h"
#include "qgsnetworkcontentfetchertask.h"

class QTemporaryFile;

/**
 * \class QgsFetchedContent
 * \ingroup core
 * FetchedContent holds useful information about a network content being fetched
 * \see QgsNetworkContentFetcherRegistry
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsFetchedContent : public QObject
{
    Q_OBJECT
  public:
    //! Status of fetched content
    enum ContentStatus
    {
      NotStarted, //!< No download started for such URL
      Downloading, //!< Currently downloading
      Finished, //!< Download finished and successful
      Failed //!< Download failed
    };

    //! Constructs a FetchedContent with pointer to the downloaded file and status of the download
    explicit QgsFetchedContent( const QString &url, QTemporaryFile *file = nullptr, ContentStatus status = NotStarted )
      : mUrl( url )
      , mFile( file )
      , mStatus( status )
    {}

    ~QgsFetchedContent() override
    {
      if ( mFile )
        mFile->close();
      delete mFile;
    }


#ifndef SIP_RUN
    //! Returns a pointer to the local file, or NULLPTR if the file is not accessible yet.
    QFile *file() const {return mFile;}
#endif

    //! Returns the path to the local file, an empty string if the file is not accessible yet.
    const QString filePath() const {return mFilePath;}

    //! Returns the status of the download
    ContentStatus status() const {return mStatus;}

    //! Returns the potential error of the download
    QNetworkReply::NetworkError error() const {return mError;}

  public slots:

    /**
     * \brief Start the download
     * \param redownload if set to TRUE, it will restart any achieved or pending download.
     */
    void download( bool redownload = false );

    /**
     * @brief Cancel the download operation
     */
    void cancel();

  signals:
    //! Emitted when the file is fetched and accessible
    void fetched();

  private slots:
    void taskCompleted();

  private:
    QString mUrl;
    QTemporaryFile *mFile = nullptr;
    QString mFilePath;
    QgsNetworkContentFetcherTask *mFetchingTask = nullptr;
    ContentStatus mStatus = NotStarted;
    QNetworkReply::NetworkError mError = QNetworkReply::NoError;
};

/**
 * \class QgsNetworkContentFetcherRegistry
 * \ingroup core
 * \brief Registry for temporary fetched files
 *
 * This provides a simple way of downloading and accessing
 * remote files during QGIS application running.
 *
 * \see QgsFetchedContent
 *
 * \since QGIS 3.2
*/
class CORE_EXPORT QgsNetworkContentFetcherRegistry : public QObject
{
    Q_OBJECT
  public:
    //! Enum to determine when the download should start
    enum FetchingMode
    {
      DownloadLater,       //!< Do not start immediately the download to properly connect the fetched signal
      DownloadImmediately, //!< The download will start immediately, not need to run QgsFecthedContent::download()
    };
    Q_ENUM( FetchingMode )

    //! Create the registry for temporary downloaded files
    explicit QgsNetworkContentFetcherRegistry() = default;

    ~QgsNetworkContentFetcherRegistry() override;

    /**
     * \brief Initialize a download for the given URL
     * \param url the URL to be fetched
     * \param fetchingMode defines if the download will start immediately or shall be manually triggered
     * \note If the download starts immediately, it will not redownload any already fetched or currently fetching file.
     */
    const QgsFetchedContent *fetch( const QString &url, FetchingMode fetchingMode = DownloadLater );

#ifndef SIP_RUN

    /**
     * \brief Returns a QFile from a local file or to a temporary file previously fetched by the registry
     * \param filePathOrUrl can either be a local file path or a remote content which has previously been fetched
     */
    QFile *localFile( const QString &filePathOrUrl );
#endif

    /**
     * \brief Returns the path to a local file or to a temporary file previously fetched by the registry
     * \param filePathOrUrl can either be a local file path or a remote content which has previously been fetched
     */
    QString localPath( const QString &filePathOrUrl );

  private:
    QMap<QString, QgsFetchedContent *> mFileRegistry;

};

#endif // QGSNETWORKCONTENTFETCHERREGISTRY_H
