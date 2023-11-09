/***************************************************************************
    qgsbackgroundcachedfeatureiterator.h
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
#include "qgsnetworkaccessmanager.h"
#include "qgsspatialindex.h"
#include "qgsvectordataprovider.h"
#include "qgscoordinatetransform.h"

class QDataStream;
class QFile;
class QPushButton;
#include <QMutex>
#include <QThread>
#include <QWaitCondition>
#include <QProgressDialog>
#include <QTimer>

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


//! Utility class for QgsFeatureDownloaderImpl
class QgsFeatureDownloaderProgressDialog: public QProgressDialog
{
    Q_OBJECT
  public:
    //! Constructor
    QgsFeatureDownloaderProgressDialog( const QString &labelText, const QString &cancelButtonText, int minimum, int maximum, QWidget *parent );

    void resizeEvent( QResizeEvent *ev ) override;

  signals:
    void hideRequest();

  private:
    QPushButton *mCancel = nullptr;
    QPushButton *mHide = nullptr;
};

class QgsBackgroundCachedSharedData;

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
    QgsFeatureDownloaderImpl( QgsBackgroundCachedSharedData *shared, QgsFeatureDownloader *downloader );
    virtual ~QgsFeatureDownloaderImpl();

    /**
     * Start the download.
     * \param serializeFeatures whether to notify the sharedData serializer.
     * \param maxFeatures user-defined limit of features to download. Overrides
     *                    the one defined in the URI. Typically by the QgsWFSProvider,
     *                    when it cannot guess the geometry type.
     */
    virtual void run( bool serializeFeatures, long long maxFeatures ) = 0;

    //! To interrupt the download. Must be thread-safe
    void stop();

    //! Emit doStop() signal
    virtual void emitDoStop() = 0;

    // To be used when new features have been received
    void emitFeatureReceived( QVector<QgsFeatureUniqueIdPair> features );

    // To be used when new features have been received
    void emitFeatureReceived( long long featureCount );

    // To be used when the download is finished (successful or not)
    void emitEndOfDownload( bool success );

    // To be used when QgsNetworkAccessManager emit signals that require
    // QgsBackgroundCachedFeatureIterator to process (authentication) events,
    // if it was started from the main thread.
    void emitResumeMainThread();

#if 0
    // NOTE: implementations should copy & paste the below block
  signals:
    /* Used internally by the stop() method */
    void doStop();

    /* Emitted with the total accumulated number of features downloaded. */
    void updateProgress( int totalFeatureCount );
#endif

  protected:
    //! Progress dialog
    QgsFeatureDownloaderProgressDialog *mProgressDialog = nullptr;

    //! Whether the download should stop
    bool mStop = false;

    /**
     * If the progress dialog should be shown immediately, or if it should be
     * let to QProgressDialog logic to decide when to show it.
    */
    bool mProgressDialogShowImmediately = false;

    QTimer *mTimer = nullptr;

    void createProgressDialog( int numberMatched );

    void setStopFlag();
    void hideProgressDialog();

    void endOfRun( bool serializeFeatures,
                   bool success, int totalDownloadedFeatureCount,
                   bool truncatedResponse, bool interrupted,
                   const QString &errorMessage );

    void connectSignals( QObject *obj, bool requestMadeFromMainThread );

  private:
    QgsBackgroundCachedSharedData *mSharedBase;
    QgsFeatureDownloader *mDownloader;
    QWidget *mMainWindow = nullptr;
    QMutex mMutexCreateProgressDialog;
};

// Sorry for ugliness. Due to QgsFeatureDownloaderImpl that cannot derive from QObject
#define QGS_FEATURE_DOWNLOADER_IMPL_CONNECT_SIGNALS_BASE(requestMadeFromMainThread) \
  do { \
    if ( requestMadeFromMainThread ) \
    { \
      auto resumeMainThread = [this]() \
      { \
        emitResumeMainThread(); \
      }; \
      QObject::connect( QgsNetworkAccessManager::instance(), &QgsNetworkAccessManager::authRequestOccurred,  \
                        this, resumeMainThread, Qt::DirectConnection );  \
      QObject::connect( QgsNetworkAccessManager::instance(), &QgsNetworkAccessManager::proxyAuthenticationRequired,  \
                        this, resumeMainThread, Qt::DirectConnection );  \
    } \
  } while(false)

#ifndef QT_NO_SSL
#define QGS_FEATURE_DOWNLOADER_IMPL_CONNECT_SIGNALS(requestMadeFromMainThread) \
  do { \
    QGS_FEATURE_DOWNLOADER_IMPL_CONNECT_SIGNALS_BASE(requestMadeFromMainThread); \
    if ( requestMadeFromMainThread ) \
    { \
      auto resumeMainThread = [this]() \
      { \
        emitResumeMainThread(); \
      }; \
      QObject::connect( QgsNetworkAccessManager::instance(), &QgsNetworkAccessManager::sslErrorsOccurred,  \
                        this, resumeMainThread, Qt::DirectConnection );  \
    } \
  } while(false)
#else
#define QGS_FEATURE_DOWNLOADER_IMPL_CONNECT_SIGNALS(requestMadeFromMainThread) \
  QGS_FEATURE_DOWNLOADER_IMPL_CONNECT_SIGNALS_BASE(requestMadeFromMainThread)
#endif

// Sorry for ugliness. Due to QgsFeatureDownloaderImpl that cannot derive from QObject
#define CONNECT_PROGRESS_DIALOG(actual_downloader_impl_class) do { \
    connect( mProgressDialog, &QProgressDialog::canceled, this, &actual_downloader_impl_class::setStopFlag, Qt::DirectConnection ); \
    connect( mProgressDialog, &QProgressDialog::canceled, this, &actual_downloader_impl_class::stop ); \
    connect( mProgressDialog, &QgsFeatureDownloaderProgressDialog::hideRequest, this, &actual_downloader_impl_class::hideProgressDialog ); \
    \
    /* Make sure the progress dialog has not been deleted by another thread */ \
    if ( mProgressDialog ) \
    {  \
      connect( this, &actual_downloader_impl_class::updateProgress, mProgressDialog, &QProgressDialog::setValue );  \
    } \
  } while(0)

// Sorry for ugliness. Due to QgsFeatureDownloaderImpl that cannot derive from QObject
#define DEFINE_FEATURE_DOWLOADER_IMPL_SLOTS \
  protected: \
  void emitDoStop() override { emit doStop(); } \
  void setStopFlag() { QgsFeatureDownloaderImpl::setStopFlag(); } \
  void stop() { QgsFeatureDownloaderImpl::stop(); } \
  void hideProgressDialog() { QgsFeatureDownloaderImpl::hideProgressDialog(); }

#define CREATE_PROGRESS_DIALOG(actual_downloader_impl_class) \
  do { \
    /* This is a bit tricky. We want the createProgressDialog() */ \
    /* method to be run into the GUI thread */ \
    mTimer = new QTimer(); \
    mTimer->setSingleShot( true ); \
    \
    /* Direct connection, since we want createProgressDialog() */  \
    /* to be invoked from the same thread as timer, and not in the */  \
    /* thread of this */  \
    connect( mTimer, &QTimer::timeout, this, &actual_downloader_impl_class::createProgressDialog, Qt::DirectConnection );  \
    \
    mTimer->moveToThread( qApp->thread() ); \
    QMetaObject::invokeMethod( mTimer, "start", Qt::QueuedConnection ); \
  } while (0)

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
    bool mRequestMadeFromMainThread = false;
};


class QgsBackgroundCachedFeatureSource;

/**
 * Feature iterator. The iterator will internally both subscribe to a live
 * downloader to receive 'fresh' features, and to a iterator on the features
 * already cached. It will actually start by consuming cache features for
 * initial feedback, and then process the live downloaded features.
*/
class QgsBackgroundCachedFeatureIterator final: public QObject,
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
    void featureReceivedSynchronous( const QVector<QgsFeatureUniqueIdPair> &list );
    void endOfDownloadSynchronous( bool success );
    void resumeMainThreadSynchronous();

  private:

    std::shared_ptr<QgsBackgroundCachedSharedData> mShared;  //!< Mutable data shared between provider and feature sources

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
    std::unique_ptr<QDataStream> mWriterStream ;

    QByteArray mReaderByteArray;
    QString mReaderFilename;
    std::unique_ptr<QFile> mReaderFile;
    std::unique_ptr<QDataStream> mReaderStream;
    bool mFetchGeometry = false;

    QgsCoordinateTransform mTransform;
    QgsRectangle mFilterRect;
    QgsGeometry mDistanceWithinGeom;
    std::unique_ptr< QgsGeometryEngine > mDistanceWithinEngine;

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


//! Feature source
class QgsBackgroundCachedFeatureSource final: public QgsAbstractFeatureSource
{
  public:
    explicit QgsBackgroundCachedFeatureSource( std::shared_ptr<QgsBackgroundCachedSharedData> shared );

    QgsFeatureIterator getFeatures( const QgsFeatureRequest &request ) override;

  private:

    std::shared_ptr<QgsBackgroundCachedSharedData> mShared;  //!< Mutable data shared between provider and feature sources
};

#endif // QGSBACKGROUNDCACHEDFEATUREITERATOR_H
