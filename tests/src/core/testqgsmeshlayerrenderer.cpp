/***************************************************************************
                         testqgsmeshlayer.cpp
                         --------------------
    begin                : April 2018
    copyright            : (C) 2018 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstest.h"
#include <QObject>
#include <QString>
#include <QLabel>
#include <QStringList>
#include <QApplication>
#include <QFileInfo>
#include <QDir>
#include <QDesktopServices>

//qgis includes...
#include "qgsmaplayer.h"
#include "qgsmeshlayer.h"
#include "qgsapplication.h"
#include "qgsproviderregistry.h"
#include "qgsproject.h"
#include "qgsmaprenderersequentialjob.h"
#include "qgsmeshmemorydataprovider.h"

//qgis test includes
#include "qgsrenderchecker.h"

/**
 * \ingroup UnitTests
 * This is a unit test for the different renderers for mesh layers.
 */
class TestQgsMeshRenderer : public QObject
{
    Q_OBJECT

  public:
    TestQgsMeshRenderer() = default;

  private:
    QString mDataDir;
    QgsMeshLayer *mMemoryLayer = nullptr;
    QgsMapSettings *mMapSettings = nullptr;

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init(); // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.
    bool imageCheck( const QString &testType );
    QString readFile( const QString &fname ) const;


    void test_native_mesh_rendering();
    void test_triangular_mesh_rendering();
    void test_vertex_scalar_dataset_rendering();
};

void TestQgsMeshRenderer::init()
{
  mMemoryLayer->toggleTriangularMeshRendering( false );
  mMemoryLayer->setActiveScalarDataset();
  mMemoryLayer->setActiveVectorDataset();
}

void TestQgsMeshRenderer::initTestCase()
{
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::showSettings();
  mDataDir = QString( TEST_DATA_DIR ); //defined in CmakeLists.txt
  mDataDir += "/mesh";

  mMapSettings = new QgsMapSettings();

  // Memory layer
  mMemoryLayer = new QgsMeshLayer( readFile( "/quad_and_triangle.txt" ), "Triangle and Quad Memory", "mesh_memory" );
  mMemoryLayer->dataProvider()->addDataset( readFile( "/quad_and_triangle_vertex_scalar.txt" ) );
  mMemoryLayer->dataProvider()->addDataset( readFile( "/quad_and_triangle_vertex_vector.txt" ) );
  mMemoryLayer->dataProvider()->addDataset( readFile( "/quad_and_triangle_face_scalar.txt" ) );
  mMemoryLayer->dataProvider()->addDataset( readFile( "/quad_and_triangle_face_vector.txt" ) );
  QVERIFY( mMemoryLayer->isValid() );
  QgsProject::instance()->addMapLayers(
    QList<QgsMapLayer *>() << mMemoryLayer );
  mMapSettings->setLayers(
    QList<QgsMapLayer *>() << mMemoryLayer );
}

void TestQgsMeshRenderer::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

QString TestQgsMeshRenderer::readFile( const QString &fname ) const
{
  QString uri;
  QFile f( mDataDir + fname );
  if ( f.open( QIODevice::ReadOnly | QIODevice::Text ) )
    uri = f.readAll();
  return uri;
}

bool TestQgsMeshRenderer::imageCheck( const QString &testType )
{
  mMapSettings->setExtent( mMemoryLayer->extent() );
  mMapSettings->setDestinationCrs( mMemoryLayer->crs() );
  mMapSettings->setOutputDpi( 96 );
  QgsRenderChecker myChecker;
  myChecker.setControlPathPrefix( QStringLiteral( "mesh" ) );
  myChecker.setControlName( "expected_" + testType );
  myChecker.setMapSettings( *mMapSettings );
  myChecker.setColorTolerance( 15 );
  bool myResultFlag = myChecker.runTest( testType, 0 );
  return myResultFlag;
}

void TestQgsMeshRenderer::test_native_mesh_rendering()
{
  mMemoryLayer->toggleTriangularMeshRendering( false );
  QVERIFY( mMemoryLayer->triangularMeshSymbol() == nullptr );
  QVERIFY( imageCheck( "quad_and_triangle_native_mesh" ) );
}

void TestQgsMeshRenderer::test_triangular_mesh_rendering()
{
  mMemoryLayer->toggleTriangularMeshRendering( true );
  QVERIFY( mMemoryLayer->triangularMeshSymbol() != nullptr );
  QVERIFY( imageCheck( "quad_and_triangle_triangular_mesh" ) );
}

void TestQgsMeshRenderer::test_vertex_scalar_dataset_rendering()
{
  int ds = 0;
  mMemoryLayer->setActiveScalarDataset( ds );
  QgsMeshDatasetMetadata metadata = mMemoryLayer->dataProvider()->datasetMetadata( ds );
  QVERIFY( metadata["name"] == "VertexScalarDataset" );

  // TODO!
  // QVERIFY( imageCheck( "quad_and_triangle_vertex_scalar_dataset" ) );
}

QGSTEST_MAIN( TestQgsMeshRenderer )
#include "testqgsmeshlayerrenderer.moc"
