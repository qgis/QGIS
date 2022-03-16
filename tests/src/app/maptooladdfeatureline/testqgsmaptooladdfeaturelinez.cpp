/***************************************************************************
     testqgsmaptooladdfeaturelinez.cpp
     ----------------------
    Date                 : April 2021
    Copyright            : (C) 2021 by Loïc Bartoletti
    Email                : loic dot bartoletti at oslandia dot com
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
#include "qgsmaptooladdfeature.h"
#include "qgsmapcanvastracer.h"
#include "qgsproject.h"
#include "qgssettings.h"
#include "qgssettingsregistrycore.h"
#include "qgsvectorlayer.h"
#include "qgswkbtypes.h"
#include "qgsmapmouseevent.h"
#include "testqgsmaptoolutils.h"

bool operator==( const QgsGeometry &g1, const QgsGeometry &g2 )
{
  if ( g1.isNull() && g2.isNull() )
    return true;
  else
    return g1.equals( g2 );
}

namespace QTest
{
  // pretty printing of geometries in comparison tests
  template<> char *toString( const QgsGeometry &geom )
  {
    QByteArray ba = geom.asWkt().toLatin1();
    return qstrdup( ba.data() );
  }
}


/**
 * \ingroup UnitTests
 * This is a unit test for the vertex tool
 */
class TestQgsMapToolAddFeatureLineZ : public QObject
{
    Q_OBJECT
  public:
    TestQgsMapToolAddFeatureLineZ();

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.

    void testZ();
    void testZSnapping();
    void testTopologicalEditingZ();

  private:
    QgisApp *mQgisApp = nullptr;
    QgsMapCanvas *mCanvas = nullptr;
    QgsMapToolAddFeature *mCaptureTool = nullptr;
    QgsVectorLayer *mLayerLine = nullptr;
    QgsVectorLayer *mLayerLineZ = nullptr;
    QgsVectorLayer *mLayerTopoZ = nullptr;
    QgsFeatureId mFidLineF1 = 0;
};

TestQgsMapToolAddFeatureLineZ::TestQgsMapToolAddFeatureLineZ() = default;


//runs before all tests
void TestQgsMapToolAddFeatureLineZ::initTestCase()
{
  qDebug() << "TestMapToolCapture::initTestCase()";
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();

  // Set up the QSettings environment
  QCoreApplication::setOrganizationName( QStringLiteral( "QGIS" ) );
  QCoreApplication::setOrganizationDomain( QStringLiteral( "qgis.org" ) );
  QCoreApplication::setApplicationName( QStringLiteral( "QGIS-TEST" ) );
  QgsSettings settings;
  settings.clear();

  mQgisApp = new QgisApp();

  mCanvas = new QgsMapCanvas();

  mCanvas->setDestinationCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:27700" ) ) );

  // make testing layers
  mLayerLine = new QgsVectorLayer( QStringLiteral( "LineString?crs=EPSG:27700" ), QStringLiteral( "layer line" ), QStringLiteral( "memory" ) );
  QVERIFY( mLayerLine->isValid() );
  QgsProject::instance()->addMapLayers( QList<QgsMapLayer *>() << mLayerLine );

  QgsPolylineXY line1;
  line1 << QgsPointXY( 1, 1 ) << QgsPointXY( 2, 1 ) << QgsPointXY( 3, 2 ) << QgsPointXY( 1, 2 ) << QgsPointXY( 1, 1 );
  QgsFeature lineF1;
  lineF1.setGeometry( QgsGeometry::fromPolylineXY( line1 ) );

  mLayerLine->startEditing();
  mLayerLine->addFeature( lineF1 );
  mFidLineF1 = lineF1.id();
  QCOMPARE( mLayerLine->featureCount(), ( long )1 );

  // just one added feature
  QCOMPARE( mLayerLine->undoStack()->index(), 1 );

  // make testing layers
  mLayerLineZ = new QgsVectorLayer( QStringLiteral( "LineStringZ?crs=EPSG:27700" ), QStringLiteral( "layer line Z" ), QStringLiteral( "memory" ) );
  QVERIFY( mLayerLineZ->isValid() );
  QgsProject::instance()->addMapLayers( QList<QgsMapLayer *>() << mLayerLineZ );

  QgsPolyline line2;
  line2 << QgsPoint( 1, 1, 0 ) << QgsPoint( 2, 1, 1 ) << QgsPoint( 3, 2, 2 ) << QgsPoint( 1, 2, 3 ) << QgsPoint( 1, 1, 0 );
  QgsFeature lineF2;
  lineF2.setGeometry( QgsGeometry::fromPolyline( line2 ) );

  mLayerLineZ->startEditing();
  mLayerLineZ->addFeature( lineF2 );
  QCOMPARE( mLayerLineZ->featureCount(), ( long )1 );

  mCanvas->setFrameStyle( QFrame::NoFrame );
  mCanvas->resize( 512, 512 );
  mCanvas->setExtent( QgsRectangle( 0, 0, 8, 8 ) );
  mCanvas->show(); // to make the canvas resize
  mCanvas->hide();
  QCOMPARE( mCanvas->mapSettings().outputSize(), QSize( 512, 512 ) );
  QCOMPARE( mCanvas->mapSettings().visibleExtent(), QgsRectangle( 0, 0, 8, 8 ) );

  // make layer for topologicalEditing with Z
  mLayerTopoZ = new QgsVectorLayer( QStringLiteral( "MultiLineStringZ?crs=EPSG:27700" ), QStringLiteral( "layer topologicalEditing Z" ), QStringLiteral( "memory" ) );
  QVERIFY( mLayerTopoZ->isValid() );
  QgsProject::instance()->addMapLayers( QList<QgsMapLayer *>() << mLayerTopoZ );

  mLayerTopoZ->startEditing();
  QgsFeature topoFeat;
  topoFeat.setGeometry( QgsGeometry::fromWkt( "MultiLineStringZ ("
                        "(10 0 0, 10 10 0),"
                        "(20 0 10, 20 10 10),"
                        "(30 0 0, 30 10 10)"
                        ")" ) );

  mLayerTopoZ->addFeature( topoFeat );
  QCOMPARE( mLayerTopoZ->featureCount(), ( long ) 1 );

  // add layers to canvas
  mCanvas->setLayers( QList<QgsMapLayer *>() << mLayerLine << mLayerLineZ << mLayerTopoZ );
  mCanvas->setSnappingUtils( new QgsMapCanvasSnappingUtils( mCanvas, this ) );

  // create the tool
  mCaptureTool = new QgsMapToolAddFeature( mCanvas, QgisApp::instance()->cadDockWidget(), QgsMapToolCapture::CaptureLine );

  mCanvas->setMapTool( mCaptureTool );
  mCanvas->setCurrentLayer( mLayerLine );

  QCOMPARE( mCanvas->mapSettings().outputSize(), QSize( 512, 512 ) );
  QCOMPARE( mCanvas->mapSettings().visibleExtent(), QgsRectangle( 0, 0, 8, 8 ) );
}

//runs after all tests
void TestQgsMapToolAddFeatureLineZ::cleanupTestCase()
{
  delete mCaptureTool;
  delete mCanvas;
  QgsApplication::exitQgis();
}

void TestQgsMapToolAddFeatureLineZ::testZ()
{
  TestQgsMapToolAdvancedDigitizingUtils utils( mCaptureTool );

  mCanvas->setCurrentLayer( mLayerLineZ );

  // test with default Z value = 333
  QgsSettingsRegistryCore::settingsDigitizingDefaultZValue.setValue( 333 );

  QSet<QgsFeatureId> oldFids = utils.existingFeatureIds();
  utils.mouseClick( 4, 0, Qt::LeftButton );
  utils.mouseClick( 5, 0, Qt::LeftButton );
  utils.mouseClick( 5, 1, Qt::LeftButton );
  utils.mouseClick( 4, 1, Qt::LeftButton );
  utils.mouseClick( 4, 1, Qt::RightButton );
  QgsFeatureId newFid = utils.newFeatureId( oldFids );

  QString wkt = "LineStringZ (4 0 333, 5 0 333, 5 1 333, 4 1 333)";
  QCOMPARE( mLayerLineZ->getFeature( newFid ).geometry(), QgsGeometry::fromWkt( wkt ) );

  mLayerLine->undoStack()->undo();

  // test with default Z value = 222
  QgsSettingsRegistryCore::settingsDigitizingDefaultZValue.setValue( 222 );

  oldFids = utils.existingFeatureIds();
  utils.mouseClick( 4, 0, Qt::LeftButton );
  utils.mouseClick( 5, 0, Qt::LeftButton );
  utils.mouseClick( 5, 1, Qt::LeftButton );
  utils.mouseClick( 4, 1, Qt::LeftButton );
  utils.mouseClick( 4, 1, Qt::RightButton );
  newFid = utils.newFeatureId( oldFids );

  wkt = "LineStringZ (4 0 222, 5 0 222, 5 1 222, 4 1 222)";
  QCOMPARE( mLayerLineZ->getFeature( newFid ).geometry(), QgsGeometry::fromWkt( wkt ) );

  mLayerLine->undoStack()->undo();

  mCanvas->setCurrentLayer( mLayerLine );
}

void TestQgsMapToolAddFeatureLineZ::testTopologicalEditingZ()
{
  TestQgsMapToolAdvancedDigitizingUtils utils( mCaptureTool );

  mCanvas->setCurrentLayer( mLayerTopoZ );

  // test with default Z value = 333
  QgsSettingsRegistryCore::settingsDigitizingDefaultZValue.setValue( 333 );

  QSet<QgsFeatureId> oldFids = utils.existingFeatureIds();

  QgsSnappingConfig cfg = mCanvas->snappingUtils()->config();
  const bool topologicalEditing = cfg.project()->topologicalEditing();
  cfg.project()->setTopologicalEditing( true );

  cfg.setMode( Qgis::SnappingMode::AllLayers );
  cfg.setEnabled( true );
  mCanvas->snappingUtils()->setConfig( cfg );

  oldFids = utils.existingFeatureIds();
  utils.mouseClick( 0, 5, Qt::LeftButton );
  utils.mouseClick( 10.1, 5, Qt::LeftButton );
  utils.mouseClick( 20.1, 5, Qt::LeftButton );
  utils.mouseClick( 30.1, 5, Qt::LeftButton );
  utils.mouseClick( 40, 5, Qt::LeftButton );
  utils.mouseClick( 40, 5, Qt::RightButton );
  const QgsFeatureId newFid = utils.newFeatureId( oldFids );

  QString wkt = "MultiLineStringZ ((0 5 333, 10 5 0, 20 5 10, 30 5 5, 40 5 333))";
  QCOMPARE( mLayerTopoZ->getFeature( newFid ).geometry(), QgsGeometry::fromWkt( wkt ) );

  wkt = "MultiLineStringZ ("
        "(10 0 0, 10 5 0, 10 10 0),"
        "(20 0 10, 20 5 10, 20 10 10),"
        "(30 0 0, 30 5 5, 30 10 10)"
        ")";
  QCOMPARE( mLayerTopoZ->getFeature( qgis::setToList( oldFids ).constLast() ).geometry(), QgsGeometry::fromWkt( wkt ) );

  mLayerLine->undoStack()->undo();

  cfg.setEnabled( false );
  mCanvas->snappingUtils()->setConfig( cfg );
  cfg.project()->setTopologicalEditing( topologicalEditing );
}

void TestQgsMapToolAddFeatureLineZ::testZSnapping()
{
  TestQgsMapToolAdvancedDigitizingUtils utils( mCaptureTool );

  mCanvas->setCurrentLayer( mLayerLine );

  QSet<QgsFeatureId> oldFids = utils.existingFeatureIds();

  QgsSnappingConfig cfg = mCanvas->snappingUtils()->config();
  cfg.setMode( Qgis::SnappingMode::AllLayers );
  cfg.setEnabled( true );
  mCanvas->snappingUtils()->setConfig( cfg );

  // snap a on a layer with ZM support
  utils.mouseClick( 6, 6, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  utils.mouseClick( 5, 0, Qt::LeftButton );
  utils.mouseClick( 5, 0, Qt::RightButton );
  QgsFeatureId newFid = utils.newFeatureId( oldFids );

  QCOMPARE( mLayerLine->getFeature( newFid ).geometry().get()->is3D(), false );
  QCOMPARE( mLayerLine->getFeature( newFid ).geometry().get()->isMeasure(), false );

  mLayerLine->undoStack()->undo();

  mCanvas->setCurrentLayer( mLayerLineZ );
  oldFids = utils.existingFeatureIds();
  // test with default Z value = 222
  QgsSettingsRegistryCore::settingsDigitizingDefaultZValue.setValue( 222 );
  // snap a on a layer without ZM support
  utils.mouseClick( 9, 9, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  utils.mouseClick( 8, 7, Qt::LeftButton );
  utils.mouseClick( 5, 0, Qt::RightButton );

  newFid = utils.newFeatureId( oldFids );

  QCOMPARE( mLayerLineZ->getFeature( newFid ).geometry().get()->is3D(), true );

  QString wkt = "LineStringZ (9 9 222, 8 7 222)";
  QCOMPARE( mLayerLineZ->getFeature( newFid ).geometry(), QgsGeometry::fromWkt( wkt ) );
  QCOMPARE( mLayerLineZ->getFeature( newFid ).geometry().get()->isMeasure(), false );

  mLayerLine->undoStack()->undo();

  // Snap on middle Segment
  mCanvas->setCurrentLayer( mLayerLineZ );
  cfg.setTypeFlag( Qgis::SnappingType::MiddleOfSegment );
  mCanvas->snappingUtils()->setConfig( cfg );

  // create geometry will be snapped
  QgsSettingsRegistryCore::settingsDigitizingDefaultZValue.setValue( 123 );

  oldFids = utils.existingFeatureIds();
  utils.mouseClick( 20, 20, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  utils.mouseClick( 30, 20, Qt::LeftButton );
  utils.mouseClick( 30, 20, Qt::RightButton );
  newFid = utils.newFeatureId( oldFids );

  QCOMPARE( mLayerLineZ->getFeature( newFid ).geometry().get()->is3D(), true );
  wkt = "LineStringZ (20 20 123, 30 20 123)";
  QCOMPARE( mLayerLineZ->getFeature( newFid ).geometry(), QgsGeometry::fromWkt( wkt ) );

  QgsSettingsRegistryCore::settingsDigitizingDefaultZValue.setValue( 321 );
  oldFids = utils.existingFeatureIds();
  utils.mouseClick( 25, 20, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  utils.mouseClick( 25, 25, Qt::LeftButton );
  utils.mouseClick( 25, 25, Qt::RightButton );
  newFid = utils.newFeatureId( oldFids );

  QCOMPARE( mLayerLineZ->getFeature( newFid ).geometry().get()->is3D(), true );
  wkt = "LineStringZ (25 20 321, 25 25 321)";
  QCOMPARE( mLayerLineZ->getFeature( newFid ).geometry(), QgsGeometry::fromWkt( wkt ) );

  mLayerLineZ->undoStack()->undo();
  mLayerLineZ->undoStack()->undo();

  cfg.setEnabled( false );
  mCanvas->snappingUtils()->setConfig( cfg );
}

QGSTEST_MAIN( TestQgsMapToolAddFeatureLineZ )
#include "testqgsmaptooladdfeaturelinez.moc"
