/***************************************************************************
     testqgsmaprenderercache.cpp
     --------------------------------------
    Date                 : January 2021
    Copyright            : (C) 2021 by Peter Petrik
    Email                : zilolv at gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgstest.h"

#include <QImage>

#include "qgsmaprenderercache.h"
#include "qgsmaptopixel.h"
#include "qgsrectangle.h"

class TestQgsMapRendereCache: public QObject
{
    Q_OBJECT

  public:
    TestQgsMapRendereCache();

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init(); // will be called before each testfunction is executed.
    void cleanup(); // will be called after every testfunction.

    void testCache();
};


TestQgsMapRendereCache::TestQgsMapRendereCache() = default;

void TestQgsMapRendereCache::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
}

void TestQgsMapRendereCache::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsMapRendereCache::init()
{
}

void TestQgsMapRendereCache::cleanup()
{
}

void TestQgsMapRendereCache::testCache()
{
  QgsMapRendererCache cache;
  QImage imgRed( 100, 100, QImage::Format::Format_ARGB32_Premultiplied );
  imgRed.fill( Qt::red );
  const QString imgRedKey( "red" );

  QImage imgBlue( 50, 50, QImage::Format::Format_ARGB32_Premultiplied );
  imgBlue.fill( Qt::blue );
  const QString imgBlueKey( "blue" );

  /* the cache images are same as inserted */
  QgsRectangle extent1( 0, 0, 100, 200 );
  QgsMapToPixel mtp1( 1, 50, 50, 100, 100, 0.0 );
  cache.updateParameters( extent1, mtp1 );

  QVERIFY( !cache.hasCacheImage( imgBlueKey ) );

  cache.setCacheImage( imgBlueKey, imgBlue );
  QVERIFY( !cache.hasCacheImage( imgRedKey ) );
  QVERIFY( cache.hasCacheImage( imgBlueKey ) );
  QVERIFY( cache.hasAnyCacheImage( imgBlueKey ) );

  cache.setCacheImage( imgRedKey, imgRed );
  QVERIFY( cache.hasCacheImage( imgRedKey ) );
  QVERIFY( cache.hasCacheImage( imgBlueKey ) );
  QVERIFY( cache.hasAnyCacheImage( imgBlueKey ) );
  QVERIFY( cache.hasAnyCacheImage( imgRedKey ) );

  {
    QImage img = cache.cacheImage( imgBlueKey );
    QVERIFY( img.size() == imgBlue.size() );
    QVERIFY( img.pixelColor( 10, 20 ) == Qt::blue );

    QImage transformed = cache.transformedCacheImage( imgBlueKey, mtp1 );
    QVERIFY( transformed.size() == imgBlue.size() );
    QVERIFY( img.pixelColor( 10, 20 ) == Qt::blue );
  }

  /* the cache images are transformed */
  QgsRectangle extent2( 20, 0, 120, 200 );
  QgsMapToPixel mtp2( 1, 70, 50, 100, 100, 0.0 ); // move by 20
  cache.updateParameters( extent2, mtp2 );
  QVERIFY( !cache.hasCacheImage( imgRedKey ) );
  QVERIFY( !cache.hasCacheImage( imgBlueKey ) );
  QVERIFY( cache.hasAnyCacheImage( imgBlueKey ) );
  QVERIFY( cache.hasAnyCacheImage( imgRedKey ) );

  QImage transformed = cache.transformedCacheImage( imgBlueKey, mtp2 );
  QVERIFY( transformed.pixelColor( 30, 20 ) == Qt::transparent );
  QVERIFY( transformed.pixelColor( 10, 20 ) == Qt::blue );

  /* we can replace the cache image */
  cache.setCacheImage( imgBlueKey, transformed );
  QVERIFY( cache.hasCacheImage( imgBlueKey ) );
  QVERIFY( cache.hasAnyCacheImage( imgBlueKey ) );

  /* clear the cache */
  cache.clearCacheImage( imgBlueKey );
  QVERIFY( !cache.hasCacheImage( imgBlueKey ) );
  QVERIFY( !cache.hasAnyCacheImage( imgBlueKey ) );

  cache.clear();
  QVERIFY( !cache.hasCacheImage( imgRedKey ) );
  QVERIFY( !cache.hasAnyCacheImage( imgRedKey ) );
}


QGSTEST_MAIN( TestQgsMapRendereCache )
#include "testqgsmaprenderercache.moc"
