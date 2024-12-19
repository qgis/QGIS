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

class TestQgsMapRendererCache: public QObject
{
    Q_OBJECT

  public:
    TestQgsMapRendererCache();

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init(); // will be called before each testfunction is executed.
    void cleanup(); // will be called after every testfunction.

    void testCache();
};


TestQgsMapRendererCache::TestQgsMapRendererCache() = default;

void TestQgsMapRendererCache::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
}

void TestQgsMapRendererCache::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsMapRendererCache::init()
{
}

void TestQgsMapRendererCache::cleanup()
{
}

void TestQgsMapRendererCache::testCache()
{
  QgsMapRendererCache cache;
  QImage imgRed( 100, 100, QImage::Format::Format_ARGB32_Premultiplied );
  imgRed.fill( Qt::red );
  const QString imgRedKey( "red" );

  QImage imgBlue( 50, 50, QImage::Format::Format_ARGB32_Premultiplied );
  imgBlue.fill( Qt::blue );
  const QString imgBlueKey( "blue" );

  /* the cache images are same as inserted */
  const QgsRectangle extent1( 0, 0, 100, 200 );
  const QgsMapToPixel mtp1( 1, 50, 50, 100, 100, 0.0 );
  cache.updateParameters( extent1, mtp1 );

  QVERIFY( !cache.hasCacheImage( imgBlueKey ) );

  cache.setCacheImage( imgBlueKey, imgBlue );
  QVERIFY( !cache.hasCacheImage( imgRedKey ) );
  QVERIFY( cache.hasCacheImage( imgBlueKey ) );
  QVERIFY( cache.hasAnyCacheImage( imgBlueKey ) );
  QVERIFY( cache.hasAnyCacheImage( imgBlueKey, 0.9, 1.1 ) );

  cache.setCacheImage( imgRedKey, imgRed );
  QVERIFY( cache.hasCacheImage( imgRedKey ) );
  QVERIFY( cache.hasCacheImage( imgBlueKey ) );
  QVERIFY( cache.hasAnyCacheImage( imgBlueKey ) );
  QVERIFY( cache.hasAnyCacheImage( imgRedKey ) );
  QVERIFY( cache.hasAnyCacheImage( imgRedKey, 0.9, 1.1 ) );

  {
    const QImage img = cache.cacheImage( imgBlueKey );
    QVERIFY( img.size() == imgBlue.size() );
    QVERIFY( img.pixelColor( 10, 20 ) == Qt::blue );

    const QImage transformed = cache.transformedCacheImage( imgBlueKey, mtp1 );
    QVERIFY( transformed.size() == imgBlue.size() );
    QVERIFY( img.pixelColor( 10, 20 ) == Qt::blue );
  }

  /* the cache images are transformed */
  const QgsRectangle extent2( 20, 0, 120, 200 );
  const QgsMapToPixel mtp2( 1, 70, 50, 100, 100, 0.0 ); // move by 20
  cache.updateParameters( extent2, mtp2 );
  QVERIFY( !cache.hasCacheImage( imgRedKey ) );
  QVERIFY( !cache.hasCacheImage( imgBlueKey ) );
  QVERIFY( cache.hasAnyCacheImage( imgBlueKey ) );
  QVERIFY( cache.hasAnyCacheImage( imgBlueKey, 0.9, 1.1 ) );
  QVERIFY( cache.hasAnyCacheImage( imgRedKey, 0.9, 1.1 ) );

  const QImage transformed = cache.transformedCacheImage( imgBlueKey, mtp2 );
  QVERIFY( transformed.pixelColor( 30, 20 ) == Qt::transparent );
  QVERIFY( transformed.pixelColor( 10, 20 ) == Qt::blue );

  const QgsMapToPixel mtp3( 2, 70, 50, 100, 100, 0.0 ); // scale by 2
  cache.updateParameters( extent2, mtp3 );
  QVERIFY( !cache.hasCacheImage( imgRedKey ) );
  QVERIFY( !cache.hasCacheImage( imgBlueKey ) );
  QVERIFY( cache.hasAnyCacheImage( imgBlueKey ) );
  // outside of range
  QVERIFY( !cache.hasAnyCacheImage( imgBlueKey, 0.9, 1.1 ) );
  QVERIFY( !cache.hasAnyCacheImage( imgRedKey, 0.9, 1.1 ) );
  QVERIFY( cache.hasAnyCacheImage( imgBlueKey, 0.9, 0 ) );
  QVERIFY( cache.hasAnyCacheImage( imgRedKey, 0.9, 0 ) );
  QVERIFY( cache.hasAnyCacheImage( imgBlueKey, 0.9, 2.0 ) );
  QVERIFY( cache.hasAnyCacheImage( imgRedKey, 0.9, 2.0 ) );

  const QgsMapToPixel mtp4( 0.5, 70, 50, 100, 100, 0.0 ); // scale by 0.5
  cache.updateParameters( extent2, mtp4 );
  QVERIFY( !cache.hasCacheImage( imgRedKey ) );
  QVERIFY( !cache.hasCacheImage( imgBlueKey ) );
  QVERIFY( cache.hasAnyCacheImage( imgBlueKey ) );
  // outside of range
  QVERIFY( !cache.hasAnyCacheImage( imgBlueKey, 0.9, 1.1 ) );
  QVERIFY( !cache.hasAnyCacheImage( imgRedKey, 0.9, 1.1 ) );
  QVERIFY( cache.hasAnyCacheImage( imgBlueKey, 0, 1.1 ) );
  QVERIFY( cache.hasAnyCacheImage( imgRedKey, 0, 1.1 ) );
  QVERIFY( cache.hasAnyCacheImage( imgBlueKey, 0.5, 1.1 ) );
  QVERIFY( cache.hasAnyCacheImage( imgRedKey, 0.5, 1.1 ) );

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


QGSTEST_MAIN( TestQgsMapRendererCache )
#include "testqgsmaprenderercache.moc"
