/***************************************************************************
     testqgsmaptoolrotatefeature.cpp
     --------------------------------
    Date                 : November 2019
    Copyright            : (C) 2019 by Nyall Dawson
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

#include "qgisapp.h"
#include "qgsgeometry.h"
#include "qgsmapcanvas.h"
#include "qgsmapcanvassnappingutils.h"
#include "qgssnappingconfig.h"
#include "qgssnappingutils.h"
#include "qgsmaptoolrotatefeature.h"
#include "qgsproject.h"
#include "qgssettings.h"
#include "qgsvectorlayer.h"
#include "qgsmapmouseevent.h"
#include "testqgsmaptoolutils.h"


/**
 * \ingroup UnitTests
 * This is a unit test for the vertex tool
 */
class TestQgsMapToolRotateFeature: public QObject
{
    Q_OBJECT
  public:
    TestQgsMapToolRotateFeature();

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.

    void testRotateFeature();

  private:
    QgisApp *mQgisApp = nullptr;
    QgsMapCanvas *mCanvas = nullptr;
    QgsMapToolRotateFeature *mRotateTool = nullptr;
    QgsVectorLayer *mLayerBase = nullptr;
};

TestQgsMapToolRotateFeature::TestQgsMapToolRotateFeature() = default;


//runs before all tests
void TestQgsMapToolRotateFeature::initTestCase()
{
  qDebug() << "TestMapToolCapture::initTestCase()";
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();

  // Set up the QSettings environment
  QCoreApplication::setOrganizationName( QStringLiteral( "QGIS" ) );
  QCoreApplication::setOrganizationDomain( QStringLiteral( "qgis.org" ) );
  QCoreApplication::setApplicationName( QStringLiteral( "QGIS-TEST" ) );

  mQgisApp = new QgisApp();

  mCanvas = new QgsMapCanvas();

  mCanvas->setDestinationCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3946" ) ) );

  mCanvas->setFrameStyle( QFrame::NoFrame );
  mCanvas->resize( 512, 512 );
  mCanvas->setExtent( QgsRectangle( 0, 0, 8, 8 ) );
  mCanvas->show(); // to make the canvas resize
  mCanvas->hide();

  // make testing layers
  mLayerBase = new QgsVectorLayer( QStringLiteral( "Polygon?crs=EPSG:3946" ), QStringLiteral( "baselayer" ), QStringLiteral( "memory" ) );
  QVERIFY( mLayerBase->isValid() );
  QgsProject::instance()->addMapLayers( QList<QgsMapLayer *>() << mLayerBase );

  mLayerBase->startEditing();
  QString wkt1 = QStringLiteral( "Polygon ((0 0, 0 1, 1 1, 1 0))" );
  QgsFeature f1;
  f1.setGeometry( QgsGeometry::fromWkt( wkt1 ) );
  QString wkt2 = QStringLiteral( "Polygon ((1.1 0, 1.1 5, 2.1 5, 2.1 0))" );
  QgsFeature f2;
  f2.setGeometry( QgsGeometry::fromWkt( wkt2 ) );

  QgsFeatureList flist;
  flist << f1 << f2;
  mLayerBase->dataProvider()->addFeatures( flist );
  QCOMPARE( mLayerBase->featureCount(), 2L );
  QCOMPARE( mLayerBase->getFeature( 1 ).geometry().asWkt(), wkt1 );
  QCOMPARE( mLayerBase->getFeature( 2 ).geometry().asWkt( 1 ), wkt2 );

  QgsSnappingConfig cfg = mCanvas->snappingUtils()->config();
  cfg.setMode( QgsSnappingConfig::AllLayers );
  cfg.setTolerance( 1 );
  cfg.setType( QgsSnappingConfig::VertexAndSegment );
  cfg.setEnabled( true );
  mCanvas->snappingUtils()->setConfig( cfg );

  mCanvas->setLayers( QList<QgsMapLayer *>() << mLayerBase );
  mCanvas->setCurrentLayer( mLayerBase );

  // create the tool
  mRotateTool = new QgsMapToolRotateFeature( mCanvas );
  mCanvas->setMapTool( mRotateTool );

  QCOMPARE( mCanvas->mapSettings().outputSize(), QSize( 512, 512 ) );
  QCOMPARE( mCanvas->mapSettings().visibleExtent(), QgsRectangle( 0, 0, 8, 8 ) );
}

//runs after all tests
void TestQgsMapToolRotateFeature::cleanupTestCase()
{
  delete mRotateTool;
  delete mCanvas;
  QgsApplication::exitQgis();
}

void TestQgsMapToolRotateFeature::testRotateFeature()
{
  TestQgsMapToolUtils utils( mRotateTool );

  utils.mouseClick( 1, 1, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  utils.mouseMove( 2, 1 );
  utils.mouseClick( 2, 1, Qt::LeftButton, Qt::KeyboardModifiers(), true );

  QCOMPARE( mLayerBase->getFeature( 1 ).geometry().asWkt( 2 ), QStringLiteral( "Polygon ((0.72 -0.17, 0.28 1.17, 1.17 0.72, 0.72 -0.17))" ) );
  QCOMPARE( mLayerBase->getFeature( 2 ).geometry().asWkt( 2 ), QStringLiteral( "Polygon ((1.1 0, 1.1 5, 2.1 5, 2.1 0))" ) );

  mLayerBase->undoStack()->undo();
}


QGSTEST_MAIN( TestQgsMapToolRotateFeature )
#include "testqgsmaptoolrotatefeature.moc"
