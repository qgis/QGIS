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
#include "qgsmapcanvas.h"
#include "qgsnurbscurve.h"
#include "qgstest.h"
#include "qgsvectorlayer.h"
#include "vertextool/qgslockedfeature.h"
#include "vertextool/qgsvertexeditor.h"

#include <QString>
#include <qnamespace.h>

using namespace Qt::StringLiterals;

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

    void testColumnZMRW_data();
    void testColumnZMRW();

  private:
    std::unique_ptr<QgsMapCanvas> mCanvas;
    QgisApp *mQgisApp = nullptr;
    std::unique_ptr<QgsVectorLayer> mLayerLine;
    std::unique_ptr<QgsVectorLayer> mLayerLineZ;
    std::unique_ptr<QgsVectorLayer> mLayerLineM;
    std::unique_ptr<QgsVectorLayer> mLayerLineZM;
    std::unique_ptr<QgsVectorLayer> mLayerNurbs;
    std::unique_ptr<QgsVectorLayer> mLayerNurbsZ;
    std::unique_ptr<QgsVectorLayer> mLayerNurbsM;
    std::unique_ptr<QgsVectorLayer> mLayerNurbsZM;
    std::unique_ptr<QgsVertexEditorWidget> mVertexEditor;
};

TestQgsVertexEditor::TestQgsVertexEditor()
  : QgsTest( u"Vertex Editor tests"_s ) {}

//runs before all tests
void TestQgsVertexEditor::initTestCase()
{
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  mQgisApp = new QgisApp();

  mCanvas = std::make_unique<QgsMapCanvas>();
  mCanvas->setDestinationCrs( QgsCoordinateReferenceSystem( u"EPSG:27700"_s ) );

  mVertexEditor = std::make_unique<QgsVertexEditorWidget>( mCanvas.get() );

  // Set up the QSettings environment
  QCoreApplication::setOrganizationName( u"QGIS"_s );
  QCoreApplication::setOrganizationDomain( u"qgis.org"_s );
  QCoreApplication::setApplicationName( u"QGIS-TEST"_s );

  mLayerLine = std::make_unique<QgsVectorLayer>( u"LineString?crs=EPSG:27700"_s, u"layer line Z"_s, u"memory"_s );
  QVERIFY( mLayerLine->isValid() );

  QgsFeature line;
  line.setGeometry( QgsGeometry::fromWkt( "LineStringZ (5 5, 6 6, 7 5)" ) );
  mLayerLine->dataProvider()->addFeature( line );
  QCOMPARE( mLayerLine->featureCount(), 1 );

  mLayerLineZ = std::make_unique<QgsVectorLayer>( u"LineStringZ?crs=EPSG:27700"_s, u"layer line Z"_s, u"memory"_s );
  QVERIFY( mLayerLineZ->isValid() );

  line.setGeometry( QgsGeometry::fromWkt( "LineStringZ (5 5 1, 6 6 1, 7 5 1)" ) );
  mLayerLineZ->dataProvider()->addFeature( line );
  QCOMPARE( mLayerLineZ->featureCount(), 1 );

  mLayerLineM = std::make_unique<QgsVectorLayer>( u"LineStringM?crs=EPSG:27700"_s, u"layer line M"_s, u"memory"_s );
  QVERIFY( mLayerLineM->isValid() );

  line.setGeometry( QgsGeometry::fromWkt( "LineStringM (5 5 1, 6 6 1, 7 5 1)" ) );
  mLayerLineM->dataProvider()->addFeature( line );
  QCOMPARE( mLayerLineM->featureCount(), 1 );

  mLayerLineZM = std::make_unique<QgsVectorLayer>( u"LineStringZM?crs=EPSG:27700"_s, u"layer line ZM"_s, u"memory"_s );
  QVERIFY( mLayerLineZM->isValid() );

  line.setGeometry( QgsGeometry::fromWkt( "LineStringZM (5 5 1, 6 6 1, 7 5 1)" ) );
  mLayerLineZM->dataProvider()->addFeature( line );
  QCOMPARE( mLayerLineZM->featureCount(), 1 );

  mLayerNurbs = std::make_unique<QgsVectorLayer>( u"NurbsCurve?crs=EPSG:27700"_s, u"layer nurbs"_s, u"memory"_s );
  QVERIFY( mLayerNurbs->isValid() );
  mLayerNurbsZ = std::make_unique<QgsVectorLayer>( u"NurbsCurveZ?crs=EPSG:27700"_s, u"layer nurbs Z"_s, u"memory"_s );
  QVERIFY( mLayerNurbsZ->isValid() );
  mLayerNurbsM = std::make_unique<QgsVectorLayer>( u"NurbsCurveM?crs=EPSG:27700"_s, u"layer nurbs M"_s, u"memory"_s );
  QVERIFY( mLayerNurbsM->isValid() );
  mLayerNurbsZM = std::make_unique<QgsVectorLayer>( u"NurbsCurveZM?crs=EPSG:27700"_s, u"layer nurbs ZM"_s, u"memory"_s );
  QVERIFY( mLayerNurbsZM->isValid() );
}

void TestQgsVertexEditor::testColumnZMRW_data()
{
  QTest::addColumn<QgsVectorLayer *>( "layer" );
  QTest::addColumn<QStringList>( "headers" );

  QTest::newRow( "Line" ) << mLayerLine.get() << ( QStringList() << u"x"_s << u"y"_s << u"r"_s );
  QTest::newRow( "LineZ" ) << mLayerLineZ.get() << ( QStringList() << u"x"_s << u"y"_s << u"z"_s << u"r"_s );
  QTest::newRow( "LineM" ) << mLayerLineM.get() << ( QStringList() << u"x"_s << u"y"_s << u"m"_s << u"r"_s );
  QTest::newRow( "LineZM" ) << mLayerLineZM.get() << ( QStringList() << u"x"_s << u"y"_s << u"z"_s << u"m"_s << u"r"_s );
  QTest::newRow( "Nurbs" ) << mLayerNurbs.get() << ( QStringList() << u"x"_s << u"y"_s << u"r"_s << u"w"_s );
  QTest::newRow( "NurbsZ" ) << mLayerNurbsZ.get() << ( QStringList() << u"x"_s << u"y"_s << u"z"_s << u"r"_s << u"w"_s );
  QTest::newRow( "NurbsM" ) << mLayerNurbsM.get() << ( QStringList() << u"x"_s << u"y"_s << u"m"_s << u"r"_s << u"w"_s );
  QTest::newRow( "NurbsZM" ) << mLayerNurbsZM.get() << ( QStringList() << u"x"_s << u"y"_s << u"z"_s << u"m"_s << u"r"_s << u"w"_s );
}

void TestQgsVertexEditor::testColumnZMRW()
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

//runs after all tests
void TestQgsVertexEditor::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

QGSTEST_MAIN( TestQgsVertexEditor )
#include "testqgsvertexeditor.moc"
