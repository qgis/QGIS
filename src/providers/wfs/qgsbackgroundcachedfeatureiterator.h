/***************************************************************************
    qgsbacckgroundcachedfeatureiterator.h
    ---------------------
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

#include "qgsfeatureiterator.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsspatialindex.h"
#include "qgsvectordataprovider.h"

class QDataStream;
class QFile;
#include <QMutex>
#include <QThread>
#include <QWaitCondition>

struct QgsBackgroundCachedFeatureIteratorConstants
{
  // Special fields of the cache
  static const QString FIELD_GEN_COUNTER;
  static const QString FIELD_UNIQUE_ID;
  static const QString FIELD_HEXWKB_GEOM;
  static const QString FIELD_MD5;
};

//! Type that associate a QgsFeature to a (hopefully) unique id across requests
typedef QPair<QgsFeature, QString> QgsFeatureUniqueIdPair;

class QgsFeatureDownloader;

/**
 * This class is an abstract class that must
 * be subclassed and whose main role is to download features, through one or
 * several requests (GetFeature in the case of WFS), process the results as
 * soon as they arrived and notify them to the serializer to fill the case,
 * and to the iterator that subscribed
 * Instances of this class may be run in a dedicated thread (QgsThreadedFeatureDownloader)
 *
 * This class is somewhat technical and in an ideal world would have been
 * merged in QgsFeatureDownloader. This is not possible since QgsFeatureDownloader
 * derives from QObject to emit signals, but the implementation of run() might
 * also use custom signals, and should derive again from QObject, which is not
 * possible regarding QT/MOC constraints.
 */
class QgsFeatureDownloaderImpl
{
  public:
    QgsFeatureDownloaderImpl( QgsFeatureDownloader *downloader );
    virtual ~QgsFeatureDownloaderImpl() = default;

    /**
     * Start the download.
     * \param serializeFeatures whether to notify the sharedData serializer.
     * \param maxFeatures user-defined limit of features to download. Overrides
     *                    the one defined in the URI. Typically by the QgsWFSProvider,
     *                    when it cannot guess the geometry type.
     */
    virtual void run( bool serializeFeatures, int maxFeatures ) = 0;

    //! To interrupt the download. Must be thread-safe
    virtual void stop() = 0;

    // To be used when new features have been received
    void emitFeatureReceived( QVector<QgsFeatureUniqueIdPair> features );

    // To be used when new features have been received
    void emitFeatureReceived( int featureCount );

    // To be used when the download is finished (successful or not)
    void emitEndOfDownload( bool success );

  private:
    QgsFeatureDownloader *mDownloader;
};

/**
 * Interface of the downloader, typically called by QgsThreadedFeatureDownloader.
 * The real work is done by the implementation passed to setImpl().
*/
class QgsFeatureDownloader: public QObject
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
    void run( bool serializeFeatures, int maxFeatures );

    //! To interrupt the download.
    void stop();

  signals:
    //! Emitted when new features have been received
    void featureReceived( QVector<QgsFeatureUniqueIdPair> );

    //! Emitted when new features have been received
    void featureReceived( int featureCount );

    //! Emitted when the download is finished (successful or not)
    void endOfDownload( bool success );

  private:
    std::unique_ptr<QgsFeatureDownloaderImpl> mImpl;
};

class QgsBackgroundCachedSharedData;

//! Downloader thread
class QgsThreadedFeatureDownloader: public QThread
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
    QgsBackgroundCachedSharedData *mShared;  //!< Mutable data shared between provider and feature sources
    QgsFeatureDownloader *mDownloader = nullptr;
    QWaitCondition mWaitCond;
    QMutex mWaitMutex;
};


class QgsBackgroundCachedFeatureSource;

/**
 * Feature iterator. The iterator will internally both subscribe to a live
    downloader to receive 'fresh' features, and to a iterator on the features
    already cached. It will actually start by consuming cache features for
    initial feedback, and then process the live downloaded features. */
class QgsBackgroundCachedFeatureIterator : public QObject,
  public QgsAbstractFeatureIteratorFromSource<QgsBackgroundCachedFeatureSource>
{
    Q_OBJECT
  public:
    explicit QgsBackgroundCachedFeatureIterator(
      QgsBackgroundCachedFeatureSource *source, bool ownSource,
      std::shared_ptr<QgsBackgroundCachedSharedData> shared,
      const QgsFeatureRequest &request );
    ~QgsBackgroundCachedFeatureIterator() override;

    bool rewind() override;

    bool close() override;

    void setInterruptionChecker( QgsFeedback *interruptionChecker ) override;

    //! Used by QgsBackgroundCachedSharedData::registerToCache()
    void connectSignals( QgsFeatureDownloader *downloader );

  private slots:
    void featureReceived( int featureCount );
    void featureReceivedSynchronous( const QVector<QgsFeatureUniqueIdPair> &list );
    void endOfDownload( bool success );
    void checkInterruption();
    void timeout();

  private:

    std::shared_ptr<QgsBackgroundCachedSharedData> mShared;  //!< Mutable data shared between provider and feature sources

    //! Subset of attributes (relatives to mShared->mFields) to fetch. Only valid if ( mRequest.flags() & QgsFeatureRequest::SubsetOfAttributes )
    QgsAttributeList mSubSetAttributes;

    bool mDownloadFinished = false;
    QEventLoop *mLoop = nullptr;
    QgsFeatureIterator mCacheIterator;
    QgsFeedback *mInterruptionChecker = nullptr;
    bool mTimeoutOccurred = false;

    //! this mutex synchronizes the mWriterXXXX variables between featureReceivedSynchronous() and fetchFeature()
    QMutex mMutex;
    //! used to forger mWriterFilename
    int mCounter = 0;
    //! maximum size in bytes of mWriterByteArray before flushing it to disk
    int mWriteTransferThreshold = 1024 * 1024;
    QByteArray mWriterByteArray;
    QString mWriterFilename;
    QFile *mWriterFile = nullptr;
    QDataStream *mWriterStream = nullptr;

    QByteArray mReaderByteArray;
    QString mReaderFilename;
    QFile *mReaderFile = nullptr;
    QDataStream *mReaderStream = nullptr;
    bool mFetchGeometry = false;

    QgsCoordinateTransform mTransform;
    QgsRectangle mFilterRect;

    ///////////////// METHODS

    //! Translate mRequest to a request compatible of the Spatialite cache
    QgsFeatureRequest buildRequestCache( int gencounter );

    bool fetchFeature( QgsFeature &f ) override;

    //! Copies feature attributes / geometry from srcFeature to dstFeature
    void copyFeature( const QgsFeature &srcFeature, QgsFeature &dstFeature, bool srcIsCache );

    void cleanupReaderStreamAndFile();
};


//! Feature source
class QgsBackgroundCachedFeatureSource : public QgsAbstractFeatureSource
{
  public:
    explicit QgsBackgroundCachedFeatureSource( std::shared_ptr<QgsBackgroundCachedSharedData> shared );

    QgsFeatureIterator getFeatures( const QgsFeatureRequest &request ) override;

  private:

    std::shared_ptr<QgsBackgroundCachedSharedData> mShared;  //!< Mutable data shared between provider and feature sources
};

#endif // QGSBACKGROUNDCACHEDFEATUREITERATOR_H
