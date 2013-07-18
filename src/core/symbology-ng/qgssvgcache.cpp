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
#include "qgis.h"
#include "qgslogger.h"
#include "qgsnetworkaccessmanager.h"
#include "qgsmessagelog.h"
#include <QApplication>
#include <QCoreApplication>
#include <QCursor>
#include <QDomDocument>
#include <QDomElement>
#include <QFile>
#include <QImage>
#include <QPainter>
#include <QPicture>
#include <QSvgRenderer>
#include <QFileInfo>
#include <QNetworkReply>
#include <QNetworkRequest>

QgsSvgCacheEntry::QgsSvgCacheEntry(): file( QString() ), size( 0.0 ), outlineWidth( 0 ), widthScaleFactor( 1.0 ), rasterScaleFactor( 1.0 ), fill( Qt::black ),
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

QgsSvgCache::QgsSvgCache( QObject *parent )
    : QObject( parent )
    , mTotalSize( 0 )
    , mLeastRecentEntry( 0 )
    , mMostRecentEntry( 0 )
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
                                       double widthScaleFactor, double rasterScaleFactor, bool& fitsInCache )
{
  fitsInCache = true;
  QgsSvgCacheEntry* currentEntry = cacheEntry( file, size, fill, outline, outlineWidth, widthScaleFactor, rasterScaleFactor );

  //if current entry image is 0: cache image for entry
  // checks to see if image will fit into cache
  //update stats for memory usage
  if ( !currentEntry->image )
  {
    QSvgRenderer r( currentEntry->svgContent );
    double hwRatio = 1.0;
    if ( r.viewBoxF().width() > 0 )
    {
      hwRatio = r.viewBoxF().height() / r.viewBoxF().width();
    }
    long cachedDataSize = 0;
    cachedDataSize += currentEntry->svgContent.size();
    cachedDataSize += ( int )( currentEntry->size * currentEntry->size * hwRatio * 32 );
    if ( cachedDataSize > mMaximumSize / 2 )
    {
      fitsInCache = false;
      delete currentEntry->image;
      currentEntry->image = 0;
      //currentEntry->image = new QImage( 0, 0 );

      // instead cache picture
      if ( !currentEntry->picture )
      {
        cachePicture( currentEntry, false );
      }
    }
    else
    {
      cacheImage( currentEntry );
    }
    trimToMaximumSize();
  }

  return *( currentEntry->image );
}

const QPicture& QgsSvgCache::svgAsPicture( const QString& file, double size, const QColor& fill, const QColor& outline, double outlineWidth,
    double widthScaleFactor, double rasterScaleFactor, bool forceVectorOutput )
{
  QgsSvgCacheEntry* currentEntry = cacheEntry( file, size, fill, outline, outlineWidth, widthScaleFactor, rasterScaleFactor );

  //if current entry picture is 0: cache picture for entry
  //update stats for memory usage
  if ( !currentEntry->picture )
  {
    cachePicture( currentEntry, forceVectorOutput );
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
  defaultFillColor = QColor( Qt::black );
  defaultOutlineColor = QColor( Qt::black );
  defaultOutlineWidth = 1.0;

  QDomDocument svgDoc;
  if ( !svgDoc.setContent( getImageData( path ) ) )
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

  QDomDocument svgDoc;
  if ( !svgDoc.setContent( getImageData( entry->file ) ) )
  {
    return;
  }

  //replace fill color, outline color, outline with in all nodes
  QDomElement docElem = svgDoc.documentElement();
  replaceElemParams( docElem, entry->fill, entry->outline, entry->outlineWidth );

  entry->svgContent = svgDoc.toByteArray();
  mTotalSize += entry->svgContent.size();
}

QByteArray QgsSvgCache::getImageData( const QString &path ) const
{
  // is it a path to local file?
  QFile svgFile( path );
  if ( svgFile.exists() )
  {
    if ( svgFile.open( QIODevice::ReadOnly ) )
    {
      return svgFile.readAll();
    }
    else
    {
      return QByteArray();
    }
  }

  // maybe it's a url...
  if ( !path.contains( "://" ) ) // otherwise short, relative SVG paths might be considered URLs
  {
    return QByteArray();
  }

  QUrl svgUrl( path );
  if ( !svgUrl.isValid() )
  {
    return QByteArray();
  }

  // check whether it's a url pointing to a local file
  if ( svgUrl.scheme().compare( "file", Qt::CaseInsensitive ) == 0 )
  {
    svgFile.setFileName( svgUrl.toLocalFile() );
    if ( svgFile.exists() )
    {
      if ( svgFile.open( QIODevice::ReadOnly ) )
      {
        return svgFile.readAll();
      }
    }

    // not found...
    return QByteArray();
  }

  // the url points to a remote resource, download it!
  QNetworkReply *reply = 0;

  // The following code blocks until the file is downloaded...
  // TODO: use signals to get reply finished notification, in this moment
  // it's executed while rendering.
  while ( 1 )
  {
    QgsDebugMsg( QString( "get svg: %1" ).arg( svgUrl.toString() ) );
    QNetworkRequest request( svgUrl );
    request.setAttribute( QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache );
    request.setAttribute( QNetworkRequest::CacheSaveControlAttribute, true );

    reply = QgsNetworkAccessManager::instance()->get( request );
    connect( reply, SIGNAL( downloadProgress( qint64, qint64 ) ), this, SLOT( downloadProgress( qint64, qint64 ) ) );

    //emit statusChanged( tr( "Downloading svg." ) );

    // wait until the image download finished
    // TODO: connect to the reply->finished() signal
    while ( !reply->isFinished() )
    {
      QCoreApplication::processEvents( QEventLoop::ExcludeUserInputEvents, 500 );
    }

    if ( reply->error() != QNetworkReply::NoError )
    {
      QgsMessageLog::logMessage( tr( "SVG request failed [error: %1 - url: %2]" ).arg( reply->errorString() ).arg( reply->url().toString() ), tr( "SVG" ) );

      reply->deleteLater();
      return QByteArray();
    }

    QVariant redirect = reply->attribute( QNetworkRequest::RedirectionTargetAttribute );
    if ( redirect.isNull() )
    {
      // neither network error nor redirection
      // TODO: cache the image
      break;
    }

    // do a new request to the redirect url
    svgUrl = redirect.toUrl();
    reply->deleteLater();
  }

  QVariant status = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute );
  if ( !status.isNull() && status.toInt() >= 400 )
  {
    QVariant phrase = reply->attribute( QNetworkRequest::HttpReasonPhraseAttribute );
    QgsMessageLog::logMessage( tr( "SVG request error [status: %1 - reason phrase: %2]" ).arg( status.toInt() ).arg( phrase.toString() ), tr( "SVG" ) );

    reply->deleteLater();
    return QByteArray();
  }

  QString contentType = reply->header( QNetworkRequest::ContentTypeHeader ).toString();
  QgsDebugMsg( "contentType: " + contentType );
  if ( !contentType.startsWith( "image/svg+xml", Qt::CaseInsensitive ) )
  {
    reply->deleteLater();
    return QByteArray();
  }

  // read the image data
  QByteArray ba = reply->readAll();
  reply->deleteLater();

  return ba;
}

void QgsSvgCache::cacheImage( QgsSvgCacheEntry* entry )
{
  if ( !entry )
  {
    return;
  }

  delete entry->image;
  entry->image = 0;

  QSvgRenderer r( entry->svgContent );
  double hwRatio = 1.0;
  if ( r.viewBoxF().width() > 0 )
  {
    hwRatio = r.viewBoxF().height() / r.viewBoxF().width();
  }
  double wSize = entry->size;
  int wImgSize = ( int )wSize;
  if ( wImgSize < 1 )
  {
    wImgSize = 1;
  }
  double hSize = wSize * hwRatio;
  int hImgSize = ( int )hSize;
  if ( hImgSize < 1 )
  {
    hImgSize = 1;
  }
  // cast double image sizes to int for QImage
  QImage* image = new QImage( wImgSize, hImgSize, QImage::Format_ARGB32_Premultiplied );
  image->fill( 0 ); // transparent background

  QPainter p( image );
  if ( r.viewBoxF().width() == r.viewBoxF().height() )
  {
    r.render( &p );
  }
  else
  {
    QSizeF s( r.viewBoxF().size() );
    s.scale( wSize, hSize, Qt::KeepAspectRatio );
    QRectF rect(( wImgSize - s.width() ) / 2, ( hImgSize - s.height() ) / 2, s.width(), s.height() );
    r.render( &p, rect );
  }

  entry->image = image;
  mTotalSize += ( image->width() * image->height() * 32 );
}

void QgsSvgCache::cachePicture( QgsSvgCacheEntry *entry, bool forceVectorOutput )
{
  if ( !entry )
  {
    return;
  }

  delete entry->picture;
  entry->picture = 0;

  //correct QPictures dpi correction
  QPicture* picture = new QPicture();
  QRectF rect;
  QSvgRenderer r( entry->svgContent );
  double hwRatio = 1.0;
  if ( r.viewBoxF().width() > 0 )
  {
    hwRatio = r.viewBoxF().height() / r.viewBoxF().width();
  }
  bool drawOnScreen = qgsDoubleNear( entry->rasterScaleFactor, 1.0, 0.1 );
  if ( drawOnScreen && forceVectorOutput ) //forceVectorOutput always true in case of composer draw / composer preview
  {
    // fix to ensure rotated symbols scale with composer page (i.e. not map item) zoom
    double wSize = entry->size;
    double hSize = wSize * hwRatio;
    QSizeF s( r.viewBoxF().size() );
    s.scale( wSize, hSize, Qt::KeepAspectRatio );
    rect = QRectF( -s.width() / 2.0, -s.height() / 2.0, s.width(), s.height() );
  }
  else
  {
    // output for print or image saving @ specific dpi
    double scaledSize = entry->size / 25.4 / ( entry->rasterScaleFactor * entry->widthScaleFactor );
    double wSize = scaledSize * picture->logicalDpiX();
    double hSize = scaledSize * picture->logicalDpiY() * r.viewBoxF().height() / r.viewBoxF().width();
    rect = QRectF( QPointF( -wSize / 2.0, -hSize / 2.0 ), QSizeF( wSize, hSize ) );
  }

  QPainter p( picture );
  r.render( &p, rect );
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
    if ( cacheEntry->file == file && qgsDoubleNear( cacheEntry->size, size ) && cacheEntry->fill == fill && cacheEntry->outline == outline &&
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
    if ( !mMostRecentEntry ) //list is empty
    {
      mMostRecentEntry = currentEntry;
      mLeastRecentEntry = currentEntry;
    }
    else
    {
      mMostRecentEntry->nextEntry = currentEntry;
      currentEntry->previousEntry = mMostRecentEntry;
      currentEntry->nextEntry = 0;
      mMostRecentEntry = currentEntry;
    }
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

        if ( entryIt != entryList.constBegin() )
        {
          newAttributeString.append( ";" );
        }
        newAttributeString.append( key + ":" + value );
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
  if ( elem.isNull() )
  {
    return;
  }

  //we already have all the information, no need to go deeper
  if ( hasFillParam && hasOutlineParam && hasOutlineWidthParam )
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
        QStringList keyValueSplit = entryIt->split( ':' );
        if ( keyValueSplit.size() < 2 )
        {
          continue;
        }
        QString key = keyValueSplit.at( 0 );
        QString value = keyValueSplit.at( 1 );
        valueSplit = value.split( " " );
        if ( !hasFillParam && value.startsWith( "param(fill)" ) )
        {
          hasFillParam = true;
          if ( valueSplit.size() > 1 )
          {
            defaultFill = QColor( valueSplit.at( 1 ) );
          }
        }
        else if ( !hasOutlineParam && value.startsWith( "param(outline)" ) )
        {
          hasOutlineParam = true;
          if ( valueSplit.size() > 1 )
          {
            defaultOutline = QColor( valueSplit.at( 1 ) );
          }
        }
        else if ( !hasOutlineWidthParam && value.startsWith( "param(outline-width)" ) )
        {
          hasOutlineWidthParam = true;
          if ( valueSplit.size() > 1 )
          {
            defaultOutlineWidth = valueSplit.at( 1 ).toDouble();
          }
        }
      }
    }
    else
    {
      QString value = attribute.value();
      valueSplit = value.split( " " );
      if ( !hasFillParam && value.startsWith( "param(fill)" ) )
      {
        hasFillParam = true;
        if ( valueSplit.size() > 1 )
        {
          defaultFill = QColor( valueSplit.at( 1 ) );
        }
      }
      else if ( !hasOutlineParam && value.startsWith( "param(outline)" ) )
      {
        hasOutlineParam = true;
        if ( valueSplit.size() > 1 )
        {
          defaultOutline = QColor( valueSplit.at( 1 ) );
        }
      }
      else if ( !hasOutlineWidthParam && value.startsWith( "param(outline-width)" ) )
      {
        hasOutlineWidthParam = true;
        if ( valueSplit.size() > 1 )
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
  //only one entry in cache
  if ( mLeastRecentEntry == mMostRecentEntry )
  {
    return;
  }
  QgsSvgCacheEntry* entry = mLeastRecentEntry;
  while ( entry && ( mTotalSize > mMaximumSize ) )
  {
    QgsSvgCacheEntry* bkEntry = entry;
    entry = entry->nextEntry;

    takeEntryFromList( bkEntry );
    mEntryLookup.remove( bkEntry->file, bkEntry );
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

void QgsSvgCache::downloadProgress( qint64 bytesReceived, qint64 bytesTotal )
{
  QString msg = tr( "%1 of %2 bytes of svg image downloaded." ).arg( bytesReceived ).arg( bytesTotal < 0 ? QString( "unknown number of" ) : QString::number( bytesTotal ) );
  QgsDebugMsg( msg );
  emit statusChanged( msg );
}
