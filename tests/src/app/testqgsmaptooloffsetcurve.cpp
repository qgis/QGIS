/***************************************************************************
     testqgsmaptooloffsetcurve.cpp
     --------------------------------
    Date                 : March 2024
    Copyright            : (C) 2024 by Juho Ervasti
    Email                : juho dot ervasti at gispo dot fi
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
#include "qgssettingsregistrycore.h"
#include "qgssettingsentryenumflag.h"
#include "qgsmaptooloffsetcurve.h"
#include "qgsvectorlayer.h"
#include "testqgsmaptoolutils.h"


/**
 * \ingroup UnitTests
 * This is a unit test for the vertex tool
 */
class TestQgsMapToolOffsetCurve : public QObject
{
    Q_OBJECT
  public:
    TestQgsMapToolOffsetCurve();

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.

    void testOffsetCurveDefault();
    void testOffsetCurveJoinStyle();
    void testOffsetCurveControlModifier();
    void testAvoidIntersectionAndTopoEdit();

  private:
    QgisApp *mQgisApp = nullptr;
    QgsMapCanvas *mCanvas = nullptr;
    QgsMapToolOffsetCurve *mOffsetCurveTool = nullptr;
    QgsVectorLayer *mLayerBase = nullptr;
};

TestQgsMapToolOffsetCurve::TestQgsMapToolOffsetCurve() = default;


//runs before all tests
void TestQgsMapToolOffsetCurve::initTestCase()
{
  qDebug() << "TestQgsMapToolOffsetCurve::initTestCase()";
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
  const QString wkt1 = QStringLiteral( "Polygon ((0 0, 0 1, 1 1, 1 0, 0 0))" );
  QgsFeature f1;
  f1.setGeometry( QgsGeometry::fromWkt( wkt1 ) );
  const QString wkt2 = QStringLiteral( "Polygon ((2 0, 2 5, 3 5, 3 0, 2 0))" );
  QgsFeature f2;
  f2.setGeometry( QgsGeometry::fromWkt( wkt2 ) );

  QgsFeatureList flist;
  flist << f1 << f2;
  mLayerBase->dataProvider()->addFeatures( flist );
  QCOMPARE( mLayerBase->featureCount(), 2L );
  QCOMPARE( mLayerBase->getFeature( 1 ).geometry().asWkt(), wkt1 );
  QCOMPARE( mLayerBase->getFeature( 2 ).geometry().asWkt(), wkt2 );

  mCanvas->setLayers( QList<QgsMapLayer *>() << mLayerBase );
  mCanvas->setCurrentLayer( mLayerBase );

  // create the tool
  mOffsetCurveTool = new QgsMapToolOffsetCurve( mCanvas );
  mCanvas->setMapTool( mOffsetCurveTool );

  QCOMPARE( mCanvas->mapSettings().outputSize(), QSize( 512, 512 ) );
  QCOMPARE( mCanvas->mapSettings().visibleExtent(), QgsRectangle( 0, 0, 8, 8 ) );

  // set default offset settings to ensure consistency

  const Qgis::JoinStyle joinStyle = Qgis::JoinStyle::Round;
  const int quadSegments = 8;
  const double miterLimit = 5.0;
  const Qgis::EndCapStyle capStyle = Qgis::EndCapStyle::Round;

  QgsSettingsRegistryCore::settingsDigitizingOffsetJoinStyle->setValue( joinStyle );
  QgsSettingsRegistryCore::settingsDigitizingOffsetQuadSeg->setValue( quadSegments );
  QgsSettingsRegistryCore::settingsDigitizingOffsetMiterLimit->setValue( miterLimit );
  QgsSettingsRegistryCore::settingsDigitizingOffsetCapStyle->setValue( capStyle );

  QCOMPARE( QgsSettingsRegistryCore::settingsDigitizingOffsetJoinStyle->value(), joinStyle );
  QCOMPARE( QgsSettingsRegistryCore::settingsDigitizingOffsetQuadSeg->value(), quadSegments );
  QCOMPARE( QgsSettingsRegistryCore::settingsDigitizingOffsetMiterLimit->value(), miterLimit );
  QCOMPARE( QgsSettingsRegistryCore::settingsDigitizingOffsetCapStyle->value(), capStyle );
}

//runs after all tests
void TestQgsMapToolOffsetCurve::cleanupTestCase()
{
  delete mOffsetCurveTool;
  delete mCanvas;
  QgsApplication::exitQgis();
}

void TestQgsMapToolOffsetCurve::testOffsetCurveDefault()
{
  TestQgsMapToolUtils utils( mOffsetCurveTool );

  // positive offset
  utils.mouseClick( 1, 1, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  utils.mouseMove( 1.25, 1.25 );
  utils.mouseClick( 1.25, 1.25, Qt::LeftButton, Qt::KeyboardModifiers(), true );

  const QString wkt1 = "Polygon ((0 -0.35, -0.07 -0.35, -0.14 -0.33, -0.2 -0.29, -0.25 -0.25, -0.29 -0.2, -0.33 -0.14, -0.35 -0.07, -0.35 0, -0.35 1, -0.35 1.07, -0.33 1.14, -0.29 1.2, -0.25 1.25, -0.2 1.29, -0.14 1.33, -0.07 1.35, 0 1.35, 1 1.35, 1.07 1.35, 1.14 1.33, 1.2 1.29, 1.25 1.25, 1.29 1.2, 1.33 1.14, 1.35 1.07, 1.35 1, 1.35 0, 1.35 -0.07, 1.33 -0.14, 1.29 -0.2, 1.25 -0.25, 1.2 -0.29, 1.14 -0.33, 1.07 -0.35, 1 -0.35, 0 -0.35))";
  QCOMPARE( mLayerBase->getFeature( 1 ).geometry().asWkt( 2 ), wkt1 );

  mLayerBase->undoStack()->undo();

  // negative offset
  utils.mouseClick( 2, 0, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  utils.mouseMove( 2.1, 0.1 );
  utils.mouseClick( 2.1, 0.1, Qt::LeftButton, Qt::KeyboardModifiers(), true );

  const QString wkt2 = "Polygon ((2.09 0.09, 2.09 4.91, 2.91 4.91, 2.91 0.09, 2.09 0.09))";
  QCOMPARE( mLayerBase->getFeature( 2 ).geometry().asWkt( 2 ), wkt2 );

  mLayerBase->undoStack()->undo();
}

void TestQgsMapToolOffsetCurve::testOffsetCurveJoinStyle()
{
  TestQgsMapToolUtils utils( mOffsetCurveTool );

  const Qgis::JoinStyle joinStyle = QgsSettingsRegistryCore::settingsDigitizingOffsetJoinStyle->value();
  QgsSettingsRegistryCore::settingsDigitizingOffsetJoinStyle->setValue( Qgis::JoinStyle::Miter );

  // positive offset miter
  utils.mouseClick( 1, 1, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  utils.mouseMove( 1.5, 1.5 );
  utils.mouseClick( 1.5, 1.5, Qt::LeftButton, Qt::KeyboardModifiers(), true );

  const QString wkt1 = "Polygon ((-0.71 -0.71, -0.71 1.71, 1.71 1.71, 1.71 -0.71, -0.71 -0.71))";
  QCOMPARE( mLayerBase->getFeature( 1 ).geometry().asWkt( 2 ), wkt1 );

  mLayerBase->undoStack()->undo();

  // negative offset miter
  utils.mouseClick( 2, 0, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  utils.mouseMove( 2.25, 0.25 );
  utils.mouseClick( 2.25, 0.25, Qt::LeftButton, Qt::KeyboardModifiers(), true );

  const QString wkt2 = "Polygon ((2.25 0.25, 2.25 4.75, 2.75 4.75, 2.75 0.25, 2.25 0.25))";
  QCOMPARE( mLayerBase->getFeature( 2 ).geometry().asWkt( 2 ), wkt2 );

  mLayerBase->undoStack()->undo();

  QgsSettingsRegistryCore::settingsDigitizingOffsetJoinStyle->setValue( Qgis::JoinStyle::Bevel );

  // negative offset bevel
  utils.mouseClick( 1, 1, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  utils.mouseMove( 0.75, 0.75 );
  utils.mouseClick( 0.75, 0.75, Qt::LeftButton, Qt::KeyboardModifiers(), true );

  const QString wkt3 = "Polygon ((0.25 0.25, 0.25 0.75, 0.75 0.75, 0.75 0.25, 0.25 0.25))";
  QCOMPARE( mLayerBase->getFeature( 1 ).geometry().asWkt( 2 ), wkt3 );

  mLayerBase->undoStack()->undo();

  // positive offset bevel
  utils.mouseClick( 2, 0, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  utils.mouseMove( 1.75, -0.25 );
  utils.mouseClick( 1.75, -0.25, Qt::LeftButton, Qt::KeyboardModifiers(), true );

  const QString wkt4 = "Polygon ((2 -0.35, 1.65 0, 1.65 5, 2 5.35, 3 5.35, 3.35 5, 3.35 0, 3 -0.35, 2 -0.35))";
  QCOMPARE( mLayerBase->getFeature( 2 ).geometry().asWkt( 2 ), wkt4 );

  mLayerBase->undoStack()->undo();

  // reset settings
  QgsSettingsRegistryCore::settingsDigitizingOffsetJoinStyle->setValue( joinStyle );
}

void TestQgsMapToolOffsetCurve::testOffsetCurveControlModifier()
{
  TestQgsMapToolUtils utils( mOffsetCurveTool );

  const Qgis::JoinStyle joinStyle = QgsSettingsRegistryCore::settingsDigitizingOffsetJoinStyle->value();
  QgsSettingsRegistryCore::settingsDigitizingOffsetJoinStyle->setValue( Qgis::JoinStyle::Miter );

  // positive offset
  utils.mouseClick( 1, 1, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  utils.mouseMove( 1.5, 1.5 );
  utils.mouseClick( 1.5, 1.5, Qt::LeftButton, Qt::ControlModifier, true );

  const QString wkt1 = "Polygon ((-0.71 -0.71, -0.71 1.71, 1.71 1.71, 1.71 -0.71, -0.71 -0.71))";
  QgsFeatureIterator fi1 = mLayerBase->getFeatures();
  QgsFeature f1;

  while ( fi1.nextFeature( f1 ) )
  {
    QCOMPARE( f1.geometry().asWkt( 2 ), wkt1 );
    break;
  }

  const QString wkt2 = "Polygon ((0 0, 0 1, 1 1, 1 0, 0 0))";
  QCOMPARE( mLayerBase->getFeature( 1 ).geometry().asWkt( 2 ), wkt2 );

  mLayerBase->undoStack()->undo();

  // negative offset
  utils.mouseClick( 2, 0, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  utils.mouseMove( 2.25, 0.25 );
  utils.mouseClick( 2.25, 0.25, Qt::LeftButton, Qt::ControlModifier, true );

  const QString wkt3 = "Polygon ((2.25 0.25, 2.25 4.75, 2.75 4.75, 2.75 0.25, 2.25 0.25))";
  QgsFeatureIterator fi2 = mLayerBase->getFeatures();
  QgsFeature f2;

  while ( fi2.nextFeature( f2 ) )
  {
    QCOMPARE( f2.geometry().asWkt( 2 ), wkt3 );
    break;
  }

  const QString wkt4 = "Polygon ((2 0, 2 5, 3 5, 3 0, 2 0))";
  QCOMPARE( mLayerBase->getFeature( 2 ).geometry().asWkt( 2 ), wkt4 );
  mLayerBase->undoStack()->undo();

  // reset settings
  QgsSettingsRegistryCore::settingsDigitizingOffsetJoinStyle->setValue( joinStyle );
}

void TestQgsMapToolOffsetCurve::testAvoidIntersectionAndTopoEdit()
{
  TestQgsMapToolUtils utils( mOffsetCurveTool );

  const Qgis::JoinStyle joinStyle = QgsSettingsRegistryCore::settingsDigitizingOffsetJoinStyle->value();
  const bool topologicalEditing = QgsProject::instance()->topologicalEditing();
  const Qgis::AvoidIntersectionsMode mode( QgsProject::instance()->avoidIntersectionsMode() );

  // test with bevel
  QgsSettingsRegistryCore::settingsDigitizingOffsetJoinStyle->setValue( Qgis::JoinStyle::Bevel );
  QgsProject::instance()->setAvoidIntersectionsMode( Qgis::AvoidIntersectionsMode::AvoidIntersectionsCurrentLayer );
  QgsProject::instance()->setTopologicalEditing( true );

  utils.mouseClick( 1, 1, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  utils.mouseMove( 2, 1.75 );
  utils.mouseClick( 2, 1.75, Qt::LeftButton, Qt::KeyboardModifiers(), true );

  const QString wkt1 = "Polygon ((-1.25 0, -1.25 1, 0 2.25, 1 2.25, 2 1.25, 2 0, 2.25 0, 1 -1.25, 0 -1.25, -1.25 0))";
  QCOMPARE( mLayerBase->getFeature( 1 ).geometry().asWkt( 2 ), wkt1 );
  const QString wkt2 = "Polygon ((2 0, 2 1.25, 2 5, 3 5, 3 0, 2.25 0, 2 0))";
  QCOMPARE( mLayerBase->getFeature( 2 ).geometry().asWkt( 2 ), wkt2 );

  mLayerBase->undoStack()->undo();

  // with control modifier
  utils.mouseClick( 1, 1, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  utils.mouseMove( 2, 1.75 );
  utils.mouseClick( 2, 1.75, Qt::LeftButton, Qt::ControlModifier, true );

  const QString wkt3 = "Polygon ((-1.25 0, -1.25 1, 0 2.25, 1 2.25, 2 1.25, 2 0, 2.25 0, 1 -1.25, 0 -1.25, -1.25 0),(0 0, 1 0, 1 1, 0 1, 0 0))";
  QgsFeatureIterator fi = mLayerBase->getFeatures();
  QgsFeature f;

  while ( fi.nextFeature( f ) )
  {
    QCOMPARE( f.geometry().asWkt( 2 ), wkt3 );
    break;
  }

  const QString wkt4 = "Polygon ((0 0, 0 1, 1 1, 1 0, 0 0))";
  QCOMPARE( mLayerBase->getFeature( 1 ).geometry().asWkt( 2 ), wkt4 );

  mLayerBase->undoStack()->undo();

  // reset settings
  QgsSettingsRegistryCore::settingsDigitizingOffsetJoinStyle->setValue( joinStyle );
  QgsProject::instance()->setTopologicalEditing( topologicalEditing );
  QgsProject::instance()->setAvoidIntersectionsMode( mode );
}

QGSTEST_MAIN( TestQgsMapToolOffsetCurve )
#include "testqgsmaptooloffsetcurve.moc"
