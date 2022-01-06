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

#include "qgsbackgroundcachedfeatureiterator.h"
#include "qgsbackgroundcachedshareddata.h"

#include "qgsapplication.h"
#include "qgsfeedback.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"
#include "qgswfsutils.h" // for isCompatibleType()
#include "qgsgeometryengine.h"

#include <QDataStream>
#include <QDir>
#include <QFile>
#include <QMutex>
#include <QPushButton>
#include <QStyle>
#include <QTimer>
#include <QElapsedTimer>

#include <sqlite3.h>

const QString QgsBackgroundCachedFeatureIteratorConstants::FIELD_GEN_COUNTER( QStringLiteral( "__qgis_gen_counter" ) );
const QString QgsBackgroundCachedFeatureIteratorConstants::FIELD_UNIQUE_ID( QStringLiteral( "__qgis_unique_id" ) );
const QString QgsBackgroundCachedFeatureIteratorConstants::FIELD_HEXWKB_GEOM( QStringLiteral( "__qgis_hexwkb_geom" ) );
const QString QgsBackgroundCachedFeatureIteratorConstants::FIELD_MD5( QStringLiteral( "__qgis_md5" ) );

// -------------------------


QgsFeatureDownloaderProgressDialog::QgsFeatureDownloaderProgressDialog( const QString &labelText,
    const QString &cancelButtonText,
    int minimum, int maximum, QWidget *parent )
  : QProgressDialog( labelText, cancelButtonText, minimum, maximum, parent )
{
  mCancel = new QPushButton( cancelButtonText, this );
  setCancelButton( mCancel );
  mHide = new QPushButton( tr( "Hide" ), this );
  connect( mHide, &QAbstractButton::clicked, this, &QgsFeatureDownloaderProgressDialog::hideRequest );
}

void QgsFeatureDownloaderProgressDialog::resizeEvent( QResizeEvent *ev )
{
  QProgressDialog::resizeEvent( ev );
  // Note: this relies heavily on the details of the layout done in QProgressDialogPrivate::layout()
  // Might be a bit fragile depending on QT versions.
  QRect rect = geometry();
  QRect cancelRect = mCancel->geometry();
  QRect hideRect = mHide->geometry();
  int mtb = style()->pixelMetric( QStyle::PM_LayoutRightMargin );
  int mlr = std::min( width() / 10, mtb );
  if ( rect.width() - cancelRect.x() - cancelRect.width() > mlr )
  {
    // Force right alignment of cancel button
    cancelRect.setX( rect.width() - cancelRect.width() - mlr );
    mCancel->setGeometry( cancelRect );
  }
  mHide->setGeometry( rect.width() - cancelRect.x() - cancelRect.width(),
                      cancelRect.y(), hideRect.width(), cancelRect.height() );
}

// -------------------------

QgsFeatureDownloaderImpl::QgsFeatureDownloaderImpl( QgsBackgroundCachedSharedData *shared, QgsFeatureDownloader *downloader ): mSharedBase( shared ), mDownloader( downloader )
{
  // Needed because used by a signal
  qRegisterMetaType< QVector<QgsFeatureUniqueIdPair> >( "QVector<QgsFeatureUniqueIdPair>" );
}

QgsFeatureDownloaderImpl::~QgsFeatureDownloaderImpl()
{
  if ( mProgressDialog )
    mProgressDialog->deleteLater();
}

void QgsFeatureDownloaderImpl::emitFeatureReceived( QVector<QgsFeatureUniqueIdPair> features )
{
  emit mDownloader->featureReceived( features );
}

void QgsFeatureDownloaderImpl::emitFeatureReceived( long long featureCount )
{
  emit mDownloader->featureReceived( featureCount );
}

void QgsFeatureDownloaderImpl::emitEndOfDownload( bool success )
{
  emit mDownloader->endOfDownload( success );
}

void QgsFeatureDownloaderImpl::emitResumeMainThread()
{
  emit mDownloader->resumeMainThread();
}

void QgsFeatureDownloaderImpl::stop()
{
  QgsDebugMsgLevel( QStringLiteral( "QgsFeatureDownloaderImpl::stop()" ), 4 );
  mStop = true;
  emitDoStop();
}

void QgsFeatureDownloaderImpl::setStopFlag()
{
  QgsDebugMsgLevel( QStringLiteral( "QgsFeatureDownloaderImpl::setStopFlag()" ), 4 );
  mStop = true;
}

void QgsFeatureDownloaderImpl::hideProgressDialog()
{
  mSharedBase->setHideProgressDialog( true );
  mProgressDialog->deleteLater();
  mProgressDialog = nullptr;
}

// Called from GUI thread
void QgsFeatureDownloaderImpl::createProgressDialog( int numberMatched )
{
  Q_ASSERT( qApp->thread() == QThread::currentThread() );

  // Make sure that the creation is done in an atomic way, so that the
  // starting thread (running QgsFeatureDownloaderImpl::run()) can be sure that
  // this function has either run completely, or not at all (mStop == true),
  // when it wants to destroy mProgressDialog
  QMutexLocker locker( &mMutexCreateProgressDialog );

  if ( mStop )
    return;
  Q_ASSERT( !mProgressDialog );

  if ( !mMainWindow )
  {
    const QWidgetList widgets = QgsApplication::topLevelWidgets();
    for ( QWidget *widget : widgets )
    {
      if ( widget->objectName() == QLatin1String( "QgisApp" ) )
      {
        mMainWindow = widget;
        break;
      }
    }
  }

  if ( !mMainWindow )
    return;

  mProgressDialog = new QgsFeatureDownloaderProgressDialog( QObject::tr( "Loading features for layer %1" ).arg( mSharedBase->layerName() ),
      QObject::tr( "Abort" ), 0, numberMatched, mMainWindow );
  mProgressDialog->setWindowTitle( QObject::tr( "QGIS" ) );
  mProgressDialog->setValue( 0 );
  if ( mProgressDialogShowImmediately )
    mProgressDialog->show();
}

void QgsFeatureDownloaderImpl::endOfRun( bool serializeFeatures,
    bool success, int totalDownloadedFeatureCount,
    bool truncatedResponse, bool interrupted,
    const QString &errorMessage )
{
  {
    QMutexLocker locker( &mMutexCreateProgressDialog );
    mStop = true;
  }

  if ( serializeFeatures )
    mSharedBase->endOfDownload( success, totalDownloadedFeatureCount, truncatedResponse, interrupted, errorMessage );
  else if ( !errorMessage.isEmpty() )
    mSharedBase->pushError( errorMessage );

  // We must emit the signal *AFTER* the previous call to mShared->endOfDownload()
  // to avoid issues with iterators that would start just now, wouldn't detect
  // that the downloader has finished, would register to itself, but would never
  // receive the endOfDownload signal. This is not just a theoretical problem.
  // If you switch both calls, it happens quite easily in Release mode with the
  // test suite.
  emitEndOfDownload( success );

  if ( mProgressDialog )
  {
    mProgressDialog->deleteLater();
    mProgressDialog = nullptr;
  }
  if ( mTimer )
  {
    mTimer->deleteLater();
    mTimer = nullptr;
  }
}

// -------------------------


void QgsFeatureDownloader::run( bool serializeFeatures, long long maxFeatures )
{
  Q_ASSERT( mImpl );
  mImpl->run( serializeFeatures, maxFeatures );
}

void QgsFeatureDownloader::stop()
{
  Q_ASSERT( mImpl );
  mImpl->stop();
}

// -------------------------

QgsThreadedFeatureDownloader::QgsThreadedFeatureDownloader( QgsBackgroundCachedSharedData *shared )
  : mShared( shared )
  , mRequestMadeFromMainThread( QThread::currentThread() == QApplication::instance()->thread() )
{
}

QgsThreadedFeatureDownloader::~QgsThreadedFeatureDownloader()
{
  stop();
}

void QgsThreadedFeatureDownloader::stop()
{
  if ( mDownloader )
  {
    mDownloader->stop();
    wait();
    delete mDownloader;
    mDownloader = nullptr;
  }
}

void QgsThreadedFeatureDownloader::startAndWait()
{
  start();

  QMutexLocker locker( &mWaitMutex );
  while ( !mDownloader )
  {
    mWaitCond.wait( &mWaitMutex );
  }
}

void QgsThreadedFeatureDownloader::run()
{
  // We need to construct it in the run() method (i.e. in the new thread)
  mDownloader = new QgsFeatureDownloader();
  mDownloader->setImpl( mShared->newFeatureDownloaderImpl( mDownloader, mRequestMadeFromMainThread ) );
  {
    QMutexLocker locker( &mWaitMutex );
    mWaitCond.wakeOne();
  }
  mDownloader->run( true, /* serialize features */
                    mShared->requestLimit() /* user max features */ );
}

// -------------------------

QgsBackgroundCachedFeatureIterator::QgsBackgroundCachedFeatureIterator(
  QgsBackgroundCachedFeatureSource *source, bool ownSource,
  std::shared_ptr<QgsBackgroundCachedSharedData> shared,
  const QgsFeatureRequest &request )
  : QgsAbstractFeatureIteratorFromSource<QgsBackgroundCachedFeatureSource>( source, ownSource, request )
  , mShared( shared )
  , mCachedFeaturesIter( mCachedFeatures.begin() )
{
  if ( !shared->clientSideFilterExpression().isEmpty() )
  {
    // backup current request because combine filter expression will remove the fid(s) filtering
    if ( mRequest.filterType() == QgsFeatureRequest::FilterFid || mRequest.filterType() == QgsFeatureRequest::FilterFids )
    {
      mAdditionalRequest = QgsFeatureRequest( shared->clientSideFilterExpression() );
    }
    else
    {
      mRequest.combineFilterExpression( shared->clientSideFilterExpression() );
    }
  }

  if ( mRequest.destinationCrs().isValid() && mRequest.destinationCrs() != mShared->sourceCrs() )
  {
    mTransform = QgsCoordinateTransform( mShared->sourceCrs(), mRequest.destinationCrs(), mRequest.transformContext() );
  }
  try
  {
    mFilterRect = filterRectToSourceCrs( mTransform );
  }
  catch ( QgsCsException & )
  {
    // can't reproject mFilterRect
    close();
    return;
  }

  // prepare spatial filter geometries for optimal speed
  switch ( mRequest.spatialFilterType() )
  {
    case Qgis::SpatialFilterType::NoFilter:
    case Qgis::SpatialFilterType::BoundingBox:
      break;

    case Qgis::SpatialFilterType::DistanceWithin:
      if ( !mRequest.referenceGeometry().isEmpty() )
      {
        mDistanceWithinGeom = mRequest.referenceGeometry();
        mDistanceWithinEngine.reset( QgsGeometry::createGeometryEngine( mDistanceWithinGeom.constGet() ) );
        mDistanceWithinEngine->prepareGeometry();
      }
      break;
  }

  // Configurable for the purpose of unit tests
  QString threshold( getenv( "QGIS_WFS_ITERATOR_TRANSFER_THRESHOLD" ) );
  if ( !threshold.isEmpty() )
    mWriteTransferThreshold = threshold.toInt();

  // Particular case: if a single feature or a reasonable set of features
  // are requested by Fid and we already have them in cache, no need to
  // download anything.
  auto cacheDataProvider = mShared->cacheDataProvider();
  if ( cacheDataProvider &&
       ( mRequest.filterType() == QgsFeatureRequest::FilterFid ||
         ( mRequest.filterType() == QgsFeatureRequest::FilterFids && mRequest.filterFids().size() < 100000 ) ) )
  {
    QgsFeatureRequest requestCache;
    QgsFeatureIds qgisIds;
    if ( mRequest.filterType() == QgsFeatureRequest::FilterFid )
      qgisIds.insert( mRequest.filterFid() );
    else
      qgisIds = mRequest.filterFids();
    QgsFeatureIds dbIds = mShared->dbIdsFromQgisIds( qgisIds );
    // Do all requested fids have been known at some point by the provider ?
    if ( dbIds.size() == qgisIds.size() )
    {
      requestCache.setFilterFids( dbIds );
      fillRequestCache( requestCache );
      QgsFeatureIterator cacheIterator = cacheDataProvider->getFeatures( requestCache );
      QgsFeature f;
      while ( cacheIterator.nextFeature( f ) )
      {
        mCachedFeatures << f;
      }
      // Are are the requested fids actually in the cache ?
      if ( mCachedFeatures.size() == dbIds.size() )
      {
        // Yes, no need to download anything.
        mCachedFeaturesIter = mCachedFeatures.begin();
        mDownloadFinished = true;
        return;
      }
      mCachedFeatures.clear();
    }
  }

  int genCounter = ( mShared->isRestrictedToRequestBBOX() && !mFilterRect.isNull() ) ?
                   mShared->registerToCache( this, static_cast<int>( mRequest.limit() ), mFilterRect ) :
                   mShared->registerToCache( this, static_cast<int>( mRequest.limit() ) );
  // Reload cacheDataProvider as registerToCache() has likely refreshed it
  cacheDataProvider = mShared->cacheDataProvider();
  mDownloadFinished = genCounter < 0;
  if ( !cacheDataProvider )
    return;

  QgsDebugMsgLevel( QStringLiteral( "QgsBackgroundCachedFeatureIterator::constructor(): genCounter=%1 " ).arg( genCounter ), 4 );

  QgsFeatureRequest requestCache = initRequestCache( genCounter ) ;
  fillRequestCache( requestCache );
  mCacheIterator = cacheDataProvider->getFeatures( requestCache );
}

QgsFeatureRequest QgsBackgroundCachedFeatureIterator::initRequestCache( int genCounter )
{
  QgsFeatureRequest requestCache;

  const QgsFields fields = mShared->fields();

  auto cacheDataProvider = mShared->cacheDataProvider();
  if ( mRequest.filterType() == QgsFeatureRequest::FilterFid ||
       mRequest.filterType() == QgsFeatureRequest::FilterFids )
  {
    QgsFeatureIds qgisIds;
    if ( mRequest.filterType() == QgsFeatureRequest::FilterFid )
      qgisIds.insert( mRequest.filterFid() );
    else
      qgisIds = mRequest.filterFids();

    requestCache.setFilterFids( mShared->dbIdsFromQgisIds( qgisIds ) );
  }
  else
  {
    if ( mRequest.filterType() == QgsFeatureRequest::FilterExpression &&
         // We cannot filter on geometry because the spatialite geometry is just
         // a bounding box and not the actual geometry of the final feature
         !mRequest.filterExpression()->needsGeometry() )
    {
      // We cannot forward expressions using dateTime fields, because they
      // are stored as milliseconds since UTC epoch in the Spatialite DB.
      bool hasDateTimeFieldInExpr = false;
      const auto setColumns = mRequest.filterExpression()->referencedColumns();
      for ( const auto &columnName : setColumns )
      {
        int idx = fields.indexOf( columnName );
        if ( idx >= 0 && fields[idx].type() == QVariant::DateTime )
        {
          hasDateTimeFieldInExpr = true;
          break;
        }
      }
      if ( !hasDateTimeFieldInExpr )
      {
        // Transfer and transform context
        requestCache.setFilterExpression( mRequest.filterExpression()->expression() );
        QgsExpressionContext ctx { *mRequest.expressionContext( ) };
        QgsExpressionContextScope *scope { ctx.activeScopeForVariable( QgsExpressionContext::EXPR_FIELDS ) };
        if ( scope )
        {
          scope->setVariable( QgsExpressionContext::EXPR_FIELDS, cacheDataProvider->fields() );
        }
        requestCache.setExpressionContext( ctx );
      }
    }
    if ( genCounter >= 0 )
    {
      requestCache.combineFilterExpression( QString( QgsBackgroundCachedFeatureIteratorConstants::FIELD_GEN_COUNTER + " <= %1" ).arg( genCounter ) );
    }
  }

  return requestCache;
}

void QgsBackgroundCachedFeatureIterator::fillRequestCache( QgsFeatureRequest requestCache )
{
  requestCache.setFilterRect( mFilterRect );

  if ( !( mRequest.flags() & QgsFeatureRequest::NoGeometry ) ||
       ( mRequest.spatialFilterType() == Qgis::SpatialFilterType::DistanceWithin ) ||
       ( mRequest.filterType() == QgsFeatureRequest::FilterExpression && mRequest.filterExpression()->needsGeometry() ) )
  {
    mFetchGeometry = true;
  }

  if ( mRequest.flags() & QgsFeatureRequest::SubsetOfAttributes )
  {
    const QgsFields fields = mShared->fields();
    auto cacheDataProvider = mShared->cacheDataProvider();
    QgsFields dataProviderFields = cacheDataProvider->fields();
    QgsAttributeList cacheSubSet;
    const auto subsetOfAttributes = mRequest.subsetOfAttributes();
    for ( int i : subsetOfAttributes )
    {
      int idx = dataProviderFields.indexFromName( mShared->getSpatialiteFieldNameFromUserVisibleName( fields.at( i ).name() ) );
      if ( idx >= 0 )
        cacheSubSet.append( idx );
      idx = fields.indexFromName( fields.at( i ).name() );
      if ( idx >= 0 )
        mSubSetAttributes.append( idx );
    }

    // ensure that all attributes required for expression filter are being fetched
    if ( mRequest.filterType() == QgsFeatureRequest::FilterExpression )
    {
      const auto referencedColumns = mRequest.filterExpression()->referencedColumns();
      for ( const QString &field : referencedColumns )
      {
        int wfsFieldIdx = fields.lookupField( field );
        if ( wfsFieldIdx != -1 )
        {
          int cacheFieldIdx = dataProviderFields.indexFromName( mShared->getSpatialiteFieldNameFromUserVisibleName( fields.at( wfsFieldIdx ).name() ) );
          if ( cacheFieldIdx >= 0 && !cacheSubSet.contains( cacheFieldIdx ) )
            cacheSubSet.append( cacheFieldIdx );

          if ( wfsFieldIdx >= 0  && !mSubSetAttributes.contains( wfsFieldIdx ) )
            mSubSetAttributes.append( wfsFieldIdx );
        }
      }
    }

    // also need attributes required by order by
    if ( mRequest.flags() & QgsFeatureRequest::SubsetOfAttributes && !mRequest.orderBy().isEmpty() )
    {
      const auto usedProviderAttributeIndices = mRequest.orderBy().usedAttributeIndices( dataProviderFields );
      for ( int attrIdx : usedProviderAttributeIndices )
      {
        if ( !cacheSubSet.contains( attrIdx ) )
          cacheSubSet.append( attrIdx );
      }

      const auto usedSharedAttributeIndices = mRequest.orderBy().usedAttributeIndices( fields );
      for ( int attrIdx : usedSharedAttributeIndices )
      {
        if ( !mSubSetAttributes.contains( attrIdx ) )
          mSubSetAttributes.append( attrIdx );
      }
    }

    if ( mFetchGeometry )
    {
      int hexwkbGeomIdx = dataProviderFields.indexFromName( QgsBackgroundCachedFeatureIteratorConstants::FIELD_HEXWKB_GEOM );
      Q_ASSERT( hexwkbGeomIdx >= 0 );
      cacheSubSet.append( hexwkbGeomIdx );
    }
    requestCache.setSubsetOfAttributes( cacheSubSet );
  }
}

QgsBackgroundCachedFeatureIterator::~QgsBackgroundCachedFeatureIterator()
{
  QgsDebugMsgLevel( QStringLiteral( "QgsBackgroundCachedFeatureIterator::~QgsBackgroundCachedFeatureIterator()" ), 4 );

  close();

  QMutexLocker locker( &mMutex );
  if ( mWriterStream )
  {
    mWriterStream.reset();
    mWriterFile.reset();
    if ( !mWriterFilename.isEmpty() )
    {
      QFile::remove( mWriterFilename );
      mShared->releaseCacheDirectory();
    }
  }

  cleanupReaderStreamAndFile();
}

void QgsBackgroundCachedFeatureIterator::connectSignals( QgsFeatureDownloader *downloader )
{
  // We want to run the slot for that signal in the same thread as the sender
  // so as to avoid the list of features to accumulate without control in
  // memory
  connect( downloader, static_cast<void ( QgsFeatureDownloader::* )( QVector<QgsFeatureUniqueIdPair> )>( &QgsFeatureDownloader::featureReceived ),
           this, &QgsBackgroundCachedFeatureIterator::featureReceivedSynchronous, Qt::DirectConnection );

  connect( downloader, &QgsFeatureDownloader::endOfDownload,
           this, &QgsBackgroundCachedFeatureIterator::endOfDownloadSynchronous, Qt::DirectConnection );

  connect( downloader, &QgsFeatureDownloader::resumeMainThread,
           this, &QgsBackgroundCachedFeatureIterator::resumeMainThreadSynchronous, Qt::DirectConnection );
}

void QgsBackgroundCachedFeatureIterator::endOfDownloadSynchronous( bool )
{
  // Wake up waiting loop
  QMutexLocker locker( &mMutex );
  mDownloadFinished = true;
  mWaitCond.wakeOne();
}

void QgsBackgroundCachedFeatureIterator::resumeMainThreadSynchronous()
{
  // Wake up waiting loop
  QMutexLocker locker( &mMutex );
  mProcessEvents = true;
  mWaitCond.wakeOne();
}

void QgsBackgroundCachedFeatureIterator::setInterruptionChecker( QgsFeedback *interruptionChecker )
{
  mInterruptionChecker = interruptionChecker;
}

// This method will serialize the receive feature list, first into memory, and
// if it goes above a given threshold, on disk
void QgsBackgroundCachedFeatureIterator::featureReceivedSynchronous( const QVector<QgsFeatureUniqueIdPair> &list )
{
  QgsDebugMsgLevel( QStringLiteral( "QgsBackgroundCachedFeatureIterator::featureReceivedSynchronous %1 features" ).arg( list.size() ), 4 );
  QMutexLocker locker( &mMutex );

  // Wake up waiting loop
  mNewFeaturesReceived = true;
  mWaitCond.wakeOne();

  if ( !mWriterStream )
  {
    mWriterStream.reset( new QDataStream( &mWriterByteArray, QIODevice::WriteOnly ) );
  }
  const auto constList = list;
  for ( const QgsFeatureUniqueIdPair &pair : constList )
  {
    *mWriterStream << pair.first;
  }
  if ( !mWriterFile && mWriterByteArray.size() > mWriteTransferThreshold )
  {
    const QString thisStr = QStringLiteral( "%1" ).arg( reinterpret_cast< quintptr >( this ), QT_POINTER_SIZE * 2, 16, QLatin1Char( '0' ) );
    ++ mCounter;
    mWriterFilename = QDir( mShared->acquireCacheDirectory() ).filePath( QStringLiteral( "iterator_%1_%2.bin" ).arg( thisStr ).arg( mCounter ) );
    QgsDebugMsgLevel( QStringLiteral( "Transferring feature iterator cache to %1" ).arg( mWriterFilename ), 4 );
    mWriterFile.reset( new QFile( mWriterFilename ) );
    if ( !mWriterFile->open( QIODevice::WriteOnly | QIODevice::Truncate ) )
    {
      QgsDebugMsg( QStringLiteral( "Cannot open %1 for writing" ).arg( mWriterFilename ) );
      mWriterFile.reset();
      mWriterFilename.clear();
      mShared->releaseCacheDirectory();
      return;
    }
    mWriterFile->write( mWriterByteArray );
    mWriterByteArray.clear();
    mWriterStream->setDevice( mWriterFile.get() );
  }
}


bool QgsBackgroundCachedFeatureIterator::fetchFeature( QgsFeature &f )
{
  f.setValid( false );

  if ( mClosed )
    return false;

  // First step is to iterate over the on-disk cache
  QgsFeature cachedFeature;
  while ( true )
  {
    if ( !mCachedFeatures.empty() )
    {
      if ( mCachedFeaturesIter == mCachedFeatures.end() )
        return false;
      cachedFeature = *mCachedFeaturesIter;
      ++mCachedFeaturesIter;
    }
    else
    {
      if ( !mCacheIterator.nextFeature( cachedFeature ) )
        break;
    }

    if ( mInterruptionChecker && mInterruptionChecker->isCanceled() )
      return false;

    if ( mTimeoutOrInterruptionOccurred )
      return false;

    //QgsDebugMsg(QString("QgsBackgroundCachedSharedData::fetchFeature() : mCacheIterator.nextFeature(cachedFeature)") );

    if ( mShared->hasGeometry() && mFetchGeometry )
    {
      int idx = cachedFeature.fields().indexFromName( QgsBackgroundCachedFeatureIteratorConstants::FIELD_HEXWKB_GEOM );
      Q_ASSERT( idx >= 0 );

      const QVariant &v = cachedFeature.attributes().value( idx );
      if ( !v.isNull() && v.type() == QVariant::String )
      {
        QByteArray wkbGeom( QByteArray::fromHex( v.toString().toLatin1() ) );
        QgsGeometry g;
        try
        {
          g.fromWkb( wkbGeom );
          cachedFeature.setGeometry( g );
        }
        catch ( const QgsWkbException & )
        {
          QgsDebugMsg( QStringLiteral( "Invalid WKB for cached feature %1" ).arg( cachedFeature.id() ) );
          cachedFeature.clearGeometry();
        }
      }
      else
      {
        cachedFeature.clearGeometry();
      }
    }
    else
    {
      cachedFeature.clearGeometry();
    }

    QgsGeometry constGeom = cachedFeature.geometry();
    if ( !mFilterRect.isNull() &&
         ( constGeom.isNull() || !constGeom.intersects( mFilterRect ) ) )
    {
      continue;
    }

    if ( !mAdditionalRequest.acceptFeature( cachedFeature ) )
    {
      continue;
    }

    copyFeature( cachedFeature, f, true );
    geometryToDestinationCrs( f, mTransform );

    if ( mDistanceWithinEngine && mDistanceWithinEngine->distance( f.geometry().constGet() ) > mRequest.distanceWithin() )
      continue;

    // Retrieve the user-visible id from the Spatialite cache database Id
    QgsFeatureId userVisibleId;
    if ( mShared->getUserVisibleIdFromSpatialiteId( cachedFeature.id(), userVisibleId ) )
    {
      f.setId( userVisibleId );
    }

    return true;
  }

  const bool requestMadeFromMainThread = QThread::currentThread() == QApplication::instance()->thread();

  // Second step is to wait for features to be notified by the downloader
  while ( true )
  {
    {
      QMutexLocker locker( &mMutex );
      mNewFeaturesReceived = false;
    }

    // Initialize a reader stream if there's a writer stream available
    if ( !mReaderStream )
    {
      {
        QMutexLocker locker( &mMutex );
        if ( mWriterStream )
        {
          // Transfer writer variables to the reader
          mWriterStream.reset();
          mWriterFile.reset();
          mReaderByteArray = mWriterByteArray;
          mWriterByteArray.clear();
          mReaderFilename = mWriterFilename;
          mWriterFilename.clear();
        }
      }
      // Instantiates the reader stream from memory buffer if not empty
      if ( !mReaderByteArray.isEmpty() )
      {
        mReaderStream.reset( new QDataStream( &mReaderByteArray, QIODevice::ReadOnly ) );
      }
      // Otherwise from the on-disk file
      else if ( !mReaderFilename.isEmpty() )
      {
        mReaderFile.reset( new QFile( mReaderFilename ) );
        if ( !mReaderFile->open( QIODevice::ReadOnly ) )
        {
          QgsDebugMsg( QStringLiteral( "Cannot open %1" ).arg( mReaderFilename ) );
          mReaderFile.reset();
          return false;
        }
        mReaderStream.reset( new QDataStream( mReaderFile.get() ) );
      }
    }

    // Read from the stream
    if ( mReaderStream )
    {
      while ( !mReaderStream->atEnd() )
      {
        if ( mInterruptionChecker && mInterruptionChecker->isCanceled() )
          return false;

        QgsFeature feat;
        ( *mReaderStream ) >> feat;
        // We need to re-attach fields explicitly
        feat.setFields( mShared->fields() );

        if ( mRequest.filterType() == QgsFeatureRequest::FilterFid )
        {
          if ( feat.id() != mRequest.filterFid() )
          {
            continue;
          }
        }
        else if ( mRequest.filterType() == QgsFeatureRequest::FilterFids )
        {
          if ( !mRequest.filterFids().contains( feat.id() ) )
          {
            continue;
          }
        }

        QgsGeometry constGeom = feat.geometry();
        if ( !mFilterRect.isNull() &&
             ( constGeom.isNull() || !constGeom.intersects( mFilterRect ) ) )
        {
          continue;
        }

        if ( !mAdditionalRequest.acceptFeature( feat ) )
        {
          continue;
        }

        copyFeature( feat, f, false );

        geometryToDestinationCrs( f, mTransform );

        if ( mDistanceWithinEngine && mDistanceWithinEngine->distance( f.geometry().constGet() ) > mRequest.distanceWithin() )
          continue;

        return true;
      }

      // When the stream is finished, cleanup
      cleanupReaderStreamAndFile();

      // And try again to check if there's a new output stream to read from
      continue;
    }

    if ( mDownloadFinished )
      return false;
    if ( mInterruptionChecker && mInterruptionChecker->isCanceled() )
      return false;

    // Wait for:
    // - mRequest.timeout to fire
    // - or new features to be notified
    // - or end of download being notified
    // - or interruption checker to notify cancellation
    QElapsedTimer timeRequestTimeout;
    const int requestTimeout = mRequest.timeout();
    if ( requestTimeout > 0 )
      timeRequestTimeout.start();
    while ( true )
    {
      QMutexLocker locker( &mMutex );
      if ( mNewFeaturesReceived || mDownloadFinished )
      {
        break;
      }
      const int delayCheckInterruption = 50;
      const int timeout = ( requestTimeout > 0 ) ?
                          std::min( requestTimeout - ( int ) timeRequestTimeout.elapsed(), delayCheckInterruption ) :
                          delayCheckInterruption;
      if ( timeout < 0 )
      {
        mTimeoutOrInterruptionOccurred = true;
        mDownloadFinished = true;
        break;
      }
      mWaitCond.wait( &mMutex, timeout );
      if ( requestMadeFromMainThread && mProcessEvents )
      {
        QApplication::processEvents();
        mProcessEvents = false;
      }
      if ( mInterruptionChecker && mInterruptionChecker->isCanceled() )
      {
        mTimeoutOrInterruptionOccurred = true;
        mDownloadFinished = true;
        break;
      }
    }

  }
}

void QgsBackgroundCachedFeatureIterator::cleanupReaderStreamAndFile()
{
  if ( mReaderStream )
  {
    mReaderStream.reset();
    mReaderFile.reset();
    mReaderByteArray.clear();
    if ( !mReaderFilename.isEmpty() )
    {
      QFile::remove( mReaderFilename );
      mReaderFilename.clear();
      mShared->releaseCacheDirectory();
    }
  }
}

bool QgsBackgroundCachedFeatureIterator::rewind()
{
  if ( mClosed )
    return false;

  if ( !mCachedFeatures.empty() )
  {
    mCachedFeaturesIter = mCachedFeatures.begin();
  }
  else
  {
    cleanupReaderStreamAndFile();

    QgsFeatureRequest requestCache;
    int genCounter = mShared->getUpdatedCounter();
    if ( genCounter >= 0 )
      requestCache.setFilterExpression( QString( QgsBackgroundCachedFeatureIteratorConstants::FIELD_GEN_COUNTER + " <= %1" ).arg( genCounter ) );
    else
      mDownloadFinished = true;
    auto cacheDataProvider = mShared->cacheDataProvider();
    if ( cacheDataProvider )
      mCacheIterator = cacheDataProvider->getFeatures( requestCache );
  }

  return true;
}

bool QgsBackgroundCachedFeatureIterator::close()
{
  if ( mClosed )
    return false;
  QgsDebugMsgLevel( QStringLiteral( "QgsBackgroundCachedFeatureIterator::close()" ), 4 );

  iteratorClosed();

  mClosed = true;
  return true;
}


void QgsBackgroundCachedFeatureIterator::copyFeature( const QgsFeature &srcFeature, QgsFeature &dstFeature, bool srcIsCache )
{
  //copy the geometry
  QgsGeometry geometry = srcFeature.geometry();
  if ( mShared->hasGeometry() && !geometry.isNull() )
  {
    dstFeature.setGeometry( geometry );
  }
  else
  {
    dstFeature.clearGeometry();
  }

  //and the attributes
  const QgsFields &fields = mShared->fields();
  dstFeature.initAttributes( fields.size() );

  auto setAttr = [ & ]( const int i )
  {
    int idx = srcFeature.fields().indexFromName( srcIsCache ? mShared->getSpatialiteFieldNameFromUserVisibleName( fields.at( i ).name() ) : fields.at( i ).name() );
    if ( idx >= 0 )
    {
      const QVariant &v = srcFeature.attributes().value( idx );
      const QVariant::Type fieldType = fields.at( i ).type();
      if ( v.isNull() )
        dstFeature.setAttribute( i, QVariant( fieldType ) );
      else if ( QgsWFSUtils::isCompatibleType( v.type(), fieldType ) )
        dstFeature.setAttribute( i, v );
      else if ( fieldType == QVariant::DateTime && !v.isNull() )
        dstFeature.setAttribute( i, QVariant( QDateTime::fromMSecsSinceEpoch( v.toLongLong() ) ) );
      else
        dstFeature.setAttribute( i, QgsVectorDataProvider::convertValue( fieldType, v.toString() ) );
    }
  };

  if ( mRequest.flags() & QgsFeatureRequest::SubsetOfAttributes )
  {
    for ( auto i : std::as_const( mSubSetAttributes ) )
    {
      setAttr( i );
    }
  }
  else
  {
    for ( int i = 0; i < fields.size(); i++ )
    {
      setAttr( i );
    }
  }

  //id and valid
  dstFeature.setValid( true );
  dstFeature.setId( srcFeature.id() );
  dstFeature.setFields( fields ); // allow name-based attribute lookups
}


// -------------------------

QgsBackgroundCachedFeatureSource::QgsBackgroundCachedFeatureSource(
  std::shared_ptr<QgsBackgroundCachedSharedData> shared )
  : mShared( shared )
{
}

QgsFeatureIterator QgsBackgroundCachedFeatureSource::getFeatures( const QgsFeatureRequest &request )
{
  return QgsFeatureIterator( new QgsBackgroundCachedFeatureIterator( this, false, mShared, request ) );
}
