/***************************************************************************
     testqgsvertexeditor.cpp
     ----------------------
    Date                 : 2023-02-08
    Copyright            : (C) 2023 by Julien Cabieces
    Email                : julien dot cabieces at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <memory>

#include "qgisapp.h"
#include "qgsapplication.h"
#include "qgscompoundcurve.h"
#include "qgsmapcanvas.h"
#include "qgsnurbscurve.h"
#include "qgstest.h"
#include "qgsvectorlayer.h"
#include "vertextool/qgslockedfeature.h"
#include "vertextool/qgsvertexeditor.h"

#include <qnamespace.h>

/**
 * \ingroup UnitTests
 * This is a unit test for the vertex editor
 */
class TestQgsVertexEditor : public QgsTest
{
    Q_OBJECT
  public:
    TestQgsVertexEditor();

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.

    void testColumnZMR_data();
    void testColumnZMR();

    void testNurbsWeightColumn();
    void testPolyBezierRecognition();

  private:
    std::unique_ptr<QgsMapCanvas> mCanvas;
    QgisApp *mQgisApp = nullptr;
    std::unique_ptr<QgsVectorLayer> mLayerLine;
    std::unique_ptr<QgsVectorLayer> mLayerLineZ;
    std::unique_ptr<QgsVectorLayer> mLayerLineM;
    std::unique_ptr<QgsVectorLayer> mLayerLineZM;
    std::unique_ptr<QgsVectorLayer> mLayerNurbs;
    std::unique_ptr<QgsVectorLayer> mLayerPolyBezier;
    std::unique_ptr<QgsVertexEditorWidget> mVertexEditor;
};

TestQgsVertexEditor::TestQgsVertexEditor()
  : QgsTest( QStringLiteral( "Vertex Editor tests" ) ) {}

//runs before all tests
void TestQgsVertexEditor::initTestCase()
{
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  mQgisApp = new QgisApp();

  mCanvas = std::make_unique<QgsMapCanvas>();
  mCanvas->setDestinationCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:27700" ) ) );

  mVertexEditor = std::make_unique<QgsVertexEditorWidget>( mCanvas.get() );

  // Set up the QSettings environment
  QCoreApplication::setOrganizationName( QStringLiteral( "QGIS" ) );
  QCoreApplication::setOrganizationDomain( QStringLiteral( "qgis.org" ) );
  QCoreApplication::setApplicationName( QStringLiteral( "QGIS-TEST" ) );

  mLayerLine = std::make_unique<QgsVectorLayer>( QStringLiteral( "LineString?crs=EPSG:27700" ), QStringLiteral( "layer line Z" ), QStringLiteral( "memory" ) );
  QVERIFY( mLayerLine->isValid() );

  QgsFeature line;
  line.setGeometry( QgsGeometry::fromWkt( "LineStringZ (5 5, 6 6, 7 5)" ) );
  mLayerLine->dataProvider()->addFeature( line );
  QCOMPARE( mLayerLine->featureCount(), 1 );

  mLayerLineZ = std::make_unique<QgsVectorLayer>( QStringLiteral( "LineStringZ?crs=EPSG:27700" ), QStringLiteral( "layer line Z" ), QStringLiteral( "memory" ) );
  QVERIFY( mLayerLineZ->isValid() );

  line.setGeometry( QgsGeometry::fromWkt( "LineStringZ (5 5 1, 6 6 1, 7 5 1)" ) );
  mLayerLineZ->dataProvider()->addFeature( line );
  QCOMPARE( mLayerLineZ->featureCount(), 1 );

  mLayerLineM = std::make_unique<QgsVectorLayer>( QStringLiteral( "LineStringM?crs=EPSG:27700" ), QStringLiteral( "layer line M" ), QStringLiteral( "memory" ) );
  QVERIFY( mLayerLineM->isValid() );

  line.setGeometry( QgsGeometry::fromWkt( "LineStringM (5 5 1, 6 6 1, 7 5 1)" ) );
  mLayerLineM->dataProvider()->addFeature( line );
  QCOMPARE( mLayerLineM->featureCount(), 1 );

  mLayerLineZM = std::make_unique<QgsVectorLayer>( QStringLiteral( "LineStringZM?crs=EPSG:27700" ), QStringLiteral( "layer line ZM" ), QStringLiteral( "memory" ) );
  QVERIFY( mLayerLineZM->isValid() );

  line.setGeometry( QgsGeometry::fromWkt( "LineStringZM (5 5 1, 6 6 1, 7 5 1)" ) );
  mLayerLineZM->dataProvider()->addFeature( line );
  QCOMPARE( mLayerLineZM->featureCount(), 1 );

  mLayerNurbs = std::make_unique<QgsVectorLayer>( QStringLiteral( "CompoundCurve?crs=EPSG:27700" ), QStringLiteral( "layer nurbs" ), QStringLiteral( "memory" ) );
  QVERIFY( mLayerNurbs->isValid() );

  auto nurbs = std::make_unique<QgsNurbsCurve>(
    QVector<QgsPoint> { QgsPoint( 0, 0 ), QgsPoint( 5, 10 ), QgsPoint( 10, 5 ), QgsPoint( 15, 10 ) },
    3,
    QVector<double> { 0, 0, 0, 0, 1, 1, 1, 1 },
    QVector<double> { 1.0, 2.0, 1.5, 1.0 }
  );
  auto cc = std::make_unique<QgsCompoundCurve>();
  cc->addCurve( nurbs.release() );
  QgsFeature nurbsFeature;
  nurbsFeature.setGeometry( QgsGeometry( cc.release() ) );
  mLayerNurbs->dataProvider()->addFeature( nurbsFeature );
  QCOMPARE( mLayerNurbs->featureCount(), 1 );

  // Add a poly-Bézier with 2 segments (7 control points) for testing Alt+drag on middle anchor
  mLayerPolyBezier = std::make_unique<QgsVectorLayer>( QStringLiteral( "NurbsCurve?crs=EPSG:27700" ), QStringLiteral( "layer poly-bezier" ), QStringLiteral( "memory" ) );
  QVERIFY( mLayerPolyBezier->isValid() );

  // Poly-Bézier: 2 cubic segments joined at anchor point
  // Points: anchor0 - handle0_right - handle1_left - anchor1 - handle1_right - handle2_left - anchor2
  auto polyBezier = std::make_unique<QgsNurbsCurve>(
    QVector<QgsPoint> {
      QgsPoint( 0, 0 ),  // anchor 0 (index 0)
      QgsPoint( 2, 5 ),  // handle 0 right (index 1)
      QgsPoint( 4, 5 ),  // handle 1 left (index 2)
      QgsPoint( 5, 0 ),  // anchor 1 (index 3) - middle anchor
      QgsPoint( 6, -5 ), // handle 1 right (index 4)
      QgsPoint( 8, -5 ), // handle 2 left (index 5)
      QgsPoint( 10, 0 )  // anchor 2 (index 6)
    },
    3,
    QVector<double> { 0, 0, 0, 0, 1, 1, 1, 2, 2, 2, 2 }, // Poly-Bézier knots
    QVector<double> { 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0 }
  );
  QgsFeature polyBezierFeature;
  polyBezierFeature.setGeometry( QgsGeometry( polyBezier.release() ) );
  mLayerPolyBezier->dataProvider()->addFeature( polyBezierFeature );
  QCOMPARE( mLayerPolyBezier->featureCount(), 1 );
}

void TestQgsVertexEditor::testColumnZMR_data()
{
  QTest::addColumn<QgsVectorLayer *>( "layer" );
  QTest::addColumn<QStringList>( "headers" );

  QTest::newRow( "Line" ) << mLayerLine.get() << ( QStringList() << QStringLiteral( "x" ) << QStringLiteral( "y" ) );
  QTest::newRow( "LineZ" ) << mLayerLineZ.get() << ( QStringList() << QStringLiteral( "x" ) << QStringLiteral( "y" ) << QStringLiteral( "z" ) );
  QTest::newRow( "LineM" ) << mLayerLineM.get() << ( QStringList() << QStringLiteral( "x" ) << QStringLiteral( "y" ) << QStringLiteral( "m" ) );
  QTest::newRow( "LineZM" ) << mLayerLineZM.get() << ( QStringList() << QStringLiteral( "x" ) << QStringLiteral( "y" ) << QStringLiteral( "z" ) << QStringLiteral( "m" ) );
}

void TestQgsVertexEditor::testColumnZMR()
{
  QFETCH( QgsVectorLayer *, layer );
  QFETCH( QStringList, headers );

  QgsLockedFeature feat( 1, layer, mCanvas.get() );
  feat.selectVertex( 0 );

  mVertexEditor->updateEditor( &feat );

  QStringList hdrs;
  for ( int i = 0; i < mVertexEditor->mVertexModel->columnCount(); i++ )
    hdrs << mVertexEditor->mVertexModel->headerData( i, Qt::Horizontal, Qt::DisplayRole ).toString();

  QCOMPARE( headers, hdrs );
}

void TestQgsVertexEditor::testNurbsWeightColumn()
{
  QgsLockedFeature feat( 1, mLayerNurbs.get(), mCanvas.get() );
  feat.selectVertex( 0 );

  mVertexEditor->updateEditor( &feat );

  QStringList hdrs;
  for ( int i = 0; i < mVertexEditor->mVertexModel->columnCount(); i++ )
    hdrs << mVertexEditor->mVertexModel->headerData( i, Qt::Horizontal, Qt::DisplayRole ).toString();

  QVERIFY( hdrs.contains( QStringLiteral( "w" ) ) );

  QCOMPARE( mVertexEditor->mVertexModel->rowCount(), 4 );

  const int wCol = hdrs.indexOf( QLatin1Char( 'w' ) );
  QVERIFY( wCol >= 0 );

  const QModelIndex idx0 = mVertexEditor->mVertexModel->index( 0, wCol );
  const QModelIndex idx1 = mVertexEditor->mVertexModel->index( 1, wCol );
  const QModelIndex idx2 = mVertexEditor->mVertexModel->index( 2, wCol );
  const QModelIndex idx3 = mVertexEditor->mVertexModel->index( 3, wCol );

  QCOMPARE( mVertexEditor->mVertexModel->data( idx0, Qt::DisplayRole ).toDouble(), 1.0 );
  QCOMPARE( mVertexEditor->mVertexModel->data( idx1, Qt::DisplayRole ).toDouble(), 2.0 );
  QCOMPARE( mVertexEditor->mVertexModel->data( idx2, Qt::DisplayRole ).toDouble(), 1.5 );
  QCOMPARE( mVertexEditor->mVertexModel->data( idx3, Qt::DisplayRole ).toDouble(), 1.0 );
}

void TestQgsVertexEditor::testPolyBezierRecognition()
{
  // Verify that the poly-Bézier is correctly recognized
  QgsFeature f = mLayerPolyBezier->getFeature( 1 );
  QVERIFY( f.isValid() );

  const QgsGeometry geom = f.geometry();
  QVERIFY( !geom.isNull() );

  const QgsAbstractGeometry *abstractGeom = geom.constGet();
  QVERIFY( abstractGeom );

  const QgsNurbsCurve *nurbs = qgsgeometry_cast<const QgsNurbsCurve *>( abstractGeom );
  QVERIFY( nurbs );
  QCOMPARE( nurbs->numPoints(), 7 );
  QCOMPARE( nurbs->degree(), 3 );
  QVERIFY( nurbs->isPolyBezier() );

  // Verify that anchor indices are correct (0, 3, 6 for 2-segment poly-Bézier)
  // and handles are at 1, 2, 4, 5
  const QVector<QgsPoint> &ctrlPts = nurbs->controlPoints();
  QCOMPARE( ctrlPts.size(), 7 );

  // Anchor 0
  QCOMPARE( ctrlPts[0].x(), 0.0 );
  QCOMPARE( ctrlPts[0].y(), 0.0 );

  // Anchor 1 (middle)
  QCOMPARE( ctrlPts[3].x(), 5.0 );
  QCOMPARE( ctrlPts[3].y(), 0.0 );

  // Anchor 2
  QCOMPARE( ctrlPts[6].x(), 10.0 );
  QCOMPARE( ctrlPts[6].y(), 0.0 );

  // Verify vertex editor displays 7 rows for 7 control points
  QgsLockedFeature feat( 1, mLayerPolyBezier.get(), mCanvas.get() );
  feat.selectVertex( 0 );
  mVertexEditor->updateEditor( &feat );
  QCOMPARE( mVertexEditor->mVertexModel->rowCount(), 7 );
}

//runs after all tests
void TestQgsVertexEditor::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

QGSTEST_MAIN( TestQgsVertexEditor )
#include "testqgsvertexeditor.moc"
