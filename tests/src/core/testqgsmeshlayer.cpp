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

/**
 * \ingroup UnitTests
 * This is a unit test for a mesh layer
 */
class TestQgsMeshLayer : public QObject
{
    Q_OBJECT

  public:
    TestQgsMeshLayer() = default;

  private:
    QgsMeshLayer *mMemoryLayer = nullptr;
    QgsMeshLayer *mMdalLayer = nullptr;

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() {} // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.

    void test_data_provider();
    void test_extent();
};

void TestQgsMeshLayer::initTestCase()
{
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::showSettings();
  QString myDataDir( TEST_DATA_DIR ); //defined in CmakeLists.txt
  myDataDir += "/mesh";
  // Memory layer
  QFile f( myDataDir + "/quad_and_triangle.txt" );
  QVERIFY( f.open( QIODevice::ReadOnly | QIODevice::Text ) );
  QString uri( f.readAll() );
  QVERIFY( !uri.isEmpty() );
  mMemoryLayer = new QgsMeshLayer( uri, "Triangle and Quad Memory", "mesh_memory" );
  QVERIFY( mMemoryLayer->isValid() );
  QgsProject::instance()->addMapLayers(
    QList<QgsMapLayer *>() << mMemoryLayer );

  // MDAL Layer
  uri = myDataDir + "/quad_and_triangle.2dm";
  mMdalLayer = new QgsMeshLayer( uri, "Triangle and Quad MDAL", "mdal" );
  QVERIFY( mMdalLayer->isValid() );
  QgsProject::instance()->addMapLayers(
    QList<QgsMapLayer *>() << mMdalLayer );
}

void TestQgsMeshLayer::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsMeshLayer::test_data_provider()
{
  QList<const QgsMeshDataProvider *> dataProviders;
  dataProviders.append( mMemoryLayer->dataProvider() );
  dataProviders.append( mMdalLayer->dataProvider() );

  QgsRectangle expectedExtent( 1000.0, 2000.0, 3000.0, 3000.0 );

  foreach ( auto dp, dataProviders )
  {
    QVERIFY( dp != nullptr );
    QVERIFY( dp->isValid() );
    QCOMPARE( expectedExtent, dp->extent() );

    QCOMPARE( ( size_t ) 5, dp->vertexCount() );
    QCOMPARE( QgsMeshVertex( 1000.0, 2000.0 ), dp->vertex( 0 ) );
    QCOMPARE( QgsMeshVertex( 2000.0, 2000.0 ), dp->vertex( 1 ) );
    QCOMPARE( QgsMeshVertex( 3000.0, 2000.0 ), dp->vertex( 2 ) );
    QCOMPARE( QgsMeshVertex( 2000.0, 3000.0 ), dp->vertex( 3 ) );
    QCOMPARE( QgsMeshVertex( 1000.0, 3000.0 ), dp->vertex( 4 ) );

    QCOMPARE( ( size_t ) 2, dp->faceCount() );
    QgsMeshFace f1;
    f1 << 0 << 1 << 3 << 4;
    QCOMPARE( f1, dp->face( 0 ) );

    QgsMeshFace f2;
    f2 << 1 << 2 << 3;
    QCOMPARE( f2, dp->face( 1 ) );
  }
}

void TestQgsMeshLayer::test_extent()
{
  QCOMPARE( mMemoryLayer->dataProvider()->extent(), mMemoryLayer->extent() );
  QCOMPARE( mMdalLayer->dataProvider()->extent(), mMdalLayer->extent() );
}

QGSTEST_MAIN( TestQgsMeshLayer )
#include "testqgsmeshlayer.moc"
