/***************************************************************************
    qgswfsfeatureiterator.h
    ---------------------
    begin                : Januar 2013
    copyright            : (C) 2013 by Marco Hugentobler
                           (C) 2016 by Even Rouault
    email                : marco dot hugentobler at sourcepole dot ch
                           even.rouault at spatialys.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSWFSFEATUREITERATOR_H
#define QGSWFSFEATUREITERATOR_H

#include "qgsfeatureiterator.h"
#include "qgswfsrequest.h"
#include "qgsgml.h"
#include "qgsspatialindex.h"

#include <QProgressDialog>
#include <QPushButton>

class QgsWFSProvider;
class QgsWFSSharedData;
class QgsVectorDataProvider;
class QProgressDialog;

typedef QPair<QgsFeature, QString> QgsWFSFeatureGmlIdPair;


/** Utility class to issue a GetFeature resultType=hits request */
class QgsWFSFeatureHitsAsyncRequest: public QgsWFSRequest
{
    Q_OBJECT
  public:
    explicit QgsWFSFeatureHitsAsyncRequest( QgsWFSDataSourceURI& uri );
    ~QgsWFSFeatureHitsAsyncRequest();

    void launch( const QUrl& url );

    /** Return result of request, or -1 if not known/error */
    int numberMatched() const { return mNumberMatched; }

  signals:
    void gotHitsResponse();

  private slots:
    void hitsReplyFinished();

  protected:
    virtual QString errorMessageWithReason( const QString& reason ) override;

  private:
    int mNumberMatched;
};


/** Utility class for QgsWFSFeatureDownloader */
class QgsWFSProgressDialog: public QProgressDialog
{
    Q_OBJECT
  public:
    /** Constructor */
    QgsWFSProgressDialog( const QString & labelText, const QString & cancelButtonText, int minimum, int maximum, QWidget * parent );

    void resizeEvent( QResizeEvent * ev ) override;

  signals:
    void hide();

  private:
    QPushButton* mCancel;
    QPushButton* mHide;
};

/** This class runs one (or several if paging is needed) GetFeature request,
    process the results as soon as they arrived and notify them to the
    serializer to fill the case, and to the iterator that subscribed
    Instances of this class may be run in a dedicated thread (QgsWFSThreadedFeatureDownloader)
    A progress dialog may pop-up in GUI mode (if the download takes a certain time)
    to allow cancelling the download.
*/
class QgsWFSFeatureDownloader: public QgsWFSRequest
{
    Q_OBJECT
  public:
    explicit QgsWFSFeatureDownloader( QgsWFSSharedData* shared );
    ~QgsWFSFeatureDownloader();

    /** Start the download.
     * @param serializeFeatures whether to notify the sharedData serializer.
     * @param maxFeatures user-defined limit of features to download. Overrides
     *                    the one defined in the URI. Typically by the QgsWFSProvider,
     *                    when it cannot guess the geometry type.
     */
    void run( bool serializeFeatures, int maxFeatures );

  public slots:
    /** To interrupt the download. Thread-safe */
    void stop();

  signals:
    /** Emitted when new features have been received */
    void featureReceived( QVector<QgsWFSFeatureGmlIdPair> );

    /** Emitted when new features have been received */
    void featureReceived( int featureCount );

    /** Emitted when the download is finished (successful or not) */
    void endOfDownload( bool success );

    /** Used internally by the stop() method */
    void doStop();

    /** Emitted with the total accumulated number of features downloaded. */
    void updateProgress( int totalFeatureCount );

  protected:
    virtual QString errorMessageWithReason( const QString& reason ) override;

  private slots:
    void createProgressDialog();
    void startHitsRequest();
    void gotHitsResponse();
    void setStopFlag();
    void hideProgressDialog();

  private:
    QUrl buildURL( int startIndex, int maxFeatures, bool forHits );
    void pushError( const QString& errorMsg );
    QString sanitizeFilter( QString filter );

    /** Mutable data shared between provider, feature sources and downloader. */
    QgsWFSSharedData* mShared;
    /** Whether the download should stop */
    bool mStop;
    /** Progress dialog */
    QProgressDialog* mProgressDialog;
    /** If the progress dialog should be shown immediately, or if it should be
        let to QProgressDialog logic to decide when to show it */
    bool mProgressDialogShowImmediately;
    bool mSupportsPaging;
    bool mRemoveNSPrefix;
    int mNumberMatched;
    QWidget* mMainWindow;
    QTimer* mTimer;
    QgsWFSFeatureHitsAsyncRequest mFeatureHitsAsyncRequest;
    int mTotalDownloadedFeatureCount;
};

/** Downloader thread */
class QgsWFSThreadedFeatureDownloader: public QThread
{
    Q_OBJECT
  public:
    explicit QgsWFSThreadedFeatureDownloader( QgsWFSSharedData* shared );
    ~QgsWFSThreadedFeatureDownloader();

    /** Return downloader object */
    QgsWFSFeatureDownloader* downloader() { return mDownloader; }

    /** Stops (synchronously) the download */
    void stop();

  signals:
    /** Emitted when the thread is ready */
    void ready();

  protected:
    /** Inherited from QThread. Starts the download */
    void run() override;

  private:
    QgsWFSSharedData* mShared;  //!< Mutable data shared between provider and feature sources
    QgsWFSFeatureDownloader* mDownloader;
};

class QgsWFSFeatureSource;

/** Feature iterator. The iterator will internally both subscribe to a live
    downloader to receive 'fresh' features, and to a iterator on the features
    already cached. It will actually start by consuming cache features for
    initial feedback, and then process the live downloaded features. */
class QgsWFSFeatureIterator : public QObject,
      public QgsAbstractFeatureIteratorFromSource<QgsWFSFeatureSource>
{
    Q_OBJECT
  public:
    explicit QgsWFSFeatureIterator( QgsWFSFeatureSource* source, bool ownSource, const QgsFeatureRequest& request );
    ~QgsWFSFeatureIterator();

    bool rewind() override;

    bool close() override;

    void setInterruptionChecker( QgsInterruptionChecker* interruptionChecker ) override;

    /** Used by QgsWFSSharedData::registerToCache() */
    void connectSignals( QObject* downloader );

  private slots:
    void featureReceived( int featureCount );
    void featureReceivedSynchronous( QVector<QgsWFSFeatureGmlIdPair> list );
    void endOfDownload( bool success );
    void checkInterruption();

  private:

    bool fetchFeature( QgsFeature& f ) override;

    /** Copies feature attributes / geometry from srcFeature to dstFeature*/
    void copyFeature( const QgsFeature& srcFeature, QgsFeature& dstFeature );

    QSharedPointer<QgsWFSSharedData> mShared;  //!< Mutable data shared between provider and feature sources

    /** Subset of attributes (relatives to mShared->mFields) to fetch. Only valid if ( mRequest.flags() & QgsFeatureRequest::SubsetOfAttributes ) */
    QgsAttributeList mSubSetAttributes;

    bool mDownloadFinished;
    QEventLoop* mLoop;
    QgsFeatureIterator mCacheIterator;
    QgsInterruptionChecker* mInterruptionChecker;

    //! this mutex synchronizes the mWriterXXXX variables between featureReceivedSynchronous() and fetchFeature()
    QMutex mMutex;
    //! used to forger mWriterFilename
    int mCounter;
    //! maximum size in bytes of mWriterByteArray before flushing it to disk
    int mWriteTransferThreshold;
    QByteArray mWriterByteArray;
    QString mWriterFilename;
    QFile* mWriterFile;
    QDataStream* mWriterStream;

    QByteArray mReaderByteArray;
    QString mReaderFilename;
    QFile* mReaderFile;
    QDataStream* mReaderStream;
    bool mFetchGeometry;
};

/** Feature source */
class QgsWFSFeatureSource : public QgsAbstractFeatureSource
{
  public:
    explicit QgsWFSFeatureSource( const QgsWFSProvider* p );
    ~QgsWFSFeatureSource();

    /** Returns features matching the request */
    QgsFeatureIterator getFeatures( const QgsFeatureRequest& request ) override;

  protected:

    QSharedPointer<QgsWFSSharedData> mShared;  //!< Mutable data shared between provider and feature sources

    friend class QgsWFSFeatureIterator;
};

#endif // QGSWFSFEATUREITERATOR_H
