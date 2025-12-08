/***************************************************************************
    qgsthreadedfeaturedownloader.h
    ------------------------------
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

#ifndef QGSTHREADEDFEATUREDOWNLOADER_H
#define QGSTHREADEDFEATUREDOWNLOADER_H

#include <QMutex>
#include <QThread>
#include <QWaitCondition>

class QgsBackgroundCachedSharedData;
class QgsFeatureDownloader;

//! Downloader thread
class QgsThreadedFeatureDownloader : public QThread
{
    Q_OBJECT
  public:
    explicit QgsThreadedFeatureDownloader( QgsBackgroundCachedSharedData *shared );
    ~QgsThreadedFeatureDownloader() override;

    //! Returns downloader object
    QgsFeatureDownloader *downloader() { return mDownloader; }

    //! Starts thread and wait for it to be started
    void startAndWait();

    //! Stops (synchronously) the download
    void stop();

  protected:
    //! Inherited from QThread. Starts the download
    void run() override;

  private:
    QgsBackgroundCachedSharedData *mShared; //!< Mutable data shared between provider and feature sources
    QgsFeatureDownloader *mDownloader = nullptr;
    QWaitCondition mWaitCond;
    QMutex mWaitMutex;
    bool mRequestMadeFromMainThread = false;
};

#endif // QGSTHREADEDFEATUREDOWNLOADER_H
