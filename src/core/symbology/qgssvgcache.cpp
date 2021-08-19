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
#include <QRegularExpression>
#include <QSvgRenderer>
#include <QFileInfo>
#include <QNetworkReply>
#include <QNetworkRequest>

///@cond PRIVATE

//
// QgsSvgCacheEntry
//

QgsSvgCacheEntry::QgsSvgCacheEntry( const QString &path, double size, double strokeWidth, double widthScaleFactor, const QColor &fill, const QColor &stroke, double fixedAspectRatio, const QMap<QString, QString> &parameters )
  : QgsAbstractContentCacheEntry( path )
  , size( size )
  , strokeWidth( strokeWidth )
  , widthScaleFactor( widthScaleFactor )
  , fixedAspectRatio( fixedAspectRatio )
  , fill( fill )
  , stroke( stroke )
  , parameters( parameters )
{
}

bool QgsSvgCacheEntry::isEqual( const QgsAbstractContentCacheEntry *other ) const
{
  const QgsSvgCacheEntry *otherSvg = dynamic_cast< const QgsSvgCacheEntry * >( other );
  // cheapest checks first!
  if ( !otherSvg
       || !qgsDoubleNear( otherSvg->fixedAspectRatio, fixedAspectRatio )
       || !qgsDoubleNear( otherSvg->size, size )
       || !qgsDoubleNear( otherSvg->strokeWidth, strokeWidth )
       || !qgsDoubleNear( otherSvg->widthScaleFactor, widthScaleFactor )
       || otherSvg->fill != fill
       || otherSvg->stroke != stroke
       || otherSvg->path != path
       || otherSvg->parameters != parameters )
    return false;

  return true;
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

void QgsSvgCacheEntry::dump() const
{
  QgsDebugMsgLevel( QStringLiteral( "path: %1, size %2, width scale factor %3" ).arg( path ).arg( size ).arg( widthScaleFactor ), 4 );
}
///@endcond


//
// QgsSvgCache
//

QgsSvgCache::QgsSvgCache( QObject *parent )
  : QgsAbstractContentCache< QgsSvgCacheEntry >( parent, QObject::tr( "SVG" ) )
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

  connect( this, &QgsAbstractContentCacheBase::remoteContentFetched, this, &QgsSvgCache::remoteSvgFetched );
}

QImage QgsSvgCache::svgAsImage( const QString &file, double size, const QColor &fill, const QColor &stroke, double strokeWidth,
                                double widthScaleFactor, bool &fitsInCache, double fixedAspectRatio, bool blocking, const QMap<QString, QString> &parameters )
{
  const QMutexLocker locker( &mMutex );

  fitsInCache = true;
  QgsSvgCacheEntry *currentEntry = cacheEntry( file, size, fill, stroke, strokeWidth, widthScaleFactor, fixedAspectRatio, parameters, blocking );

  QImage result;

  //if current entry image is 0: cache image for entry
  // checks to see if image will fit into cache
  //update stats for memory usage
  if ( !currentEntry->image )
  {
    const QSvgRenderer r( currentEntry->svgContent );
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
    if ( cachedDataSize > mMaxCacheSize / 2 )
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
                                    double widthScaleFactor, bool forceVectorOutput, double fixedAspectRatio, bool blocking, const QMap<QString, QString> &parameters )
{
  const QMutexLocker locker( &mMutex );

  QgsSvgCacheEntry *currentEntry = cacheEntry( path, size, fill, stroke, strokeWidth, widthScaleFactor, fixedAspectRatio, parameters, blocking );

  //if current entry picture is 0: cache picture for entry
  //update stats for memory usage
  if ( !currentEntry->picture )
  {
    cachePicture( currentEntry, forceVectorOutput );
    trimToMaximumSize();
  }

  QPicture p;
  // For some reason p.detach() doesn't seem to always work as intended, at
  // least with QT 5.5 on Ubuntu 16.04
  // Serialization/deserialization is a safe way to be ensured we don't
  // share a copy.
  p.setData( currentEntry->picture->data(), currentEntry->picture->size() );
  return p;
}

QByteArray QgsSvgCache::svgContent( const QString &path, double size, const QColor &fill, const QColor &stroke, double strokeWidth,
                                    double widthScaleFactor, double fixedAspectRatio, bool blocking, const QMap<QString, QString> &parameters, bool *isMissingImage )
{
  const QMutexLocker locker( &mMutex );

  QgsSvgCacheEntry *currentEntry = cacheEntry( path, size, fill, stroke, strokeWidth, widthScaleFactor, fixedAspectRatio, parameters, blocking, isMissingImage );

  return currentEntry->svgContent;
}

QSizeF QgsSvgCache::svgViewboxSize( const QString &path, double size, const QColor &fill, const QColor &stroke, double strokeWidth,
                                    double widthScaleFactor, double fixedAspectRatio, bool blocking, const QMap<QString, QString> &parameters )
{
  const QMutexLocker locker( &mMutex );

  QgsSvgCacheEntry *currentEntry = cacheEntry( path, size, fill, stroke, strokeWidth, widthScaleFactor, fixedAspectRatio, parameters, blocking );
  return currentEntry->viewboxSize;
}

void QgsSvgCache::containsParams( const QString &path, bool &hasFillParam, QColor &defaultFillColor, bool &hasStrokeParam, QColor &defaultStrokeColor,
                                  bool &hasStrokeWidthParam, double &defaultStrokeWidth, bool blocking ) const
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
                  hasStrokeOpacityParam, hasDefaultStrokeOpacity, defaultStrokeOpacity,
                  blocking );
}

void QgsSvgCache::containsParams( const QString &path,
                                  bool &hasFillParam, bool &hasDefaultFillParam, QColor &defaultFillColor,
                                  bool &hasFillOpacityParam, bool &hasDefaultFillOpacity, double &defaultFillOpacity,
                                  bool &hasStrokeParam, bool &hasDefaultStrokeColor, QColor &defaultStrokeColor,
                                  bool &hasStrokeWidthParam, bool &hasDefaultStrokeWidth, double &defaultStrokeWidth,
                                  bool &hasStrokeOpacityParam, bool &hasDefaultStrokeOpacity, double &defaultStrokeOpacity,
                                  bool blocking ) const
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
  if ( !svgDoc.setContent( getContent( path, mMissingSvg, mFetchingSvg, blocking ) ) )
  {
    return;
  }

  const QDomElement docElem = svgDoc.documentElement();
  containsElemParams( docElem, hasFillParam, hasDefaultFillParam, defaultFillColor,
                      hasFillOpacityParam, hasDefaultFillOpacity, defaultFillOpacity,
                      hasStrokeParam, hasDefaultStrokeColor, defaultStrokeColor,
                      hasStrokeWidthParam, hasDefaultStrokeWidth, defaultStrokeWidth,
                      hasStrokeOpacityParam, hasDefaultStrokeOpacity, defaultStrokeOpacity );
}

void QgsSvgCache::replaceParamsAndCacheSvg( QgsSvgCacheEntry *entry, bool blocking )
{
  if ( !entry )
  {
    return;
  }

  const QByteArray content = getContent( entry->path, mMissingSvg, mFetchingSvg, blocking ) ;
  entry->isMissingImage = content == mMissingSvg;
  QDomDocument svgDoc;
  if ( !svgDoc.setContent( content ) )
  {
    return;
  }

  //replace fill color, stroke color, stroke with in all nodes
  QDomElement docElem = svgDoc.documentElement();

  QSizeF viewboxSize;
  const double sizeScaleFactor = calcSizeScaleFactor( entry, docElem, viewboxSize );
  entry->viewboxSize = viewboxSize;
  replaceElemParams( docElem, entry->fill, entry->stroke, entry->strokeWidth * sizeScaleFactor, entry->parameters );

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
    const QDomElement svgElem = docElem.firstChildElement( QStringLiteral( "svg" ) );
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
  {
    // trying looking for width/height and use them as a fallback
    if ( docElem.tagName() == QLatin1String( "svg" ) && docElem.hasAttribute( QStringLiteral( "width" ) ) )
    {
      const QString widthString = docElem.attribute( QStringLiteral( "width" ) );
      const QRegularExpression measureRegEx( QStringLiteral( "([\\d\\.]+).*?$" ) );
      const QRegularExpressionMatch widthMatch = measureRegEx.match( widthString );
      if ( widthMatch.hasMatch() )
      {
        const double width = widthMatch.captured( 1 ).toDouble();
        const QString heightString = docElem.attribute( QStringLiteral( "height" ) );

        const QRegularExpressionMatch heightMatch = measureRegEx.match( heightString );
        if ( heightMatch.hasMatch() )
        {
          const double height = heightMatch.captured( 1 ).toDouble();
          viewboxSize = QSizeF( width, height );
          return width / entry->size;
        }
      }
    }

    return 1.0;
  }


  //width should be 3rd element in a 4 part space delimited string
  const QStringList parts = viewBox.split( ' ' );
  if ( parts.count() != 4 )
    return 1.0;

  bool heightOk = false;
  const double height = parts.at( 3 ).toDouble( &heightOk );

  bool widthOk = false;
  const double width = parts.at( 2 ).toDouble( &widthOk );
  if ( widthOk )
  {
    if ( heightOk )
      viewboxSize = QSizeF( width, height );
    return width / entry->size;
  }

  return 1.0;
}


QByteArray QgsSvgCache::getImageData( const QString &path, bool blocking ) const
{
  return getContent( path, mMissingSvg, mFetchingSvg, blocking );
};

bool QgsSvgCache::checkReply( QNetworkReply *reply, const QString &path ) const
{
  // we accept both real SVG mime types AND plain text types - because some sites
  // (notably github) serve up svgs as raw text
  const QString contentType = reply->header( QNetworkRequest::ContentTypeHeader ).toString();
  if ( !contentType.startsWith( QLatin1String( "image/svg+xml" ), Qt::CaseInsensitive )
       && !contentType.startsWith( QLatin1String( "text/plain" ), Qt::CaseInsensitive ) )
  {
    QgsMessageLog::logMessage( tr( "Unexpected MIME type %1 received for %2" ).arg( contentType, path ), tr( "SVG" ) );
    return false;
  }
  return true;
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
  const QSize imageSize = sizeForImage( *entry, viewBoxSize, scaledSize );

  // cast double image sizes to int for QImage
  std::unique_ptr< QImage > image = std::make_unique< QImage >( imageSize, QImage::Format_ARGB32_Premultiplied );
  image->fill( 0 ); // transparent background

  const bool isFixedAR = entry->fixedAspectRatio > 0;

  QPainter p( image.get() );
  QSvgRenderer r( entry->svgContent );
  if ( qgsDoubleNear( viewBoxSize.width(), viewBoxSize.height() ) )
  {
    r.render( &p );
  }
  else
  {
    QSizeF s( viewBoxSize );
    s.scale( scaledSize.width(), scaledSize.height(), isFixedAR ? Qt::IgnoreAspectRatio : Qt::KeepAspectRatio );
    const QRectF rect( ( imageSize.width() - s.width() ) / 2, ( imageSize.height() - s.height() ) / 2, s.width(), s.height() );
    r.render( &p, rect );
  }

  mTotalSize += ( image->width() * image->height() * 32 );
  entry->image = std::move( image );
}

void QgsSvgCache::cachePicture( QgsSvgCacheEntry *entry, bool forceVectorOutput )
{
  Q_UNUSED( forceVectorOutput )
  if ( !entry )
  {
    return;
  }

  entry->picture.reset();

  const bool isFixedAR = entry->fixedAspectRatio > 0;

  //correct QPictures dpi correction
  std::unique_ptr< QPicture > picture = std::make_unique< QPicture >();
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

  const double wSize = entry->size;
  const double hSize = wSize * hwRatio;

  QSizeF s( r.viewBoxF().size() );
  s.scale( wSize, hSize, isFixedAR ? Qt::IgnoreAspectRatio : Qt::KeepAspectRatio );
  rect = QRectF( -s.width() / 2.0, -s.height() / 2.0, s.width(), s.height() );

  QPainter p( picture.get() );
  r.render( &p, rect );
  entry->picture = std::move( picture );
  mTotalSize += entry->picture->size();
}

QgsSvgCacheEntry *QgsSvgCache::cacheEntry( const QString &path, double size, const QColor &fill, const QColor &stroke, double strokeWidth,
    double widthScaleFactor, double fixedAspectRatio, const QMap<QString, QString> &parameters, bool blocking, bool *isMissingImage )
{
  QgsSvgCacheEntry *currentEntry = findExistingEntry( new QgsSvgCacheEntry( path, size, strokeWidth, widthScaleFactor, fill, stroke, fixedAspectRatio, parameters ) );

  if ( currentEntry->svgContent.isEmpty() )
  {
    replaceParamsAndCacheSvg( currentEntry, blocking );
  }

  if ( isMissingImage )
    *isMissingImage = currentEntry->isMissingImage;

  return currentEntry;
}


void QgsSvgCache::replaceElemParams( QDomElement &elem, const QColor &fill, const QColor &stroke, double strokeWidth, const QMap<QString, QString> &parameters )
{
  if ( elem.isNull() )
  {
    return;
  }

  //go through attributes
  const QDomNamedNodeMap attributes = elem.attributes();
  const int nAttributes = attributes.count();
  for ( int i = 0; i < nAttributes; ++i )
  {
    const QDomAttr attribute = attributes.item( i ).toAttr();
    //e.g. style="fill:param(fill);param(stroke)"
    if ( attribute.name().compare( QLatin1String( "style" ), Qt::CaseInsensitive ) == 0 )
    {
      //entries separated by ';'
      QString newAttributeString;

      const QStringList entryList = attribute.value().split( ';' );
      QStringList::const_iterator entryIt = entryList.constBegin();
      for ( ; entryIt != entryList.constEnd(); ++entryIt )
      {
        const QStringList keyValueSplit = entryIt->split( ':' );
        if ( keyValueSplit.size() < 2 )
        {
          continue;
        }
        const QString key = keyValueSplit.at( 0 );
        QString value = keyValueSplit.at( 1 );
        QString newValue = value;
        value = value.trimmed().toLower();

        if ( value.startsWith( QLatin1String( "param(fill)" ) ) )
        {
          newValue = fill.name();
        }
        else if ( value.startsWith( QLatin1String( "param(fill-opacity)" ) ) )
        {
          newValue = QString::number( fill.alphaF() );
        }
        else if ( value.startsWith( QLatin1String( "param(outline)" ) ) )
        {
          newValue = stroke.name();
        }
        else if ( value.startsWith( QLatin1String( "param(outline-opacity)" ) ) )
        {
          newValue = QString::number( stroke.alphaF() );
        }
        else if ( value.startsWith( QLatin1String( "param(outline-width)" ) ) )
        {
          newValue = QString::number( strokeWidth );
        }

        if ( entryIt != entryList.constBegin() )
        {
          newAttributeString.append( ';' );
        }
        newAttributeString.append( key + ':' + newValue );
      }
      elem.setAttribute( attribute.name(), newAttributeString );
    }
    else
    {
      const QString value = attribute.value().trimmed().toLower();
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
      else
      {
        QMap<QString, QString>::const_iterator paramIt = parameters.constBegin();
        for ( ; paramIt != parameters.constEnd(); ++paramIt )
        {
          if ( value.startsWith( QString( QLatin1String( "param(%1)" ) ).arg( paramIt.key() ) ) )
          {
            elem.setAttribute( attribute.name(), paramIt.value() );
            break;
          }
        }
      }
    }
  }

  QDomNode child = elem.firstChild();
  if ( child.isText() && child.nodeValue().startsWith( "param(" ) )
  {
    QMap<QString, QString>::const_iterator paramIt = parameters.constBegin();
    for ( ; paramIt != parameters.constEnd(); ++paramIt )
    {
      if ( child.toText().data().startsWith( QString( QLatin1String( "param(%1)" ) ).arg( paramIt.key() ) ) )
      {
        child.setNodeValue( paramIt.value() );
        break;
      }
    }
  }

  const QDomNodeList childList = elem.childNodes();
  const int nChildren = childList.count();
  for ( int i = 0; i < nChildren; ++i )
  {
    QDomElement childElem = childList.at( i ).toElement();
    replaceElemParams( childElem, fill, stroke, strokeWidth, parameters );
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
  const QDomNamedNodeMap attributes = elem.attributes();
  const int nAttributes = attributes.count();

  QStringList valueSplit;
  for ( int i = 0; i < nAttributes; ++i )
  {
    const QDomAttr attribute = attributes.item( i ).toAttr();
    if ( attribute.name().compare( QLatin1String( "style" ), Qt::CaseInsensitive ) == 0 )
    {
      //entries separated by ';'
      const QStringList entryList = attribute.value().split( ';' );
      QStringList::const_iterator entryIt = entryList.constBegin();
      for ( ; entryIt != entryList.constEnd(); ++entryIt )
      {
        const QStringList keyValueSplit = entryIt->split( ':' );
        if ( keyValueSplit.size() < 2 )
        {
          continue;
        }
        const QString value = keyValueSplit.at( 1 );
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
            const double opacity = valueSplit.at( 1 ).toDouble( &ok );
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
            const double opacity = valueSplit.at( 1 ).toDouble( &ok );
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
      const QString value = attribute.value();
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
          const double opacity = valueSplit.at( 1 ).toDouble( &ok );
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
          const double opacity = valueSplit.at( 1 ).toDouble( &ok );
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
  const QDomNodeList childList = elem.childNodes();
  const int nChildren = childList.count();
  for ( int i = 0; i < nChildren; ++i )
  {
    const QDomElement childElem = childList.at( i ).toElement();
    containsElemParams( childElem, hasFillParam, hasDefaultFill, defaultFill,
                        hasFillOpacityParam, hasDefaultFillOpacity, defaultFillOpacity,
                        hasStrokeParam, hasDefaultStroke, defaultStroke,
                        hasStrokeWidthParam, hasDefaultStrokeWidth, defaultStrokeWidth,
                        hasStrokeOpacityParam, hasDefaultStrokeOpacity, defaultStrokeOpacity );
  }
}

QSize QgsSvgCache::sizeForImage( const QgsSvgCacheEntry &entry, QSizeF &viewBoxSize, QSizeF &scaledSize ) const
{
  const bool isFixedAR = entry.fixedAspectRatio > 0;

  const QSvgRenderer r( entry.svgContent );
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

