/***************************************************************************
                              qgssvgcache.h
                            ------------------------------
  begin                :  2011
  copyright            : (C) 2011 by Marco Hugentobler
  email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgssvgcache.h"
#include <QDomDocument>
#include <QDomElement>
#include <QFile>
#include <QImage>
#include <QPainter>
#include <QPicture>
#include <QSvgRenderer>

QgsSvgCacheEntry::QgsSvgCacheEntry(): file( QString() ), size( 0 ), outlineWidth( 0 ), widthScaleFactor( 1.0 ), rasterScaleFactor( 1.0 ), fill( Qt::black ),
    outline( Qt::black ), image( 0 ), picture( 0 )
{
}

QgsSvgCacheEntry::QgsSvgCacheEntry( const QString& f, double s, double ow, double wsf, double rsf, const QColor& fi, const QColor& ou ): file( f ), size( s ), outlineWidth( ow ),
    widthScaleFactor( wsf ), rasterScaleFactor( rsf ), fill( fi ), outline( ou ), image( 0 ), picture( 0 )
{
}


QgsSvgCacheEntry::~QgsSvgCacheEntry()
{
  delete image;
  delete picture;
}

bool QgsSvgCacheEntry::operator==( const QgsSvgCacheEntry& other ) const
{
  return ( other.file == file && other.size == size && other.outlineWidth == outlineWidth && other.widthScaleFactor == widthScaleFactor
           && other.rasterScaleFactor == rasterScaleFactor && other.fill == fill && other.outline == outline );
}

QString file;
double size;
double outlineWidth;
double widthScaleFactor;
double rasterScaleFactor;
QColor fill;
QColor outline;

QgsSvgCache* QgsSvgCache::mInstance = 0;

QgsSvgCache* QgsSvgCache::instance()
{
  if ( !mInstance )
  {
    mInstance = new QgsSvgCache();
  }
  return mInstance;
}

QgsSvgCache::QgsSvgCache()
{
}

QgsSvgCache::~QgsSvgCache()
{
  QMap< QDateTime, QgsSvgCacheEntry* >::iterator it = mEntries.begin();
  for ( ; it != mEntries.end(); ++it )
  {
    delete it.value();
  }
}


const QImage& QgsSvgCache::svgAsImage( const QString& file, double size, const QColor& fill, const QColor& outline, double outlineWidth,
                                       double widthScaleFactor, double rasterScaleFactor )
{
  QgsSvgCacheEntry* currentEntry = cacheEntry( file, size, fill, outline, outlineWidth, widthScaleFactor, rasterScaleFactor );

  //if current entry image is 0: cache image for entry
  //update stats for memory usage
  if ( !currentEntry->image )
  {
    cacheImage( currentEntry );
  }

  //update lastUsed with current date time

  return *( currentEntry->image );
}

const QPicture& QgsSvgCache::svgAsPicture( const QString& file, double size, const QColor& fill, const QColor& outline, double outlineWidth,
    double widthScaleFactor, double rasterScaleFactor )
{
  QgsSvgCacheEntry* currentEntry = cacheEntry( file, size, fill, outline, outlineWidth, widthScaleFactor, rasterScaleFactor );

  //if current entry image is 0: cache image for entry
  //update stats for memory usage
  if ( !currentEntry->picture )
  {
    cachePicture( currentEntry );
  }

  //update lastUsed with current date time

  return *( currentEntry->picture );
}

QgsSvgCacheEntry* QgsSvgCache::insertSVG( const QString& file, double size, const QColor& fill, const QColor& outline, double outlineWidth,
    double widthScaleFactor, double rasterScaleFactor )
{
  QgsSvgCacheEntry* entry = new QgsSvgCacheEntry( file, size, outlineWidth, widthScaleFactor, rasterScaleFactor, fill, outline );
  entry->lastUsed = QDateTime::currentDateTime();

  replaceParamsAndCacheSvg( entry );

  mEntries.insert( entry->lastUsed, entry );
  mEntryLookup.insert( file, entry );
  return entry;
}

void QgsSvgCache::replaceParamsAndCacheSvg( QgsSvgCacheEntry* entry )
{
  if ( !entry )
  {
    return;
  }

  QFile svgFile( entry->file );
  if ( !svgFile.open( QIODevice::ReadOnly ) )
  {
    return;
  }

  QDomDocument svgDoc;
  if ( !svgDoc.setContent( &svgFile ) )
  {
    return;
  }

  //replace fill color, outline color, outline with in all nodes
  QDomElement docElem = svgDoc.documentElement();
  replaceElemParams( docElem, entry->fill, entry->outline, entry->outlineWidth );

  entry->svgContent = svgDoc.toByteArray();
}

void QgsSvgCache::cacheImage( QgsSvgCacheEntry* entry )
{
  if ( !entry )
  {
    return;
  }

  delete entry->image;
  entry->image = 0;

  double imageSize = entry->size * entry->widthScaleFactor * entry->rasterScaleFactor;
  QImage* image = new QImage( imageSize, imageSize, QImage::Format_ARGB32_Premultiplied );
  image->fill( 0 ); // transparent background

  QPainter p( image );
  QSvgRenderer r( entry->svgContent );
  r.render( &p );

  entry->image = image;
}

void QgsSvgCache::cachePicture( QgsSvgCacheEntry *entry )
{
  if ( !entry )
  {
    return;
  }

  delete entry->picture;
  entry->picture = 0;

  //correct QPictures dpi correction
  QPicture* picture = new QPicture();
  double dpi = entry->widthScaleFactor * 25.4 * entry->rasterScaleFactor;
  double pictureSize = entry->size * entry->widthScaleFactor / dpi * picture->logicalDpiX();
  QRectF rect( QPointF( -pictureSize / 2.0, -pictureSize / 2.0 ), QSizeF( pictureSize, pictureSize ) );


  QSvgRenderer renderer( entry->svgContent );
  QPainter painter( picture );
  renderer.render( &painter, rect );
  entry->picture = picture;
}

QgsSvgCacheEntry* QgsSvgCache::cacheEntry( const QString& file, double size, const QColor& fill, const QColor& outline, double outlineWidth,
    double widthScaleFactor, double rasterScaleFactor )
{
  //search entries in mEntryLookup
  QgsSvgCacheEntry* currentEntry = 0;
  QList<QgsSvgCacheEntry*> entries = mEntryLookup.values( file );

  QList<QgsSvgCacheEntry*>::iterator entryIt = entries.begin();
  for ( ; entryIt != entries.end(); ++entryIt )
  {
    QgsSvgCacheEntry* cacheEntry = *entryIt;
    if ( cacheEntry->file == file && cacheEntry->size == size && cacheEntry->fill == fill && cacheEntry->outline == outline &&
         cacheEntry->outlineWidth == outlineWidth && cacheEntry->widthScaleFactor == widthScaleFactor && cacheEntry->rasterScaleFactor == rasterScaleFactor )
    {
      currentEntry = cacheEntry;
      break;
    }
  }


  //if not found: create new entry
  //cache and replace params in svg content
  if ( !currentEntry )
  {
    currentEntry = insertSVG( file, size, fill, outline, outlineWidth, widthScaleFactor, rasterScaleFactor );
  }
  return currentEntry;
}

void QgsSvgCache::replaceElemParams( QDomElement& elem, const QColor& fill, const QColor& outline, double outlineWidth )
{
  if ( elem.isNull() )
  {
    return;
  }

  //go through attributes
  QDomNamedNodeMap attributes = elem.attributes();
  int nAttributes = attributes.count();
  for ( int i = 0; i < nAttributes; ++i )
  {
    QDomAttr attribute = attributes.item( i ).toAttr();
    QString value = attribute.value();
    if ( value.startsWith( "param(fill)" ) )
    {
      elem.setAttribute( attribute.name(), fill.name() );
    }
    else if ( value.startsWith( "param(outline)" ) )
    {
      elem.setAttribute( attribute.name(), outline.name() );
    }
    else if ( value.startsWith( "param(outline-width)" ) )
    {
      elem.setAttribute( attribute.name(), QString::number( outlineWidth ) );
    }
  }

  QDomNodeList childList = elem.childNodes();
  int nChildren = childList.count();
  for ( int i = 0; i < nChildren; ++i )
  {
    QDomElement childElem = childList.at( i ).toElement();
    replaceElemParams( childElem, fill, outline, outlineWidth );
  }
}

