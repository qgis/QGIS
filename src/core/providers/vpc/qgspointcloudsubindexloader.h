/***************************************************************************
    qgspointcloudsubindexloader.h
    ---------------------
    begin                : March 2026
    copyright            : (C) 2026 by Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPOINTCLOUDSUBINDEXLOADER_H
#define QGSPOINTCLOUDSUBINDEXLOADER_H

#include "qgspointcloudindex.h"

#include <QFutureWatcher>
#include <QObject>

#define SIP_NO_FILE

///@cond PRIVATE

class QgsPointCloudSubIndexLoader : public QObject
{
    Q_OBJECT
  public:
    explicit QgsPointCloudSubIndexLoader( const QString &uri, int id, bool emitDataChanged, QObject *parent = nullptr );
    ~QgsPointCloudSubIndexLoader() override;
    void start();
    QgsPointCloudIndex index() const { return mIndex; }
    bool emitDataChangedWhenLoaded() const { return mEmitDataChanged; }

  signals:
    void finished( int id );

  private:
    static QgsPointCloudIndex loadSubIndex( const QString &uri );
    void onFutureFinished();
    QFutureWatcher<QgsPointCloudIndex> *mFutureWatcher = nullptr;
    QgsPointCloudIndex mIndex;
    QString mUri;
    int mId;
    bool mEmitDataChanged = false;
};

///@endcond
#endif // QGSPOINTCLOUDSUBINDEXLOADER_H
