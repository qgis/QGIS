/***************************************************************************
     testqgsmaptoolmovefeature.cpp
     --------------------------------
    Date                 : August 2019
    Copyright            : (C) 2019 by Loïc Bartoletti
    Email                : loic.bartoletti@oslandia.com
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
#include "qgsadvanceddigitizingdockwidget.h"
#include "qgsgeometry.h"
#include "qgsmapcanvas.h"
#include "qgsmapcanvassnappingutils.h"
#include "qgssnappingconfig.h"
#include "qgssnappingutils.h"
#include "qgsmaptoolmovefeature.h"
#include "qgsproject.h"
#include "qgssettings.h"
#include "qgsvectorlayer.h"
#include "qgsmapmouseevent.h"
#include "testqgsmaptoolutils.h"


/**
 * \ingroup UnitTests
 * This is a unit test for the vertex tool
 */
class TestQgsMapToolMoveFeature: public QObject
{
    Q_OBJECT
  public:
    TestQgsMapToolMoveFeature();

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.

    void testMoveFeature();
    void testTopologicalMoveFeature();

  private:
    QgisApp *mQgisApp = nullptr;
    QgsMapCanvas *mCanvas = nullptr;
    QgsMapToolMoveFeature *mCaptureTool = nullptr;
    QgsVectorLayer *mLayerBase = nullptr;
};

TestQgsMapToolMoveFeature::TestQgsMapToolMoveFeature() = default;


//runs before all tests
void TestQgsMapToolMoveFeature::initTestCase()
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
  const QString wkt1 = "Polygon ((0 0, 0 1, 1 1, 1 0, 0 0))";
  QgsFeature f1;
  f1.setGeometry( QgsGeometry::fromWkt( wkt1 ) );
  const QString wkt2 = "Polygon ((2 0, 2 5, 3 5, 3 0, 2 0))";
  QgsFeature f2;
  f2.setGeometry( QgsGeometry::fromWkt( wkt2 ) );

  QgsFeatureList flist;
  flist << f1 << f2;
  mLayerBase->dataProvider()->addFeatures( flist );
  QCOMPARE( mLayerBase->featureCount(), ( long )2 );
  QCOMPARE( mLayerBase->getFeature( 1 ).geometry().asWkt(), wkt1 );
  QCOMPARE( mLayerBase->getFeature( 2 ).geometry().asWkt(), wkt2 );

  QgsSnappingConfig cfg = mCanvas->snappingUtils()->config();
  cfg.setMode( Qgis::SnappingMode::AllLayers );
  cfg.setTolerance( 1 );
  cfg.setTypeFlag( static_cast<Qgis::SnappingTypes>( Qgis::SnappingType::Vertex | Qgis::SnappingType::Segment ) );
  cfg.setEnabled( true );
  mCanvas->snappingUtils()->setConfig( cfg );

  mCanvas->setLayers( QList<QgsMapLayer *>() << mLayerBase );
  mCanvas->setCurrentLayer( mLayerBase );

  // create the tool
  mCaptureTool = new QgsMapToolMoveFeature( mCanvas, QgsMapToolMoveFeature::Move );
  mCanvas->setMapTool( mCaptureTool );

  QCOMPARE( mCanvas->mapSettings().outputSize(), QSize( 512, 512 ) );
  QCOMPARE( mCanvas->mapSettings().visibleExtent(), QgsRectangle( 0, 0, 8, 8 ) );
}

//runs after all tests
void TestQgsMapToolMoveFeature::cleanupTestCase()
{
  delete mCaptureTool;
  delete mCanvas;
  QgsApplication::exitQgis();
}

void TestQgsMapToolMoveFeature::testMoveFeature()
{
  TestQgsMapToolAdvancedDigitizingUtils utils( mCaptureTool );

  utils.mouseClick( 1, 1, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  utils.mouseClick( 2, 1, Qt::LeftButton, Qt::KeyboardModifiers(), true );

  const QString wkt1 = "Polygon ((1 0, 1 1, 2 1, 2 0, 1 0))";
  QCOMPARE( mLayerBase->getFeature( 1 ).geometry().asWkt(), wkt1 );
  const QString wkt2 = "Polygon ((2 0, 2 5, 3 5, 3 0, 2 0))";
  QCOMPARE( mLayerBase->getFeature( 2 ).geometry().asWkt(), wkt2 );

  mLayerBase->undoStack()->undo();
}

void TestQgsMapToolMoveFeature::testTopologicalMoveFeature()
{
  const bool topologicalEditing = QgsProject::instance()->topologicalEditing();
  QgsProject::instance()->setTopologicalEditing( true );

  TestQgsMapToolAdvancedDigitizingUtils utils( mCaptureTool );

  utils.mouseClick( 1, 1, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  utils.mouseClick( 2, 1, Qt::LeftButton, Qt::KeyboardModifiers(), true );

  const QString wkt1 = "Polygon ((1 0, 1 1, 2 1, 2 0, 1 0))";
  QCOMPARE( mLayerBase->getFeature( 1 ).geometry().asWkt(), wkt1 );
  const QString wkt2 = "Polygon ((2 0, 2 1, 2 5, 3 5, 3 0, 2 0))";
  QCOMPARE( mLayerBase->getFeature( 2 ).geometry().asWkt(), wkt2 );

  mLayerBase->undoStack()->undo();

  QgsProject::instance()->setTopologicalEditing( topologicalEditing );
}

QGSTEST_MAIN( TestQgsMapToolMoveFeature )
#include "testqgsmaptoolmovefeature.moc"
