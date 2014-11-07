
#include <QtTest/QtTest>
#include <QObject>

#include "qgsapplication.h"
#include "qgsmaplayerregistry.h"
#include "qgsmaprenderercache.h"
#include "qgsmaprendererjob.h"
#include "qgsvectorlayer.h"

class TestQgsMapRendererJob : public QObject
{
    Q_OBJECT
  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.

    void testNormal();

    void testTwoTimes();

    void testCancelWithoutStart();
    void testWaitWithoutStart();
    void testCancelFinished();
    void testStartWhileRunning();
    void testDestructWhileRunning();

    void testErrors();

    void testCache();

  private:
    QStringList mLayerIds;
};

static QString _loadLayer( QString path )
{
  QgsMapLayer* layer = new QgsVectorLayer( path, "testlayer", "ogr" );
  Q_ASSERT( layer->isValid() );
  QgsMapLayerRegistry::instance()->addMapLayer( layer );
  return layer->id();
}

static QgsMapSettings _mapSettings( QStringList layerIds )
{
  QgsMapSettings settings;
  settings.setLayers( layerIds );
  settings.setExtent( settings.fullExtent() );
  settings.setOutputSize( QSize( 512, 512 ) );
  return settings;
}


void TestQgsMapRendererJob::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  mLayerIds << _loadLayer( "/data/gis/sas/trans-trail-l.dbf" );
  mLayerIds << _loadLayer( "/data/gis/sas/bnd-ocean-a.shp" );
  mLayerIds << _loadLayer( "/data/gis/sas/bnd-political-boundary-a.shp" );

}

void TestQgsMapRendererJob::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsMapRendererJob::testNormal()
{
  QgsMapSettings settings( _mapSettings( mLayerIds ) );

  QImage imgS, imgP;

  QgsMapRendererSequentialJob jobS( settings );
  QCOMPARE( jobS.isActive(), false );
  jobS.start();
  QCOMPARE( jobS.isActive(), true );
  jobS.waitForFinished();
  QCOMPARE( jobS.isActive(), false );
  imgS = jobS.renderedImage();

  //

  QgsMapRendererParallelJob jobP( settings );
  QCOMPARE( jobP.isActive(), false );
  jobP.start();
  QCOMPARE( jobP.isActive(), true );
  jobP.waitForFinished();
  QCOMPARE( jobP.isActive(), false );
  imgP = jobP.renderedImage();

  // TODO: custom painter

  imgS.save( "/tmp/imgS.png" );
  imgP.save( "/tmp/imgP.png" );
  //img.save("/tmp/img5.png");

  QCOMPARE( imgS, imgP );
}

void TestQgsMapRendererJob::testTwoTimes()
{
  QgsMapSettings settings( _mapSettings( mLayerIds ) );

  QgsMapRendererSequentialJob job( settings );
  job.start();
  job.waitForFinished();

  QImage img1 = job.renderedImage();

  job.start();
  job.waitForFinished();

  QImage img2 = job.renderedImage();

  QVERIFY( img1 == img2 );

  //

  QgsMapRendererParallelJob jobP( settings );
  jobP.start();
  jobP.waitForFinished();

  QImage img3 = jobP.renderedImage();

  jobP.start();
  jobP.waitForFinished();

  QImage img4 = jobP.renderedImage();

  QVERIFY( img3 == img4 );
}


void TestQgsMapRendererJob::testCancelWithoutStart()
{
  QgsMapSettings settings( _mapSettings( mLayerIds ) );

  QgsMapRendererSequentialJob job( settings );
  job.cancel();

  //

  QgsMapRendererParallelJob jobP( settings );
  jobP.cancel();
}

void TestQgsMapRendererJob::testWaitWithoutStart()
{
  QgsMapSettings settings( _mapSettings( mLayerIds ) );

  QgsMapRendererSequentialJob job( settings );
  job.waitForFinished();

  //

  QgsMapRendererParallelJob jobP( settings );
  jobP.waitForFinished();
}

void TestQgsMapRendererJob::testCancelFinished()
{
  QgsMapSettings settings( _mapSettings( mLayerIds ) );

  QgsMapRendererSequentialJob job( settings );
  job.start();
  job.waitForFinished();
  job.cancel();

  //

  QgsMapRendererParallelJob jobP( settings );
  jobP.start();
  jobP.waitForFinished();
  jobP.cancel();
}

void TestQgsMapRendererJob::testStartWhileRunning()
{
  QgsMapSettings settings( _mapSettings( mLayerIds ) );

  QgsMapRendererSequentialJob job( settings );
  job.start();
  job.start();
  job.waitForFinished();

  //

  QgsMapRendererParallelJob jobP( settings );
  jobP.start();
  jobP.start();
  jobP.waitForFinished();
}

void TestQgsMapRendererJob::testDestructWhileRunning()
{
  QgsMapSettings settings( _mapSettings( mLayerIds ) );

  QgsMapRendererSequentialJob job( settings );
  job.start();

  //

  QgsMapRendererParallelJob jobP( settings );
  jobP.start();
}

void TestQgsMapRendererJob::testErrors()
{
  QgsVectorLayer* l = new QgsVectorLayer( "/data/gis/sas/trans-trail-l.dbf", "test", "ogr" );
  QVERIFY( l->isValid() );
  QgsMapLayerRegistry::instance()->addMapLayer( l );
  QgsMapSettings settings( _mapSettings( QStringList( l->id() ) ) );

  l->setRendererV2( 0 ); // this has to produce an error

  QgsMapRendererSequentialJob job( settings );
  job.start();
  job.waitForFinished();

  QCOMPARE( job.errors().count(), 1 );
  QCOMPARE( job.errors()[0].layerID, l->id() );

  QgsMapLayerRegistry::instance()->removeMapLayer( l->id() );

  QString fakeLayerID = "non-existing layer ID";
  QgsMapSettings settings2( _mapSettings( QStringList( fakeLayerID ) ) );

  QgsMapRendererSequentialJob job2( settings2 );
  job2.start();
  job2.waitForFinished();

  QCOMPARE( job2.errors().count(), 1 );
  QCOMPARE( job2.errors()[0].layerID, fakeLayerID );
}

void TestQgsMapRendererJob::testCache()
{
  QgsVectorLayer* l = new QgsVectorLayer( "/data/gis/sas/trans-trail-l.dbf", "test", "ogr" );
  QVERIFY( l->isValid() );
  QgsMapLayerRegistry::instance()->addMapLayer( l );
  QgsMapSettings settings( _mapSettings( QStringList( l->id() ) ) );

  QgsMapRendererCache cache;

  QgsMapRendererSequentialJob job( settings );
  job.setCache( &cache );

  QTime t;
  t.start();
  job.start();
  job.waitForFinished();
  int timeNotCachedMS = t.elapsed();

  QImage i1 = job.renderedImage();

  QgsMapRendererSequentialJob job2( settings );
  job2.setCache( &cache );

  t.start();
  job2.start();
  job2.waitForFinished();
  int timeCachedMS = t.elapsed();

  QImage i2 = job2.renderedImage();

  QCOMPARE( i1, i2 );
  QVERIFY( timeNotCachedMS > 100 );
  QVERIFY( timeCachedMS < 10 );
  qDebug( "CACHING %d vs %d (ms)", timeNotCachedMS, timeCachedMS );

  QgsMapLayerRegistry::instance()->removeMapLayer( l->id() );
}


QTEST_MAIN( TestQgsMapRendererJob )
#include "testmaprendererjob.moc"
