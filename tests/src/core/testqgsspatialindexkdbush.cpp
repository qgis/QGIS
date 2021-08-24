/***************************************************************************
     TestQgsSpatialIndexKdBushkdbush.cpp
     --------------------------------------
    Date                 : July 2018
    Copyright            : (C) 2018 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
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
#include "qgsgeometry.h"
#include "qgsspatialindexkdbush.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include "qgsspatialindexkdbush_p.h"

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

bool testContains( const QList<QgsSpatialIndexKDBushData> &data, QgsFeatureId id, const QgsPointXY &point )
{
  for ( const QgsSpatialIndexKDBushData &d : data )
  {
    if ( d.id == id )
    {
      return d.point() == point;
    }
  }
  return false;
}

class TestQgsSpatialIndexKdBush : public QObject
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
      std::unique_ptr< QgsVectorLayer > vl = std::make_unique< QgsVectorLayer >( "Point", QString(), QStringLiteral( "memory" ) );
      for ( QgsFeature f : _pointFeatures() )
        vl->dataProvider()->addFeature( f );
      const QgsSpatialIndexKDBush index( *vl->dataProvider() );
      QVERIFY( index.size() == 4 );

      const QList<QgsSpatialIndexKDBushData> fids = index.intersects( QgsRectangle( 0, 0, 10, 10 ) );
      QVERIFY( fids.count() == 1 );
      QVERIFY( testContains( fids, 1, QgsPointXY( 1, 1 ) ) );

      const QList<QgsSpatialIndexKDBushData> fids2 = index.intersects( QgsRectangle( -10, -10, 0, 10 ) );
      QCOMPARE( fids2.count(), 2 );
      QVERIFY( testContains( fids2, 2, QgsPointXY( -1, 1 ) ) );
      QVERIFY( testContains( fids2, 3, QgsPointXY( -1, -1 ) ) );

      const QList<QgsSpatialIndexKDBushData> fids3 = index.within( QgsPointXY( 0, 0 ), 2 );
      QCOMPARE( fids3.count(), 4 );
      QVERIFY( testContains( fids3, 1, QgsPointXY( 1, 1 ) ) );
      QVERIFY( testContains( fids3, 2, QgsPointXY( -1, 1 ) ) );
      QVERIFY( testContains( fids3, 3, QgsPointXY( -1, -1 ) ) );
      QVERIFY( testContains( fids3, 4, QgsPointXY( 1, -1 ) ) );

      const QList<QgsSpatialIndexKDBushData> fids4 = index.within( QgsPointXY( 0, 0 ), 1 );
      QCOMPARE( fids4.count(), 0 );

      const QList<QgsSpatialIndexKDBushData> fids5 = index.within( QgsPointXY( -1, -1 ), 2.1 );
      QCOMPARE( fids5.count(), 3 );
      QVERIFY( testContains( fids5, 2, QgsPointXY( -1, 1 ) ) );
      QVERIFY( testContains( fids5, 3, QgsPointXY( -1, -1 ) ) );
      QVERIFY( testContains( fids5, 4, QgsPointXY( 1, -1 ) ) );
    }

    void testCopy()
    {
      std::unique_ptr< QgsVectorLayer > vl = std::make_unique< QgsVectorLayer >( "Point", QString(), QStringLiteral( "memory" ) );
      for ( QgsFeature f : _pointFeatures() )
        vl->dataProvider()->addFeature( f );

      std::unique_ptr< QgsSpatialIndexKDBush > index( new QgsSpatialIndexKDBush( *vl->dataProvider() ) );

      // create copy of the index
      std::unique_ptr< QgsSpatialIndexKDBush > indexCopy( new QgsSpatialIndexKDBush( *index ) );

      QVERIFY( index->d == indexCopy->d );
      QVERIFY( index->d->ref == 2 );

      // test that copied index works
      QList<QgsSpatialIndexKDBushData> fids = indexCopy->intersects( QgsRectangle( 0, 0, 10, 10 ) );
      QVERIFY( fids.count() == 1 );
      QVERIFY( testContains( fids, 1, QgsPointXY( 1, 1 ) ) );

      // check that the index is still shared
      QVERIFY( index->d == indexCopy->d );
      QVERIFY( index->d->ref == 2 );

      index.reset();

      // test that copied index still works
      fids = indexCopy->intersects( QgsRectangle( 0, 0, 10, 10 ) );
      QVERIFY( fids.count() == 1 );
      QVERIFY( testContains( fids, 1, QgsPointXY( 1, 1 ) ) );
      QVERIFY( indexCopy->d->ref == 1 );

      // assignment operator
      std::unique_ptr< QgsVectorLayer > vl2 = std::make_unique< QgsVectorLayer >( "Point", QString(), QStringLiteral( "memory" ) );
      QgsSpatialIndexKDBush index3( *vl2->dataProvider() );
      QVERIFY( index3.size() == 0 );
      fids = index3.intersects( QgsRectangle( 0, 0, 10, 10 ) );
      QVERIFY( fids.count() == 0 );
      QVERIFY( index3.d->ref == 1 );

      index3 = *indexCopy;
      QVERIFY( index3.d == indexCopy->d );
      QVERIFY( index3.d->ref == 2 );
      fids = index3.intersects( QgsRectangle( 0, 0, 10, 10 ) );
      QVERIFY( fids.count() == 1 );
      QVERIFY( testContains( fids, 1, QgsPointXY( 1, 1 ) ) );

      indexCopy.reset();
      QVERIFY( index3.d->ref == 1 );
    }

};

QGSTEST_MAIN( TestQgsSpatialIndexKdBush )

#include "testqgsspatialindexkdbush.moc"


