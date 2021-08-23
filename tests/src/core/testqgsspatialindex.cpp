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

#include "qgstest.h"
#include <QObject>
#include <QString>

#include <qgsapplication.h>
#include "qgsfeatureiterator.h"
#include <qgsgeometry.h>
#include <qgsspatialindex.h>
#include <qgsvectordataprovider.h>
#include <qgsvectorlayer.h>
#include "qgslinestring.h"
#include "qgslogger.h"

static QgsFeature _pointFeature( QgsFeatureId id, qreal x, qreal y )
{
  QgsFeature f( id );
  const QgsGeometry g = QgsGeometry::fromPointXY( QgsPointXY( x, y ) );
  f.setGeometry( g );
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
      for ( const QgsFeature &f : _pointFeatures() )
      {
        QgsFeature indexFeature( f );
        index.addFeature( indexFeature );
      }

      QList<QgsFeatureId> fids = index.intersects( QgsRectangle( 0, 0, 10, 10 ) );
      QVERIFY( fids.count() == 1 );
      QVERIFY( fids[0] == 1 );

      const QList<QgsFeatureId> fids2 = index.intersects( QgsRectangle( -10, -10, 0, 10 ) );
      QVERIFY( fids2.count() == 2 );
      QVERIFY( fids2.contains( 2 ) );
      QVERIFY( fids2.contains( 3 ) );
    }

    void testQueryManualInsert()
    {
      QgsSpatialIndex index;
      index.addFeature( 1, QgsRectangle( 2, 3, 2, 3 ) );
      index.addFeature( 2, QgsRectangle( 12, 13, 12, 13 ) );
      index.addFeature( 3, QgsRectangle( 14, 13, 14, 13 ) );

      const QList<QgsFeatureId> fids = index.intersects( QgsRectangle( 1, 2, 3, 4 ) );
      QVERIFY( fids.count() == 1 );
      QVERIFY( fids.at( 0 ) == 1 );

      const QList<QgsFeatureId> fids2 = index.intersects( QgsRectangle( 10, 12, 15, 14 ) );
      QVERIFY( fids2.count() == 2 );
      QVERIFY( fids2.contains( 2 ) );
      QVERIFY( fids2.contains( 3 ) );
    }

    void testInitFromEmptyIterator()
    {
      const QgsFeatureIterator it;
      const QgsSpatialIndex index( it );
      // we just test that we survive the above command without exception from libspatialindex raised
    }

    void testCopy()
    {
      QgsSpatialIndex *index = new QgsSpatialIndex;
      for ( const QgsFeature &f : _pointFeatures() )
      {
        QgsFeature indexFeature( f );
        index->addFeature( indexFeature );
      }

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
      const QgsFeature f2( _pointFeatures().at( 1 ) );
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
          QgsFeature f( i * 1000 + k );
          const QgsGeometry g = QgsGeometry::fromPointXY( QgsPointXY( i / 10, i % 10 ) );
          f.setGeometry( g );
          index.addFeature( f );
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
      QgsVectorLayer *vl = new QgsVectorLayer( QStringLiteral( "Point" ), QStringLiteral( "x" ), QStringLiteral( "memory" ) );
      for ( int i = 0; i < 100; ++i )
      {
        QgsFeatureList flist;
        for ( int k = 0; k < 500; ++k )
        {
          QgsFeature f( i * 1000 + k );
          const QgsGeometry g = QgsGeometry::fromPointXY( QgsPointXY( i / 10, i % 10 ) );
          f.setGeometry( g );
          flist << f;
        }
        vl->dataProvider()->addFeatures( flist );
      }

      QElapsedTimer t;
      QgsSpatialIndex *indexBulk = nullptr;
      QgsSpatialIndex *indexInsert = nullptr;

      t.start();
      {
        QgsFeature f;
        QgsFeatureIterator fi = vl->getFeatures();
        while ( fi.nextFeature( f ) )
          ;
      }
      QgsDebugMsg( QStringLiteral( "iter only: %1 ms" ).arg( t.elapsed() ) );

      t.start();
      {
        const QgsFeatureIterator fi = vl->getFeatures();
        indexBulk = new QgsSpatialIndex( fi );
      }
      QgsDebugMsg( QStringLiteral( "bulk load: %1 ms" ).arg( t.elapsed() ) );

      t.start();
      {
        QgsFeatureIterator fi = vl->getFeatures();
        QgsFeature f;
        indexInsert = new QgsSpatialIndex;
        while ( fi.nextFeature( f ) )
          indexInsert->addFeature( f );
      }
      QgsDebugMsg( QStringLiteral( "insert:    %1 ms" ).arg( t.elapsed() ) );

      // test whether a query will give us the same results
      const QgsRectangle rect( 4.9, 4.9, 5.1, 5.1 );
      QList<QgsFeatureId> resBulk = indexBulk->intersects( rect );
      QList<QgsFeatureId> resInsert = indexInsert->intersects( rect );

      QCOMPARE( resBulk.count(), 500 );
      QCOMPARE( resInsert.count(), 500 );
      // the trees are built differently so they will give also different order of fids
      std::sort( resBulk.begin(), resBulk.end() );
      std::sort( resInsert.begin(), resInsert.end() );
      QCOMPARE( resBulk, resInsert );

      delete indexBulk;
      delete indexInsert;
    }

    void bulkLoadWithCallback()
    {
      std::unique_ptr< QgsVectorLayer > vl = std::make_unique< QgsVectorLayer >( QStringLiteral( "Point" ), QStringLiteral( "x" ), QStringLiteral( "memory" ) );
      QList< QgsFeatureId >  addedIds;
      for ( int i = 0; i < 10; ++i )
      {
        QgsFeature f( i );
        const QgsGeometry g = QgsGeometry::fromPointXY( QgsPointXY( i, 1 ) );
        f.setGeometry( g );
        vl->dataProvider()->addFeature( f );
        addedIds.append( f.id() );
      }
      QCOMPARE( vl->featureCount(), 10L );

      QgsFeatureIds ids;
      const QgsSpatialIndex i( vl->getFeatures(), [ & ]( const QgsFeature & f )->bool
      {
        ids.insert( f.id() );
        return true;
      } );

      QCOMPARE( ids.size(), 10 );
      for ( int i = 0; i < 10; ++i )
        QVERIFY( ids.contains( addedIds.at( i ) ) );

      QList<QgsFeatureId> res = i.intersects( QgsRectangle( 1.5, 0, 3.5, 10 ) );
      QCOMPARE( res.size(), 2 );
      QVERIFY( res.contains( addedIds.at( 2 ) ) );
      QVERIFY( res.contains( addedIds.at( 3 ) ) );

      // try canceling
      ids.clear();
      const QgsSpatialIndex i2( vl->getFeatures(), [ & ]( const QgsFeature & f )->bool
      {
        ids.insert( f.id() );
        return false;
      } );

      QCOMPARE( ids.size(), 1 );
      QVERIFY( ids.contains( addedIds.at( 0 ) ) );

      res = i2.intersects( QgsRectangle( 1.5, 0, 3.5, 10 ) );
      QVERIFY( res.isEmpty() );
    }

    void testRetrieveGeometries()
    {
      QgsVectorLayer *vl = new QgsVectorLayer( QStringLiteral( "LineString" ), QStringLiteral( "x" ), QStringLiteral( "memory" ) );
      int fid = 0;
      for ( int x = 0; x < 10; ++x )
      {
        QgsFeatureList flist;
        for ( int y = 100; y < 110; ++y )
        {
          QgsFeature f( fid++ );
          f.setGeometry( std::make_unique< QgsLineString >( QgsPoint( x, y ), QgsPoint( x + 0.5, y - 0.5 ) ) );
          flist << f;
        }
        vl->dataProvider()->addFeatures( flist );
      }

      // iterator based population

      // not storing geometries
      const QgsSpatialIndex i1( vl->getFeatures() );
      QVERIFY( i1.geometry( 1 ).isNull() );
      QVERIFY( i1.geometry( 50 ).isNull() );

      // storing geometries
      const QgsSpatialIndex i2( vl->getFeatures(), nullptr, QgsSpatialIndex::FlagStoreFeatureGeometries );
      QCOMPARE( i2.geometry( 1 ).asWkt( 1 ), QStringLiteral( "LineString (0 100, 0.5 99.5)" ) );
      QCOMPARE( i2.geometry( 50 ).asWkt( 1 ), QStringLiteral( "LineString (4 109, 4.5 108.5)" ) );

      // manual population

      QgsSpatialIndex i3;
      QgsFeatureIterator fi = vl->getFeatures();
      QgsFeature f;
      while ( fi.nextFeature( f ) )
      {
        i3.addFeature( f );
      }
      QVERIFY( i3.geometry( 1 ).isNull() );
      QVERIFY( i3.geometry( 50 ).isNull() );


      // storing geometries
      QgsSpatialIndex i4( QgsSpatialIndex::FlagStoreFeatureGeometries );
      fi = vl->getFeatures();
      while ( fi.nextFeature( f ) )
      {
        i4.addFeature( f );
      }
      QCOMPARE( i4.geometry( 1 ).asWkt( 1 ), QStringLiteral( "LineString (0 100, 0.5 99.5)" ) );
      QCOMPARE( i4.geometry( 50 ).asWkt( 1 ), QStringLiteral( "LineString (4 109, 4.5 108.5)" ) );


      // not storing geometries
      const QgsSpatialIndex i5( *vl );
      QVERIFY( i5.geometry( 1 ).isNull() );
      QVERIFY( i5.geometry( 50 ).isNull() );

      // storing geometries
      QgsSpatialIndex i6( *vl, nullptr, QgsSpatialIndex::FlagStoreFeatureGeometries );
      QCOMPARE( i6.geometry( 1 ).asWkt( 1 ), QStringLiteral( "LineString (0 100, 0.5 99.5)" ) );
      QCOMPARE( i6.geometry( 50 ).asWkt( 1 ), QStringLiteral( "LineString (4 109, 4.5 108.5)" ) );

      f = vl->getFeature( 1 );
      QVERIFY( i6.deleteFeature( f ) );
      QVERIFY( i6.geometry( 1 ).isNull() );
      QCOMPARE( i6.geometry( 50 ).asWkt( 1 ), QStringLiteral( "LineString (4 109, 4.5 108.5)" ) );
      f = vl->getFeature( 50 );

      QVERIFY( i6.deleteFeature( f ) );
      QVERIFY( i6.geometry( 50 ).isNull() );
    }

    void testNearestNeighbour()
    {
      QgsSpatialIndex i;
      QgsSpatialIndex i2( QgsSpatialIndex::FlagStoreFeatureGeometries );

      QgsFeature f1( 1 );
      f1.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "LineString(1 1, 3 1, 3 3)" ) ) );
      QgsFeature f2( 2 );
      f2.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "LineString(0 1, 0 3)" ) ) );
      QgsFeature f3( 3 );
      f3.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "LineString(0 4, 1 5, 3 3)" ) ) );
      i.addFeature( f1 );
      i2.addFeature( f1 );
      i.addFeature( f2 );
      i2.addFeature( f2 );
      i.addFeature( f3 );
      i2.addFeature( f3 );

      // i does not store feature geometries, so nearest neighbour search uses bounding box only
      QCOMPARE( i.nearestNeighbor( QgsPointXY( 1, 2.9 ), 1 ), QList< QgsFeatureId >() << 1 );
      QCOMPARE( i.nearestNeighbor( QgsPointXY( 1, 2.9 ), 2 ), QList< QgsFeatureId >() << 1 << 3 );
      // i2 does store feature geometries, so nearest neighbour is exact
      QCOMPARE( i2.nearestNeighbor( QgsPointXY( 1, 2.9 ), 1 ), QList< QgsFeatureId >() << 2 );
      QCOMPARE( i2.nearestNeighbor( QgsPointXY( 1, 2.9 ), 2 ), QList< QgsFeatureId >() << 2 << 3 );

      // with maximum distance
      QCOMPARE( i.nearestNeighbor( QgsPointXY( 1, 2.9 ), 1, 0.5 ), QList< QgsFeatureId >() << 1 );
      QCOMPARE( i2.nearestNeighbor( QgsPointXY( 1, 2.9 ), 1, 0.5 ), QList< QgsFeatureId >() );
      QCOMPARE( i.nearestNeighbor( QgsPointXY( 1, 2.9 ), 2, 0.5 ), QList< QgsFeatureId >() << 1 << 3 );
      QCOMPARE( i2.nearestNeighbor( QgsPointXY( 1, 2.9 ), 2, 0.5 ), QList< QgsFeatureId >() );
      QCOMPARE( i.nearestNeighbor( QgsPointXY( 1, 2.9 ), 1, 1.1 ), QList< QgsFeatureId >() << 1 );
      QCOMPARE( i2.nearestNeighbor( QgsPointXY( 1, 2.9 ), 1, 1.1 ), QList< QgsFeatureId >() << 2 );
      QCOMPARE( i.nearestNeighbor( QgsPointXY( 1, 2.9 ), 2, 1.1 ), QList< QgsFeatureId >() << 1 << 3 );
      QCOMPARE( i2.nearestNeighbor( QgsPointXY( 1, 2.9 ), 2, 1.1 ), QList< QgsFeatureId >() << 2 );
      QCOMPARE( i.nearestNeighbor( QgsPointXY( 1, 2.9 ), 2, 2 ), QList< QgsFeatureId >() << 1 << 3 );
      QCOMPARE( i2.nearestNeighbor( QgsPointXY( 1, 2.9 ), 2, 2 ), QList< QgsFeatureId >() << 2 << 3 );

      // using geometries as input, not points
      QgsGeometry g = QgsGeometry::fromWkt( QStringLiteral( "LineString (1 0, 1 -1, -2 -1, -2 7, 5 4, 5 0)" ) );
      QCOMPARE( i2.nearestNeighbor( g, 1 ), QList< QgsFeatureId >() << 3 );
      QCOMPARE( i2.nearestNeighbor( g, 2 ), QList< QgsFeatureId >() << 3 << 1 );
      QCOMPARE( i2.nearestNeighbor( g, 2, 1.1 ), QList< QgsFeatureId >() << 3 << 1 );
      QCOMPARE( i2.nearestNeighbor( g, 2, 0.2 ), QList< QgsFeatureId >() );

      g = QgsGeometry::fromWkt( QStringLiteral( "LineString (3 7, 3 6, 5 6, 4 2)" ) );
      QCOMPARE( i.nearestNeighbor( g, 1 ), QList< QgsFeatureId >() << 1 << 3 ); // bounding box search only
      QCOMPARE( i.nearestNeighbor( g, 2 ), QList< QgsFeatureId >() << 1 << 3 );
      QCOMPARE( i.nearestNeighbor( g, 2, 1.1 ), QList< QgsFeatureId >() << 1 << 3 );
      QCOMPARE( i.nearestNeighbor( g, 2, 0.2 ), QList< QgsFeatureId >() << 1 << 3 );
      QCOMPARE( i2.nearestNeighbor( g, 1 ), QList< QgsFeatureId >() << 1 );
      QCOMPARE( i2.nearestNeighbor( g, 2 ), QList< QgsFeatureId >() << 1 << 3 );
      QCOMPARE( i2.nearestNeighbor( g, 2, 1.1 ), QList< QgsFeatureId >() << 1 );
      QCOMPARE( i2.nearestNeighbor( g, 2, 0.2 ), QList< QgsFeatureId >() );

      g = QgsGeometry::fromWkt( QStringLiteral( "Polygon ((2 3, -3 4, 1 7, 6 6, 6 1, 3 4, 2 3))" ) );
      QCOMPARE( i.nearestNeighbor( g, 1 ), QList< QgsFeatureId >() << 1 << 2 << 3 ); // bounding box search only
      QCOMPARE( i.nearestNeighbor( g, 2 ), QList< QgsFeatureId >() << 1 << 2 << 3 );
      QCOMPARE( i.nearestNeighbor( g, 2, 1.1 ), QList< QgsFeatureId >() << 1 << 2 << 3 );
      QCOMPARE( i.nearestNeighbor( g, 2, 0.2 ), QList< QgsFeatureId >() << 1 << 2 << 3 );
      QCOMPARE( i2.nearestNeighbor( g, 1 ), QList< QgsFeatureId >() << 3 );
      QCOMPARE( i2.nearestNeighbor( g, 2 ), QList< QgsFeatureId >() << 3 << 2 );
      QCOMPARE( i2.nearestNeighbor( g, 2, 1.1 ), QList< QgsFeatureId >() << 3 << 2 );
      QCOMPARE( i2.nearestNeighbor( g, 2, 0.2 ), QList< QgsFeatureId >() << 3 );

      g = QgsGeometry::fromWkt( QStringLiteral( "MultiPoint (1.5 2.5, 3 4.5)" ) );
      QCOMPARE( i.nearestNeighbor( g, 1 ), QList< QgsFeatureId >() << 1 << 3 ); // bounding box search only
      QCOMPARE( i.nearestNeighbor( g, 2 ), QList< QgsFeatureId >() << 1 << 3 );
      QCOMPARE( i.nearestNeighbor( g, 2, 1.1 ), QList< QgsFeatureId >() <<  1 << 3 );
      QCOMPARE( i.nearestNeighbor( g, 2, 0.2 ), QList< QgsFeatureId >() <<  1 << 3 );
      QCOMPARE( i2.nearestNeighbor( g, 1 ), QList< QgsFeatureId >() << 3 );
      QCOMPARE( i2.nearestNeighbor( g, 2 ), QList< QgsFeatureId >() << 3 << 2 << 1 );
      QCOMPARE( i2.nearestNeighbor( g, 2, 1.1 ), QList< QgsFeatureId >() << 3 );
      QCOMPARE( i2.nearestNeighbor( g, 2, 0.2 ), QList< QgsFeatureId >() );
    }

};

QGSTEST_MAIN( TestQgsSpatialIndex )

#include "testqgsspatialindex.moc"


