/***************************************************************************
  qgswebdavexternalstorage.h
  --------------------------------------
  Date                 : March 2021
  Copyright            : (C) 2021 by Julien Cabieces
  Email                : julien dot cabieces at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSWEBDAVEXTERNALSTORAGE_H
#define QGSWEBDAVEXTERNALSTORAGE_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgstaskmanager.h"

#include "externalstorage/qgsexternalstorage.h"

#include <QPointer>
#include <QUrl>

class QgsFeedback;
class QgsHttpExternalStorageStoreTask;
class QgsFetchedContent;

///@cond PRIVATE
#define SIP_NO_FILE

/**
 * \brief External storage implementation using the protocol WebDAV.
 *
 * \since QGIS 3.22
 */
class CORE_EXPORT QgsWebDavExternalStorage : public QgsExternalStorage
{
  public:

    QString type() const override;

    QString displayName() const override;

  protected:

    QgsExternalStorageStoredContent *doStore( const QString &filePath, const QString &url, const QString &authcfg = QString() ) const override;

    QgsExternalStorageFetchedContent *doFetch( const QString &url, const QString &authConfig = QString() ) const override;
};

/**
 * \brief External storage implementation using the protocol AWS S3.
 *
 * \since QGIS 3.28
 */
class CORE_EXPORT QgsAwsS3ExternalStorage : public QgsExternalStorage
{
  public:

    QString type() const override;

    QString displayName() const override;

  protected:

    QgsExternalStorageStoredContent *doStore( const QString &filePath, const QString &url, const QString &authcfg = QString() ) const override;

    QgsExternalStorageFetchedContent *doFetch( const QString &url, const QString &authConfig = QString() ) const override;
};

/**
 * \brief Class for HTTP stored content
 *
 * \since QGIS 3.22
 */
class QgsHttpExternalStorageStoredContent  : public QgsExternalStorageStoredContent
{
    Q_OBJECT

  public:

    QgsHttpExternalStorageStoredContent( const QString &filePath, const QString &url, const QString &authcfg = QString() );

    void cancel() override;

    QString url() const override;

    void store() override;

    void setPrepareRequestHandler( std::function< void( QNetworkRequest &request, QFile *f ) > );

  private:

    std::function< void( QNetworkRequest &request, QFile *f ) > mPrepareRequestHandler = nullptr;
    QPointer<QgsHttpExternalStorageStoreTask> mUploadTask;
    QString mUrl;
};

/**
 * \brief Class for HTTP fetched content
 *
 * \since QGIS 3.22
 */
class QgsHttpExternalStorageFetchedContent : public QgsExternalStorageFetchedContent
{
    Q_OBJECT

  public:

    QgsHttpExternalStorageFetchedContent( QgsFetchedContent *fetchedContent );

    QString filePath() const override;

    void cancel() override;

    void fetch() override;

  private slots:

    void onFetched();

  private:

    QPointer<QgsFetchedContent> mFetchedContent;
};


/**
 * \brief Task to store a file to a given url
 *
 * \since QGIS 3.22
 */
class QgsHttpExternalStorageStoreTask : public QgsTask
{
    Q_OBJECT

  public:

    QgsHttpExternalStorageStoreTask( const QUrl &url, const QString &filePath, const QString &authCfg );

    bool run() override;

    void cancel() override;

    QString errorString() const;

    void setPrepareRequestHandler( std::function< void( QNetworkRequest &request, QFile *f ) > );

  private:

    std::function< void( QNetworkRequest &request, QFile *f ) > mPrepareRequestHandler = nullptr;
    const QUrl mUrl;
    const QString mFilePath;
    const QString mAuthCfg;
    std::unique_ptr<QgsFeedback> mFeedback;
    QString mErrorString;
};


///@endcond
#endif // QGSWEBDAVEXTERNALSTORAGE_H
