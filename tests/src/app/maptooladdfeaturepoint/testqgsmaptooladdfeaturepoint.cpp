/***************************************************************************
     testqgsmaptooladdfeaturepoint.cpp
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
#include "qgssnappingutils.h"
#include "qgsmaptooladdfeature.h"
#include "qgsmapcanvastracer.h"
#include "qgsproject.h"
#include "qgssettings.h"
#include "qgssettingsregistrycore.h"
#include "qgsvectorlayer.h"
#include "qgsmapmouseevent.h"
#include "testqgsmaptoolutils.h"

#include <QSignalSpy>


/**
 * \ingroup UnitTests
 * This is a unit test for the vertex tool
 */
class TestQgsMapToolAddFeaturePoint : public QObject
{
    Q_OBJECT
  public:
    TestQgsMapToolAddFeaturePoint();

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.

    void testPoint();
    void testMultiPoint(); //!< Test crash from regression GH #45153

  private:
    QgisApp *mQgisApp = nullptr;
    QgsMapCanvas *mCanvas = nullptr;
    QgsMapToolAddFeature *mCaptureTool = nullptr;
    QgsVectorLayer *mLayerPoint = nullptr;
};

TestQgsMapToolAddFeaturePoint::TestQgsMapToolAddFeaturePoint() = default;


//runs before all tests
void TestQgsMapToolAddFeaturePoint::initTestCase()
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

  mCanvas->setDestinationCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:27700" ) ) );

  mCanvas->setFrameStyle( QFrame::NoFrame );
  mCanvas->resize( 512, 512 );
  mCanvas->setExtent( QgsRectangle( 0, 0, 8, 8 ) );
  mCanvas->show(); // to make the canvas resize
  mCanvas->hide();

  // make testing M layer
  mLayerPoint = new QgsVectorLayer( QStringLiteral( "Point?crs=EPSG:27700" ), QStringLiteral( "layer point" ), QStringLiteral( "memory" ) );
  QVERIFY( mLayerPoint->isValid() );
  QgsProject::instance()->addMapLayers( QList<QgsMapLayer *>() << mLayerPoint );

  mLayerPoint->startEditing();
  QgsFeature pointF;
  const QString pointWkt = "Point(7 7)";
  pointF.setGeometry( QgsGeometry::fromWkt( pointWkt ) );

  mLayerPoint->addFeature( pointF );
  QCOMPARE( mLayerPoint->featureCount(), ( long )1 );

  // create the tool
  mCaptureTool = new QgsMapToolAddFeature( mCanvas, QgisApp::instance()->cadDockWidget(), QgsMapToolCapture::CapturePoint );
  mCanvas->setMapTool( mCaptureTool );

  QCOMPARE( mCanvas->mapSettings().outputSize(), QSize( 512, 512 ) );
  QCOMPARE( mCanvas->mapSettings().visibleExtent(), QgsRectangle( 0, 0, 8, 8 ) );
}

//runs after all tests
void TestQgsMapToolAddFeaturePoint::cleanupTestCase()
{
  delete mCaptureTool;
  delete mCanvas;
  QgsApplication::exitQgis();
}

void TestQgsMapToolAddFeaturePoint::testPoint()
{
  TestQgsMapToolAdvancedDigitizingUtils utils( mCaptureTool );
  mCanvas->setCurrentLayer( mLayerPoint );

  QSet<QgsFeatureId> oldFids = utils.existingFeatureIds();

  utils.mouseClick( 4, 0, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  QgsFeatureId newFid = utils.newFeatureId( oldFids );

  QCOMPARE( mLayerPoint->featureCount(), ( long )2 );

  QString wkt = "Point (4 0)";
  QCOMPARE( mLayerPoint->getFeature( newFid ).geometry().asWkt(), wkt );

  mLayerPoint->undoStack()->undo();

  oldFids = utils.existingFeatureIds();
  utils.mouseClick( 6, 6, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  newFid = utils.newFeatureId( oldFids );

  wkt = "Point (6 6)";
  QCOMPARE( mLayerPoint->getFeature( newFid ).geometry().asWkt(), wkt );

  mLayerPoint->undoStack()->undo();
}

void TestQgsMapToolAddFeaturePoint::testMultiPoint()
{

  QgsVectorLayer layerMultiPoint { QStringLiteral( "MultiPoint?crs=EPSG:27700" ), QStringLiteral( "layer multi point" ), QStringLiteral( "memory" ) };
  layerMultiPoint.startEditing();
  mCanvas->setCurrentLayer( &layerMultiPoint );

  TestQgsMapToolAdvancedDigitizingUtils utils( mCaptureTool );

  QSet<QgsFeatureId> oldFids = utils.existingFeatureIds();

  utils.mouseClick( 4, 0, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  QgsFeatureId fid1 = utils.newFeatureId( oldFids );

  QCOMPARE( layerMultiPoint.featureCount(), ( long )1 );

  QString wkt = "MultiPoint ((4 0))";
  QCOMPARE( layerMultiPoint.getFeature( fid1 ).geometry().asWkt(), wkt );


  oldFids = utils.existingFeatureIds();
  utils.mouseClick( 6, 6, Qt::LeftButton, Qt::KeyboardModifiers(), true );

  QCOMPARE( layerMultiPoint.featureCount(), ( long )2 );

  QgsFeatureId fid2 = utils.newFeatureId( oldFids );

  QString wkt2 = "MultiPoint ((6 6))";
  QCOMPARE( layerMultiPoint.getFeature( fid2 ).geometry().asWkt(), wkt2 );

  layerMultiPoint.undoStack()->undo(); // first point
  layerMultiPoint.undoStack()->undo(); // second point

}

QGSTEST_MAIN( TestQgsMapToolAddFeaturePoint )
#include "testqgsmaptooladdfeaturepoint.moc"
