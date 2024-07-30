/***************************************************************************
                             testqgsquantizedmeshutils.cpp
                             ----------------------------
    begin                : May 2024
    copyright            : (C) 2024 by David Koňařík
    email                : dvdkon at konarici dot cz

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// qgis_core doesn't export its tinygltf, so we build our own for the test
#include <cstdlib>
#include <qnamespace.h>
#include <qtestcase.h>
#include <sstream>
#define TINYGLTF_IMPLEMENTATION

#include "qgstest.h"
#include "qgsquantizedmeshtiles.h"
#include <fstream>
#include <iostream>

class TestQgsQuantizedMeshUtils : public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();
    void cleanupTestCase();

    void tileToGltf();
};

void TestQgsQuantizedMeshUtils::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
}

void TestQgsQuantizedMeshUtils::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsQuantizedMeshUtils::tileToGltf()
{
  QString sampleFilePath = getenv( "QUANTIZED_MESH_SAMPLE_IN" );
  const char *outFilePath = getenv( "QUANTIZED_MESH_SAMPLE_OUT" );

  bool checkOutput = false;
  if ( sampleFilePath.isEmpty() )
  {
    // Taken from https://terrain2.geo.admin.ch/1.0.0/ch.swisstopo.terrain.3d/default/20180601/4326/12/4309/970.terrain
    sampleFilePath = QStringLiteral( TEST_DATA_DIR ) + "/quantized_mesh.terrain";
    checkOutput = true; // We're using a known input, compare to known output
  }

  QFile sampleFile( sampleFilePath );
  sampleFile.open( QIODevice::ReadOnly );
  auto sampleData = sampleFile.readAll();

  auto tile = QgsQuantizedMeshTile( sampleData );

  auto model = tile.toGltf();
  tinygltf::TinyGLTF gltfLoader;

  if ( outFilePath != nullptr )
  {
    gltfLoader.WriteGltfSceneToFile( &model, outFilePath, true, true );
  }

  if ( checkOutput )
  {
    QFile correctOutFile( QStringLiteral( TEST_DATA_DIR ) + "/quantized_mesh.terrain.gltf" );
    correctOutFile.open( QIODevice::ReadOnly );
    auto correctOutput = correctOutFile.readAll();
    std::ostringstream newOutputStream;
    gltfLoader.WriteGltfSceneToStream( &model, newOutputStream, true, false );
    QByteArray newOutput( newOutputStream.str().data(), ( int ) newOutputStream.str().size() );
    QVERIFY( newOutput == correctOutput );
  }
}


QGSTEST_MAIN( TestQgsQuantizedMeshUtils )
#include "testqgsquantizedmeshtiles.moc"
