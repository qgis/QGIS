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
#include "qgslogger.h"
#include "qgsnetworkaccessmanager.h"
#include "qgsmessagelog.h"
#include "qgssymbollayerutils.h"
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

QgsImageCacheEntry::QgsImageCacheEntry( const QString &path, QSize size, const bool keepAspectRatio )
  : QgsAbstractContentCacheEntry( path )
  , size( size )
  , keepAspectRatio( keepAspectRatio )
{
}

bool QgsImageCacheEntry::isEqual( const QgsAbstractContentCacheEntry *other ) const
{
  const QgsImageCacheEntry *otherImage = dynamic_cast< const QgsImageCacheEntry * >( other );
  // cheapest checks first!
  if ( !otherImage || otherImage->keepAspectRatio != keepAspectRatio || otherImage->size != size || otherImage->path != path )
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

QImage QgsImageCache::pathAsImage( const QString &file, const QSize size, const bool keepAspectRatio, bool &fitsInCache )
{
  QMutexLocker locker( &mMutex );

  fitsInCache = true;

  QgsImageCacheEntry *currentEntry = findExistingEntry( new QgsImageCacheEntry( file, size, keepAspectRatio ) );

  QImage result;

  //if current entry image is null: create the image
  // checks to see if image will fit into cache
  //update stats for memory usage
  if ( currentEntry->image.isNull() )
  {
    long cachedDataSize = 0;
    cachedDataSize += currentEntry->size.width() * currentEntry->size.height() * 32;
    result = renderImage( file, size, keepAspectRatio );
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

QImage QgsImageCache::renderImage( const QString &path, QSize size, const bool keepAspectRatio ) const
{
  QImage im;
  // direct read if path is a file -- maybe more efficient than going the bytearray route? (untested!)
  if ( QFile::exists( path ) )
  {
    im = QImage( path );
  }
  else
  {
    QByteArray ba = getContent( path, QByteArray( "broken" ), QByteArray( "fetching" ) );

    if ( ba == "broken" )
    {
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

  // render image at desired size -- null size means original size
  if ( !size.isValid() || size.isNull() || im.size() == size )
    return im;
  else
    return im.scaled( size, keepAspectRatio ? Qt::KeepAspectRatio : Qt::IgnoreAspectRatio, Qt::SmoothTransformation );
}
