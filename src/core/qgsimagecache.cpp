/***************************************************************************
                         qgsimagecache.cpp
                         -----------------
    begin                : December 2018
    copyright            : (C) 2018 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsimagecache.h"

#include "qgis.h"
#include "qgsimageoperation.h"
#include "qgslogger.h"
#include "qgsnetworkaccessmanager.h"
#include "qgsmessagelog.h"
#include "qgsnetworkcontentfetchertask.h"
#include "qgssettings.h"

#include <QApplication>
#include <QCoreApplication>
#include <QCursor>
#include <QDomDocument>
#include <QDomElement>
#include <QFile>
#include <QImage>
#include <QPainter>
#include <QPicture>
#include <QFileInfo>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QBuffer>
#include <QImageReader>
#include <QSvgRenderer>
#include <QTemporaryDir>
#include <QUuid>

///@cond PRIVATE

QgsImageCacheEntry::QgsImageCacheEntry( const QString &path, QSize size, const bool keepAspectRatio, const double opacity, double dpi, int frameNumber )
  : QgsAbstractContentCacheEntry( path )
  , size( size )
  , keepAspectRatio( keepAspectRatio )
  , opacity( opacity )
  , targetDpi( dpi )
  , frameNumber( frameNumber )
{
}

bool QgsImageCacheEntry::isEqual( const QgsAbstractContentCacheEntry *other ) const
{
  const QgsImageCacheEntry *otherImage = dynamic_cast< const QgsImageCacheEntry * >( other );
  // cheapest checks first!
  if ( !otherImage
       || otherImage->keepAspectRatio != keepAspectRatio
       || otherImage->frameNumber != frameNumber
       || otherImage->size != size
       || ( !size.isValid() && otherImage->targetDpi != targetDpi )
       || otherImage->opacity != opacity
       || otherImage->path != path )
    return false;

  return true;
}

int QgsImageCacheEntry::dataSize() const
{
  int size = 0;
  if ( !image.isNull() )
  {
    size += image.sizeInBytes();
  }
  return size;
}

void QgsImageCacheEntry::dump() const
{
  QgsDebugMsgLevel( QStringLiteral( "path: %1, size %2x%3" ).arg( path ).arg( size.width() ).arg( size.height() ), 3 );
}

///@endcond

QgsImageCache::QgsImageCache( QObject *parent )
  : QgsAbstractContentCache< QgsImageCacheEntry >( parent, QObject::tr( "Image" ) )
{
  mTemporaryDir.reset( new QTemporaryDir() );

  const int bytes = QgsSettings().value( QStringLiteral( "/qgis/maxImageCacheSize" ), 0 ).toInt();
  if ( bytes > 0 )
  {
    mMaxCacheSize = bytes;
  }
  else
  {
    const int sysMemory = QgsApplication::systemMemorySizeMb();
    if ( sysMemory > 0 )
    {
      if ( sysMemory >= 32000 ) // 32 gb RAM (or more) = 500mb cache size
        mMaxCacheSize = 500000000;
      else if ( sysMemory >= 16000 ) // 16 gb RAM = 250mb cache size
        mMaxCacheSize = 250000000;
      else
        mMaxCacheSize = 104857600; // otherwise default to 100mb cache size
    }
  }

  mMissingSvg = QStringLiteral( "<svg width='10' height='10'><text x='5' y='10' font-size='10' text-anchor='middle'>?</text></svg>" ).toLatin1();

  const QString downloadingSvgPath = QgsApplication::defaultThemePath() + QStringLiteral( "downloading_svg.svg" );
  if ( QFile::exists( downloadingSvgPath ) )
  {
    QFile file( downloadingSvgPath );
    if ( file.open( QIODevice::ReadOnly ) )
    {
      mFetchingSvg = file.readAll();
    }
  }

  if ( mFetchingSvg.isEmpty() )
  {
    mFetchingSvg = QStringLiteral( "<svg width='10' height='10'><text x='5' y='10' font-size='10' text-anchor='middle'>?</text></svg>" ).toLatin1();
  }

  connect( this, &QgsAbstractContentCacheBase::remoteContentFetched, this, &QgsImageCache::remoteImageFetched );
}

QgsImageCache::~QgsImageCache() = default;

QImage QgsImageCache::pathAsImage( const QString &f, const QSize size, const bool keepAspectRatio, const double opacity, bool &fitsInCache, bool blocking, double targetDpi, int frameNumber, bool *isMissing )
{
  int totalFrameCount = -1;
  int nextFrameDelayMs = 0;
  return pathAsImagePrivate( f, size, keepAspectRatio, opacity, fitsInCache, blocking, targetDpi, frameNumber, isMissing, totalFrameCount, nextFrameDelayMs );
}

QImage QgsImageCache::pathAsImagePrivate( const QString &f, const QSize size, const bool keepAspectRatio, const double opacity, bool &fitsInCache, bool blocking, double targetDpi, int frameNumber, bool *isMissing, int &totalFrameCount, int &nextFrameDelayMs )
{
  QString file = f.trimmed();
  if ( isMissing )
    *isMissing = true;

  if ( file.isEmpty() )
    return QImage();

  const QMutexLocker locker( &mMutex );

  const auto extractedAnimationIt = mExtractedAnimationPaths.constFind( file );
  if ( extractedAnimationIt != mExtractedAnimationPaths.constEnd() )
  {
    file = QDir( extractedAnimationIt.value() ).filePath( QStringLiteral( "frame_%1.png" ).arg( frameNumber ) );
    frameNumber = -1;
  }

  fitsInCache = true;

  QgsImageCacheEntry *currentEntry = findExistingEntry( new QgsImageCacheEntry( file, size, keepAspectRatio, opacity, targetDpi, frameNumber ) );

  QImage result;

  //if current entry image is null: create the image
  // checks to see if image will fit into cache
  //update stats for memory usage
  if ( currentEntry->image.isNull() )
  {
    long cachedDataSize = 0;
    bool isBroken = false;
    result = renderImage( file, size, keepAspectRatio, opacity, targetDpi, frameNumber, isBroken, totalFrameCount, nextFrameDelayMs, blocking );
    cachedDataSize += result.sizeInBytes();
    if ( cachedDataSize > mMaxCacheSize / 2 )
    {
      fitsInCache = false;
      currentEntry->image = QImage();
    }
    else
    {
      mTotalSize += result.sizeInBytes();
      currentEntry->image = result;
      currentEntry->totalFrameCount = totalFrameCount;
      currentEntry->nextFrameDelay = nextFrameDelayMs;
    }

    if ( isMissing )
      *isMissing = isBroken;
    currentEntry->isMissingImage = isBroken;

    trimToMaximumSize();
  }
  else
  {
    result = currentEntry->image;
    totalFrameCount = currentEntry->totalFrameCount;
    nextFrameDelayMs = currentEntry->nextFrameDelay;
    if ( isMissing )
      *isMissing = currentEntry->isMissingImage;
  }

  return result;
}

QSize QgsImageCache::originalSize( const QString &path, bool blocking ) const
{
  if ( path.isEmpty() )
    return QSize();

  // direct read if path is a file -- maybe more efficient than going the bytearray route? (untested!)
  if ( !path.startsWith( QLatin1String( "base64:" ) ) && QFile::exists( path ) )
  {
    const QImageReader reader( path );
    if ( reader.size().isValid() )
      return reader.size();
    else
      return QImage( path ).size();
  }
  else
  {
    QByteArray ba = getContent( path, QByteArray( "broken" ), QByteArray( "fetching" ), blocking );

    if ( ba != "broken" && ba != "fetching" )
    {
      QBuffer buffer( &ba );
      buffer.open( QIODevice::ReadOnly );

      QImageReader reader( &buffer );
      // if QImageReader::size works, then it's more efficient as it doesn't
      // read the whole image (see Qt docs)
      const QSize s = reader.size();
      if ( s.isValid() )
        return s;
      const QImage im = reader.read();
      return im.isNull() ? QSize() : im.size();
    }
  }
  return QSize();
}

int QgsImageCache::totalFrameCount( const QString &path, bool blocking )
{
  const QString file = path.trimmed();

  if ( file.isEmpty() )
    return -1;

  const QMutexLocker locker( &mMutex );

  auto it = mTotalFrameCounts.find( path );
  if ( it != mTotalFrameCounts.end() )
    return it.value(); // already prepared

  int res = -1;
  int nextFrameDelayMs = 0;
  bool fitsInCache = false;
  bool isMissing = false;
  ( void )pathAsImagePrivate( file, QSize(), true, 1.0, fitsInCache, blocking, 96, 0, &isMissing, res, nextFrameDelayMs );

  return res;
}

int QgsImageCache::nextFrameDelay( const QString &path, int currentFrame, bool blocking )
{
  const QString file = path.trimmed();

  if ( file.isEmpty() )
    return -1;

  const QMutexLocker locker( &mMutex );

  auto it = mImageDelays.find( path );
  if ( it != mImageDelays.end() )
    return it.value().value( currentFrame ); // already prepared

  int frameCount = -1;
  int nextFrameDelayMs = 0;
  bool fitsInCache = false;
  bool isMissing = false;
  const QImage res = pathAsImagePrivate( file, QSize(), true, 1.0, fitsInCache, blocking, 96, currentFrame, &isMissing, frameCount, nextFrameDelayMs );

  return nextFrameDelayMs <= 0 || res.isNull() ? -1 : nextFrameDelayMs;
}

void QgsImageCache::prepareAnimation( const QString &path )
{
  const QMutexLocker locker( &mMutex );

  auto it = mExtractedAnimationPaths.find( path );
  if ( it != mExtractedAnimationPaths.end() )
    return; // already prepared

  QString filePath;
  std::unique_ptr< QImageReader > reader;
  std::unique_ptr< QBuffer > buffer;

  if ( !path.startsWith( QLatin1String( "base64:" ) ) && QFile::exists( path ) )
  {
    const QString basePart = QFileInfo( path ).baseName();
    int id = 1;
    filePath = mTemporaryDir->filePath( QStringLiteral( "%1_%2" ).arg( basePart ).arg( id ) );
    while ( QFile::exists( filePath ) )
      filePath = mTemporaryDir->filePath( QStringLiteral( "%1_%2" ).arg( basePart ).arg( ++id ) );

    reader = std::make_unique< QImageReader >( path );
  }
  else
  {
    QByteArray ba = getContent( path, QByteArray( "broken" ), QByteArray( "fetching" ), false );
    if ( ba == "broken" || ba == "fetching" )
    {
      return;
    }
    else
    {
      const QString path = QUuid::createUuid().toString( QUuid::WithoutBraces );
      filePath = mTemporaryDir->filePath( path );

      buffer = std::make_unique< QBuffer >( &ba );
      buffer->open( QIODevice::ReadOnly );
      reader = std::make_unique< QImageReader> ( buffer.get() );
    }
  }

  QDir().mkpath( filePath );
  mExtractedAnimationPaths.insert( path, filePath );

  const QDir frameDirectory( filePath );
  // extract all the frames to separate images

  reader->setAutoTransform( true );
  int frameNumber = 0;
  while ( true )
  {
    const QImage frame = reader->read();
    if ( frame.isNull() )
      break;

    mImageDelays[ path ].append( reader->nextImageDelay() );

    const QString framePath = frameDirectory.filePath( QStringLiteral( "frame_%1.png" ).arg( frameNumber++ ) );
    frame.save( framePath, "PNG" );
  }

  mTotalFrameCounts.insert( path, frameNumber );
}

QImage QgsImageCache::renderImage( const QString &path, QSize size, const bool keepAspectRatio, const double opacity, double targetDpi, int frameNumber, bool &isBroken, int &totalFrameCount, int &nextFrameDelayMs, bool blocking ) const
{
  QImage im;
  isBroken = false;

  // direct read if path is a file -- maybe more efficient than going the bytearray route? (untested!)
  if ( !path.startsWith( QLatin1String( "base64:" ) ) && QFile::exists( path ) )
  {
    QImageReader reader( path );
    reader.setAutoTransform( true );

    if ( reader.format() == "pdf" )
    {
      if ( !size.isEmpty() )
      {
        // special handling for this format -- we need to pass the desired target size onto the image reader
        // so that it can correctly render the (vector) pdf content at the desired dpi. Otherwise it returns
        // a very low resolution image (the driver assumes points == pixels!)
        // For other image formats, we read the original image size only and defer resampling to later in this
        // function. That gives us more control over the resampling method used.
        reader.setScaledSize( size );
      }
      else
      {
        // driver assumes points == pixels, so driver image size is reported assuming 72 dpi.
        const QSize sizeAt72Dpi = reader.size();
        const QSize sizeAtTargetDpi = sizeAt72Dpi * targetDpi / 72;
        reader.setScaledSize( sizeAtTargetDpi );
      }
    }

    totalFrameCount = reader.imageCount();

    if ( frameNumber == -1 )
    {
      im = reader.read();
    }
    else
    {
      im = getFrameFromReader( reader, frameNumber );
    }
    nextFrameDelayMs = reader.nextImageDelay();
  }
  else
  {
    QByteArray ba = getContent( path, QByteArray( "broken" ), QByteArray( "fetching" ), blocking );

    if ( ba == "broken" )
    {
      isBroken = true;

      // if the size parameter is not valid, skip drawing of missing image symbol
      if ( !size.isValid() )
        return im;

      // if image size is set to respect aspect ratio, correct for broken image aspect ratio
      if ( size.width() == 0 )
        size.setWidth( size.height() );
      if ( size.height() == 0 )
        size.setHeight( size.width() );
      // render "broken" svg
      im = QImage( size, QImage::Format_ARGB32_Premultiplied );
      im.fill( 0 ); // transparent background

      QPainter p( &im );
      QSvgRenderer r( mMissingSvg );

      QSizeF s( r.viewBox().size() );
      s.scale( size.width(), size.height(), Qt::KeepAspectRatio );
      const QRectF rect( ( size.width() - s.width() ) / 2, ( size.height() - s.height() ) / 2, s.width(), s.height() );
      r.render( &p, rect );
    }
    else if ( ba == "fetching" )
    {
      // if image size is set to respect aspect ratio, correct for broken image aspect ratio
      if ( size.width() == 0 )
        size.setWidth( size.height() );
      if ( size.height() == 0 )
        size.setHeight( size.width() );

      // render "fetching" svg
      im = QImage( size, QImage::Format_ARGB32_Premultiplied );
      im.fill( 0 ); // transparent background

      QPainter p( &im );
      QSvgRenderer r( mFetchingSvg );

      QSizeF s( r.viewBox().size() );
      s.scale( size.width(), size.height(), Qt::KeepAspectRatio );
      const QRectF rect( ( size.width() - s.width() ) / 2, ( size.height() - s.height() ) / 2, s.width(), s.height() );
      r.render( &p, rect );
    }
    else
    {
      QBuffer buffer( &ba );
      buffer.open( QIODevice::ReadOnly );

      QImageReader reader( &buffer );
      reader.setAutoTransform( true );

      if ( reader.format() == "pdf" )
      {
        if ( !size.isEmpty() )
        {
          // special handling for this format -- we need to pass the desired target size onto the image reader
          // so that it can correctly render the (vector) pdf content at the desired dpi. Otherwise it returns
          // a very low resolution image (the driver assumes points == pixels!)
          // For other image formats, we read the original image size only and defer resampling to later in this
          // function. That gives us more control over the resampling method used.
          reader.setScaledSize( size );
        }
        else
        {
          // driver assumes points == pixels, so driver image size is reported assuming 72 dpi.
          const QSize sizeAt72Dpi = reader.size();
          const QSize sizeAtTargetDpi = sizeAt72Dpi * targetDpi / 72;
          reader.setScaledSize( sizeAtTargetDpi );
        }
      }

      totalFrameCount = reader.imageCount();
      if ( frameNumber == -1 )
      {
        im = reader.read();
      }
      else
      {
        im = getFrameFromReader( reader, frameNumber );
      }
      nextFrameDelayMs = reader.nextImageDelay();
    }
  }

  if ( !im.hasAlphaChannel() )
    im = im.convertToFormat( QImage::Format_ARGB32 );

  if ( opacity < 1.0 )
    QgsImageOperation::multiplyOpacity( im, opacity );

  // render image at desired size -- null size means original size
  if ( !size.isValid() || size.isNull() || im.size() == size )
    return im;
  // when original aspect ratio is respected and provided height value is 0, automatically compute height
  else if ( keepAspectRatio && size.height() == 0 )
    return im.scaledToWidth( size.width(), Qt::SmoothTransformation );
  // when original aspect ratio is respected and provided width value is 0, automatically compute width
  else if ( keepAspectRatio && size.width() == 0 )
    return im.scaledToHeight( size.height(), Qt::SmoothTransformation );
  else
    return im.scaled( size, keepAspectRatio ? Qt::KeepAspectRatio : Qt::IgnoreAspectRatio, Qt::SmoothTransformation );
}

QImage QgsImageCache::getFrameFromReader( QImageReader &reader, int frameNumber )
{
  if ( reader.jumpToImage( frameNumber ) )
    return reader.read();

  // couldn't seek directly, may require iteration through previous frames
  for ( int frame = 0; frame < frameNumber; ++frame )
  {
    if ( reader.read().isNull() )
      return QImage();
  }
  return reader.read();
}
