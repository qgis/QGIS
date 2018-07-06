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
  QgsGeometry g = QgsGeometry::fromPointXY( QgsPointXY( x, y ) );
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
      std::unique_ptr< QgsVectorLayer > vl = qgis::make_unique< QgsVectorLayer >( "Point", QString(), QStringLiteral( "memory" ) );
      for ( QgsFeature f : _pointFeatures() )
        vl->dataProvider()->addFeature( f );
      QgsSpatialIndexKDBush index( *vl->dataProvider() );

      QList<QgsFeatureId> fids = index.intersect( QgsRectangle( 0, 0, 10, 10 ) );
      QVERIFY( fids.count() == 1 );
      QCOMPARE( fids[0], 1 );

      QList<QgsFeatureId> fids2 = index.intersect( QgsRectangle( -10, -10, 0, 10 ) );
      QCOMPARE( fids2.count(), 2 );
      QVERIFY( fids2.contains( 2 ) );
      QVERIFY( fids2.contains( 3 ) );

      QList<QgsFeatureId> fids3 = index.within( QgsPointXY( 0, 0 ), 2 );
      QCOMPARE( fids3.count(), 4 );
      QVERIFY( fids3.contains( 1 ) );
      QVERIFY( fids3.contains( 2 ) );
      QVERIFY( fids3.contains( 3 ) );
      QVERIFY( fids3.contains( 4 ) );

      QList<QgsFeatureId> fids4 = index.within( QgsPointXY( 0, 0 ), 1 );
      QCOMPARE( fids4.count(), 0 );

      QList<QgsFeatureId> fids5 = index.within( QgsPointXY( -1, -1 ), 2.1 );
      QCOMPARE( fids5.count(), 3 );
      QVERIFY( fids5.contains( 2 ) );
      QVERIFY( fids5.contains( 3 ) );
      QVERIFY( fids5.contains( 4 ) );

      QgsPointXY p;
      QVERIFY( !index.point( -1, p ) );
      QVERIFY( !index.point( 5, p ) );
      QVERIFY( index.point( 1, p ) );
      QCOMPARE( p, QgsPointXY( 1, 1 ) );
      QVERIFY( index.point( 2, p ) );
      QCOMPARE( p, QgsPointXY( -1, 1 ) );
      QVERIFY( index.point( 3, p ) );
      QCOMPARE( p, QgsPointXY( -1, -1 ) );
      QVERIFY( index.point( 4, p ) );
      QCOMPARE( p, QgsPointXY( 1, -1 ) );
    }

    void testCopy()
    {
      std::unique_ptr< QgsVectorLayer > vl = qgis::make_unique< QgsVectorLayer >( "Point", QString(), QStringLiteral( "memory" ) );
      for ( QgsFeature f : _pointFeatures() )
        vl->dataProvider()->addFeature( f );

      std::unique_ptr< QgsSpatialIndexKDBush > index( new QgsSpatialIndexKDBush( *vl->dataProvider() ) );

      // create copy of the index
      std::unique_ptr< QgsSpatialIndexKDBush > indexCopy( new QgsSpatialIndexKDBush( *index ) );

      QCOMPARE( index->d, indexCopy->d );
      QVERIFY( index->d->ref == 2 );

      // test that copied index works
      QList<QgsFeatureId> fids = indexCopy->intersect( QgsRectangle( 0, 0, 10, 10 ) );
      QVERIFY( fids.count() == 1 );
      QCOMPARE( fids[0], 1 );

      // check that the index is still shared
      QCOMPARE( index->d, indexCopy->d );
      QVERIFY( index->d->ref == 2 );

      index.reset();

      // test that copied index still works
      fids = indexCopy->intersect( QgsRectangle( 0, 0, 10, 10 ) );
      QVERIFY( fids.count() == 1 );
      QCOMPARE( fids[0], 1 );
      QVERIFY( indexCopy->d->ref == 1 );

      // assignment operator
      std::unique_ptr< QgsVectorLayer > vl2 = qgis::make_unique< QgsVectorLayer >( "Point", QString(), QStringLiteral( "memory" ) );
      QgsSpatialIndexKDBush index3( *vl2->dataProvider() );
      fids = index3.intersect( QgsRectangle( 0, 0, 10, 10 ) );
      QCOMPARE( fids.count(), 0 );
      QVERIFY( index3.d->ref == 1 );

      index3 = *indexCopy;
      QCOMPARE( index3.d, indexCopy->d );
      QVERIFY( index3.d->ref == 2 );
      fids = index3.intersect( QgsRectangle( 0, 0, 10, 10 ) );
      QVERIFY( fids.count() == 1 );
      QCOMPARE( fids[0], 1 );

      indexCopy.reset();
      QVERIFY( index3.d->ref == 1 );
    }

};

QGSTEST_MAIN( TestQgsSpatialIndexKdBush )

#include "testqgsspatialindexkdbush.moc"


