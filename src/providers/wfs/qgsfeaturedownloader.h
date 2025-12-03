/***************************************************************************
    qgsfeaturedownloader.h
    ----------------------
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

#ifndef QGSFEATUREDOWNLOADER_H
#define QGSFEATUREDOWNLOADER_H

#include <memory>

#include "qgsfeaturedownloadcommon.h"
#include "qgsfeaturedownloaderimpl.h"

#include <QObject>
#include <QVector>

/**
 * Interface of the downloader, typically called by QgsThreadedFeatureDownloader.
 * The real work is done by the implementation passed to setImpl().
*/
class QgsFeatureDownloader : public QObject
{
    Q_OBJECT
  public:
    explicit QgsFeatureDownloader() = default;

    //! Set the implementation. This method must be called before calling any other method
    void setImpl( std::unique_ptr<QgsFeatureDownloaderImpl> &&impl ) { mImpl = std::move( impl ); }

    /**
     * Start the download.
     * \param serializeFeatures whether to notify the sharedData serializer.
     * \param maxFeatures user-defined limit of features to download. Overrides
     *                    the one defined in the URI. Typically by the QgsWFSProvider,
     *                    when it cannot guess the geometry type.
     */
    void run( bool serializeFeatures, long long maxFeatures );

    //! To interrupt the download.
    void stop();

  signals:
    //! Emitted when new features have been received
    void featureReceived( QVector<QgsFeatureUniqueIdPair> );

    //! Emitted when new features have been received
    void featureReceived( long long featureCount );

    //! Emitted when the download is finished (successful or not)
    void endOfDownload( bool success );

    // Emitted when QgsNetworkAccessManager emit signals that require
    // QgsBackgroundCachedFeatureIterator to process (authentication) events,
    // if it was started from the main thread.
    void resumeMainThread();

  private:
    std::unique_ptr<QgsFeatureDownloaderImpl> mImpl;
};


#endif // QGSFEATUREDOWNLOADER_H
