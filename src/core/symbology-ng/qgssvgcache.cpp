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
#include "qgslogger.h"
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

int QgsSvgCacheEntry::dataSize() const
{
  int size = svgContent.size();
  if ( picture )
  {
    size += picture->size();
  }
  if ( image )
  {
    size += ( image->width() * image->height() * 32 );
  }
  return size;
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

QgsSvgCache::QgsSvgCache(): mTotalSize( 0 ), mLeastRecentEntry( 0 ), mMostRecentEntry( 0 )
{
}

QgsSvgCache::~QgsSvgCache()
{
  QMultiHash< QString, QgsSvgCacheEntry* >::iterator it = mEntryLookup.begin();
  for ( ; it != mEntryLookup.end(); ++it )
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
    trimToMaximumSize();
  }

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
    trimToMaximumSize();
  }

  return *( currentEntry->picture );
}

QgsSvgCacheEntry* QgsSvgCache::insertSVG( const QString& file, double size, const QColor& fill, const QColor& outline, double outlineWidth,
    double widthScaleFactor, double rasterScaleFactor )
{
  QgsSvgCacheEntry* entry = new QgsSvgCacheEntry( file, size, outlineWidth, widthScaleFactor, rasterScaleFactor, fill, outline );

  replaceParamsAndCacheSvg( entry );

  mEntryLookup.insert( file, entry );

  //insert to most recent place in entry list
  if ( !mMostRecentEntry ) //inserting first entry
  {
    mLeastRecentEntry = entry;
    mMostRecentEntry = entry;
    entry->previousEntry = 0;
    entry->nextEntry = 0;
  }
  else
  {
    entry->previousEntry = mMostRecentEntry;
    entry->nextEntry = 0;
    mMostRecentEntry->nextEntry = entry;
    mMostRecentEntry = entry;
  }

  trimToMaximumSize();
  return entry;
}

void QgsSvgCache::containsParams( const QString& path, bool& hasFillParam, QColor& defaultFillColor, bool& hasOutlineParam, QColor& defaultOutlineColor,
                                 bool& hasOutlineWidthParam, double& defaultOutlineWidth ) const
{
  /*hasFillParam = false;
  hasOutlineParam = false;
  hasOutlineWidthParam = false;

  QFile svgFile( path );
  if ( !svgFile.open( QIODevice::ReadOnly ) )
  {
    return;
  }

  QDomDocument svgDoc;
  if ( !svgDoc.setContent( &svgFile ) )
  {
    return;
  }

  //there are surely faster ways to get this information
  QString content = svgDoc.toString();
  if ( content.contains( "param(fill" ) )
  {
    hasFillParam = true;
  }
  if ( content.contains( "param(outline" ) )
  {
    hasOutlineParam = true;
  }
  if ( content.contains( "param(outline-width)" ) )
  {
    hasOutlineWidthParam = true;
  }*/

  defaultFillColor = QColor( Qt::black );
  defaultOutlineColor= QColor( Qt::black );
  defaultOutlineWidth = 1.0;

  QFile svgFile( path );
  if ( !svgFile.open( QIODevice::ReadOnly ) )
  {
    return;
  }

  QDomDocument svgDoc;
  if ( !svgDoc.setContent( &svgFile ) )
  {
    return;
  }

  QDomElement docElem = svgDoc.documentElement();
  containsElemParams( docElem, hasFillParam, defaultFillColor, hasOutlineParam, defaultOutlineColor, hasOutlineWidthParam, defaultOutlineWidth );
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
  mTotalSize += entry->svgContent.size();
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
  mTotalSize += ( image->width() * image->height() * 32 );
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
  mTotalSize += entry->picture->size();
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
  else
  {
    takeEntryFromList( currentEntry );
    mMostRecentEntry->nextEntry = currentEntry;
    currentEntry->previousEntry = mMostRecentEntry;
    currentEntry->nextEntry = 0;
    mMostRecentEntry = currentEntry;
  }

  //debugging
  //printEntryList();

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
    //e.g. style="fill:param(fill);param(stroke)"
    if ( attribute.name().compare( "style", Qt::CaseInsensitive ) == 0 )
    {
      //entries separated by ';'
      QString newAttributeString;

      QStringList entryList = attribute.value().split( ';' );
      QStringList::const_iterator entryIt = entryList.constBegin();
      for ( ; entryIt != entryList.constEnd(); ++entryIt )
      {
        QStringList keyValueSplit = entryIt->split( ':' );
        if ( keyValueSplit.size() < 2 )
        {
          continue;
        }
        QString key = keyValueSplit.at( 0 );
        QString value = keyValueSplit.at( 1 );
        if ( value.startsWith( "param(fill" ) )
        {
          value = fill.name();
        }
        else if ( value.startsWith( "param(outline)" ) )
        {
          value = outline.name();
        }
        else if ( value.startsWith( "param(outline-width)" ) )
        {
          value = QString::number( outlineWidth );
        }
        newAttributeString.append( key + ":" + value + ";" );
      }
      elem.setAttribute( attribute.name(), newAttributeString );
    }
    else
    {
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
  }

  QDomNodeList childList = elem.childNodes();
  int nChildren = childList.count();
  for ( int i = 0; i < nChildren; ++i )
  {
    QDomElement childElem = childList.at( i ).toElement();
    replaceElemParams( childElem, fill, outline, outlineWidth );
  }
}

void QgsSvgCache::containsElemParams( const QDomElement& elem, bool& hasFillParam, QColor& defaultFill, bool& hasOutlineParam, QColor& defaultOutline,
                        bool& hasOutlineWidthParam, double& defaultOutlineWidth ) const
{
  if( elem.isNull() )
  {
    return;
  }

  //we already have all the information, no need to go deeper
  if( hasFillParam && hasOutlineParam && hasOutlineWidthParam )
  {
    return;
  }

  //check this elements attribute
  QDomNamedNodeMap attributes = elem.attributes();
  int nAttributes = attributes.count();

  QStringList valueSplit;
  for ( int i = 0; i < nAttributes; ++i )
  {
    QDomAttr attribute = attributes.item( i ).toAttr();
    if ( attribute.name().compare( "style", Qt::CaseInsensitive ) == 0 )
    {
      //entries separated by ';'
      QStringList entryList = attribute.value().split( ';' );
      QStringList::const_iterator entryIt = entryList.constBegin();
      for ( ; entryIt != entryList.constEnd(); ++entryIt )
      {
        valueSplit = entryIt->split(" ");
        if( !hasFillParam && entryIt->startsWith( "param(fill)" ) )
        {
          hasFillParam = true;
          if( valueSplit.size() > 1 )
          {
            defaultFill = QColor( valueSplit.at( 1 ) );
          }
        }
        else if( !hasOutlineParam && entryIt->startsWith( "param(outline)" ) )
        {
          hasOutlineParam = true;
          if( valueSplit.size() > 1 )
          {
            defaultOutline = QColor( valueSplit.at( 1 ) );
          }
        }
        else if( !hasOutlineWidthParam && entryIt->startsWith( "param(outlineWidth)" ) )
        {
          hasOutlineWidthParam = true;
          if( valueSplit.size() > 1 )
          {
            defaultOutlineWidth = valueSplit.at( 1 ).toDouble();
          }
        }
      }
    }
    else
    {
      QString value = attribute.value();
      valueSplit = value.split(" ");
      if ( !hasFillParam && value.startsWith( "param(fill)" ) )
      {
        hasFillParam = true;
        if( valueSplit.size() > 1 )
        {
          defaultFill = QColor( valueSplit.at( 1 ) );
        }
      }
      else if( !hasOutlineParam && value.startsWith( "param(outline)" ) )
      {
        hasOutlineParam = true;
        if( valueSplit.size() > 1 )
        {
          defaultOutline = QColor( valueSplit.at( 1 ) );
        }
      }
      else if( !hasOutlineWidthParam && value.startsWith( "param(outlineWidth)" ) )
      {
        hasOutlineWidthParam = true;
        if( valueSplit.size() > 1 )
        {
          defaultOutlineWidth = valueSplit.at( 1 ).toDouble();
        }
      }
    }
  }

  //pass it further to child items
  QDomNodeList childList = elem.childNodes();
  int nChildren = childList.count();
  for ( int i = 0; i < nChildren; ++i )
  {
    QDomElement childElem = childList.at( i ).toElement();
    containsElemParams( childElem, hasFillParam, defaultFill, hasOutlineParam, defaultOutline, hasOutlineWidthParam, defaultOutlineWidth );
  }
}

void QgsSvgCache::removeCacheEntry( QString s, QgsSvgCacheEntry* entry )
{
  delete entry;
  mEntryLookup.remove( s , entry );
}

void QgsSvgCache::printEntryList()
{
  QgsDebugMsg( "****************svg cache entry list*************************" );
  QgsDebugMsg( "Cache size: " + QString::number( mTotalSize ) );
  QgsSvgCacheEntry* entry = mLeastRecentEntry;
  while ( entry )
  {
    QgsDebugMsg( "***Entry:" );
    QgsDebugMsg( "File:" + entry->file );
    QgsDebugMsg( "Size:" + QString::number( entry->size ) );
    QgsDebugMsg( "Width scale factor" + QString::number( entry->widthScaleFactor ) );
    QgsDebugMsg( "Raster scale factor" + QString::number( entry->rasterScaleFactor ) );
    entry = entry->nextEntry;
  }
}

void QgsSvgCache::trimToMaximumSize()
{
  QgsSvgCacheEntry* entry = mLeastRecentEntry;
  while ( entry && ( mTotalSize > mMaximumSize ) )
  {
    QgsSvgCacheEntry* bkEntry = entry;
    entry = entry->nextEntry;

    takeEntryFromList( bkEntry );
    mTotalSize -= bkEntry->dataSize();
    delete bkEntry;
  }
}

void QgsSvgCache::takeEntryFromList( QgsSvgCacheEntry* entry )
{
  if ( !entry )
  {
    return;
  }

  if ( entry->previousEntry )
  {
    entry->previousEntry->nextEntry = entry->nextEntry;
  }
  else
  {
    mLeastRecentEntry = entry->nextEntry;
  }
  if ( entry->nextEntry )
  {
    entry->nextEntry->previousEntry = entry->previousEntry;
  }
  else
  {
    mMostRecentEntry = entry->previousEntry;
  }
}

