/***************************************************************************
     test_template.cpp
     --------------------------------------
    Date                 : Sun Sep 16 12:22:23 AKDT 2007
    Copyright            : (C) 2007 by Gary E. Sherman
    Email                : sherman at mrcc dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QtTest/QtTest>
#include <QObject>
#include <QString>

#include <qgsapplication.h>
#include <qgsgeometry.h>
#include <qgsspatialindex.h>
#include <qgsvectordataprovider.h>
#include <qgsvectorlayer.h>

static QgsFeature _pointFeature( QgsFeatureId id, qreal x, qreal y )
{
  QgsFeature f( id );
  f.setGeometry( QgsGeometry::fromPoint( QgsPoint( x, y ) ) );
  return f;
}

static QList<QgsFeature> _pointFeatures()
{
  /*
   *  2   |   1
   *      |
   * -----+-----
   *      |
   *  3   |   4
   */

  QList<QgsFeature> feats;
  feats << _pointFeature( 1,  1,  1 )
  << _pointFeature( 2, -1,  1 )
  << _pointFeature( 3, -1, -1 )
  << _pointFeature( 4,  1, -1 );
  return feats;
}

class TestQgsSpatialIndex : public QObject
{
    Q_OBJECT

  private slots:

    void initTestCase()
    {
      QgsApplication::init();
      QgsApplication::initQgis();
    }
    void cleanupTestCase()
    {
      QgsApplication::exitQgis();
    }

    void testQuery()
    {
      QgsSpatialIndex index;
      Q_FOREACH ( const QgsFeature& f, _pointFeatures() )
        index.insertFeature( f );

      QList<QgsFeatureId> fids = index.intersects( QgsRectangle( 0, 0, 10, 10 ) );
      QVERIFY( fids.count() == 1 );
      QVERIFY( fids[0] == 1 );

      QList<QgsFeatureId> fids2 = index.intersects( QgsRectangle( -10, -10, 0, 10 ) );
      QVERIFY( fids2.count() == 2 );
      QVERIFY( fids2.contains( 2 ) );
      QVERIFY( fids2.contains( 3 ) );
    }

    void testCopy()
    {
      QgsSpatialIndex* index = new QgsSpatialIndex;
      Q_FOREACH ( const QgsFeature& f, _pointFeatures() )
        index->insertFeature( f );

      // create copy of the index
      QgsSpatialIndex indexCopy( *index );

      QVERIFY( index->refs() == 2 );
      QVERIFY( indexCopy.refs() == 2 );

      // test that copied index works
      QList<QgsFeatureId> fids1 = indexCopy.intersects( QgsRectangle( 0, 0, 10, 10 ) );
      QVERIFY( fids1.count() == 1 );
      QVERIFY( fids1[0] == 1 );

      // check that the index is still shared
      QVERIFY( index->refs() == 2 );
      QVERIFY( indexCopy.refs() == 2 );

      // do a modification
      QgsFeature f2( _pointFeatures().at( 1 ) );
      indexCopy.deleteFeature( f2 );

      // check that the index is not shared anymore
      QVERIFY( index->refs() == 1 );
      QVERIFY( indexCopy.refs() == 1 );

      delete index;

      // test that copied index still works
      QList<QgsFeatureId> fids = indexCopy.intersects( QgsRectangle( 0, 0, 10, 10 ) );
      QVERIFY( fids.count() == 1 );
      QVERIFY( fids[0] == 1 );
    }

    void benchmarkIntersect()
    {
      // add 50K features to the index
      QgsSpatialIndex index;
      for ( int i = 0; i < 100; ++i )
      {
        for ( int k = 0; k < 500; ++k )
        {
          QgsFeature f( i*1000 + k );
          f.setGeometry( QgsGeometry::fromPoint( QgsPoint( i / 10, i % 10 ) ) );
          index.insertFeature( f );
        }
      }

      QBENCHMARK
      {
        for ( int i = 0; i < 100; ++i )
          index.intersects( QgsRectangle( i / 10, i % 10, i / 10 + 1, i % 10 + 1 ) );
      }
    }

    void benchmarkBulkLoad()
    {
      QgsVectorLayer* vl = new QgsVectorLayer( "Point", "x", "memory" );
      for ( int i = 0; i < 100; ++i )
      {
        QgsFeatureList flist;
        for ( int k = 0; k < 500; ++k )
        {
          QgsFeature f( i*1000 + k );
          f.setGeometry( QgsGeometry::fromPoint( QgsPoint( i / 10, i % 10 ) ) );
          flist << f;
        }
        vl->dataProvider()->addFeatures( flist );
      }

      QTime t;
      QgsSpatialIndex* indexBulk;
      QgsSpatialIndex* indexInsert;

      t.start();
      {
        QgsFeature f;
        QgsFeatureIterator fi = vl->getFeatures();
        while ( fi.nextFeature( f ) )
          ;
      }
      qDebug( "iter only: %d ms", t.elapsed() );

      t.start();
      {
        QgsFeatureIterator fi = vl->getFeatures();
        indexBulk = new QgsSpatialIndex( fi );
      }
      qDebug( "bulk load: %d ms", t.elapsed() );

      t.start();
      {
        QgsFeatureIterator fi = vl->getFeatures();
        QgsFeature f;
        indexInsert = new QgsSpatialIndex;
        while ( fi.nextFeature( f ) )
          indexInsert->insertFeature( f );
      }
      qDebug( "insert:    %d ms", t.elapsed() );

      // test whether a query will give us the same results
      QgsRectangle rect( 4.9, 4.9, 5.1, 5.1 );
      QList<QgsFeatureId> resBulk = indexBulk->intersects( rect );
      QList<QgsFeatureId> resInsert = indexInsert->intersects( rect );

      QCOMPARE( resBulk.count(), 500 );
      QCOMPARE( resInsert.count(), 500 );
      // the trees are built differently so they will give also different order of fids
      qSort( resBulk );
      qSort( resInsert );
      QCOMPARE( resBulk, resInsert );

      delete indexBulk;
      delete indexInsert;
    }

};

QTEST_MAIN( TestQgsSpatialIndex )

#include "testqgsspatialindex.moc"


