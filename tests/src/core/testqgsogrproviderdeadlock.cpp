/***************************************************************************
  testqgsogrproviderlock.cpp - TestQgsOgrProviderLock

 ---------------------
 begin                : June 2026
 copyright            : (C) 2026 by Jan Caha
 email                : jan.caha at outlook dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgstest.h"

#include <QString>

using namespace Qt::StringLiterals;

//qgis includes...
#include "qgis.h"
#include "qgsapplication.h"
#include "qgsogrproviderutils.h"
#include "qgstestutils.h"

#include <QDeadlineTimer>
#include <QObject>
#include <QThread>

#include <atomic>

#include <gdal.h>
#include <ogr_api.h>

// QgsOgrLayerReleaser::operator() is defined in qgsogrproviderutils.cpp but
// its symbol is not exported from qgis_core. Provide a local definition so
// this translation unit links without requiring an export macro on the struct.
void QgsOgrLayerReleaser::operator()( QgsOgrLayer *layer ) const
{
  QgsOgrProviderUtils::release( layer );
}

// ==========
// Worker Threads
// ==========

//! hammer QgsOgrProviderUtils::getLayer()/release() on one dataset
class OgrLayerStressWorker : public QThread
{
    Q_OBJECT

  public:
    OgrLayerStressWorker( const QString &dsName, const QString &layerName, bool updateMode, std::atomic<bool> &stop )
      : mDsName( dsName )
      , mLayerName( layerName )
      , mUpdateMode( updateMode )
      , mStop( stop )
    {}

    void run() override
    {
      for ( int i = 0; !mStop; i++ )
      {
        mLoops++;
        QString errCause;
        QgsOgrLayerUniquePtr layer = QgsOgrProviderUtils::getLayer( mDsName, mUpdateMode, QStringList(), mLayerName, errCause, true );
        if ( !layer )
        {
          mFailedAcquisitions++;
          continue;
        }
        mAcquisitions++;

        // briefly keep the layer in use, so that concurrent requests for the
        // same layer have to open additional datasets and the dataset reuse
        // and closing paths of getLayer()/release() are exercised
        QThread::usleep( 100 );

        // transaction code path: acquire and release the shared update dataset
        if ( mUpdateMode && i % 5 == 0 )
        {
          QgsOgrDatasetSharedPtr ds = QgsOgrProviderUtils::getAlreadyOpenedDataset( mDsName );
          Q_UNUSED( ds )
        }
      }
    }

    int loops() const { return mLoops; }
    int acquisitions() const { return mAcquisitions; }
    int failedAcquisitions() const { return mFailedAcquisitions; }

  private:
    QString mDsName;
    QString mLayerName;
    bool mUpdateMode = false;
    std::atomic<bool> &mStop;
    int mLoops = 0;
    int mAcquisitions = 0;
    int mFailedAcquisitions = 0;
};

//! Concurrently invalidates the dataset cache, racing against getLayer()/release()
class OgrCacheInvalidator : public QThread
{
    Q_OBJECT

  public:
    OgrCacheInvalidator( const QString &dsName, std::atomic<bool> &stop )
      : mDsName( dsName )
      , mStop( stop )
    {}

    void run() override
    {
      while ( !mStop )
      {
        QgsOgrProviderUtils::invalidateCachedDatasets( mDsName );
        QgsOgrProviderUtils::invalidateCachedLastModifiedDate( mDsName );
        // long enough that shared datasets live between invalidations and
        // threads actually contend on reusing them
        QThread::msleep( 250 );
      }
    }

  private:
    QString mDsName;
    std::atomic<bool> &mStop;
};

// ==========
// Test
// ==========

/**
 * \ingroup UnitTests
 * Tests for deadlocks between the global mutex of QgsOgrProviderUtils
 * (protecting the shared dataset cache) and the per-dataset mutexes of
 * QgsOgrLayer, when one GDAL dataset is shared among layers used from
 * several threads -- as happens when the browser queries sublayers or
 * creates children while layers on the same file are opened, renamed or
 * closed.
 */
class TestQgsOgrProviderDeadLock : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsOgrProviderDeadLock()
      : QgsTest( u"OGR Provider DeadLock Tests"_s )
    {}

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.

    void testConcurrentLayerAccess();

  private:
    QTemporaryDir mDir;
    QString mDsName;
};


//runs before all tests
void TestQgsOgrProviderDeadLock::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  QVERIFY( mDir.isValid() );
  mDsName = mDir.path() + u"/concurrent_access.gpkg"_s;

  // build a two-layer GPKG so threads contend on the same shared dataset
  GDALDriverH hDriver = GDALGetDriverByName( "GPKG" );
  QVERIFY( hDriver );
  GDALDatasetH hDS = GDALCreate( hDriver, mDsName.toUtf8().constData(), 0, 0, 0, GDT_Unknown, nullptr );
  QVERIFY( hDS );
  for ( const char *layerName : { "layer1", "layer2" } )
  {
    OGRLayerH hLayer = GDALDatasetCreateLayer( hDS, layerName, nullptr, wkbPoint, nullptr );
    QVERIFY( hLayer );
    OGRFieldDefnH hFieldDefn = OGR_Fld_Create( "name", OFTString );
    QCOMPARE( OGR_L_CreateField( hLayer, hFieldDefn, TRUE ), OGRERR_NONE );
    OGR_Fld_Destroy( hFieldDefn );
    for ( int i = 0; i < 10; i++ )
    {
      OGRFeatureH hFeature = OGR_F_Create( OGR_L_GetLayerDefn( hLayer ) );
      OGR_F_SetFieldString( hFeature, 0, "point" );
      OGRGeometryH hGeometry = OGR_G_CreateGeometry( wkbPoint );
      OGR_G_SetPoint_2D( hGeometry, 0, i, i );
      OGR_F_SetGeometryDirectly( hFeature, hGeometry );
      QCOMPARE( OGR_L_CreateFeature( hLayer, hFeature ), OGRERR_NONE );
      OGR_F_Destroy( hFeature );
    }
  }
  GDALClose( hDS );
}

//runs after all tests
void TestQgsOgrProviderDeadLock::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsOgrProviderDeadLock::testConcurrentLayerAccess()
{
  const QString dsName = mDsName;

  constexpr int WORKER_COUNT = 8;
  constexpr int RUN_TIME_MS = 2000;
  constexpr int TIMEOUT_MS = 60000;

  std::atomic<bool> stop( false );

  std::vector<std::unique_ptr<QThread>> threads;
  const auto addThread = [&threads]( auto *thread ) {
    threads.emplace_back( thread );
    return thread;
  };

  addThread( new OgrCacheInvalidator( dsName, stop ) );

  std::vector<OgrLayerStressWorker *> workers;
  for ( int i = 0; i < WORKER_COUNT; i++ )
  {
    workers.push_back( addThread( new OgrLayerStressWorker( dsName, i == 0 ? u"layer1"_s : u"layer2"_s, i < 2, stop ) ) );
  }

  for ( auto &thread : threads )
    thread->start();

  // Run the generic concurrent access stress test from this thread while the
  // OGR specific workers hammer the same dataset in the background: its layer
  // opens run loadMetadata() and its sublayer queries acquire the dataset
  // mutex of the shared dataset and the global mutex in close succession,
  // racing against getLayer() calls from the stress workers which take them
  // in the opposite order. It blocks for RUN_TIME_MS, defining the contention
  // window, and returns FALSE if its own threads deadlock.
  const bool genericTestPassed = QgsTestUtils::testConcurrentLayerAccess( dsName, u"ogr"_s, RUN_TIME_MS, TIMEOUT_MS );
  stop = true;

  // Use a single shared deadline across all threads: if the mutex ordering is
  // broken, some threads will deadlock and never return from wait(). A per-
  // thread timeout would reset the clock for each thread, allowing an
  // arbitrarily long hang to go undetected.
  QDeadlineTimer deadline( TIMEOUT_MS );
  bool allFinished = true;
  for ( auto &thread : threads )
    allFinished = thread->wait( deadline ) && allFinished;

  if ( !allFinished )
  {
    // deliberately leak the thread objects: some threads are deadlocked and
    // destroying a still-running QThread is a fatal error which would mask
    // the failure report below
    // this only leaks if the process actually deadlocks but we need to report
    // the deadlock, so it is intended and acceptable here
    for ( auto &thread : threads )
      ( void ) thread.release();
  }

  // a thread stuck past the deadline means a deadlock between the global mutex
  // and a dataset mutex (we cannot recover the stuck threads, only report)
  QVERIFY2( allFinished, "Deadlock detected: not all threads accessing the OGR dataset finished in time" );
  QVERIFY2( genericTestPassed, "Generic concurrent layer access test deadlocked or gave inconsistent results" );

  for ( const OgrLayerStressWorker *worker : workers )
  {
    QCOMPARE( worker->failedAcquisitions(), 0 );
    QCOMPARE( worker->acquisitions(), worker->loops() );
    QVERIFY( worker->acquisitions() > 0 );
  }
}

QGSTEST_MAIN( TestQgsOgrProviderDeadLock )
#include "testqgsogrproviderdeadlock.moc"
