/***************************************************************************
                         testprojectionissues.cpp
                         ---------------------------
    begin                : September 2012
    copyright            : (C) 2012 by Magnus Homann
    email                : magnus at homann dot se
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
#include "qgsmapcanvas.h"
#include "qgsmaplayerregistry.h"
#include "qgsmaprenderer.h"
#include "qgsmultibandcolorrenderer.h"
#include "qgsrasterlayer.h"
#include <QObject>
#include <QtTest/QtTest>

class TestProjectionIssues : public QObject
{
    Q_OBJECT
  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
    void issue5895();// test for #5895
  private:
    QgsRasterLayer* mRasterLayer;
    QgsMapCanvas*   mMapCanvas;
};

void TestProjectionIssues::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  //create maplayer from testdata and add to layer registry
  QFileInfo rasterFileInfo( QString( TEST_DATA_DIR ) + QDir::separator() +  "checker360by180.asc" );
  mRasterLayer = new QgsRasterLayer( rasterFileInfo.filePath(),
                                     rasterFileInfo.completeBaseName() );
  // Set to WGS84
  QgsCoordinateReferenceSystem sourceCRS;
  sourceCRS.createFromId( 4326, QgsCoordinateReferenceSystem::EpsgCrsId );
  mRasterLayer->setCrs( sourceCRS, false );

  QgsMultiBandColorRenderer* rasterRenderer = new QgsMultiBandColorRenderer( mRasterLayer->dataProvider(), 2, 3, 4 );
  mRasterLayer->setRenderer( rasterRenderer );

  QList<QgsMapLayer *> mapLayers;
  mapLayers.append( mRasterLayer );
  QgsMapLayerRegistry::instance()->addMapLayers( mapLayers );

  // Add all layers in registry to the canvas
  QList<QgsMapCanvasLayer> canvasLayers;
  foreach ( QgsMapLayer* layer, QgsMapLayerRegistry::instance()->mapLayers().values() )
  {
    canvasLayers.append( QgsMapCanvasLayer( layer ) );
  }

  // create canvas
  mMapCanvas = new QgsMapCanvas();
  mMapCanvas->setLayerSet( canvasLayers );

  //reproject to SWEDREF 99 TM
  QgsCoordinateReferenceSystem destCRS;
  destCRS.createFromId( 3006, QgsCoordinateReferenceSystem::EpsgCrsId );
  mMapCanvas->setDestinationCrs( destCRS );
  mMapCanvas->setCrsTransformEnabled( true );

};

void TestProjectionIssues::cleanupTestCase()
{
  delete mMapCanvas;

  QgsApplication::exitQgis();
};

void TestProjectionIssues::init()
{

};

void TestProjectionIssues::cleanup()
{

};

void TestProjectionIssues::issue5895()
{
  QgsRectangle largeExtent( -610861, 5101721, 2523921, 6795055 );
  mMapCanvas->setExtent( largeExtent );
  mMapCanvas->zoomByFactor( 2.0 ); // Zoom out. This should exceed the transform limits.
};

QTEST_MAIN( TestProjectionIssues )
#include "testprojectionissues.moc"
