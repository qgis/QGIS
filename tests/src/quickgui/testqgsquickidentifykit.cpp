/***************************************************************************
     testqgsquickidentifykit.cpp
     --------------------------------------
  Date                 : May 2018
  Copyright            : (C) 2018 by Viktor Sklencar
  Email                : vsklencar at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <QObject>
#include <QApplication>
#include <QDesktopWidget>

#include "qgsapplication.h"
#include "qgstest.h"
#include "qgis.h"

#include "qgsvectorlayer.h"
#include "qgsfeature.h"
#include "qgsgeometry.h"
#include "qgsvectordataprovider.h"

#include "qgsquickmapcanvasmap.h"
#include "qgsquickidentifykit.h"


class TestQgsQuickScaleBarKit: public QObject
{
    Q_OBJECT
  private slots:
    void init() {} // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.

    void identifyOne(); // tests identifyOne function without given layer
    void identifyOneDefinedVector(); // tests identifyOne function with given layer
    void identifyInRadius();
};

void TestQgsQuickScaleBarKit::identifyOne()
{
  QgsCoordinateReferenceSystem crsGPS = QgsCoordinateReferenceSystem::fromEpsgId( 4326 );
  QVERIFY( crsGPS.authid() == "EPSG:4326" );

  QgsRectangle extent = QgsRectangle( -120, 23, -82, 47 );
  QgsQuickMapCanvasMap canvas;

  QgsVectorLayer *tempLayer = new QgsVectorLayer( QStringLiteral( "Point?crs=epsg:4326" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) );
  QVERIFY( tempLayer->isValid() );

  QgsQuickMapSettings *ms = canvas.mapSettings();
  ms->setDestinationCrs( crsGPS );
  ms->setExtent( extent );
  ms->setOutputSize( QSize( 1000, 500 ) );
  ms->setLayers( QList<QgsMapLayer *>() << tempLayer );

  QgsQuickIdentifyKit kit;
  kit.setMapSettings( ms );

  double pointX = -31.208;
  double pointY = 20.407999999999998;
  double pointX2 = pointX + 0.5;

  // add feature
  QgsFeature f1( tempLayer->dataProvider()->fields(), 1 );
  QgsPointXY point( pointX, pointY );
  QgsGeometry geom = QgsGeometry::fromPointXY( point ) ;
  f1.setGeometry( geom );

  // add another feature
  QgsFeature f2( tempLayer->dataProvider()->fields(), 1 );
  QgsPointXY point2( pointX2, pointY );
  QgsGeometry geom2 = QgsGeometry::fromPointXY( point2 ) ;
  f2.setGeometry( geom2 );

  tempLayer->dataProvider()->addFeatures( QgsFeatureList() << f1 << f2 );

  // exactly matches f1 point
  QgsPointXY screenPoint( 1954.0, 554.0 );
  QgsQuickFeatureLayerPair identifiedFeature = kit.identifyOne( screenPoint.toQPointF() );
  QVERIFY( identifiedFeature.isValid() );
  QVERIFY( identifiedFeature.feature().geometry().asPoint() == point );
}

void TestQgsQuickScaleBarKit::identifyOneDefinedVector()
{
  QgsCoordinateReferenceSystem crsGPS = QgsCoordinateReferenceSystem::fromEpsgId( 4326 );
  QVERIFY( crsGPS.authid() == "EPSG:4326" );

  QgsRectangle extent = QgsRectangle( -120, 23, -82, 47 );
  QgsQuickMapCanvasMap canvas;

  QgsVectorLayer *tempLayer = new QgsVectorLayer( QStringLiteral( "Point?crs=epsg:4326" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) );
  QVERIFY( tempLayer->isValid() );

  QgsVectorLayer *tempLayer2 = new QgsVectorLayer( QStringLiteral( "Point?crs=epsg:4326" ), QStringLiteral( "vl2" ), QStringLiteral( "memory" ) );
  QVERIFY( tempLayer->isValid() );

  QgsQuickMapSettings *ms = canvas.mapSettings();
  ms->setDestinationCrs( crsGPS );
  ms->setExtent( extent );
  ms->setOutputSize( QSize( 1000, 500 ) );
  ms->setLayers( QList<QgsMapLayer *>() << tempLayer );

  QgsQuickIdentifyKit kit;
  kit.setMapSettings( ms );

  double pointX = -31.208;
  double pointY = 20.407999999999998;
  double pointX2 = pointX + 0.5;

  // add feature
  QgsFeature f1( tempLayer->dataProvider()->fields(), 1 );
  QgsPointXY point( pointX, pointY );
  QgsGeometry geom = QgsGeometry::fromPointXY( point ) ;
  f1.setGeometry( geom );

  // add another feature
  QgsFeature f2( tempLayer2->dataProvider()->fields(), 1 );
  QgsPointXY point2( pointX2, pointY );
  QgsGeometry geom2 = QgsGeometry::fromPointXY( point2 ) ;
  f2.setGeometry( geom2 );

  tempLayer->dataProvider()->addFeatures( QgsFeatureList() << f1 );
  tempLayer2->dataProvider()->addFeatures( QgsFeatureList() << f2 );

  QgsPointXY screenPoint( 1954.0, 554.0 );
  QgsQuickFeatureLayerPair identifiedFeature = kit.identifyOne( screenPoint.toQPointF(), tempLayer2 );
  QVERIFY( identifiedFeature.isValid() );
  QVERIFY( identifiedFeature.feature().geometry().asPoint() == point2 );

}

void TestQgsQuickScaleBarKit::identifyInRadius()
{
  QgsCoordinateReferenceSystem crsGPS = QgsCoordinateReferenceSystem::fromEpsgId( 4326 );
  QVERIFY( crsGPS.authid() == "EPSG:4326" );

  QgsRectangle extent = QgsRectangle( -120, 23, -82, 47 );
  QgsQuickMapCanvasMap canvas;

  QgsVectorLayer *tempLayer = new QgsVectorLayer( QStringLiteral( "Point?crs=epsg:4326" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) );
  QVERIFY( tempLayer->isValid() );

  QgsQuickMapSettings *ms = canvas.mapSettings();
  ms->setDestinationCrs( crsGPS );
  ms->setExtent( extent );
  ms->setOutputSize( QSize( 1000, 500 ) );
  ms->setLayers( QList<QgsMapLayer *>() << tempLayer );

  QgsQuickIdentifyKit kit;
  kit.setMapSettings( ms );

  double pointX = -31.208;
  double pointY = 20.407999999999998;
  double pointX2 = pointX + 5;

  QgsFeature f1( tempLayer->dataProvider()->fields(), 1 );
  QgsPointXY point( pointX, pointY );
  QgsGeometry geom = QgsGeometry::fromPointXY( point ) ;
  f1.setGeometry( geom );

  QgsFeature f2( tempLayer->dataProvider()->fields(), 1 );
  QgsPointXY point2( pointX2, pointY );
  QgsGeometry geom2 = QgsGeometry::fromPointXY( point2 ) ;
  f2.setGeometry( geom2 );

  tempLayer->dataProvider()->addFeatures( QgsFeatureList() << f1 << f2 );

  kit.setSearchRadiusMm( 1.0 );
  QgsPointXY screenPoint( 1954.0, 554.0 );
  QgsQuickFeatureLayerPairs res = kit.identify( screenPoint.toQPointF() );
  QVERIFY( res.size() == 1 );

  kit.setSearchRadiusMm( 100.0 );
  res = kit.identify( screenPoint.toQPointF() );
  QVERIFY( res.size() == 2 );
}

QGSTEST_MAIN( TestQgsQuickScaleBarKit )
#include "testqgsquickidentifykit.moc"
