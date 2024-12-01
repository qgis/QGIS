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

#include "qgisapp.h"
#include "qgsapplication.h"
#include "qgsmapcanvas.h"
#include "qgstest.h"
#include "qgsvectorlayer.h"
#include "vertextool/qgslockedfeature.h"
#include "vertextool/qgsvertexeditor.h"
#include <memory>
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

  private:
    std::unique_ptr<QgsMapCanvas> mCanvas;
    QgisApp *mQgisApp = nullptr;
    std::unique_ptr<QgsVectorLayer> mLayerLine;
    std::unique_ptr<QgsVectorLayer> mLayerLineZ;
    std::unique_ptr<QgsVectorLayer> mLayerLineM;
    std::unique_ptr<QgsVectorLayer> mLayerLineZM;
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
}

void TestQgsVertexEditor::testColumnZMR_data()
{
  QTest::addColumn<QgsVectorLayer *>( "layer" );
  QTest::addColumn<QStringList>( "headers" );

  QTest::newRow( "Line" ) << mLayerLine.get() << ( QStringList() << QStringLiteral( "x" ) << QStringLiteral( "y" ) << QStringLiteral( "r" ) );
  QTest::newRow( "LineZ" ) << mLayerLineZ.get() << ( QStringList() << QStringLiteral( "x" ) << QStringLiteral( "y" ) << QStringLiteral( "z" ) << QStringLiteral( "r" ) );
  QTest::newRow( "LineM" ) << mLayerLineM.get() << ( QStringList() << QStringLiteral( "x" ) << QStringLiteral( "y" ) << QStringLiteral( "m" ) << QStringLiteral( "r" ) );
  QTest::newRow( "LineZM" ) << mLayerLineZM.get() << ( QStringList() << QStringLiteral( "x" ) << QStringLiteral( "y" ) << QStringLiteral( "z" ) << QStringLiteral( "m" ) << QStringLiteral( "r" ) );
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

//runs after all tests
void TestQgsVertexEditor::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

QGSTEST_MAIN( TestQgsVertexEditor )
#include "testqgsvertexeditor.moc"
