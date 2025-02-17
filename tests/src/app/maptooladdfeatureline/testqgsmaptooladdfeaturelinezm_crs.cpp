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

#include "qgstest.h"
#include "qgisapp.h"
#include "qgsadvanceddigitizingdockwidget.h"
#include "qgsgeometry.h"
#include "qgsmapcanvas.h"
#include "qgsmapcanvassnappingutils.h"
#include "qgssnappingconfig.h"
#include "qgsmaptooladdfeature.h"
#include "qgsproject.h"
#include "qgssettings.h"
#include "qgssettingsregistrycore.h"
#include "qgsvectorlayer.h"
#include "qgswkbtypes.h"
#include "qgsmapmouseevent.h"
#include <QTest>
#include <QDebug>

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
  mCanvas->setDestinationCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3857" ) ) );
  mCanvas->setFrameStyle( QFrame::NoFrame );
  mCanvas->resize( 512, 512 );
  mCanvas->setExtent( QgsRectangle( 0, 0, 10, 10 ) );
  mCanvas->show();


  mCaptureLayer = new QgsVectorLayer( QStringLiteral( "LineStringZM?crs=EPSG:3857" ), QStringLiteral( "Line Capture Layer" ), QStringLiteral( "memory" ) );
  QVERIFY( mCaptureLayer->isValid() );
  QgsProject::instance()->addMapLayers( { mCaptureLayer } );


  mSnappingLayer = new QgsVectorLayer( QStringLiteral( "PointZM?crs=EPSG:4326" ), QStringLiteral( "Snapping Points" ), QStringLiteral( "memory" ) );
  QVERIFY( mSnappingLayer->isValid() );

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
  QgsSettingsRegistryCore::settingsDigitizingDefaultMValue->setValue( 222 );
  QgsSettingsRegistryCore::settingsDigitizingDefaultZValue->setValue( 456 );

  mCanvas->setCurrentLayer( mCaptureLayer );
  mCaptureLayer->startEditing();

  QgsSnappingConfig snapConfig = mCanvas->snappingUtils()->config();
  snapConfig.setEnabled( true );
  snapConfig.setTolerance( 2 );
  snapConfig.setUnits( Qgis::MapToolUnit::Layer );
  snapConfig.setMode( Qgis::SnappingMode::AllLayers );
  snapConfig.setTypeFlag( Qgis::SnappingType::Vertex );
  mCanvas->snappingUtils()->setConfig( snapConfig );
  mCanvas->snappingUtils()->locatorForLayer( mSnappingLayer )->init();
  mCanvas->snappingUtils()->locatorForLayer( mCaptureLayer )->init();
  mCaptureTool = new QgsMapToolAddFeature( mCanvas, QgisApp::instance()->cadDockWidget(), QgsMapToolCapture::CaptureLine );
  mCanvas->setMapTool( mCaptureTool );

  QSet<QgsFeatureId> oldFids;
  QgsFeatureIterator it = mCaptureLayer->getFeatures();
  QgsFeature f;
  while ( it.nextFeature( f ) )
    oldFids.insert( f.id() );

  QgsPointXY p2XY = mCanvas->mapSettings().layerToMapCoordinates( mSnappingLayer, QgsPointXY( 7, 4 ) );
  QPoint p1( int( ( 0.0 / 10 ) * 512 ), int( ( ( 10 - 1.0 ) / 10 ) * 512 ) );           // (0,1)
  QPoint p2( int( ( p2XY.x() / 10 ) * 512 ), int( ( ( 10 - p2XY.y() ) / 10 ) * 512 ) ); // (7,4) – should snap
  QPoint p3( int( ( 2.0 / 10 ) * 512 ), int( ( ( 10 - 2.0 ) / 10 ) * 512 ) );           // (2,2) – should remain unsnapped

  QTest::mouseClick( mCanvas->viewport(), Qt::LeftButton, Qt::NoModifier, p1 );
  QTest::mouseMove( mCanvas->viewport(), p2 );
  QTest::mouseClick( mCanvas->viewport(), Qt::LeftButton, Qt::NoModifier, p2 );
  QTest::mouseClick( mCanvas->viewport(), Qt::LeftButton, Qt::NoModifier, p3 );
  QTest::mouseClick( mCanvas->viewport(), Qt::RightButton, Qt::NoModifier, p3 );

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

  QString expectedWkt = QStringLiteral( "LineString ZM (0 1 456 222, 7 4 999 888, 2 2 456 222)" );
  qDebug() << "Captured geometry:" << newFeature.geometry().asWkt();
  QCOMPARE( newFeature.geometry().asWkt( 0 ), expectedWkt );
}

QGSTEST_MAIN( TestQgsMapToolAddFeatureLineZMCRS )
#include "testqgsmaptooladdfeaturelinezm_crs.moc"
