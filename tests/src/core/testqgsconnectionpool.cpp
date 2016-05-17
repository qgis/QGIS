/***************************************************************************
                         testqgsconnectionpool.cpp
                         -----------------------
    begin                : April 2016
    copyright            : (C) 2016 by Sandro Mani
    email                : manisandro@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsapplication.h"
#include "qgsgeometry.h"
#include "qgspointv2.h"
#include "qgslinestringv2.h"
#include "qgsvectorlayer.h"
#include <QEventLoop>
#include <QObject>
#include <QTemporaryFile>
#include <QtConcurrentMap>
#include <QtTest/QtTest>

class TestQgsConnectionPool: public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();
    void cleanupTestCase();
    void layersFromSameDatasetGPX();

  private:
    struct ReadJob
    {
      explicit ReadJob( QgsVectorLayer* _layer ) : layer( _layer ) {}
      QgsVectorLayer* layer;
      QList<QgsFeature> features;
    };

    static void processJob( ReadJob& job )
    {
      QgsFeatureIterator it = job.layer->getFeatures();
      QgsFeature f;
      while ( it.nextFeature( f ) )
      {
        job.features.append( f );
      }
    }

};

void TestQgsConnectionPool::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

}

void TestQgsConnectionPool::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsConnectionPool::layersFromSameDatasetGPX()
{
  // Tests whether features are correctly retrevied from different layers which are
  // loaded from the same dataset. See issue #14560
  int nWaypoints = 100000;
  int nRoutes = 100000;
  int nRoutePts = 10;
  QTemporaryFile testFile( "testXXXXXX.gpx" );
  testFile.setAutoRemove( false );
  testFile.open();
  testFile.write( "<gpx version=\"1.1\" creator=\"qgis\">\n" );
  for ( int i = 0; i < nWaypoints; ++i )
  {
    testFile.write( QString( "<wpt lon=\"%1\" lat=\"%1\"><name></name></wpt>\n" ).arg( i ).toLocal8Bit() );
  }
  for ( int i = 0; i < nRoutes; ++i )
  {
    testFile.write( "<rte><name></name><number></number>\n" );
    for ( int j = 0; j < nRoutePts; ++j )
    {
      testFile.write( QString( "<rtept lon=\"%1\" lat=\"%2\"/>\n" ).arg( j ).arg( i ).toLocal8Bit() );
    }
    testFile.write( "</rte>\n" );
  }
  testFile.write( "</gpx>\n" );
  testFile.close();

  QgsVectorLayer* layer1 = new QgsVectorLayer( testFile.fileName() + "|layername=waypoints", "Waypoints", "ogr" );
  QVERIFY( layer1->isValid() );
  QgsVectorLayer* layer2 = new QgsVectorLayer( testFile.fileName() + "|layername=routes", "Routes", "ogr" );
  QVERIFY( layer2->isValid() );

  QList<ReadJob> jobs = QList<ReadJob>() << ReadJob( layer1 ) << ReadJob( layer2 );

  QEventLoop evLoop;
  QFutureWatcher<void> futureWatcher;
  connect( &futureWatcher, SIGNAL( finished() ), &evLoop, SLOT( quit() ) );
  futureWatcher.setFuture( QtConcurrent::map( jobs, processJob ) );
  evLoop.exec();

  QList<QgsFeature>& layer1Features = jobs[0].features;
  QList<QgsFeature>& layer2Features = jobs[1].features;

  QVERIFY( layer1Features.count() == nWaypoints );
  QVERIFY( layer2Features.count() == nRoutes );

  for ( int i = 0, n = layer1Features.count(); i < n; ++i )
  {
    const QgsPointV2* geom = dynamic_cast<const QgsPointV2*>( layer1Features[i].geometry()->geometry() );
    QVERIFY( geom != nullptr );
    QVERIFY( qFuzzyCompare( geom->x(), i ) );
    QVERIFY( qFuzzyCompare( geom->y(), i ) );
  }
  for ( int i = 0, n = layer2Features.count(); i < n; ++i )
  {
    const QgsLineStringV2* geom = dynamic_cast<const QgsLineStringV2*>( layer2Features[i].geometry()->geometry() );
    QVERIFY( geom != nullptr );
    int nVtx = geom->vertexCount();
    QVERIFY( nVtx == nRoutePts );
    for ( int j = 0; j < nVtx; ++j )
    {
      QgsPointV2 p = geom->vertexAt( QgsVertexId( 0, 0, j ) );
      QVERIFY( qFuzzyCompare( p.x(), j ) );
      QVERIFY( qFuzzyCompare( p.y(), i ) );
    }
  }
  delete layer1;
  delete layer2;
  QFile( testFile.fileName() ).remove();
}

QTEST_MAIN( TestQgsConnectionPool )
#include "testqgsconnectionpool.moc"
