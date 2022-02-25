/***************************************************************************
     testqgsmaptooladdfeaturelinem.cpp
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
class TestQgsMapToolAddFeatureLineM : public QObject
{
    Q_OBJECT
  public:
    TestQgsMapToolAddFeatureLineM();

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.

    void testM();
    void testTopologicalEditingM();

  private:
    QgisApp *mQgisApp = nullptr;
    QgsMapCanvas *mCanvas = nullptr;
    QgsMapToolAddFeature *mCaptureTool = nullptr;
    QgsVectorLayer *mLayerLine = nullptr;
    QgsVectorLayer *mLayerLineM = nullptr;
    QgsVectorLayer *mLayerTopoM = nullptr;
    QgsFeatureId mFidLineF1 = 0;
};

TestQgsMapToolAddFeatureLineM::TestQgsMapToolAddFeatureLineM() = default;


//runs before all tests
void TestQgsMapToolAddFeatureLineM::initTestCase()
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
  mLayerLineM = new QgsVectorLayer( QStringLiteral( "LineStringM?crs=EPSG:27700" ), QStringLiteral( "layer line M" ), QStringLiteral( "memory" ) );
  QVERIFY( mLayerLineM->isValid() );
  QgsProject::instance()->addMapLayers( QList<QgsMapLayer *>() << mLayerLineM );

  QgsPolyline line2;
  line2 << QgsPoint( QgsWkbTypes::PointM, 1, 1, 0, 0 ) << QgsPoint( QgsWkbTypes::PointM, 2, 1, 0, 1 ) << QgsPoint( QgsWkbTypes::PointM, 3, 2, 0, 2 ) << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 3 ) << QgsPoint( QgsWkbTypes::PointM, 1, 1, 0, 0 );
  QgsFeature lineF2;
  lineF2.setGeometry( QgsGeometry::fromPolyline( line2 ) );

  mLayerLineM->startEditing();
  mLayerLineM->addFeature( lineF2 );
  QCOMPARE( mLayerLineM->featureCount(), ( long )1 );

  mCanvas->setFrameStyle( QFrame::NoFrame );
  mCanvas->resize( 512, 512 );
  mCanvas->setExtent( QgsRectangle( 0, 0, 8, 8 ) );
  mCanvas->show(); // to make the canvas resize
  mCanvas->hide();
  QCOMPARE( mCanvas->mapSettings().outputSize(), QSize( 512, 512 ) );
  QCOMPARE( mCanvas->mapSettings().visibleExtent(), QgsRectangle( 0, 0, 8, 8 ) );

  // make layer for topologicalEditing with M
  mLayerTopoM = new QgsVectorLayer( QStringLiteral( "MultiLineStringM?crs=EPSG:27700" ), QStringLiteral( "layer topologicalEditing M" ), QStringLiteral( "memory" ) );
  QVERIFY( mLayerTopoM->isValid() );
  QgsProject::instance()->addMapLayers( QList<QgsMapLayer *>() << mLayerTopoM );

  mLayerTopoM->startEditing();
  QgsFeature topoFeat;
  topoFeat.setGeometry( QgsGeometry::fromWkt( "MultiLineStringM ((7.25 6 0, 7.25 7 0, 7.5 7 0, 7.5 6 0, 7.25 6 0),(6 6 0, 6 7 10, 7 7 0, 7 6 0, 6 6 0),(6.25 6.25 0, 6.75 6.25 0, 6.75 6.75 0, 6.25 6.75 0, 6.25 6.25 0))" ) );

  mLayerTopoM->addFeature( topoFeat );
  QCOMPARE( mLayerTopoM->featureCount(), ( long ) 1 );

  // add layers to canvas
  mCanvas->setLayers( QList<QgsMapLayer *>() << mLayerLine << mLayerLineM << mLayerTopoM );
  mCanvas->setSnappingUtils( new QgsMapCanvasSnappingUtils( mCanvas, this ) );

  // create the tool
  mCaptureTool = new QgsMapToolAddFeature( mCanvas, QgisApp::instance()->cadDockWidget(), QgsMapToolCapture::CaptureLine );

  mCanvas->setMapTool( mCaptureTool );
  mCanvas->setCurrentLayer( mLayerLine );

  QCOMPARE( mCanvas->mapSettings().outputSize(), QSize( 512, 512 ) );
  QCOMPARE( mCanvas->mapSettings().visibleExtent(), QgsRectangle( 0, 0, 8, 8 ) );
}

//runs after all tests
void TestQgsMapToolAddFeatureLineM::cleanupTestCase()
{
  delete mCaptureTool;
  delete mCanvas;
  QgsApplication::exitQgis();
}

void TestQgsMapToolAddFeatureLineM::testM()
{
  TestQgsMapToolAdvancedDigitizingUtils utils( mCaptureTool );

  mCanvas->setCurrentLayer( mLayerLineM );

  // test with default M value = 333
  QgsSettingsRegistryCore::settingsDigitizingDefaultMValue.setValue( 333 );

  QSet<QgsFeatureId> oldFids = utils.existingFeatureIds();
  utils.mouseClick( 4, 0, Qt::LeftButton );
  utils.mouseClick( 5, 0, Qt::LeftButton );
  utils.mouseClick( 5, 1, Qt::LeftButton );
  utils.mouseClick( 4, 1, Qt::LeftButton );
  utils.mouseClick( 4, 1, Qt::RightButton );
  QgsFeatureId newFid = utils.newFeatureId( oldFids );

  QString wkt = "LineStringM (4 0 333, 5 0 333, 5 1 333, 4 1 333)";
  QCOMPARE( mLayerLineM->getFeature( newFid ).geometry(), QgsGeometry::fromWkt( wkt ) );

  mLayerLine->undoStack()->undo();

  // test with default M value = 222
  QgsSettingsRegistryCore::settingsDigitizingDefaultMValue.setValue( 222 );

  oldFids = utils.existingFeatureIds();
  utils.mouseClick( 4, 0, Qt::LeftButton );
  utils.mouseClick( 5, 0, Qt::LeftButton );
  utils.mouseClick( 5, 1, Qt::LeftButton );
  utils.mouseClick( 4, 1, Qt::LeftButton );
  utils.mouseClick( 4, 1, Qt::RightButton );
  newFid = utils.newFeatureId( oldFids );

  wkt = "LineStringM (4 0 222, 5 0 222, 5 1 222, 4 1 222)";
  QCOMPARE( mLayerLineM->getFeature( newFid ).geometry(), QgsGeometry::fromWkt( wkt ) );

  mLayerLine->undoStack()->undo();

  mCanvas->setCurrentLayer( mLayerLine );
}

void TestQgsMapToolAddFeatureLineM::testTopologicalEditingM()
{
  TestQgsMapToolAdvancedDigitizingUtils utils( mCaptureTool );

  mCanvas->setCurrentLayer( mLayerTopoM );

  // test with default M value = 333
  QgsSettingsRegistryCore::settingsDigitizingDefaultMValue.setValue( 333 );

  QSet<QgsFeatureId> oldFids = utils.existingFeatureIds();

  QgsSnappingConfig cfg = mCanvas->snappingUtils()->config();
  const bool topologicalEditing = cfg.project()->topologicalEditing();
  cfg.project()->setTopologicalEditing( true );

  cfg.setMode( Qgis::SnappingMode::AllLayers );
  cfg.setEnabled( true );
  mCanvas->snappingUtils()->setConfig( cfg );

  oldFids = utils.existingFeatureIds();
  utils.mouseClick( 6, 6.5, Qt::LeftButton );
  utils.mouseClick( 6.25, 6.5, Qt::LeftButton );
  utils.mouseClick( 6.75, 6.5, Qt::LeftButton );
  utils.mouseClick( 7.25, 6.5, Qt::LeftButton );
  utils.mouseClick( 7.5, 6.5, Qt::LeftButton );
  utils.mouseClick( 8, 6.5, Qt::RightButton );
  const QgsFeatureId newFid = utils.newFeatureId( oldFids );

  QString wkt = "MultiLineStringM((6 6.5 333, 6.25 6.5 333, 6.75 6.5 333, 7.25 6.5 333, 7.5 6.5 333))";
  QCOMPARE( mLayerTopoM->getFeature( newFid ).geometry(), QgsGeometry::fromWkt( wkt ) );
  wkt = "MultiLineStringM ((7.25 6 0, 7.25 6.5 333, 7.25 7 0, 7.5 7 0, 7.5 6.5 333, 7.5 6 0, 7.25 6 0),(6 6 0, 6 6.5 333, 6 7 10, 7 7 0, 7 6 0, 6 6 0),(6.25 6.25 0, 6.75 6.25 0, 6.75 6.5 333, 6.75 6.75 0, 6.25 6.75 0, 6.25 6.5 333, 6.25 6.25 0))";
  QCOMPARE( mLayerTopoM->getFeature( qgis::setToList( oldFids ).constLast() ).geometry(), QgsGeometry::fromWkt( wkt ) );

  mLayerLine->undoStack()->undo();

  cfg.setEnabled( false );
  mCanvas->snappingUtils()->setConfig( cfg );
  cfg.project()->setTopologicalEditing( topologicalEditing );
}

QGSTEST_MAIN( TestQgsMapToolAddFeatureLineM )
#include "testqgsmaptooladdfeaturelinem.moc"
