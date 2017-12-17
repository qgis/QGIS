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
#include <QElapsedTimer>
#include "qgssvgcache.h"
#include "qgsmultirenderchecker.h"
#include "qgsapplication.h"

/**
 * \ingroup UnitTests
 * This is a unit test for QgsSvgCache.
 */
class TestQgsSvgCache : public QObject
{
    Q_OBJECT

  private:

    QString mReport;

    bool imageCheck( const QString &testName, QImage &image, int mismatchCount );

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() {} // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.
    void fillCache();
    void threadSafePicture();
    void threadSafeImage();
    void changeImage(); //check that cache is updated if svg source file changes

};


void TestQgsSvgCache::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
  mReport += "<h1>QgsSvgCache Tests</h1>\n";
}

void TestQgsSvgCache::cleanupTestCase()
{
  QgsApplication::exitQgis();

  QString myReportFile = QDir::tempPath() + "/qgistest.html";
  QFile myFile( myReportFile );
  if ( myFile.open( QIODevice::WriteOnly | QIODevice::Append ) )
  {
    QTextStream myQTextStream( &myFile );
    myQTextStream << mReport;
    myFile.close();
    //QDesktopServices::openUrl( "file:///" + myReportFile );
  }
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

void TestQgsSvgCache::changeImage()
{
  bool inCache;
  QgsSvgCache cache;
  // no minimum time between checks
  cache.mFileModifiedCheckTimeout = 0;

  //copy an image to the temp folder
  QString tempImagePath = QDir::tempPath() + "/svg_cache.svg";

  QString originalImage = TEST_DATA_DIR + QStringLiteral( "/test_symbol_svg.svg" );
  if ( QFileInfo::exists( tempImagePath ) )
    QFile::remove( tempImagePath );
  QFile::copy( originalImage, tempImagePath );

  //render it through the cache
  QImage img = cache.svgAsImage( tempImagePath, 200, QColor( 0, 0, 0 ), QColor( 0, 0, 0 ), 1.0,
                                 1.0, inCache );
  QVERIFY( imageCheck( "svgcache_changed_before", img, 30 ) );

  // wait a second so that modified time is different
  QElapsedTimer t;
  t.start();
  while ( !t.hasExpired( 1000 ) )
  {}

  //replace the image in the temp folder
  QString newImage = TEST_DATA_DIR + QStringLiteral( "/test_symbol_svg2.svg" );
  QFile::remove( tempImagePath );
  QFile::copy( newImage, tempImagePath );

  //re-render it
  img = cache.svgAsImage( tempImagePath, 200, QColor( 0, 0, 0 ), QColor( 0, 0, 0 ), 1.0,
                          1.0, inCache );
  QVERIFY( imageCheck( "svgcache_changed_after", img, 30 ) );

  // repeat, with minimum time between checks
  QgsSvgCache cache2;
  QFile::remove( tempImagePath );
  QFile::copy( originalImage, tempImagePath );
  img = cache2.svgAsImage( tempImagePath, 200, QColor( 0, 0, 0 ), QColor( 0, 0, 0 ), 1.0,
                           1.0, inCache );
  QVERIFY( imageCheck( "svgcache_changed_before", img, 30 ) );

  // wait a second so that modified time is different
  t.restart();
  while ( !t.hasExpired( 1000 ) )
  {}

  //replace the image in the temp folder
  QFile::remove( tempImagePath );
  QFile::copy( newImage, tempImagePath );

  //re-render it - not enough time has elapsed between checks, so file modification time will NOT be rechecked and
  // existing cached image should be used
  img = cache2.svgAsImage( tempImagePath, 200, QColor( 0, 0, 0 ), QColor( 0, 0, 0 ), 1.0,
                           1.0, inCache );
  QVERIFY( imageCheck( "svgcache_changed_before", img, 30 ) );
}

bool TestQgsSvgCache::imageCheck( const QString &testName, QImage &image, int mismatchCount )
{
  //draw background
  QImage imageWithBackground( image.width(), image.height(), QImage::Format_RGB32 );
  QgsRenderChecker::drawBackground( &imageWithBackground );
  QPainter painter( &imageWithBackground );
  painter.drawImage( 0, 0, image );
  painter.end();

  mReport += "<h2>" + testName + "</h2>\n";
  QString tempDir = QDir::tempPath() + '/';
  QString fileName = tempDir + testName + ".png";
  imageWithBackground.save( fileName, "PNG" );
  QgsRenderChecker checker;
  checker.setControlName( "expected_" + testName );
  checker.setRenderedImage( fileName );
  checker.setColorTolerance( 2 );
  bool resultFlag = checker.compareImages( testName, mismatchCount );
  mReport += checker.report();
  return resultFlag;
}
QGSTEST_MAIN( TestQgsSvgCache )
#include "testqgssvgcache.moc"
