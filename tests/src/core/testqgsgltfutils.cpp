/***************************************************************************
     testqgsgltutils.cpp
     ----------------------
    Date                 : August 2023
    Copyright            : (C) 2023 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstest.h"

#include "qgsgltfutils.h"
#include "tiny_gltf.h"

class TestQgsGltfUtils : public QObject
{
    Q_OBJECT
  public:
    TestQgsGltfUtils() = default;

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.

    void testImageResourceType();
    void testExtractEmbeddedImage();
    void testExtractTextureCoordinates();

  private:
};

//runs before all tests
void TestQgsGltfUtils::initTestCase()
{
}

//runs after all tests
void TestQgsGltfUtils::cleanupTestCase()
{
}

void TestQgsGltfUtils::testImageResourceType()
{
  const QString dataDir( TEST_DATA_DIR );
  QString dataFile = dataDir + "/gltf/BoxTextured.glb";

  QFile f( dataFile );
  QVERIFY( f.open( QIODevice::ReadOnly ) );
  const QByteArray data = f.readAll();

  tinygltf::Model model;
  QVERIFY( QgsGltfUtils::loadGltfModel( data, model, nullptr, nullptr ) );
  QCOMPARE( QgsGltfUtils::imageResourceType( model, 0 ), QgsGltfUtils::ResourceType::Embedded );
}

void TestQgsGltfUtils::testExtractEmbeddedImage()
{
  const QString dataDir( TEST_DATA_DIR );
  QString dataFile = dataDir + "/gltf/BoxTextured.glb";

  QFile f( dataFile );
  QVERIFY( f.open( QIODevice::ReadOnly ) );
  const QByteArray data = f.readAll();

  tinygltf::Model model;
  QVERIFY( QgsGltfUtils::loadGltfModel( data, model, nullptr, nullptr ) );
  QImage image = QgsGltfUtils::extractEmbeddedImage( model, 0 );
  QVERIFY( !image.isNull() );
  QCOMPARE( image.width(), 256 );
  QCOMPARE( image.height(), 256 );
  QCOMPARE( image.format(), QImage::Format_ARGB32 );
}

void TestQgsGltfUtils::testExtractTextureCoordinates()
{
  const QString dataDir( TEST_DATA_DIR );
  QString dataFile = dataDir + "/gltf/BoxTextured.glb";

  QFile f( dataFile );
  QVERIFY( f.open( QIODevice::ReadOnly ) );
  const QByteArray data = f.readAll();

  tinygltf::Model model;
  QVERIFY( QgsGltfUtils::loadGltfModel( data, model, nullptr, nullptr ) );
  QVector<float> x;
  QVector<float> y;
  QVERIFY( QgsGltfUtils::extractTextureCoordinates( model, 3, x, y ) );
  QCOMPARE( x.size(), 24 );
  QCOMPARE( y.size(), 24 );
  QCOMPARE( x.at( 0 ), 6.0 );
  QCOMPARE( y.at( 0 ), 0.0 );
  QCOMPARE( x.at( 23 ), 1.0 );
  QGSCOMPARENEAR( y.at( 23 ), 1.0, 0.01 );
}

QGSTEST_MAIN( TestQgsGltfUtils )
#include "testqgsgltfutils.moc"
