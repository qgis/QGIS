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
class QgsWebDAVExternalStorageStoreTask;
class QgsFetchedContent;

///@cond PRIVATE
#define SIP_NO_FILE

/**
 * \ingroup core
 * \brief External storage implementation using the protocol WebDAV.
 *
 * \since QGIS 3.22
 */
class CORE_EXPORT QgsWebDAVExternalStorage : public QgsExternalStorage
{
  public:

    QString type() const override;

    QString displayName() const override;

  protected:

    QgsExternalStorageStoredContent *doStore( const QString &filePath, const QString &url, const QString &authcfg = QString() ) const override;

    QgsExternalStorageFetchedContent *doFetch( const QString &url, const QString &authConfig = QString() ) const override;
};

/**
 * \ingroup core
 * \brief Class for WebDAV stored content
 *
 * \since QGIS 3.22
 */
class QgsWebDAVExternalStorageStoredContent  : public QgsExternalStorageStoredContent
{
    Q_OBJECT

  public:

    QgsWebDAVExternalStorageStoredContent( const QString &filePath, const QString &url, const QString &authcfg = QString() );

    void cancel() override;

    QString url() const override;

    void store() override;

  private:

    QPointer<QgsWebDAVExternalStorageStoreTask> mUploadTask;
    QString mUrl;
};

/**
 * \ingroup core
 * \brief Class for WebDAV fetched content
 *
 * \since QGIS 3.22
 */
class QgsWebDAVExternalStorageFetchedContent : public QgsExternalStorageFetchedContent
{
    Q_OBJECT

  public:

    QgsWebDAVExternalStorageFetchedContent( QgsFetchedContent *fetchedContent );

    QString filePath() const override;

    void cancel() override;

    void fetch() override;

  private slots:

    void onFetched();

  private:

    QPointer<QgsFetchedContent> mFetchedContent;
};


/**
 * \ingroup core
 * \brief Task to store a file to a given WebDAV url
 *
 * \since QGIS 3.22
 */
class QgsWebDAVExternalStorageStoreTask : public QgsTask
{
    Q_OBJECT

  public:

    QgsWebDAVExternalStorageStoreTask( const QUrl &url, const QString &filePath, const QString &authCfg );

    bool run() override;

    void cancel() override;

    QString errorString() const;

  private:

    const QUrl mUrl;
    const QString mFilePath;
    const QString mAuthCfg;
    std::unique_ptr<QgsFeedback> mFeedback;
    QString mErrorString;
};


///@endcond
#endif // QGSWEBDAVEXTERNALSTORAGE_H
