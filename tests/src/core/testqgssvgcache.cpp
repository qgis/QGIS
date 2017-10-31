/***************************************************************************
     testqgssvgcache.cpp
     --------------------
    Date                 : October 2017
    Copyright            : (C) 2017 by Nyall Dawson
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
#include <QObject>
#include <QString>
#include <QStringList>
#include <QApplication>
#include <QFileInfo>
#include <QDir>
#include <QDesktopServices>
#include <QPicture>
#include <QPainter>
#include <QtConcurrent>
#include "qgssvgcache.h"

/**
 * \ingroup UnitTests
 * This is a unit test for QgsSvgCache.
 */
class TestQgsSvgCache : public QObject
{
    Q_OBJECT

  public:
    TestQgsSvgCache() = default;

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() {} // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.
    void fillCache();
    void threadSafePicture();
    void threadSafeImage();

};


void TestQgsSvgCache::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
}

void TestQgsSvgCache::cleanupTestCase()
{
}

void TestQgsSvgCache::fillCache()
{
  QgsSvgCache cache;
  // flood cache to fill it
  QString svgPath = TEST_DATA_DIR + QStringLiteral( "/sample_svg.svg" );
  bool fitInCache = false;

  // we loop forever, continually increasing the size of the requested
  // svg render. The continually changing image size should quickly fill
  // the svg cache size, forcing use of non-cached images.
  // We break after hitting a certain threshold of non-cached images,
  // (after testing that the result is non-null, i.e. rendered on demand,
  // not from cache).
  int uncached = 0;
  for ( double size = 1000; uncached < 10; size += 100 )
  {
    QImage image = cache.svgAsImage( svgPath, size, QColor( 255, 0, 0 ), QColor( 0, 255, 0 ), 1, 1, fitInCache );
    QVERIFY( !image.isNull() );
    if ( !fitInCache )
      uncached++;
  }
}

struct RenderPictureWrapper
{
  QgsSvgCache &cache;
  QString svgPath;
  double size = 100;
  explicit RenderPictureWrapper( QgsSvgCache &cache, const QString &svgPath )
    : cache( cache )
    , svgPath( svgPath )
  {}
  void operator()( int )
  {
    QPicture pic = cache.svgAsPicture( svgPath, size, QColor( 255, 0, 0 ), QColor( 0, 255, 0 ), 1, 1, true );
    QSize imageSize = pic.boundingRect().size();
    QImage image( imageSize, QImage::Format_ARGB32_Premultiplied );
    image.fill( 0 ); // transparent background
    QPainter p( &image );
    p.drawPicture( 0, 0, pic );
  }
};

void TestQgsSvgCache::threadSafePicture()
{
  // QPicture playback is NOT thread safe with implicitly shared copies - this
  // unit test checks that concurrent drawing of svg as QPicture from QgsSvgCache
  // returns a detached copy which is safe to use across threads

  // refs:
  // https://issues.qgis.org/issues/17077
  // https://issues.qgis.org/issues/17089

  QgsSvgCache cache;
  QString svgPath = TEST_DATA_DIR + QStringLiteral( "/sample_svg.svg" );

  // smash picture rendering over multiple threads
  QVector< int > list;
  list.resize( 100 );
  QtConcurrent::blockingMap( list, RenderPictureWrapper( cache, svgPath ) );
}


struct RenderImageWrapper
{
  QgsSvgCache &cache;
  QString svgPath;
  double size = 100;
  explicit RenderImageWrapper( QgsSvgCache &cache, const QString &svgPath )
    : cache( cache )
    , svgPath( svgPath )
  {}
  void operator()( int )
  {
    bool fitsInCache = false;
    QImage cachedImage = cache.svgAsImage( svgPath, size, QColor( 255, 0, 0 ), QColor( 0, 255, 0 ), 1, 1, fitsInCache );
    QImage image( cachedImage.size(), QImage::Format_ARGB32_Premultiplied );
    image.fill( 0 ); // transparent background
    QPainter p( &image );
    p.drawImage( 0, 0, cachedImage );
  }
};

void TestQgsSvgCache::threadSafeImage()
{
  // This unit test checks that concurrent rendering of svg as QImage from QgsSvgCache
  // works without issues across threads

  QgsSvgCache cache;
  QString svgPath = TEST_DATA_DIR + QStringLiteral( "/sample_svg.svg" );

  // smash image rendering over multiple threads
  QVector< int > list;
  list.resize( 100 );
  QtConcurrent::blockingMap( list, RenderImageWrapper( cache, svgPath ) );
}

QGSTEST_MAIN( TestQgsSvgCache )
#include "testqgssvgcache.moc"
