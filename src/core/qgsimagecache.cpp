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

///@cond PRIVATE

QgsImageCacheEntry::QgsImageCacheEntry( const QString &path, QSize size, const bool keepAspectRatio, const double opacity )
  : QgsAbstractContentCacheEntry( path )
  , size( size )
  , keepAspectRatio( keepAspectRatio )
  , opacity( opacity )
{
}

bool QgsImageCacheEntry::isEqual( const QgsAbstractContentCacheEntry *other ) const
{
  const QgsImageCacheEntry *otherImage = dynamic_cast< const QgsImageCacheEntry * >( other );
  // cheapest checks first!
  if ( !otherImage || otherImage->keepAspectRatio != keepAspectRatio || otherImage->size != size || otherImage->path != path || otherImage->opacity != opacity )
    return false;

  return true;
}

int QgsImageCacheEntry::dataSize() const
{
  int size = 0;
  if ( !image.isNull() )
  {
    size += ( image.width() * image.height() * 32 );
  }
  return size;
}

void QgsImageCacheEntry::dump() const
{
  QgsDebugMsg( QStringLiteral( "path: %1, size %2x%3" ).arg( path ).arg( size.width() ).arg( size.height() ) );
}

///@endcond


QgsImageCache::QgsImageCache( QObject *parent )
  : QgsAbstractContentCache< QgsImageCacheEntry >( parent, QObject::tr( "Image" ) )
{
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

QImage QgsImageCache::pathAsImage( const QString &f, const QSize size, const bool keepAspectRatio, const double opacity, bool &fitsInCache, bool blocking )
{
  const QString file = f.trimmed();

  if ( file.isEmpty() )
    return QImage();

  QMutexLocker locker( &mMutex );

  fitsInCache = true;

  QgsImageCacheEntry *currentEntry = findExistingEntry( new QgsImageCacheEntry( file, size, keepAspectRatio, opacity ) );

  QImage result;

  //if current entry image is null: create the image
  // checks to see if image will fit into cache
  //update stats for memory usage
  if ( currentEntry->image.isNull() )
  {
    long cachedDataSize = 0;
    result = renderImage( file, size, keepAspectRatio, opacity, blocking );
    cachedDataSize += result.width() * result.height() * 32;
    if ( cachedDataSize > mMaxCacheSize / 2 )
    {
      fitsInCache = false;
      currentEntry->image = QImage();
    }
    else
    {
      mTotalSize += ( result.width() * result.height() * 32 );
      currentEntry->image = result;
    }
    trimToMaximumSize();
  }
  else
  {
    result = currentEntry->image;
  }

  return result;
}

QSize QgsImageCache::originalSize( const QString &path, bool blocking ) const
{
  if ( path.isEmpty() )
    return QSize();

  // direct read if path is a file -- maybe more efficient than going the bytearray route? (untested!)
  if ( !path.startsWith( QStringLiteral( "base64:" ) ) && QFile::exists( path ) )
  {
    QImageReader reader( path );
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
      QImage im = reader.read();
      return im.isNull() ? QSize() : im.size();
    }
  }
  return QSize();
}

QImage QgsImageCache::renderImage( const QString &path, QSize size, const bool keepAspectRatio, const double opacity, bool blocking ) const
{
  QImage im;
  // direct read if path is a file -- maybe more efficient than going the bytearray route? (untested!)
  if ( !path.startsWith( QStringLiteral( "base64:" ) ) && QFile::exists( path ) )
  {
    im = QImage( path );
  }
  else
  {
    QByteArray ba = getContent( path, QByteArray( "broken" ), QByteArray( "fetching" ), blocking );

    if ( ba == "broken" )
    {
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
      QRectF rect( ( size.width() - s.width() ) / 2, ( size.height() - s.height() ) / 2, s.width(), s.height() );
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
      QRectF rect( ( size.width() - s.width() ) / 2, ( size.height() - s.height() ) / 2, s.width(), s.height() );
      r.render( &p, rect );
    }
    else
    {
      QBuffer buffer( &ba );
      buffer.open( QIODevice::ReadOnly );

      QImageReader reader( &buffer );
      im = reader.read();
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
