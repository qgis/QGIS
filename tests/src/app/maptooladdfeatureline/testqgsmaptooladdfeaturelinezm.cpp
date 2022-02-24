/***************************************************************************
     testqgsmaptooladdfeaturelinezm.cpp
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
class TestQgsMapToolAddFeatureLineZM : public QObject
{
    Q_OBJECT
  public:
    TestQgsMapToolAddFeatureLineZM();

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.

    void testZM();

  private:
    QgisApp *mQgisApp = nullptr;
    QgsMapCanvas *mCanvas = nullptr;
    QgsMapToolAddFeature *mCaptureTool = nullptr;
    QgsVectorLayer *mLayerLineZM = nullptr;
};

TestQgsMapToolAddFeatureLineZM::TestQgsMapToolAddFeatureLineZM() = default;


//runs before all tests
void TestQgsMapToolAddFeatureLineZM::initTestCase()
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
  mLayerLineZM = new QgsVectorLayer( QStringLiteral( "LineStringZM?crs=EPSG:27700" ), QStringLiteral( "layer line M" ), QStringLiteral( "memory" ) );
  QVERIFY( mLayerLineZM->isValid() );
  QgsProject::instance()->addMapLayers( QList<QgsMapLayer *>() << mLayerLineZM );

  QgsPolyline line;
  line << QgsPoint( 1, 1, 0, 0 ) << QgsPoint( 2, 1, 0, 1 ) << QgsPoint( 3, 2, 0, 2 ) << QgsPoint( 1, 2, 0, 3 ) << QgsPoint( 1, 1, 0, 0 );
  QgsFeature lineF;
  lineF.setGeometry( QgsGeometry::fromPolyline( line ) );

  mLayerLineZM->startEditing();
  mLayerLineZM->addFeature( lineF );
  QCOMPARE( mLayerLineZM->featureCount(), ( long )1 );

  mCanvas->setFrameStyle( QFrame::NoFrame );
  mCanvas->resize( 512, 512 );
  mCanvas->setExtent( QgsRectangle( 0, 0, 8, 8 ) );
  mCanvas->show(); // to make the canvas resize
  mCanvas->hide();
  QCOMPARE( mCanvas->mapSettings().outputSize(), QSize( 512, 512 ) );
  QCOMPARE( mCanvas->mapSettings().visibleExtent(), QgsRectangle( 0, 0, 8, 8 ) );

  // add layers to canvas
  mCanvas->setLayers( QList<QgsMapLayer *>() << mLayerLineZM );
  mCanvas->setSnappingUtils( new QgsMapCanvasSnappingUtils( mCanvas, this ) );

  // create the tool
  mCaptureTool = new QgsMapToolAddFeature( mCanvas, QgisApp::instance()->cadDockWidget(), QgsMapToolCapture::CaptureLine );

  mCanvas->setMapTool( mCaptureTool );

  QCOMPARE( mCanvas->mapSettings().outputSize(), QSize( 512, 512 ) );
  QCOMPARE( mCanvas->mapSettings().visibleExtent(), QgsRectangle( 0, 0, 8, 8 ) );
}

//runs after all tests
void TestQgsMapToolAddFeatureLineZM::cleanupTestCase()
{
  delete mCaptureTool;
  delete mCanvas;
  QgsApplication::exitQgis();
}

void TestQgsMapToolAddFeatureLineZM::testZM()
{
  TestQgsMapToolAdvancedDigitizingUtils utils( mCaptureTool );

  mCanvas->setCurrentLayer( mLayerLineZM );

  // test with default M value = 333
  QgsSettingsRegistryCore::settingsDigitizingDefaultMValue.setValue( 333 );
  // test with default Z value = 123
  QgsSettingsRegistryCore::settingsDigitizingDefaultZValue.setValue( 123 );


  QSet<QgsFeatureId> oldFids = utils.existingFeatureIds();
  utils.mouseClick( 4, 0, Qt::LeftButton );
  utils.mouseClick( 5, 0, Qt::LeftButton );
  utils.mouseClick( 5, 1, Qt::LeftButton );
  utils.mouseClick( 4, 1, Qt::LeftButton );
  utils.mouseClick( 4, 1, Qt::RightButton );
  QgsFeatureId newFid = utils.newFeatureId( oldFids );

  QString wkt = "LineStringZM (4 0 123 333, 5 0 123 333, 5 1 123 333, 4 1 123 333)";
  QCOMPARE( mLayerLineZM->getFeature( newFid ).geometry(), QgsGeometry::fromWkt( wkt ) );

  mLayerLineZM->undoStack()->undo();

  // test with default M value = 222
  QgsSettingsRegistryCore::settingsDigitizingDefaultMValue.setValue( 222 );
  // test with default z value = 456
  QgsSettingsRegistryCore::settingsDigitizingDefaultZValue.setValue( 456 );

  oldFids = utils.existingFeatureIds();
  utils.mouseClick( 4, 0, Qt::LeftButton );
  utils.mouseClick( 5, 0, Qt::LeftButton );
  utils.mouseClick( 5, 1, Qt::LeftButton );
  utils.mouseClick( 4, 1, Qt::LeftButton );
  utils.mouseClick( 4, 1, Qt::RightButton );
  newFid = utils.newFeatureId( oldFids );

  wkt = "LineStringZM (4 0 456 222, 5 0 456 222, 5 1 456 222, 4 1 456 222)";
  QCOMPARE( mLayerLineZM->getFeature( newFid ).geometry(), QgsGeometry::fromWkt( wkt ) );

  mLayerLineZM->undoStack()->undo();

}

QGSTEST_MAIN( TestQgsMapToolAddFeatureLineZM )
#include "testqgsmaptooladdfeaturelinezm.moc"
