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
#include "qgsproject.h"
#include "qgsmultibandcolorrenderer.h"
#include "qgsrasterdataprovider.h"
#include "qgsrasterlayer.h"
#include <QObject>
#include "qgstest.h"

class TestProjectionIssues : public QObject
{
    Q_OBJECT
  public:
    TestProjectionIssues() = default;

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
    void issue5895();// test for #5895

  private:
    QgsRasterLayer *mRasterLayer = nullptr;
    QgsMapCanvas   *mMapCanvas = nullptr;
};

void TestProjectionIssues::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  //create maplayer from testdata and add to layer registry
  const QFileInfo rasterFileInfo( QStringLiteral( TEST_DATA_DIR ) + '/' +  "checker360by180.asc" );
  mRasterLayer = new QgsRasterLayer( rasterFileInfo.filePath(),
                                     rasterFileInfo.completeBaseName() );
  // Set to WGS84
  const QgsCoordinateReferenceSystem sourceCRS( QStringLiteral( "EPSG:4326" ) );
  mRasterLayer->setCrs( sourceCRS, false );

  QgsMultiBandColorRenderer *rasterRenderer = new QgsMultiBandColorRenderer( mRasterLayer->dataProvider(), 2, 3, 4 );
  mRasterLayer->setRenderer( rasterRenderer );

  QList<QgsMapLayer *> mapLayers;
  mapLayers.append( mRasterLayer );
  QgsProject::instance()->addMapLayers( mapLayers );

  // Add all layers in registry to the canvas
  QList<QgsMapLayer *> canvasLayers;
  for ( QgsMapLayer *layer : QgsProject::instance()->mapLayers() )
  {
    canvasLayers.append( layer );
  }

  // create canvas
  mMapCanvas = new QgsMapCanvas();
  mMapCanvas->setLayers( canvasLayers );

  //reproject to SWEDREF 99 TM
  const QgsCoordinateReferenceSystem destCRS( QStringLiteral( "EPSG:3006" ) );
  mMapCanvas->setDestinationCrs( destCRS );

}

void TestProjectionIssues::cleanupTestCase()
{
  delete mMapCanvas;

  QgsApplication::exitQgis();
}

void TestProjectionIssues::init()
{

}

void TestProjectionIssues::cleanup()
{

}

void TestProjectionIssues::issue5895()
{
  const QgsRectangle largeExtent( -610861, 5101721, 2523921, 6795055 );
  mMapCanvas->setExtent( largeExtent );
  mMapCanvas->zoomByFactor( 2.0 ); // Zoom out. This should exceed the transform limits.
}

QGSTEST_MAIN( TestProjectionIssues )
#include "testprojectionissues.moc"
