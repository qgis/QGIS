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

#include "qgsbackgroundcachedfeatureiterator.h"

#include <memory>
#include <QProgressDialog>
#include <QPushButton>
#include <QMutex>
#include <QWaitCondition>

class QgsWFSProvider;
class QgsWFSSharedData;
class QgsVectorDataProvider;
class QProgressDialog;


//! Utility class to issue a GetFeature resultType=hits request
class QgsWFSFeatureHitsAsyncRequest: public QgsWfsRequest
{
    Q_OBJECT
  public:
    explicit QgsWFSFeatureHitsAsyncRequest( QgsWFSDataSourceURI &uri );

    void launch( const QUrl &url );

    //! Returns result of request, or -1 if not known/error
    int numberMatched() const { return mNumberMatched; }

  signals:
    void gotHitsResponse();

  private slots:
    void hitsReplyFinished();

  protected:
    QString errorMessageWithReason( const QString &reason ) override;

  private:
    int mNumberMatched;
};


//! Utility class for QgsWFSFeatureDownloader
class QgsWFSProgressDialog: public QProgressDialog
{
    Q_OBJECT
  public:
    //! Constructor
    QgsWFSProgressDialog( const QString &labelText, const QString &cancelButtonText, int minimum, int maximum, QWidget *parent );

    void resizeEvent( QResizeEvent *ev ) override;

  signals:
    void hideRequest();

  private:
    QPushButton *mCancel = nullptr;
    QPushButton *mHide = nullptr;
};


/**
 * This class runs one (or several if paging is needed) GetFeature request,
    process the results as soon as they arrived and notify them to the
    serializer to fill the case, and to the iterator that subscribed
    Instances of this class may be run in a dedicated thread (QgsWFSThreadedFeatureDownloader)
    A progress dialog may pop-up in GUI mode (if the download takes a certain time)
    to allow canceling the download.
*/
class QgsWFSFeatureDownloaderImpl: public QgsWfsRequest, public QgsFeatureDownloaderImpl
{
    Q_OBJECT
  public:
    QgsWFSFeatureDownloaderImpl( QgsWFSSharedData *shared, QgsFeatureDownloader *downloader );
    ~QgsWFSFeatureDownloaderImpl() override;

    void run( bool serializeFeatures, int maxFeatures ) override;

    void stop() override;

  signals:

    //! Used internally by the stop() method
    void doStop();

    //! Emitted with the total accumulated number of features downloaded.
    void updateProgress( int totalFeatureCount );

  protected:
    QString errorMessageWithReason( const QString &reason ) override;

  private slots:
    void createProgressDialog();
    void startHitsRequest();
    void gotHitsResponse();
    void setStopFlag();
    void hideProgressDialog();

  private:
    QUrl buildURL( qint64 startIndex, int maxFeatures, bool forHits );
    void pushError( const QString &errorMsg );
    QString sanitizeFilter( QString filter );

    //! Mutable data shared between provider, feature sources and downloader.
    QgsWFSSharedData *mShared = nullptr;
    //! Whether the download should stop
    bool mStop = false;
    //! Progress dialog
    QgsWFSProgressDialog *mProgressDialog = nullptr;

    /**
     * If the progress dialog should be shown immediately, or if it should be
        let to QProgressDialog logic to decide when to show it */
    bool mProgressDialogShowImmediately = false;
    int mPageSize = 0;
    bool mRemoveNSPrefix = false;
    int mNumberMatched = -1;
    bool mUseProgressDialog = false;
    QWidget *mMainWindow = nullptr;
    QTimer *mTimer = nullptr;
    QgsWFSFeatureHitsAsyncRequest mFeatureHitsAsyncRequest;
    qint64 mTotalDownloadedFeatureCount = 0;
    QMutex mMutexCreateProgressDialog;
};


#endif // QGSWFSFEATUREITERATOR_H
