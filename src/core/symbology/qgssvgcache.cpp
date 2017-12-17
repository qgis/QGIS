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
#include "qgssymbollayerutils.h"

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

///@cond PRIVATE

QgsSvgCacheEntry::QgsSvgCacheEntry( const QString &p, double s, double ow, double wsf, const QColor &fi, const QColor &ou, double far )
  : path( p )
  , fileModified( QFileInfo( p ).lastModified() )
  , size( s )
  , strokeWidth( ow )
  , widthScaleFactor( wsf )
  , fixedAspectRatio( far )
  , fill( fi )
  , stroke( ou )
{
  fileModifiedLastCheckTimer.start();
}

bool QgsSvgCacheEntry::operator==( const QgsSvgCacheEntry &other ) const
{
  bool equal = other.path == path && qgsDoubleNear( other.size, size ) && qgsDoubleNear( other.strokeWidth, strokeWidth ) && qgsDoubleNear( other.widthScaleFactor, widthScaleFactor )
               && other.fixedAspectRatio == fixedAspectRatio && other.fill == fill && other.stroke == stroke;

  if ( equal && ( mFileModifiedCheckTimeout <= 0 || fileModifiedLastCheckTimer.hasExpired( mFileModifiedCheckTimeout ) ) )
    equal = other.fileModified == fileModified;

  return equal;
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
///@endcond


QgsSvgCache::QgsSvgCache( QObject *parent )
  : QObject( parent )
{
  mMissingSvg = QStringLiteral( "<svg width='10' height='10'><text x='5' y='10' font-size='10' text-anchor='middle'>?</text></svg>" ).toLatin1();
}

QgsSvgCache::~QgsSvgCache()
{
  qDeleteAll( mEntryLookup );
}


QImage QgsSvgCache::svgAsImage( const QString &file, double size, const QColor &fill, const QColor &stroke, double strokeWidth,
                                double widthScaleFactor, bool &fitsInCache, double fixedAspectRatio )
{
  QMutexLocker locker( &mMutex );

  fitsInCache = true;
  QgsSvgCacheEntry *currentEntry = cacheEntry( file, size, fill, stroke, strokeWidth, widthScaleFactor, fixedAspectRatio );

  QImage result;

  //if current entry image is 0: cache image for entry
  // checks to see if image will fit into cache
  //update stats for memory usage
  if ( !currentEntry->image )
  {
    QSvgRenderer r( currentEntry->svgContent );
    double hwRatio = 1.0;
    if ( r.viewBoxF().width() > 0 )
    {
      if ( currentEntry->fixedAspectRatio > 0 )
      {
        hwRatio = currentEntry->fixedAspectRatio;
      }
      else
      {
        hwRatio = r.viewBoxF().height() / r.viewBoxF().width();
      }
    }
    long cachedDataSize = 0;
    cachedDataSize += currentEntry->svgContent.size();
    cachedDataSize += static_cast< int >( currentEntry->size * currentEntry->size * hwRatio * 32 );
    if ( cachedDataSize > MAXIMUM_SIZE / 2 )
    {
      fitsInCache = false;
      currentEntry->image.reset();

      // instead cache picture
      if ( !currentEntry->picture )
      {
        cachePicture( currentEntry, false );
      }

      // ...and render cached picture to result image
      result = imageFromCachedPicture( *currentEntry );
    }
    else
    {
      cacheImage( currentEntry );
      result = *( currentEntry->image );
    }
    trimToMaximumSize();
  }
  else
  {
    result = *( currentEntry->image );
  }

  return result;
}

QPicture QgsSvgCache::svgAsPicture( const QString &path, double size, const QColor &fill, const QColor &stroke, double strokeWidth,
                                    double widthScaleFactor, bool forceVectorOutput, double fixedAspectRatio )
{
  QMutexLocker locker( &mMutex );

  QgsSvgCacheEntry *currentEntry = cacheEntry( path, size, fill, stroke, strokeWidth, widthScaleFactor, fixedAspectRatio );

  //if current entry picture is 0: cache picture for entry
  //update stats for memory usage
  if ( !currentEntry->picture )
  {
    cachePicture( currentEntry, forceVectorOutput );
    trimToMaximumSize();
  }

  QPicture p = *( currentEntry->picture );
  p.detach();
  return p;
}

QByteArray QgsSvgCache::svgContent( const QString &path, double size, const QColor &fill, const QColor &stroke, double strokeWidth,
                                    double widthScaleFactor, double fixedAspectRatio )
{
  QMutexLocker locker( &mMutex );

  QgsSvgCacheEntry *currentEntry = cacheEntry( path, size, fill, stroke, strokeWidth, widthScaleFactor, fixedAspectRatio );

  return currentEntry->svgContent;
}

QSizeF QgsSvgCache::svgViewboxSize( const QString &path, double size, const QColor &fill, const QColor &stroke, double strokeWidth, double widthScaleFactor, double fixedAspectRatio )
{
  QMutexLocker locker( &mMutex );

  QgsSvgCacheEntry *currentEntry = cacheEntry( path, size, fill, stroke, strokeWidth, widthScaleFactor, fixedAspectRatio );

  return currentEntry->viewboxSize;
}

QgsSvgCacheEntry *QgsSvgCache::insertSvg( const QString &path, double size, const QColor &fill, const QColor &stroke, double strokeWidth,
    double widthScaleFactor, double fixedAspectRatio )
{
  QgsSvgCacheEntry *entry = new QgsSvgCacheEntry( path, size, strokeWidth, widthScaleFactor, fill, stroke, fixedAspectRatio );
  entry->mFileModifiedCheckTimeout = mFileModifiedCheckTimeout;

  replaceParamsAndCacheSvg( entry );

  mEntryLookup.insert( path, entry );

  //insert to most recent place in entry list
  if ( !mMostRecentEntry ) //inserting first entry
  {
    mLeastRecentEntry = entry;
    mMostRecentEntry = entry;
    entry->previousEntry = nullptr;
    entry->nextEntry = nullptr;
  }
  else
  {
    entry->previousEntry = mMostRecentEntry;
    entry->nextEntry = nullptr;
    mMostRecentEntry->nextEntry = entry;
    mMostRecentEntry = entry;
  }

  trimToMaximumSize();
  return entry;
}

void QgsSvgCache::containsParams( const QString &path, bool &hasFillParam, QColor &defaultFillColor, bool &hasStrokeParam, QColor &defaultStrokeColor,
                                  bool &hasStrokeWidthParam, double &defaultStrokeWidth ) const
{
  bool hasDefaultFillColor = false;
  bool hasFillOpacityParam = false;
  bool hasDefaultFillOpacity = false;
  double defaultFillOpacity = 1.0;
  bool hasDefaultStrokeColor = false;
  bool hasDefaultStrokeWidth = false;
  bool hasStrokeOpacityParam = false;
  bool hasDefaultStrokeOpacity = false;
  double defaultStrokeOpacity = 1.0;

  containsParams( path, hasFillParam, hasDefaultFillColor, defaultFillColor,
                  hasFillOpacityParam, hasDefaultFillOpacity, defaultFillOpacity,
                  hasStrokeParam, hasDefaultStrokeColor, defaultStrokeColor,
                  hasStrokeWidthParam, hasDefaultStrokeWidth, defaultStrokeWidth,
                  hasStrokeOpacityParam, hasDefaultStrokeOpacity, defaultStrokeOpacity );
}

void QgsSvgCache::containsParams( const QString &path,
                                  bool &hasFillParam, bool &hasDefaultFillParam, QColor &defaultFillColor,
                                  bool &hasFillOpacityParam, bool &hasDefaultFillOpacity, double &defaultFillOpacity,
                                  bool &hasStrokeParam, bool &hasDefaultStrokeColor, QColor &defaultStrokeColor,
                                  bool &hasStrokeWidthParam, bool &hasDefaultStrokeWidth, double &defaultStrokeWidth,
                                  bool &hasStrokeOpacityParam, bool &hasDefaultStrokeOpacity, double &defaultStrokeOpacity ) const
{
  hasFillParam = false;
  hasFillOpacityParam = false;
  hasStrokeParam = false;
  hasStrokeWidthParam = false;
  hasStrokeOpacityParam = false;
  defaultFillColor = QColor( Qt::white );
  defaultFillOpacity = 1.0;
  defaultStrokeColor = QColor( Qt::black );
  defaultStrokeWidth = 0.2;
  defaultStrokeOpacity = 1.0;

  hasDefaultFillParam = false;
  hasDefaultFillOpacity = false;
  hasDefaultStrokeColor = false;
  hasDefaultStrokeWidth = false;
  hasDefaultStrokeOpacity = false;

  QDomDocument svgDoc;
  if ( !svgDoc.setContent( getImageData( path ) ) )
  {
    return;
  }

  QDomElement docElem = svgDoc.documentElement();
  containsElemParams( docElem, hasFillParam, hasDefaultFillParam, defaultFillColor,
                      hasFillOpacityParam, hasDefaultFillOpacity, defaultFillOpacity,
                      hasStrokeParam, hasDefaultStrokeColor, defaultStrokeColor,
                      hasStrokeWidthParam, hasDefaultStrokeWidth, defaultStrokeWidth,
                      hasStrokeOpacityParam, hasDefaultStrokeOpacity, defaultStrokeOpacity );
}

void QgsSvgCache::replaceParamsAndCacheSvg( QgsSvgCacheEntry *entry )
{
  if ( !entry )
  {
    return;
  }

  QDomDocument svgDoc;
  if ( !svgDoc.setContent( getImageData( entry->path ) ) )
  {
    return;
  }

  //replace fill color, stroke color, stroke with in all nodes
  QDomElement docElem = svgDoc.documentElement();

  QSizeF viewboxSize;
  double sizeScaleFactor = calcSizeScaleFactor( entry, docElem, viewboxSize );
  entry->viewboxSize = viewboxSize;
  replaceElemParams( docElem, entry->fill, entry->stroke, entry->strokeWidth * sizeScaleFactor );

  entry->svgContent = svgDoc.toByteArray( 0 );

  // toByteArray screws up tspans inside text by adding new lines before and after each span... this should help, at the
  // risk of potentially breaking some svgs where the newline is desired
  entry->svgContent.replace( "\n<tspan", "<tspan" );
  entry->svgContent.replace( "</tspan>\n", "</tspan>" );

  mTotalSize += entry->svgContent.size();
}

double QgsSvgCache::calcSizeScaleFactor( QgsSvgCacheEntry *entry, const QDomElement &docElem, QSizeF &viewboxSize ) const
{
  QString viewBox;

  //bad size
  if ( !entry || qgsDoubleNear( entry->size, 0.0 ) )
    return 1.0;

  //find svg viewbox attribute
  //first check if docElem is svg element
  if ( docElem.tagName() == QLatin1String( "svg" ) && docElem.hasAttribute( QStringLiteral( "viewBox" ) ) )
  {
    viewBox = docElem.attribute( QStringLiteral( "viewBox" ), QString() );
  }
  else if ( docElem.tagName() == QLatin1String( "svg" ) && docElem.hasAttribute( QStringLiteral( "viewbox" ) ) )
  {
    viewBox = docElem.attribute( QStringLiteral( "viewbox" ), QString() );
  }
  else
  {
    QDomElement svgElem = docElem.firstChildElement( QStringLiteral( "svg" ) );
    if ( !svgElem.isNull() )
    {
      if ( svgElem.hasAttribute( QStringLiteral( "viewBox" ) ) )
        viewBox = svgElem.attribute( QStringLiteral( "viewBox" ), QString() );
      else if ( svgElem.hasAttribute( QStringLiteral( "viewbox" ) ) )
        viewBox = svgElem.attribute( QStringLiteral( "viewbox" ), QString() );
    }
  }

  //could not find valid viewbox attribute
  if ( viewBox.isEmpty() )
    return 1.0;

  //width should be 3rd element in a 4 part space delimited string
  QStringList parts = viewBox.split( ' ' );
  if ( parts.count() != 4 )
    return 1.0;

  bool heightOk = false;
  double height = parts.at( 3 ).toDouble( &heightOk );

  bool widthOk = false;
  double width = parts.at( 2 ).toDouble( &widthOk );
  if ( widthOk )
  {
    if ( heightOk )
      viewboxSize = QSizeF( width, height );
    return width / entry->size;
  }

  return 1.0;
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
      return mMissingSvg;
    }
  }

  // maybe it's a url...
  if ( !path.contains( QLatin1String( "://" ) ) ) // otherwise short, relative SVG paths might be considered URLs
  {
    return mMissingSvg;
  }

  QUrl svgUrl( path );
  if ( !svgUrl.isValid() )
  {
    return mMissingSvg;
  }

  // check whether it's a url pointing to a local file
  if ( svgUrl.scheme().compare( QLatin1String( "file" ), Qt::CaseInsensitive ) == 0 )
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
    return mMissingSvg;
  }

  // the url points to a remote resource, download it!
  QNetworkReply *reply = nullptr;

  // The following code blocks until the file is downloaded...
  // TODO: use signals to get reply finished notification, in this moment
  // it's executed while rendering.
  while ( true )
  {
    QgsDebugMsg( QString( "get svg: %1" ).arg( svgUrl.toString() ) );
    QNetworkRequest request( svgUrl );
    request.setAttribute( QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache );
    request.setAttribute( QNetworkRequest::CacheSaveControlAttribute, true );

    reply = QgsNetworkAccessManager::instance()->get( request );
    connect( reply, &QNetworkReply::downloadProgress, this, &QgsSvgCache::downloadProgress );

    //emit statusChanged( tr( "Downloading svg." ) );

    // wait until the image download finished
    // TODO: connect to the reply->finished() signal
    while ( !reply->isFinished() )
    {
      QCoreApplication::processEvents( QEventLoop::ExcludeUserInputEvents, 500 );
    }

    if ( reply->error() != QNetworkReply::NoError )
    {
      QgsMessageLog::logMessage( tr( "SVG request failed [error: %1 - url: %2]" ).arg( reply->errorString(), reply->url().toString() ), tr( "SVG" ) );

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
    return mMissingSvg;
  }

  QString contentType = reply->header( QNetworkRequest::ContentTypeHeader ).toString();
  QgsDebugMsg( "contentType: " + contentType );
  if ( !contentType.startsWith( QLatin1String( "image/svg+xml" ), Qt::CaseInsensitive ) )
  {
    reply->deleteLater();
    return mMissingSvg;
  }

  // read the image data
  QByteArray ba = reply->readAll();
  reply->deleteLater();

  return ba;
}

void QgsSvgCache::cacheImage( QgsSvgCacheEntry *entry )
{
  if ( !entry )
  {
    return;
  }

  entry->image.reset();

  QSizeF viewBoxSize;
  QSizeF scaledSize;
  QSize imageSize = sizeForImage( *entry, viewBoxSize, scaledSize );

  // cast double image sizes to int for QImage
  std::unique_ptr< QImage > image = qgis::make_unique< QImage >( imageSize, QImage::Format_ARGB32_Premultiplied );
  image->fill( 0 ); // transparent background

  QPainter p( image.get() );
  QSvgRenderer r( entry->svgContent );
  if ( qgsDoubleNear( viewBoxSize.width(), viewBoxSize.height() ) )
  {
    r.render( &p );
  }
  else
  {
    QSizeF s( viewBoxSize );
    s.scale( scaledSize.width(), scaledSize.height(), Qt::KeepAspectRatio );
    QRectF rect( ( imageSize.width() - s.width() ) / 2, ( imageSize.height() - s.height() ) / 2, s.width(), s.height() );
    r.render( &p, rect );
  }

  mTotalSize += ( image->width() * image->height() * 32 );
  entry->image = std::move( image );
}

void QgsSvgCache::cachePicture( QgsSvgCacheEntry *entry, bool forceVectorOutput )
{
  Q_UNUSED( forceVectorOutput );
  if ( !entry )
  {
    return;
  }

  entry->picture.reset();

  bool isFixedAR = entry->fixedAspectRatio > 0;

  //correct QPictures dpi correction
  std::unique_ptr< QPicture > picture = qgis::make_unique< QPicture >();
  QRectF rect;
  QSvgRenderer r( entry->svgContent );
  double hwRatio = 1.0;
  if ( r.viewBoxF().width() > 0 )
  {
    if ( isFixedAR )
    {
      hwRatio = entry->fixedAspectRatio;
    }
    else
    {
      hwRatio = r.viewBoxF().height() / r.viewBoxF().width();
    }
  }

  double wSize = entry->size;
  double hSize = wSize * hwRatio;

  QSizeF s( r.viewBoxF().size() );
  s.scale( wSize, hSize, isFixedAR ? Qt::IgnoreAspectRatio : Qt::KeepAspectRatio );
  rect = QRectF( -s.width() / 2.0, -s.height() / 2.0, s.width(), s.height() );

  QPainter p( picture.get() );
  r.render( &p, rect );
  entry->picture = std::move( picture );
  mTotalSize += entry->picture->size();
}

QgsSvgCacheEntry *QgsSvgCache::cacheEntry( const QString &path, double size, const QColor &fill, const QColor &stroke, double strokeWidth,
    double widthScaleFactor, double fixedAspectRatio )
{
  //search entries in mEntryLookup
  QgsSvgCacheEntry *currentEntry = nullptr;
  QList<QgsSvgCacheEntry *> entries = mEntryLookup.values( path );
  QDateTime modified;
  QList<QgsSvgCacheEntry *>::iterator entryIt = entries.begin();
  for ( ; entryIt != entries.end(); ++entryIt )
  {
    QgsSvgCacheEntry *cacheEntry = *entryIt;
    if ( qgsDoubleNear( cacheEntry->size, size ) && cacheEntry->fill == fill && cacheEntry->stroke == stroke &&
         qgsDoubleNear( cacheEntry->strokeWidth, strokeWidth ) && qgsDoubleNear( cacheEntry->widthScaleFactor, widthScaleFactor ) &&
         qgsDoubleNear( cacheEntry->fixedAspectRatio, fixedAspectRatio ) )
    {
      if ( mFileModifiedCheckTimeout <= 0 || cacheEntry->fileModifiedLastCheckTimer.hasExpired( mFileModifiedCheckTimeout ) )
      {
        if ( !modified.isValid() )
          modified = QFileInfo( path ).lastModified();

        if ( cacheEntry->fileModified != modified )
          continue;
      }
      currentEntry = cacheEntry;
      break;
    }
  }

  //if not found: create new entry
  //cache and replace params in svg content
  if ( !currentEntry )
  {
    currentEntry = insertSvg( path, size, fill, stroke, strokeWidth, widthScaleFactor, fixedAspectRatio );
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
      currentEntry->nextEntry = nullptr;
      mMostRecentEntry = currentEntry;
    }
  }

  //debugging
  //printEntryList();

  return currentEntry;
}

void QgsSvgCache::replaceElemParams( QDomElement &elem, const QColor &fill, const QColor &stroke, double strokeWidth )
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
    if ( attribute.name().compare( QLatin1String( "style" ), Qt::CaseInsensitive ) == 0 )
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
        if ( value.startsWith( QLatin1String( "param(fill)" ) ) )
        {
          value = fill.name();
        }
        else if ( value.startsWith( QLatin1String( "param(fill-opacity)" ) ) )
        {
          value = fill.alphaF();
        }
        else if ( value.startsWith( QLatin1String( "param(outline)" ) ) )
        {
          value = stroke.name();
        }
        else if ( value.startsWith( QLatin1String( "param(outline-opacity)" ) ) )
        {
          value = stroke.alphaF();
        }
        else if ( value.startsWith( QLatin1String( "param(outline-width)" ) ) )
        {
          value = QString::number( strokeWidth );
        }

        if ( entryIt != entryList.constBegin() )
        {
          newAttributeString.append( ';' );
        }
        newAttributeString.append( key + ':' + value );
      }
      elem.setAttribute( attribute.name(), newAttributeString );
    }
    else
    {
      QString value = attribute.value();
      if ( value.startsWith( QLatin1String( "param(fill)" ) ) )
      {
        elem.setAttribute( attribute.name(), fill.name() );
      }
      else if ( value.startsWith( QLatin1String( "param(fill-opacity)" ) ) )
      {
        elem.setAttribute( attribute.name(), fill.alphaF() );
      }
      else if ( value.startsWith( QLatin1String( "param(outline)" ) ) )
      {
        elem.setAttribute( attribute.name(), stroke.name() );
      }
      else if ( value.startsWith( QLatin1String( "param(outline-opacity)" ) ) )
      {
        elem.setAttribute( attribute.name(), stroke.alphaF() );
      }
      else if ( value.startsWith( QLatin1String( "param(outline-width)" ) ) )
      {
        elem.setAttribute( attribute.name(), QString::number( strokeWidth ) );
      }
    }
  }

  QDomNodeList childList = elem.childNodes();
  int nChildren = childList.count();
  for ( int i = 0; i < nChildren; ++i )
  {
    QDomElement childElem = childList.at( i ).toElement();
    replaceElemParams( childElem, fill, stroke, strokeWidth );
  }
}

void QgsSvgCache::containsElemParams( const QDomElement &elem, bool &hasFillParam, bool &hasDefaultFill, QColor &defaultFill,
                                      bool &hasFillOpacityParam, bool &hasDefaultFillOpacity, double &defaultFillOpacity,
                                      bool &hasStrokeParam, bool &hasDefaultStroke, QColor &defaultStroke,
                                      bool &hasStrokeWidthParam, bool &hasDefaultStrokeWidth, double &defaultStrokeWidth,
                                      bool &hasStrokeOpacityParam, bool &hasDefaultStrokeOpacity, double &defaultStrokeOpacity ) const
{
  if ( elem.isNull() )
  {
    return;
  }

  //we already have all the information, no need to go deeper
  if ( hasFillParam && hasStrokeParam && hasStrokeWidthParam && hasFillOpacityParam && hasStrokeOpacityParam )
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
    if ( attribute.name().compare( QLatin1String( "style" ), Qt::CaseInsensitive ) == 0 )
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
        QString value = keyValueSplit.at( 1 );
        valueSplit = value.split( ' ' );
        if ( !hasFillParam && value.startsWith( QLatin1String( "param(fill)" ) ) )
        {
          hasFillParam = true;
          if ( valueSplit.size() > 1 )
          {
            defaultFill = QColor( valueSplit.at( 1 ) );
            hasDefaultFill = true;
          }
        }
        else if ( !hasFillOpacityParam && value.startsWith( QLatin1String( "param(fill-opacity)" ) ) )
        {
          hasFillOpacityParam = true;
          if ( valueSplit.size() > 1 )
          {
            bool ok;
            double opacity = valueSplit.at( 1 ).toDouble( &ok );
            if ( ok )
            {
              defaultFillOpacity = opacity;
              hasDefaultFillOpacity = true;
            }
          }
        }
        else if ( !hasStrokeParam && value.startsWith( QLatin1String( "param(outline)" ) ) )
        {
          hasStrokeParam = true;
          if ( valueSplit.size() > 1 )
          {
            defaultStroke = QColor( valueSplit.at( 1 ) );
            hasDefaultStroke = true;
          }
        }
        else if ( !hasStrokeWidthParam && value.startsWith( QLatin1String( "param(outline-width)" ) ) )
        {
          hasStrokeWidthParam = true;
          if ( valueSplit.size() > 1 )
          {
            defaultStrokeWidth = valueSplit.at( 1 ).toDouble();
            hasDefaultStrokeWidth = true;
          }
        }
        else if ( !hasStrokeOpacityParam && value.startsWith( QLatin1String( "param(outline-opacity)" ) ) )
        {
          hasStrokeOpacityParam = true;
          if ( valueSplit.size() > 1 )
          {
            bool ok;
            double opacity = valueSplit.at( 1 ).toDouble( &ok );
            if ( ok )
            {
              defaultStrokeOpacity = opacity;
              hasDefaultStrokeOpacity = true;
            }
          }
        }
      }
    }
    else
    {
      QString value = attribute.value();
      valueSplit = value.split( ' ' );
      if ( !hasFillParam && value.startsWith( QLatin1String( "param(fill)" ) ) )
      {
        hasFillParam = true;
        if ( valueSplit.size() > 1 )
        {
          defaultFill = QColor( valueSplit.at( 1 ) );
          hasDefaultFill = true;
        }
      }
      else if ( !hasFillOpacityParam && value.startsWith( QLatin1String( "param(fill-opacity)" ) ) )
      {
        hasFillOpacityParam = true;
        if ( valueSplit.size() > 1 )
        {
          bool ok;
          double opacity = valueSplit.at( 1 ).toDouble( &ok );
          if ( ok )
          {
            defaultFillOpacity = opacity;
            hasDefaultFillOpacity = true;
          }
        }
      }
      else if ( !hasStrokeParam && value.startsWith( QLatin1String( "param(outline)" ) ) )
      {
        hasStrokeParam = true;
        if ( valueSplit.size() > 1 )
        {
          defaultStroke = QColor( valueSplit.at( 1 ) );
          hasDefaultStroke = true;
        }
      }
      else if ( !hasStrokeWidthParam && value.startsWith( QLatin1String( "param(outline-width)" ) ) )
      {
        hasStrokeWidthParam = true;
        if ( valueSplit.size() > 1 )
        {
          defaultStrokeWidth = valueSplit.at( 1 ).toDouble();
          hasDefaultStrokeWidth = true;
        }
      }
      else if ( !hasStrokeOpacityParam && value.startsWith( QLatin1String( "param(outline-opacity)" ) ) )
      {
        hasStrokeOpacityParam = true;
        if ( valueSplit.size() > 1 )
        {
          bool ok;
          double opacity = valueSplit.at( 1 ).toDouble( &ok );
          if ( ok )
          {
            defaultStrokeOpacity = opacity;
            hasDefaultStrokeOpacity = true;
          }
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
    containsElemParams( childElem, hasFillParam, hasDefaultFill, defaultFill,
                        hasFillOpacityParam, hasDefaultFillOpacity, defaultFillOpacity,
                        hasStrokeParam, hasDefaultStroke, defaultStroke,
                        hasStrokeWidthParam, hasDefaultStrokeWidth, defaultStrokeWidth,
                        hasStrokeOpacityParam, hasDefaultStrokeOpacity, defaultStrokeOpacity );
  }
}

void QgsSvgCache::removeCacheEntry( const QString &s, QgsSvgCacheEntry *entry )
{
  delete entry;
  mEntryLookup.remove( s, entry );
}

void QgsSvgCache::printEntryList()
{
  QgsDebugMsg( "****************svg cache entry list*************************" );
  QgsDebugMsg( "Cache size: " + QString::number( mTotalSize ) );
  QgsSvgCacheEntry *entry = mLeastRecentEntry;
  while ( entry )
  {
    QgsDebugMsg( "***Entry:" );
    QgsDebugMsg( "File:" + entry->path );
    QgsDebugMsg( "Size:" + QString::number( entry->size ) );
    QgsDebugMsg( "Width scale factor" + QString::number( entry->widthScaleFactor ) );
    entry = entry->nextEntry;
  }
}

QSize QgsSvgCache::sizeForImage( const QgsSvgCacheEntry &entry, QSizeF &viewBoxSize, QSizeF &scaledSize ) const
{
  bool isFixedAR = entry.fixedAspectRatio > 0;

  QSvgRenderer r( entry.svgContent );
  double hwRatio = 1.0;
  viewBoxSize = r.viewBoxF().size();
  if ( viewBoxSize.width() > 0 )
  {
    if ( isFixedAR )
    {
      hwRatio = entry.fixedAspectRatio;
    }
    else
    {
      hwRatio = viewBoxSize.height() / viewBoxSize.width();
    }
  }

  // cast double image sizes to int for QImage
  scaledSize.setWidth( entry.size );
  int wImgSize = static_cast< int >( scaledSize.width() );
  if ( wImgSize < 1 )
  {
    wImgSize = 1;
  }
  scaledSize.setHeight( scaledSize.width() * hwRatio );
  int hImgSize = static_cast< int >( scaledSize.height() );
  if ( hImgSize < 1 )
  {
    hImgSize = 1;
  }
  return QSize( wImgSize, hImgSize );
}

QImage QgsSvgCache::imageFromCachedPicture( const QgsSvgCacheEntry &entry ) const
{
  QSizeF viewBoxSize;
  QSizeF scaledSize;
  QImage image( sizeForImage( entry, viewBoxSize, scaledSize ), QImage::Format_ARGB32_Premultiplied );
  image.fill( 0 ); // transparent background

  QPainter p( &image );
  p.drawPicture( QPoint( 0, 0 ), *entry.picture );
  return image;
}

void QgsSvgCache::trimToMaximumSize()
{
  //only one entry in cache
  if ( mLeastRecentEntry == mMostRecentEntry )
  {
    return;
  }
  QgsSvgCacheEntry *entry = mLeastRecentEntry;
  while ( entry && ( mTotalSize > MAXIMUM_SIZE ) )
  {
    QgsSvgCacheEntry *bkEntry = entry;
    entry = entry->nextEntry;

    takeEntryFromList( bkEntry );
    mEntryLookup.remove( bkEntry->path, bkEntry );
    mTotalSize -= bkEntry->dataSize();
    delete bkEntry;
  }
}

void QgsSvgCache::takeEntryFromList( QgsSvgCacheEntry *entry )
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
  QString msg = tr( "%1 of %2 bytes of svg image downloaded." ).arg( bytesReceived ).arg( bytesTotal < 0 ? QStringLiteral( "unknown number of" ) : QString::number( bytesTotal ) );
  QgsDebugMsg( msg );
  emit statusChanged( msg );
}
