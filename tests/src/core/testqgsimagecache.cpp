/***************************************************************************
     testqgsimagecache.cpp
     --------------------
    Date                 : Decemeber 2018
    Copyright            : (C) 2018 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsapplication.h"
#include "qgsimagecache.h"
#include "qgsmessagelog.h"
#include "qgsrenderchecker.h"
#include "qgssettings.h"
#include "qgstest.h"

#include <QApplication>
#include <QDesktopServices>
#include <QDir>
#include <QElapsedTimer>
#include <QFileInfo>
#include <QObject>
#include <QPainter>
#include <QPicture>
#include <QSignalSpy>
#include <QString>
#include <QStringList>
#include <QtConcurrent>

/**
 * \ingroup UnitTests
 * This is a unit test for QgsImageCache.
 */
class TestQgsImageCache : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsImageCache()
      : QgsTest( u"QgsImageCache Tests"_s ) {}

  private:
    bool imageCheck( const QString &testName, const QImage &image, int mismatchCount );

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void fillCache();
    void threadSafeImage();
    void broken();
    void changeImage(); // check that cache is updated if image source file changes
    void size();        // check various size-specific handling
    void opacity();     // check non-opaque image rendering
    void base64();
    void empty();
    void dpi();
    void cachesize();
    void frameCount();
    void nextFrameDelay();
    void imageFrames();
    void preseedAnimation();
    void cmykImage();
    void htmlDataUrl();
    void invalidateCacheEntry();
};


void TestQgsImageCache::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
}

void TestQgsImageCache::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsImageCache::fillCache()
{
  QgsImageCache cache;
  // flood cache to fill it
  const QString imagePath = TEST_DATA_DIR + u"/sample_image.png"_s;
  bool fitInCache = false;

  // we loop forever, continually increasing the size of the requested
  // image render. The continually changing image size should quickly fill
  // the image cache size, forcing use of non-cached images.
  // We break after hitting a certain threshold of non-cached images,
  // (after testing that the result is non-null, i.e. rendered on demand,
  // not from cache).
  int uncached = 0;
  for ( int size = 1000; uncached < 10; size += 100 )
  {
    const QImage image = cache.pathAsImage( imagePath, QSize( size, size ), true, 1.0, fitInCache );
    QVERIFY( !image.isNull() );
    if ( !fitInCache )
      uncached++;
  }
}

struct RenderImageWrapper
{
    QgsImageCache &cache;
    QString imagePath;
    QSize size = QSize { 100, 100 };
    explicit RenderImageWrapper( QgsImageCache &cache, const QString &imagePath )
      : cache( cache )
      , imagePath( imagePath )
    {}
    void operator()( int )
    {
      bool fitInCache = false;
      const QImage im = cache.pathAsImage( imagePath, size, true, 1.0, fitInCache );
      const QSize imageSize = im.size();
      QImage image( imageSize, QImage::Format_ARGB32_Premultiplied );
      image.fill( 0 ); // transparent background
      QPainter p( &image );
      p.drawImage( 0, 0, im );
    }
};

void TestQgsImageCache::threadSafeImage()
{
  // This unit test checks that concurrent rendering of paths as QImage from QgsImageCache
  // works without issues across threads

  // test deadlocks on 5.15 - see https://bugreports.qt.io/browse/QTBUG-84857
  // this is supposed to be fixed in 5.15.1, but I can still reproduce in Fedora on 5.15.5..!

  QgsImageCache cache;
  const QString imagePath = TEST_DATA_DIR + u"/sample_image.png"_s;

  // smash picture rendering over multiple threads
  QVector<int> list;
  list.resize( 100 );
  QtConcurrent::blockingMap( list, RenderImageWrapper( cache, imagePath ) );
}

void TestQgsImageCache::broken()
{
  QgsImageCache cache;
  bool inCache = false;
  bool missingImage = false;
  cache.pathAsImage( u"bbbbbbb"_s, QSize( 200, 200 ), true, 1.0, inCache, false, 96, -1, &missingImage );
  QVERIFY( missingImage );
  cache.pathAsImage( u"bbbbbbb"_s, QSize( 200, 200 ), true, 1.0, inCache, false, 96, -1, &missingImage );
  QVERIFY( missingImage );
  const QString originalImage = TEST_DATA_DIR + u"/sample_image.png"_s;
  cache.pathAsImage( originalImage, QSize( 200, 200 ), true, 1.0, inCache, false, 96, -1, &missingImage );
  QVERIFY( !missingImage );
}

void TestQgsImageCache::changeImage()
{
  bool inCache;
  QgsImageCache cache;
  // no minimum time between checks
  cache.mFileModifiedCheckTimeout = 0;

  //copy an image to the temp folder
  const QString tempImagePath = QDir::tempPath() + "/test_sample_image.png";

  const QString originalImage = TEST_DATA_DIR + u"/sample_image.png"_s;
  if ( QFileInfo::exists( tempImagePath ) )
    QFile::remove( tempImagePath );
  QFile::copy( originalImage, tempImagePath );

  //render it through the cache
  QImage img = cache.pathAsImage( tempImagePath, QSize( 200, 200 ), true, 1.0, inCache );
  QVERIFY( imageCheck( "imagecache_changed_before", img, 30 ) );
  QCOMPARE( cache.originalSize( tempImagePath ).width(), 511 );
  QCOMPARE( cache.originalSize( tempImagePath ).height(), 800 );

  // wait a second so that modified time is different
  QElapsedTimer t;
  t.start();
  while ( !t.hasExpired( 1000 ) )
  {
  }

  //replace the image in the temp folder
  const QString newImage = TEST_DATA_DIR + u"/sample_image2.png"_s;
  QFile::remove( tempImagePath );
  QFile::copy( newImage, tempImagePath );

  //re-render it
  img = cache.pathAsImage( tempImagePath, QSize( 200, 200 ), true, 1.0, inCache );
  QVERIFY( imageCheck( "imagecache_changed_after", img, 30 ) );

  // repeat, with minimum time between checks
  QgsImageCache cache2;
  QFile::remove( tempImagePath );
  QFile::copy( originalImage, tempImagePath );
  img = cache2.pathAsImage( tempImagePath, QSize( 200, 200 ), true, 1.0, inCache );
  QVERIFY( imageCheck( "imagecache_changed_before", img, 30 ) );

  // wait a second so that modified time is different
  t.restart();
  while ( !t.hasExpired( 1000 ) )
  {
  }

  //replace the image in the temp folder
  QFile::remove( tempImagePath );
  QFile::copy( newImage, tempImagePath );

  //re-render it - not enough time has elapsed between checks, so file modification time will NOT be rechecked and
  // existing cached image should be used
  img = cache2.pathAsImage( tempImagePath, QSize( 200, 200 ), true, 1.0, inCache );
  QVERIFY( imageCheck( "imagecache_changed_before", img, 30 ) );
}

void TestQgsImageCache::size()
{
  QgsImageCache cache;
  QImage img;
  bool inCache;
  const QString originalImage = TEST_DATA_DIR + u"/sample_image.png"_s;

  // null size should return image using original size
  img = cache.pathAsImage( originalImage, QSize(), true, 1.0, inCache );
  QCOMPARE( img.width(), 511 );
  QCOMPARE( img.height(), 800 );

  // a size with an height set to 0 while keep aspect ratio is true should return an image with automatically computed height
  img = cache.pathAsImage( originalImage, QSize( 100, 0 ), true, 1.0, inCache );
  QCOMPARE( img.width(), 100 );
  QCOMPARE( img.height(), 157 );

  // a size with an width set to 0 while keep aspect ratio is true should return an image with automatically computed width
  img = cache.pathAsImage( originalImage, QSize( 0, 100 ), true, 1.0, inCache );
  QCOMPARE( img.width(), 64 );
  QCOMPARE( img.height(), 100 );

  // broken images should fallback to square aspect ratios, not the originally specified 0 px width or height
  img = cache.pathAsImage( u"broken"_s, QSize( 0, 100 ), true, 1.0, inCache );
  QCOMPARE( img.width(), 100 );
  QCOMPARE( img.height(), 100 );

  img = cache.pathAsImage( u"broken"_s, QSize( 100, 0 ), true, 1.0, inCache );
  QCOMPARE( img.width(), 100 );
  QCOMPARE( img.height(), 100 );

  img = cache.pathAsImage( u"broken"_s, QSize( 0, 100 ), false, 1.0, inCache );
  QCOMPARE( img.width(), 100 );
  QCOMPARE( img.height(), 100 );

  img = cache.pathAsImage( u"broken"_s, QSize( 100, 0 ), false, 1.0, inCache );
  QCOMPARE( img.width(), 100 );
  QCOMPARE( img.height(), 100 );
}

void TestQgsImageCache::opacity()
{
  QgsImageCache cache;
  QImage img;
  bool inCache;
  const QString originalImage = TEST_DATA_DIR + u"/sample_image.png"_s;

  // null size should return image using original size
  img = cache.pathAsImage( originalImage, QSize( 200, 200 ), true, 0.5, inCache );
  QVERIFY( imageCheck( u"opaque_image"_s, img, 30 ) );
}

void TestQgsImageCache::base64()
{
  // test rendering images encoded in base 64
  QgsImageCache cache;
  bool inCache = false;

  // invalid base64 strings
  QImage img = cache.pathAsImage( u"base64:"_s, QSize( 200, 200 ), true, 1.0, inCache );
  QVERIFY( img.isNull() );
  QVERIFY( !cache.originalSize( u"base64:"_s ).isValid() );

  img = cache.pathAsImage( u"base64:zzzzzzzzzzzzzzzzzzzz"_s, QSize( 200, 200 ), true, 1.0, inCache );
  QVERIFY( img.isNull() );
  QVERIFY( !cache.originalSize( u"base64:zzzzzzzzzzzzzzz"_s ).isValid() );

  //valid base 64
  img = cache.pathAsImage( u"base64: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"_s, QSize( 200, 200 ), true, 1.0, inCache );
  QVERIFY( imageCheck( u"imagecache_base64"_s, img, 30 ) );
  const QSize size = cache.originalSize( u"base64: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"_s );
  QCOMPARE( size.width(), 60 );
  QCOMPARE( size.height(), 60 );
}

void TestQgsImageCache::empty()
{
  QgsImageCache cache;
  bool inCache = false;

  QImage img = cache.pathAsImage( QString(), QSize( 200, 200 ), true, 1.0, inCache );
  QVERIFY( img.isNull() );

  QVERIFY( !cache.originalSize( QString() ).isValid() );

  img = cache.pathAsImage( u" "_s, QSize( 200, 200 ), true, 1.0, inCache );
  QVERIFY( img.isNull() );

  QVERIFY( !cache.originalSize( u" "_s ).isValid() );
}

void TestQgsImageCache::dpi()
{
  QgsImageCacheEntry entry1( u"my path"_s, QSize(), true, 1, 96, -1 );
  QgsImageCacheEntry entry2( u"my path"_s, QSize(), true, 1, 300, -1 );
  QVERIFY( !entry1.isEqual( &entry2 ) );
  entry2.targetDpi = 96;
  QVERIFY( entry1.isEqual( &entry2 ) );
  entry2.targetDpi = 300;
  // target dpi is ignored if a valid size is set
  entry1.size = QSize( 100, 100 );
  entry2.size = QSize( 100, 100 );
  QVERIFY( entry1.isEqual( &entry2 ) );
}

void TestQgsImageCache::cachesize()
{
  uint cacheSize = 100000000;
  QgsSettings().setValue( u"/qgis/maxImageCacheSize"_s, cacheSize );
  QgsImageCache cache;
  QCOMPARE( cache.mMaxCacheSize, cacheSize );
}

void TestQgsImageCache::frameCount()
{
  QgsImageCache cache;

  // not an animated image
  const QString notAnimatedImage = TEST_DATA_DIR + u"/sample_image.png"_s;

  QCOMPARE( cache.totalFrameCount( notAnimatedImage ), 1 );
  // call twice to test caching
  QCOMPARE( cache.totalFrameCount( notAnimatedImage ), 1 );

  const QString animatedImage = TEST_DATA_DIR + u"/qgis_logo_animated.gif"_s;

  QCOMPARE( cache.totalFrameCount( animatedImage ), 4 );
  // call twice to test caching
  QCOMPARE( cache.totalFrameCount( animatedImage ), 4 );
}

void TestQgsImageCache::nextFrameDelay()
{
  QgsImageCache cache;

  // not an animated image
  const QString notAnimatedImage = TEST_DATA_DIR + u"/sample_image.png"_s;

  QCOMPARE( cache.nextFrameDelay( notAnimatedImage ), -1 );
  // call twice to test caching
  QCOMPARE( cache.nextFrameDelay( notAnimatedImage ), -1 );

  const QString animatedImage = TEST_DATA_DIR + u"/qgis_logo_animated.gif"_s;

  QCOMPARE( cache.nextFrameDelay( animatedImage ), 100 );
  // call twice to test caching
  QCOMPARE( cache.nextFrameDelay( animatedImage ), 100 );

  // different frame
  QCOMPARE( cache.nextFrameDelay( animatedImage, 1 ), 100 );
  QCOMPARE( cache.nextFrameDelay( animatedImage, 1 ), 100 );

  QCOMPARE( cache.nextFrameDelay( animatedImage, 5 ), -1 );
  QCOMPARE( cache.nextFrameDelay( animatedImage, 5 ), -1 );
}

void TestQgsImageCache::imageFrames()
{
  QgsImageCache cache;
  QImage img;
  bool inCache;
  const QString originalImage = TEST_DATA_DIR + u"/qgis_logo_animated.gif"_s;

  // call twice to test caching
  img = cache.pathAsImage( originalImage, QSize(), true, 1.0, inCache, false, 96, 0 );
  QVERIFY( !img.isNull() );
  img = cache.pathAsImage( originalImage, QSize(), true, 1.0, inCache, false, 96, 0 );
  QVERIFY( !img.isNull() );
  QVERIFY( imageCheck( u"imagecache_animation_0"_s, img, 0 ) );

  img = cache.pathAsImage( originalImage, QSize(), true, 1.0, inCache, false, 96, 1 );
  QVERIFY( !img.isNull() );
  img = cache.pathAsImage( originalImage, QSize(), true, 1.0, inCache, false, 96, 1 );
  QVERIFY( !img.isNull() );
  QVERIFY( imageCheck( u"imagecache_animation_1"_s, img, 0 ) );

  img = cache.pathAsImage( originalImage, QSize(), true, 1.0, inCache, false, 96, 2 );
  QVERIFY( !img.isNull() );
  img = cache.pathAsImage( originalImage, QSize(), true, 1.0, inCache, false, 96, 2 );
  QVERIFY( !img.isNull() );
  QVERIFY( imageCheck( u"imagecache_animation_2"_s, img, 0 ) );

  img = cache.pathAsImage( originalImage, QSize(), true, 1.0, inCache, false, 96, 3 );
  QVERIFY( !img.isNull() );
  img = cache.pathAsImage( originalImage, QSize(), true, 1.0, inCache, false, 96, 3 );
  QVERIFY( !img.isNull() );
  QVERIFY( imageCheck( u"imagecache_animation_3"_s, img, 0 ) );

  // invalid frame
  img = cache.pathAsImage( originalImage, QSize(), true, 1.0, inCache, false, 96, 4 );
  QVERIFY( img.isNull() );
}

void TestQgsImageCache::preseedAnimation()
{
  QgsImageCache cache;
  const QString originalImage = TEST_DATA_DIR + u"/qgis_logo_animated.gif"_s;

  const QString tempDir = cache.mTemporaryDir->path();
  QVERIFY( QFile::exists( tempDir ) );

  QStringList entries = QDir( tempDir ).entryList( QDir::Dirs | QDir::Filter::NoDotAndDotDot );
  QVERIFY( entries.isEmpty() );

  cache.prepareAnimation( originalImage );

  entries = QDir( tempDir ).entryList( QDir::Dirs | QDir::Filter::NoDotAndDotDot );
  QCOMPARE( entries.size(), 1 );
  QStringList files = QDir( QDir( tempDir ).filePath( entries.at( 0 ) ) ).entryList( QDir::Files | QDir::Filter::NoDotAndDotDot );
  QCOMPARE( files.size(), 4 );

  // call twice to test caching
  bool inCache = false;
  QImage img = cache.pathAsImage( originalImage, QSize(), true, 1.0, inCache, false, 96, 0 );
  QVERIFY( !img.isNull() );
  img = cache.pathAsImage( originalImage, QSize(), true, 1.0, inCache, false, 96, 0 );
  QVERIFY( !img.isNull() );
  QVERIFY( imageCheck( u"imagecache_animation_0"_s, img, 0 ) );

  img = cache.pathAsImage( originalImage, QSize(), true, 1.0, inCache, false, 96, 1 );
  QVERIFY( !img.isNull() );
  img = cache.pathAsImage( originalImage, QSize(), true, 1.0, inCache, false, 96, 1 );
  QVERIFY( !img.isNull() );
  QVERIFY( imageCheck( u"imagecache_animation_1"_s, img, 0 ) );

  img = cache.pathAsImage( originalImage, QSize(), true, 1.0, inCache, false, 96, 2 );
  QVERIFY( !img.isNull() );
  img = cache.pathAsImage( originalImage, QSize(), true, 1.0, inCache, false, 96, 2 );
  QVERIFY( !img.isNull() );
  QVERIFY( imageCheck( u"imagecache_animation_2"_s, img, 0 ) );

  img = cache.pathAsImage( originalImage, QSize(), true, 1.0, inCache, false, 96, 3 );
  QVERIFY( !img.isNull() );
  img = cache.pathAsImage( originalImage, QSize(), true, 1.0, inCache, false, 96, 3 );
  QVERIFY( !img.isNull() );
  QVERIFY( imageCheck( u"imagecache_animation_3"_s, img, 0 ) );

  // invalid frame
  img = cache.pathAsImage( originalImage, QSize(), true, 1.0, inCache, false, 96, 4 );
  QVERIFY( img.isNull() );
}

bool TestQgsImageCache::imageCheck( const QString &testName, const QImage &image, int mismatchCount )
{
  //draw background
  QImage imageWithBackground( image.width(), image.height(), QImage::Format_RGB32 );
  QgsRenderChecker::drawBackground( &imageWithBackground );
  QPainter painter( &imageWithBackground );
  painter.drawImage( 0, 0, image );
  painter.end();

  const QString tempDir = QDir::tempPath() + '/';
  const QString fileName = tempDir + testName + ".png";
  imageWithBackground.save( fileName, "PNG" );
  QgsRenderChecker checker;
  checker.setControlPathPrefix( u"image_cache"_s );
  checker.setControlName( "expected_" + testName );
  checker.setRenderedImage( fileName );
  checker.setColorTolerance( 2 );
  const bool resultFlag = checker.compareImages( testName, mismatchCount );
  mReport += checker.report();
  return resultFlag;
}

void TestQgsImageCache::cmykImage()
{
  QgsImageCache cache;
  const QString imagePath = TEST_DATA_DIR + u"/cmyk.jpg"_s;
  bool fitInCache;
  const QImage image = cache.pathAsImage( imagePath, QSize( 130, 50 ), true, 1.0, fitInCache );
  QVERIFY( !image.isNull() );
  QCOMPARE( image.size(), QSize( 130, 50 ) );

#if QT_VERSION >= QT_VERSION_CHECK( 6, 8, 0 )
  QCOMPARE( image.format(), QImage::Format_CMYK8888 );
#else
  QCOMPARE( image.format(), QImage::Format_ARGB32 );
#endif
}

void TestQgsImageCache::htmlDataUrl()
{
  QVERIFY( !QgsImageCache::parseBase64DataUrl( QString() ) );
  QVERIFY( !QgsImageCache::parseBase64DataUrl( testDataPath( u"/cmyk.jpg"_s ) ) );
  QVERIFY( !QgsImageCache::isBase64Data( testDataPath( u"/cmyk.jpg"_s ) ) );
  QVERIFY( QgsImageCache::isBase64Data( u"base64:zzzzzzzzzzzzzzz"_s ) );
  QVERIFY( QgsImageCache::isBase64Data( u"data:image/jpeg;base64,SGVsbG8sIFdvcmxkIQ=="_s ) );

  QString mimeType;
  QString base64Data;
  QVERIFY( QgsImageCache::parseBase64DataUrl( u"data:image/jpeg;base64,SGVsbG8sIFdvcmxkIQ=="_s, &mimeType, &base64Data ) );
  QCOMPARE( mimeType, u"image/jpeg"_s );
  QCOMPARE( base64Data, u"SGVsbG8sIFdvcmxkIQ=="_s );

  // "base64" string is optional
  QVERIFY( QgsImageCache::parseBase64DataUrl( u"data:image/jpeg,SGVsbG8sIFdvcmxkIQ=="_s, &mimeType, &base64Data ) );
  QCOMPARE( mimeType, u"image/jpeg"_s );
  QCOMPARE( base64Data, u"SGVsbG8sIFdvcmxkIQ=="_s );

  QgsImageCache cache;
  bool inCache = false;
  const QImage img = cache.pathAsImage( u"data:image/jpeg;base64,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"_s, QSize( 200, 200 ), true, 1.0, inCache );
  QVERIFY( imageCheck( u"imagecache_base64"_s, img, 30 ) );
  const QSize size = cache.originalSize( u"base64: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"_s );
  QCOMPARE( size.width(), 60 );
  QCOMPARE( size.height(), 60 );
}

void TestQgsImageCache::invalidateCacheEntry()
{
  QgsImageCache cache;
  bool inCache = false;

  const QString tempImagePath = QDir::tempPath() + "/test_sample_image.png";
  const QString originalImage = TEST_DATA_DIR + u"/sample_image.png"_s;
  if ( QFileInfo::exists( tempImagePath ) )
    QFile::remove( tempImagePath );
  QFile::copy( originalImage, tempImagePath );

  bool invalidated_initial = cache.invalidateCacheEntry( tempImagePath );
  QVERIFY( !invalidated_initial );

  QImage img = cache.pathAsImage( tempImagePath, QSize( 200, 200 ), true, 1.0, inCache );
  QVERIFY( !img.isNull() );
  QVERIFY( inCache );

  bool invalidated = cache.invalidateCacheEntry( tempImagePath );
  QVERIFY( invalidated );

  bool invalidated_twice = cache.invalidateCacheEntry( tempImagePath );
  QVERIFY( !invalidated_twice );

  img = cache.pathAsImage( tempImagePath, QSize( 200, 200 ), true, 1.0, inCache );
  QVERIFY( !img.isNull() );
  QVERIFY( inCache );
}

QGSTEST_MAIN( TestQgsImageCache )
#include "testqgsimagecache.moc"
