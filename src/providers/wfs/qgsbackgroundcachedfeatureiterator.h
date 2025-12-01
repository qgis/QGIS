/***************************************************************************
    qgsbackgroundcachedfeatureiterator.h
    ------------------------------------
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
#ifndef QGSBACKGROUNDCACHEDFEATUREITERATOR_H
#define QGSBACKGROUNDCACHEDFEATUREITERATOR_H

#include "qgscoordinatereferencesystem.h"
#include "qgscoordinatetransform.h"
#include "qgsfeaturedownloadcommon.h"
#include "qgsfeaturedownloaderprogresstask.h"
#include "qgsfeatureiterator.h"

class QDataStream;
class QFile;

#include <QMutex>
#include <QWaitCondition>

class QgsBackgroundCachedSharedData;
class QgsBackgroundCachedFeatureSource;
class QgsFeatureDownloader;

/**
 * Feature iterator. The iterator will internally both subscribe to a live
 * downloader to receive 'fresh' features, and to a iterator on the features
 * already cached. It will actually start by consuming cache features for
 * initial feedback, and then process the live downloaded features.
*/
class QgsBackgroundCachedFeatureIterator final : public QObject,
                                                 public QgsAbstractFeatureIteratorFromSource<QgsBackgroundCachedFeatureSource>
{
    Q_OBJECT
  public:
    explicit QgsBackgroundCachedFeatureIterator(
      QgsBackgroundCachedFeatureSource *source, bool ownSource,
      std::shared_ptr<QgsBackgroundCachedSharedData> shared,
      const QgsFeatureRequest &request
    );
    ~QgsBackgroundCachedFeatureIterator() override;

    bool rewind() override;

    bool close() override;

    void setInterruptionChecker( QgsFeedback *interruptionChecker ) override;

    //! Used by QgsBackgroundCachedSharedData::registerToCache()
    void connectSignals( QgsFeatureDownloader *downloader );

    struct Constants
    {
        // Special fields of the cache
        static const QString FIELD_GEN_COUNTER;
        static const QString FIELD_UNIQUE_ID;
        static const QString FIELD_HEXWKB_GEOM;
        static const QString FIELD_MD5;
    };

  private slots:
    void featureReceivedSynchronous( const QVector<QgsFeatureUniqueIdPair> &list );
    void endOfDownloadSynchronous( bool success );
    void resumeMainThreadSynchronous();

  private:
    std::shared_ptr<QgsBackgroundCachedSharedData> mShared; //!< Mutable data shared between provider and feature sources

    //! Subset of attributes (relatives to mShared->mFields) to fetch. Only valid if ( mRequest.flags() & QgsFeatureRequest::SubsetOfAttributes )
    QgsAttributeList mSubSetAttributes;

    bool mNewFeaturesReceived = false;
    bool mDownloadFinished = false;
    bool mProcessEvents = false;
    QgsFeatureIterator mCacheIterator;
    QgsFeedback *mInterruptionChecker = nullptr;
    bool mTimeoutOrInterruptionOccurred = false;

    //! Cached features for request by fid
    QVector<QgsFeature> mCachedFeatures;
    QVector<QgsFeature>::iterator mCachedFeaturesIter;

    //! this mutex synchronizes the mWriterXXXX variables between featureReceivedSynchronous() and fetchFeature()
    QMutex mMutex;
    QWaitCondition mWaitCond;
    //! used to forger mWriterFilename
    int mCounter = 0;
    //! maximum size in bytes of mWriterByteArray before flushing it to disk
    int mWriteTransferThreshold = 1024 * 1024;
    QByteArray mWriterByteArray;
    QString mWriterFilename;
    std::unique_ptr<QFile> mWriterFile;
    std::unique_ptr<QDataStream> mWriterStream;

    QByteArray mReaderByteArray;
    QString mReaderFilename;
    std::unique_ptr<QFile> mReaderFile;
    std::unique_ptr<QDataStream> mReaderStream;
    bool mFetchGeometry = false;

    QgsCoordinateTransform mTransform;
    QgsRectangle mFilterRect;
    QgsGeometry mDistanceWithinGeom;
    std::unique_ptr<QgsGeometryEngine> mDistanceWithinEngine;

    //! typically to save a FilterFid/FilterFids request that will not be captured by mRequest
    QgsFeatureRequest mAdditionalRequest;

    ///////////////// METHODS

    //! Translate mRequest to a request compatible of the Spatialite cache (first part)
    QgsFeatureRequest initRequestCache( int gencounter );

    //! Finishes to fill the request to the cache (second part)
    void fillRequestCache( QgsFeatureRequest );

    bool fetchFeature( QgsFeature &f ) override;

    //! Copies feature attributes / geometry from srcFeature to dstFeature
    void copyFeature( const QgsFeature &srcFeature, QgsFeature &dstFeature, bool srcIsCache );

    void cleanupReaderStreamAndFile();
};

#endif // QGSBACKGROUNDCACHEDFEATUREITERATOR_H
