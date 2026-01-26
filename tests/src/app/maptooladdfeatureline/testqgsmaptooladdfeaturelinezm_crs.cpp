/***************************************************************************
     testqgsmaptooladdfeaturelinezm.cpp
     ----------------------
    Date                 : February 2025
    Copyright            : (C) 2025 by Loïc Bartoletti
    Email                : loic dot bartoletti at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgisapp.h"
#include "qgsadvanceddigitizingdockwidget.h"
#include "qgsgeometry.h"
#include "qgsmapcanvas.h"
#include "qgsmapcanvassnappingutils.h"
#include "qgsmapmouseevent.h"
#include "qgsmaptooladdfeature.h"
#include "qgsproject.h"
#include "qgssettings.h"
#include "qgssettingsregistrycore.h"
#include "qgssnappingconfig.h"
#include "qgstest.h"
#include "qgsvectorlayer.h"
#include "qgswkbtypes.h"

#include <QDebug>
#include <QTest>

// Comparison operator for QgsGeometry.
bool operator==( const QgsGeometry &g1, const QgsGeometry &g2 )
{
  if ( g1.isNull() && g2.isNull() )
    return true;
  else
    return g1.equals( g2 );
}

namespace QTest
{
  // Pretty-printing for QgsGeometry.
  template<> char *toString( const QgsGeometry &geom )
  {
    QByteArray ba = geom.asWkt().toLatin1();
    return qstrdup( ba.data() );
  }
} // namespace QTest

class TestQgsMapToolAddFeatureLineZMCRS : public QObject
{
    Q_OBJECT
  public:
    TestQgsMapToolAddFeatureLineZMCRS() = default;

  private slots:
    void initTestCase();
    void cleanupTestCase();
    void testZMCRS();

  private:
    QgisApp *mQgisApp = nullptr;
    QgsMapCanvas *mCanvas = nullptr;
    QgsMapToolAddFeature *mCaptureTool = nullptr;
    QgsVectorLayer *mCaptureLayer = nullptr;
    QgsVectorLayer *mSnappingLayer = nullptr;
};

void TestQgsMapToolAddFeatureLineZMCRS::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
  mQgisApp = new QgisApp();

  mCanvas = new QgsMapCanvas();
  mCanvas->setDestinationCrs( QgsCoordinateReferenceSystem( u"EPSG:3857"_s ) );
  mCanvas->setFrameStyle( QFrame::NoFrame );
  mCanvas->resize( 512, 512 );
  mCanvas->setExtent( QgsRectangle( 0, 0, 10, 10 ) );
  mCanvas->show();

  // Create capture layer in EPSG:3857
  mCaptureLayer = new QgsVectorLayer( u"LineStringZM?crs=EPSG:3857"_s, u"Line Capture Layer"_s, u"memory"_s );
  QVERIFY( mCaptureLayer->isValid() );
  QgsProject::instance()->addMapLayers( { mCaptureLayer } );

  // Create snapping layer in EPSG:3946
  mSnappingLayer = new QgsVectorLayer( u"PointZM?crs=EPSG:3946"_s, u"Snapping Points"_s, u"memory"_s );
  QVERIFY( mSnappingLayer->isValid() );

  // Add a snapping point with ZM values
  QgsFeature snapFeature( mSnappingLayer->fields() );
  QgsPoint snapPoint( 7, 4, 999, 888 );
  snapFeature.setGeometry( QgsGeometry::fromPoint( snapPoint ) );
  mSnappingLayer->startEditing();
  mSnappingLayer->addFeature( snapFeature );
  mSnappingLayer->commitChanges();
  QCOMPARE( mSnappingLayer->featureCount(), ( long ) 1 );

  QgsProject::instance()->addMapLayers( { mSnappingLayer, mCaptureLayer } );
  mCanvas->setLayers( { mCaptureLayer, mSnappingLayer } );
  mCanvas->setSnappingUtils( new QgsMapCanvasSnappingUtils( mCanvas, this ) );
}

void TestQgsMapToolAddFeatureLineZMCRS::cleanupTestCase()
{
  delete mCaptureTool;
  delete mCanvas;
  QgsApplication::exitQgis();
}

void TestQgsMapToolAddFeatureLineZMCRS::testZMCRS()
{
  // Set default Z and M values for digitizing
  QgsSettingsRegistryCore::settingsDigitizingDefaultMValue->setValue( 222 );
  QgsSettingsRegistryCore::settingsDigitizingDefaultZValue->setValue( 456 );

  mCanvas->setCurrentLayer( mCaptureLayer );
  mCaptureLayer->startEditing();

  // Configure snapping
  QgsSnappingConfig snapConfig = mCanvas->snappingUtils()->config();
  snapConfig.setEnabled( true );
  snapConfig.setTolerance( 20 );
  snapConfig.setUnits( Qgis::MapToolUnit::Pixels );
  snapConfig.setMode( Qgis::SnappingMode::AllLayers );
  snapConfig.setTypeFlag( Qgis::SnappingType::Vertex );
  mCanvas->snappingUtils()->setConfig( snapConfig );

  // Wait for snapping locators to be initialized
  mCanvas->snappingUtils()->locatorForLayer( mSnappingLayer )->init();
  mCanvas->snappingUtils()->locatorForLayer( mCaptureLayer )->init();

  // Create the capture tool
  mCaptureTool = new QgsMapToolAddFeature( mCanvas, QgisApp::instance()->cadDockWidget(), QgsMapToolCapture::CaptureLine );
  mCanvas->setMapTool( mCaptureTool );

  // Record existing features before capture
  QSet<QgsFeatureId> oldFids;
  QgsFeatureIterator it = mCaptureLayer->getFeatures();
  QgsFeature f;
  while ( it.nextFeature( f ) )
    oldFids.insert( f.id() );

  // Get transformed coordinates of the snapping point
  QgsPointXY p2XY = mCanvas->mapSettings().layerToMapCoordinates( mSnappingLayer, QgsPointXY( 7, 4 ) );

  // Convert map coordinates to screen coordinates for mouse events
  QPoint p1( int( ( 0.0 / 10 ) * 512 ), int( ( ( 10 - 1.0 ) / 10 ) * 512 ) );           // (0,1)
  QPoint p2( int( ( p2XY.x() / 10 ) * 512 ), int( ( ( 10 - p2XY.y() ) / 10 ) * 512 ) ); // Transformed point
  QPoint p3( int( ( 2.0 / 10 ) * 512 ), int( ( ( 10 - 2.0 ) / 10 ) * 512 ) );           // (2,2)

  // Click to add first point (default Z/M)
  QTest::mouseClick( mCanvas->viewport(), Qt::LeftButton, Qt::NoModifier, p1 );

  // Move to and click the second point (should snap and inherit Z/M)
  QTest::mouseMove( mCanvas->viewport(), p2 );
  QTest::mouseClick( mCanvas->viewport(), Qt::LeftButton, Qt::NoModifier, p2 );

  // Add third point (default Z/M)
  QTest::mouseClick( mCanvas->viewport(), Qt::LeftButton, Qt::NoModifier, p3 );

  // Finish the line with right click
  QTest::mouseClick( mCanvas->viewport(), Qt::RightButton, Qt::NoModifier, p3 );

  // Find the newly created feature
  QgsFeature newFeature;
  QgsFeatureIterator itNew = mCaptureLayer->getFeatures();
  while ( itNew.nextFeature( f ) )
  {
    if ( !oldFids.contains( f.id() ) )
    {
      newFeature = f;
      break;
    }
  }
  QVERIFY( newFeature.isValid() );

  // Get the geometry as a QgsLineString to access Z and M values
  const QgsLineString *linestring = qgsgeometry_cast<const QgsLineString *>( newFeature.geometry().constGet() );
  QVERIFY( linestring );

  // Verify geometry has 3 vertices
  QCOMPARE( linestring->numPoints(), 3 );

  // Check first vertex: should have default Z/M
  QgsPoint point1 = linestring->pointN( 0 );
  QCOMPARE( point1.z(), 456.0 );
  QCOMPARE( point1.m(), 222.0 );

  // Check second vertex: should have the snapped point's Z/M
  QgsPoint point2 = linestring->pointN( 1 );
  QCOMPARE( point2.z(), 999.0 );
  QCOMPARE( point2.m(), 888.0 );

  // Check third vertex: should have default Z/M
  QgsPoint point3 = linestring->pointN( 2 );
  QCOMPARE( point3.z(), 456.0 );
  QCOMPARE( point3.m(), 222.0 );

  // Undo the changes
  mCaptureLayer->undoStack()->undo();
  mCaptureLayer->rollBack();
}

QGSTEST_MAIN( TestQgsMapToolAddFeatureLineZMCRS )
#include "testqgsmaptooladdfeaturelinezm_crs.moc"
