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
#include <QFile>
#include <QTemporaryFile>

#include "qgis_core.h"
#include "qgstaskmanager.h"
#include "qgsnetworkcontentfetchertask.h"

/**
 * \class QgsFetchedContent
 * \ingroup core
 * \brief FetchedContent holds useful information about a network content being fetched
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
    explicit QgsFetchedContent( const QString &url, QTemporaryFile *file = nullptr, ContentStatus status = NotStarted,
                                const QString &authConfig = QString() )
      : mUrl( url )
      , mFile( file )
      , mStatus( status )
      , mAuthConfig( authConfig )
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

    /**
     * Returns the authentication configuration id use for this fetched content
     */
    QString authConfig() const {return mAuthConfig;}

  public slots:

    /**
     * \brief Start the download
     * \param redownload if set to TRUE, it will restart any achieved or pending download.
     */
    void download( bool redownload = false );

    /**
     * Cancel the download operation.
     */
    void cancel();

  signals:
    //! Emitted when the file is fetched and accessible
    void fetched();

    /**
     * Emitted when an error with \a code error occurred while processing the request
     * \a errorMsg is a textual description of the error
     * \since QGIS 3.22
     */
    void errorOccurred( QNetworkReply::NetworkError code, const QString &errorMsg );

  private slots:
    void taskCompleted();

  private:
    QString mUrl;
    QTemporaryFile *mFile = nullptr;
    QString mFilePath;
    QgsNetworkContentFetcherTask *mFetchingTask = nullptr;
    ContentStatus mStatus = NotStarted;
    QNetworkReply::NetworkError mError = QNetworkReply::NoError;
    QString mAuthConfig;
    QString mErrorString;
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

    //! Create the registry for temporary downloaded files
    explicit QgsNetworkContentFetcherRegistry() = default;

    ~QgsNetworkContentFetcherRegistry() override;

    /**
     * \brief Initialize a download for the given URL
     * \param url the URL to be fetched
     * \param fetchingMode defines if the download will start immediately or shall be manually triggered
     * \param authConfig authentication configuration id to be used while fetching
     * \note If the download starts immediately, it will not redownload any already fetched or currently fetching file.
     */
    QgsFetchedContent *fetch( const QString &url, Qgis::ActionStart fetchingMode = Qgis::ActionStart::Deferred, const QString &authConfig = QString() );

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
