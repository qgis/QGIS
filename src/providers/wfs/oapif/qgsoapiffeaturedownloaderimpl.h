/***************************************************************************
    qgsoapiffeaturedownloaderimpl.h
    -------------------------------
    begin                : October 2019
    copyright            : (C) 2019 by Even Rouault
    email                : even.rouault at spatialys.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSOAPIFFILTERDOWNLOADERIMPL_H
#define QGSOAPIFFILTERDOWNLOADERIMPL_H

#include "qgsfeaturedownloaderimpl.h"

class QgsFeatureDownloader;
class QgsOapifSharedData;

class QgsOapifFeatureDownloaderImpl final : public QObject, public QgsFeatureDownloaderImpl
{
    Q_OBJECT

    DEFINE_FEATURE_DOWNLOADER_IMPL_SLOTS

  signals:
    /* Used internally by the stop() method */
    void doStop();

    /* Emitted with the total accumulated number of features downloaded. */
    void updateProgress( long long totalFeatureCount );

  public:
    QgsOapifFeatureDownloaderImpl( QgsOapifSharedData *shared, QgsFeatureDownloader *downloader, bool requestMadeFromMainThread );
    ~QgsOapifFeatureDownloaderImpl() override;

    void run( bool serializeFeatures, long long maxFeatures ) override;

  private slots:
    void createProgressTask();

  private:
    //! Mutable data shared between provider, feature sources and downloader.
    QgsOapifSharedData *mShared = nullptr;

    int mNumberMatched = -1;
};

#endif
