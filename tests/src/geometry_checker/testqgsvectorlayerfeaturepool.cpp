/***************************************************************************
     testqgsgeometrychecks.cpp
     --------------------------------------
    Date                 : September 2017
    Copyright            : (C) 2017 Sandro Mani
    Email                : manisandro at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstest.h"
#include "qgsvectorlayerfeaturepool.h"
#include "qgsvectorlayer.h"


class TestQgsVectorLayerFeaturePool : public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();
    void cleanupTestCase();
    void getFeatures();
    void addFeature();
    void deleteFeature();
    void changeGeometry();

  private:
    std::unique_ptr<QgsVectorLayer> createPopulatedLayer();
};

void TestQgsVectorLayerFeaturePool::initTestCase()
{
  QgsApplication::initQgis();
}

void TestQgsVectorLayerFeaturePool::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsVectorLayerFeaturePool::getFeatures()
{
  const std::unique_ptr<QgsVectorLayer> vl = createPopulatedLayer();
  QgsVectorLayerFeaturePool pool( vl.get() );

  const QgsFeatureIds ids1 = pool.getFeatures( QgsFeatureRequest().setFilterRect( QgsRectangle( 0, 0, 10, 10 ) ) );

  // One feature is within the requested area
  QCOMPARE( ids1.size(), 1 );

  const QgsFeatureIds ids2 = pool.getIntersects( QgsRectangle( 0, 0, 10, 10 ) );
  // Also one within the spatial index
  QCOMPARE( ids2.size(), 1 );
}

void TestQgsVectorLayerFeaturePool::addFeature()
{
  std::unique_ptr<QgsVectorLayer> vl = createPopulatedLayer();
  QgsVectorLayerFeaturePool pool( vl.get() );

  const QgsFeatureIds ids1 = pool.getFeatures( QgsFeatureRequest().setFilterRect( QgsRectangle( 0, 0, 10, 10 ) ) );

  // One feature is within the requested area
  QCOMPARE( ids1.size(), 1 );

  const QgsFeatureIds ids2 = pool.getIntersects( QgsRectangle( 0, 0, 10, 10 ) );
  // Also one within the spatial index
  QCOMPARE( ids2.size(), 1 );

  QgsFeature feature;
  feature.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "Polygon((1 1, 9 1, 9 9, 1 9, 1 1))" ) ) );

  // Add another feature...
  vl->startEditing();

  pool.addFeature( feature );

  const QgsFeatureIds ids3 = pool.getIntersects( QgsRectangle( 0, 0, 10, 10 ) );

  // Two features within the requested area
  QCOMPARE( ids3.size(), 2 );
}

void TestQgsVectorLayerFeaturePool::deleteFeature()
{
  std::unique_ptr<QgsVectorLayer> vl = createPopulatedLayer();

  QgsVectorLayerFeaturePool pool( vl.get() );

  const QgsFeatureIds ids1 = pool.getFeatures( QgsFeatureRequest().setFilterRect( QgsRectangle( 0, 0, 10, 10 ) ) );

  // One feature is within the requested area
  QCOMPARE( ids1.size(), 1 );

  const QgsFeatureIds ids2 = pool.getIntersects( QgsRectangle( 0, 0, 10, 10 ) );
  // Also one within the spatial index
  QCOMPARE( ids2.size(), 1 );

  vl->startEditing();
  // Delete a feature outside the AOI (0, 0, 10, 10)
  pool.deleteFeature( 2 );

  const QgsFeatureIds ids3 = pool.getIntersects( QgsRectangle( 0, 0, 10, 10 ) );

  // Still 1 feature
  QCOMPARE( ids3.size(), 1 );

  // Delete a feature inside the AOI (0, 0, 10, 10)
  pool.deleteFeature( 1 );

  const QgsFeatureIds ids4 = pool.getIntersects( QgsRectangle( 0, 0, 10, 10 ) );

  // No more features here
  QCOMPARE( ids4.size(), 0 );
}

void TestQgsVectorLayerFeaturePool::changeGeometry()
{
  std::unique_ptr<QgsVectorLayer> vl = createPopulatedLayer();

  QgsVectorLayerFeaturePool pool( vl.get() );

  // One feature is within the requested area
  const QgsFeatureIds ids1 = pool.getFeatures( QgsFeatureRequest().setFilterRect( QgsRectangle( 0, 0, 10, 10 ) ) );
  QCOMPARE( ids1.size(), 1 );

  // Also one when using the spatial index
  const QgsFeatureIds ids2 = pool.getIntersects( QgsRectangle( 0, 0, 10, 10 ) );
  QCOMPARE( ids2.size(), 1 );

  vl->startEditing();
  QgsFeature feat;
  pool.getFeature( 1, feat );

  // Update a feature to be outside the AOI
  feat.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "Polygon((100 100, 110 100, 110 110, 100 110, 100 100))" ) ) );
  vl->updateFeature( feat );

  // Still working on the cached data
  const QgsFeatureIds ids3 = pool.getIntersects( QgsRectangle( 0, 0, 10, 10 ) );
  QCOMPARE( ids3.size(), 1 );

  // Repopulate the cache
  const QgsFeatureIds ids4 = pool.getFeatures( QgsFeatureRequest().setFilterRect( QgsRectangle( 0, 0, 10, 10 ) ) );
  QCOMPARE( ids4.size(), 0 );

  // Still working on the cached data
  const QgsFeatureIds ids5 = pool.getIntersects( QgsRectangle( 0, 0, 10, 10 ) );
  QCOMPARE( ids5.size(), 0 );

  // Update a feature to be inside the AOI
  feat.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "Polygon((0 0, 10 0, 10 10, 0 10, 0 0))" ) ) );
  vl->updateFeature( feat );

  // Still cached
  const QgsFeatureIds ids6 = pool.getIntersects( QgsRectangle( 0, 0, 10, 10 ) );
  QCOMPARE( ids6.size(), 0 );

  // One in there again
  const QgsFeatureIds ids7 = pool.getFeatures( QgsFeatureRequest().setFilterRect( QgsRectangle( 0, 0, 10, 10 ) ) );
  QCOMPARE( ids7.size(), 1 );
}

std::unique_ptr<QgsVectorLayer> TestQgsVectorLayerFeaturePool::createPopulatedLayer()
{
  std::unique_ptr<QgsVectorLayer> vl = std::make_unique<QgsVectorLayer>( QStringLiteral( "Polygon" ), QStringLiteral( "Polygons" ), QStringLiteral( "memory" ) );

  QgsFeature feature;
  feature.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "Polygon((0 0, 10 0, 10 10, 0 10, 0 0))" ) ) );
  vl->dataProvider()->addFeature( feature );
  feature.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "Polygon((100 100, 110 100, 110 110, 100 110, 100 100))" ) ) );
  vl->dataProvider()->addFeature( feature );

  return vl;
}
QGSTEST_MAIN( TestQgsVectorLayerFeaturePool )
#include "testqgsvectorlayerfeaturepool.moc"
