/***************************************************************************
  qgssimplecopyexternalstorage_p.h
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

#ifndef QGSSIMPLECOPYEXTERNALSTORAGE_H
#define QGSSIMPLECOPYEXTERNALSTORAGE_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsexternalstorage.h"

#include <QPointer>

///@cond PRIVATE
#define SIP_NO_FILE

class QgsCopyFileTask;

/**
 * \brief External storage implementation which simply copy the given resource
 * on a given directory file path.
 *
 * \since QGIS 3.22
 */
class CORE_EXPORT QgsSimpleCopyExternalStorage : public QgsExternalStorage
{
  public:

    [[nodiscard]] QString type() const override;

    [[nodiscard]] QString displayName() const override;

    [[nodiscard]] QgsExternalStorageStoredContent *doStore( const QString &filePath, const QString &url, const QString &authcfg = QString() ) const override;

    [[nodiscard]] QgsExternalStorageFetchedContent *doFetch( const QString &url, const QString &authConfig = QString() ) const override;
};

/**
 * \brief Class for Simple copy stored content
 *
 * \since QGIS 3.22
 */
class QgsSimpleCopyExternalStorageStoredContent  : public QgsExternalStorageStoredContent
{
    Q_OBJECT

  public:

    QgsSimpleCopyExternalStorageStoredContent( const QString &filePath, const QString &url, const QString &authcfg = QString() );

    void cancel() override;

    [[nodiscard]] QString url() const override;

    void store() override;

  private:

    QPointer<QgsCopyFileTask> mCopyTask;
    QString mUrl;
};

/**
 * \brief Class for Simple copy fetched content
 *
 * \since QGIS 3.22
 */
class QgsSimpleCopyExternalStorageFetchedContent : public QgsExternalStorageFetchedContent
{
    Q_OBJECT

  public:

    QgsSimpleCopyExternalStorageFetchedContent( const QString &filePath );

    [[nodiscard]] QString filePath() const override;

    void fetch() override;

  private:

    QString mFilePath;
    QString mResultFilePath;
};

///@endcond
#endif // QGSSIMPLECOPYEXTERNALSTORAGE_H
