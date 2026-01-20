/***************************************************************************
     testqgsmaptoolchamferfillet.cpp
     --------------------------------
    begin                : September 2025
    copyright            : (C) 2025 by Oslandia
    email                : benoit dot de dot mezzo at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgisapp.h"
#include "qgsgeometry.h"
#include "qgsmapcanvas.h"
#include "qgsmaptoolchamferfillet.h"
#include "qgssettingsentryenumflag.h"
#include "qgssettingsentryimpl.h"
#include "qgstest.h"
#include "qgsvectorlayer.h"
#include "testqgsmaptoolutils.h"

/**
 * \ingroup UnitTests
 * This is a unit test for the vertex tool
 */
class TestQgsMapToolChamferFillet : public QObject
{
    Q_OBJECT
  public:
    TestQgsMapToolChamferFillet();

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.

    void testChamferFilletDefault();
    void testChamfer();
    void testFillet();
    void testInvalidGeom();

  private:
    bool compareGeom( const QgsGeometry &geom, const QString &wkt, double tolerance );

    QgisApp *mQgisApp = nullptr;
    QgsMapCanvas *mCanvas = nullptr;
    QgsMapToolChamferFillet *mChamferFilletTool = nullptr;
    QgsVectorLayer *mLayerBase = nullptr;
};

TestQgsMapToolChamferFillet::TestQgsMapToolChamferFillet() = default;


bool TestQgsMapToolChamferFillet::compareGeom( const QgsGeometry &geom, const QString &wkt, double tolerance )
{
  QgsGeometry geomB = QgsGeometry::fromWkt( wkt );
  bool out = geom.constGet()->fuzzyEqual( *geomB.constGet(), tolerance );
  if ( !out )
  {
    qDebug() << "Failure with actual:" << geom.asWkt( 2 );
    qDebug() << "           expected:" << geomB.asWkt( 2 );
  }
  return out;
}

//runs before all tests
void TestQgsMapToolChamferFillet::initTestCase()
{
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();

  // Set up the QSettings environment
  QCoreApplication::setOrganizationName( u"QGIS"_s );
  QCoreApplication::setOrganizationDomain( u"qgis.org"_s );
  QCoreApplication::setApplicationName( u"QGIS-TEST"_s );

  mQgisApp = new QgisApp();

  mCanvas = new QgsMapCanvas();

  mCanvas->setDestinationCrs( QgsCoordinateReferenceSystem( u"EPSG:3946"_s ) );

  mCanvas->setFrameStyle( QFrame::NoFrame );
  mCanvas->resize( 512, 512 );
  mCanvas->setExtent( QgsRectangle( 0, 0, 8, 8 ) );
  mCanvas->show(); // to make the canvas resize
  mCanvas->hide();

  // make testing layers
  mLayerBase = new QgsVectorLayer( u"Polygon?crs=EPSG:3946"_s, u"baselayer"_s, u"memory"_s );
  QVERIFY( mLayerBase->isValid() );
  QgsProject::instance()->addMapLayers( QList<QgsMapLayer *>() << mLayerBase );

  mLayerBase->startEditing();
  const QString wkt1 = u"Polygon ((0 0, 0 1, 1 1, 1 0, 0 0))"_s;
  QgsFeature f1;
  f1.setGeometry( QgsGeometry::fromWkt( wkt1 ) );
  const QString wkt2 = u"Polygon ((2 0, 2 5, 3 5, 3 0, 2 5, 3 5, 2 0))"_s;
  QgsFeature f2;
  f2.setGeometry( QgsGeometry::fromWkt( wkt2 ) );

  QgsFeatureList flist;
  flist << f1 << f2;
  mLayerBase->dataProvider()->addFeatures( flist );
  QCOMPARE( mLayerBase->featureCount(), 2L );
  QVERIFY( compareGeom( mLayerBase->getFeature( 1 ).geometry(), wkt1, 0.05 ) );
  QVERIFY( compareGeom( mLayerBase->getFeature( 2 ).geometry(), wkt2, 0.05 ) );

  mCanvas->setLayers( QList<QgsMapLayer *>() << mLayerBase );
  mCanvas->setCurrentLayer( mLayerBase );

  // create the tool
  mChamferFilletTool = new QgsMapToolChamferFillet( mCanvas );
  mCanvas->setMapTool( mChamferFilletTool );

  QCOMPARE( mCanvas->mapSettings().outputSize(), QSize( 512, 512 ) );
  QCOMPARE( mCanvas->mapSettings().visibleExtent(), QgsRectangle( 0, 0, 8, 8 ) );

  // set default offset settings to ensure consistency

  QgsMapToolChamferFillet::settingsFilletSegment->setValue( 8 );
  QgsMapToolChamferFillet::settingsOperation->setValue( QgsGeometry::ChamferFilletOperationType::Chamfer );
  QgsMapToolChamferFillet::settingsLock1->setValue( false );
  QgsMapToolChamferFillet::settingsLock2->setValue( true );

  QCOMPARE( QgsMapToolChamferFillet::settingsFilletSegment->value(), 8 );
  QCOMPARE( QgsMapToolChamferFillet::settingsOperation->value(), QgsGeometry::ChamferFilletOperationType::Chamfer );
  QCOMPARE( QgsMapToolChamferFillet::settingsLock1->value(), false );
  QCOMPARE( QgsMapToolChamferFillet::settingsLock2->value(), true );

  QgsMapToolChamferFillet::settingsLock2->setValue( false );
}

//runs after all tests
void TestQgsMapToolChamferFillet::cleanupTestCase()
{
  delete mChamferFilletTool;
  delete mCanvas;
  QgsApplication::exitQgis();
}

void TestQgsMapToolChamferFillet::testChamferFilletDefault()
{
  TestQgsMapToolUtils utils( mChamferFilletTool );

  // asymmetric
  utils.mouseClick( 1, 1, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  utils.mouseMove( 0.85, 0.75 );
  utils.mouseClick( 0.85, 0.75, Qt::LeftButton, Qt::KeyboardModifiers(), true );

  const QString wkt1 = "Polygon ((0 0, 0 1, 0.44 1, 1 0.65, 1 0, 0 0))";
  QVERIFY( compareGeom( mLayerBase->getFeature( 1 ).geometry(), wkt1, 0.05 ) );

  mLayerBase->undoStack()->undo();

  // symmetric
  utils.mouseClick( 1, 1, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  utils.mouseMove( 0.85, 0.75 );
  utils.mouseClick( 0.85, 0.75, Qt::LeftButton, Qt::ShiftModifier, true );

  const QString wkt2 = "Polygon ((0 0, 0 1, 0.55 1, 1 0.55, 1 0, 0 0))";
  QVERIFY( compareGeom( mLayerBase->getFeature( 1 ).geometry(), wkt2, 0.05 ) );

  mLayerBase->undoStack()->undo();
}

void TestQgsMapToolChamferFillet::testInvalidGeom()
{
  TestQgsMapToolUtils utils( mChamferFilletTool );

  QString message;
  connect( mChamferFilletTool, &QgsMapTool::messageEmitted, this, [&message]( const QString &msg, Qgis::MessageLevel ) { message = msg; } );

  QgsMapToolChamferFillet::settingsOperation->setValue( QgsGeometry::ChamferFilletOperationType::Chamfer );

  utils.mouseClick( 2, 0, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  QCOMPARE( message, "Chamfer/fillet: input geometry is invalid!" );
}

void TestQgsMapToolChamferFillet::testChamfer()
{
  TestQgsMapToolUtils utils( mChamferFilletTool );

  QgsMapToolChamferFillet::settingsOperation->setValue( QgsGeometry::ChamferFilletOperationType::Chamfer );

  // asymmetric
  utils.mouseClick( 1, 1, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  utils.mouseMove( 0.85, 0.75 );
  utils.mouseClick( 0.85, 0.75, Qt::LeftButton, Qt::KeyboardModifiers(), true );

  const QString wkt1 = "Polygon ((0 0, 0 1, 0.44 1, 1 0.65, 1 0, 0 0))";
  QVERIFY( compareGeom( mLayerBase->getFeature( 1 ).geometry(), wkt1, 0.05 ) );

  mLayerBase->undoStack()->undo();

  // symmetric
  utils.mouseClick( 1, 1, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  utils.mouseMove( 0.85, 0.75 );
  utils.mouseClick( 0.85, 0.75, Qt::LeftButton, Qt::ShiftModifier, true );

  const QString wkt2 = "Polygon ((0 0, 0 1, 0.55 1, 1 0.55, 1 0, 0 0))";
  QVERIFY( compareGeom( mLayerBase->getFeature( 1 ).geometry(), wkt2, 0.05 ) );

  mLayerBase->undoStack()->undo();

  // outside
  utils.mouseClick( 1, 1, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  utils.mouseMove( 1.15, 1.25 );
  utils.mouseClick( 1.15, 1.25, Qt::LeftButton, Qt::ShiftModifier, true );

  const QString wkt3 = "Polygon ((0 0, 0 1, 0.55 1, 1 0.55, 1 0, 0 0))";
  QVERIFY( compareGeom( mLayerBase->getFeature( 1 ).geometry(), wkt3, 0.05 ) );

  mLayerBase->undoStack()->undo();
}

void TestQgsMapToolChamferFillet::testFillet()
{
  TestQgsMapToolUtils utils( mChamferFilletTool );

  QgsMapToolChamferFillet::settingsOperation->setValue( QgsGeometry::ChamferFilletOperationType::Fillet );

  // coarse fillet - click one side
  QgsMapToolChamferFillet::settingsFilletSegment->setValue( 3 );
  utils.mouseClick( 1, 1, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  utils.mouseMove( 0.25, 0.5 );
  utils.mouseClick( 0.25, 0.5, Qt::LeftButton, Qt::KeyboardModifiers(), true );

  const QString wkt1 = "Polygon ((0 0, 0 1, 0.05 1, 0.52 0.87, 0.87 0.53, 1 0.05, 1 0, 0 0))";
  QVERIFY( compareGeom( mLayerBase->getFeature( 1 ).geometry(), wkt1, 0.05 ) );

  mLayerBase->undoStack()->undo();

  // coarse fillet - click other side
  utils.mouseClick( 1, 1, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  utils.mouseMove( 0.5, 0.25 );
  utils.mouseClick( 0.5, 0.25, Qt::LeftButton, Qt::KeyboardModifiers(), true );

  QCOMPARE( mLayerBase->getFeature( 1 ).geometry().asWkt( 2 ), wkt1 );

  mLayerBase->undoStack()->undo();

  // coarse fillet - click outside side
  utils.mouseClick( 1, 1, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  utils.mouseMove( 1.5, 1.15 );
  utils.mouseClick( 1.5, 1.15, Qt::LeftButton, Qt::KeyboardModifiers(), true );

  QCOMPARE( mLayerBase->getFeature( 1 ).geometry().asWkt( 2 ), wkt1 );

  mLayerBase->undoStack()->undo();

  // fine fillet
  QgsMapToolChamferFillet::settingsFilletSegment->setValue( 16 );
  utils.mouseClick( 1, 1, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  utils.mouseMove( 0.25, 0.5 );
  utils.mouseClick( 0.25, 0.5, Qt::LeftButton, Qt::KeyboardModifiers(), true );

  const QString wkt2 = QString( "Polygon ((0 0, 0 1, 0.05 1, 0.14 1, 0.24 0.98, 0.33 0.96, 0.41 0.93, 0.5 0.89, "
                                "0.58 0.84, 0.65 0.78, 0.72 0.72, 0.78 0.65, 0.84 0.58, 0.89 0.5, 0.93 0.41, "
                                "0.96 0.33, 0.98 0.24, 1 0.14, 1 0.05, 1 0, 0 0))" );
  QVERIFY( compareGeom( mLayerBase->getFeature( 1 ).geometry(), wkt2, 0.05 ) );

  mLayerBase->undoStack()->undo();
}

QGSTEST_MAIN( TestQgsMapToolChamferFillet )
#include "testqgsmaptoolchamferfillet.moc"
