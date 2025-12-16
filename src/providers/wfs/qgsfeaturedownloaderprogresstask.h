/***************************************************************************
    qgsfeaturedownloaderprogresstask.h
    ----------------------------------
    begin                : October 2019
    copyright            : (C) 2016-2019 by Even Rouault
    email                : even.rouault at spatialys.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSFEATUREDOWNLOADERPROGRESSTASK_H
#define QGSFEATUREDOWNLOADERPROGRESSTASK_H

#include "qgstaskmanager.h"

#include <QMutex>
#include <QString>
#include <QWaitCondition>

class QgsFeatureDownloaderProgressTask : public QgsTask
{
    Q_OBJECT

  public:
    QgsFeatureDownloaderProgressTask( const QString &description, long long totalCount );

    bool run() override;

    void cancel() override;

  public slots:

    void finalize();
    void setDownloaded( long long count );

  signals:

    void canceled();

  private:
    long long mTotalCount = 0;
    QWaitCondition mNotFinishedWaitCondition;
    QMutex mNotFinishedMutex;
    bool mAlreadyFinished = false;
};

#endif // QGSFEATUREDOWNLOADERPROGRESSTASK_H
