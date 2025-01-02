/***************************************************************************
    qgsannotationpictureitem.cpp
    ----------------
    begin                : July 2024
    copyright            : (C) 2024 by Nyall Dawson
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

#include "qgsannotationpictureitem.h"
#include "qgsapplication.h"
#include "qgsimagecache.h"
#include "qgssvgcache.h"
#include "qgsgeometry.h"
#include "qgsrendercontext.h"
#include "qgspainting.h"
#include "qgssymbollayerutils.h"
#include "qgscalloutsregistry.h"

#include <QFileInfo>

QgsAnnotationPictureItem::QgsAnnotationPictureItem( Qgis::PictureFormat format, const QString &path, const QgsRectangle &bounds )
  : QgsAnnotationRectItem( bounds )
{
  setPath( format, path );
}

QgsAnnotationPictureItem::~QgsAnnotationPictureItem() = default;

QString QgsAnnotationPictureItem::type() const
{
  return QStringLiteral( "picture" );
}

void QgsAnnotationPictureItem::renderInBounds( QgsRenderContext &context, const QRectF &painterBounds, QgsFeedback * )
{
  bool lockAspectRatio = mLockAspectRatio;
  bool fitsInCache = false;
  switch ( mFormat )
  {
    case Qgis::PictureFormat::SVG:
    {
      const QSizeF svgViewbox = QgsApplication::svgCache()->svgViewboxSize( mPath, painterBounds.width(), QColor(), QColor(), 1, context.scaleFactor() );
      double aspectRatio = painterBounds.height() / painterBounds.width();
      double svgWidth = painterBounds.width();
      if ( lockAspectRatio )
      {
        aspectRatio = svgViewbox.height() / svgViewbox.width();
        if ( ( painterBounds.height() / painterBounds.width() ) < aspectRatio )
        {
          svgWidth = painterBounds.height() / aspectRatio;
        }
      }

      const QPicture picture = QgsApplication::svgCache()->svgAsPicture( mPath, svgWidth, QColor(), QColor(), 1, context.scaleFactor(), context.forceVectorOutput(), aspectRatio );
      const double pictureWidth = picture.boundingRect().width();
      const double pictureHeight = picture.boundingRect().height();

      double xOffset = 0;
      if ( lockAspectRatio && static_cast< int >( painterBounds.width() ) > pictureWidth )
      {
        xOffset = ( painterBounds.width() - pictureWidth ) * 0.5;
      }
      double yOffset = 0;
      if ( lockAspectRatio && static_cast< int >( painterBounds.height() ) > pictureHeight )
      {
        yOffset = ( painterBounds.height() - pictureHeight ) * 0.5;
      }

      QgsPainting::drawPicture( context.painter(), QPointF( painterBounds.left() + pictureWidth / 2 + xOffset,
                                painterBounds.top() + pictureHeight / 2 + yOffset ), picture );

      break;
    }

    case Qgis::PictureFormat::Raster:
    {
      const QImage im = QgsApplication::imageCache()->pathAsImage( mPath,
                        QSize( static_cast< int >( std::round( painterBounds.width() ) ), static_cast< int >( std::round( painterBounds.height() ) ) ),
                        lockAspectRatio, 1, fitsInCache );
      double xOffset = 0;
      if ( lockAspectRatio && static_cast< int >( painterBounds.width() ) > im.width() )
      {
        xOffset = ( painterBounds.width() - im.width() ) * 0.5;
      }
      double yOffset = 0;
      if ( lockAspectRatio && static_cast< int >( painterBounds.height() ) > im.height() )
      {
        yOffset = ( painterBounds.height() - im.height() ) * 0.5;
      }
      context.painter()->drawImage( QPointF( painterBounds.left() + xOffset, painterBounds.top() + yOffset ), im );
      break;
    }

    case Qgis::PictureFormat::Unknown:
      break;
  }
}

bool QgsAnnotationPictureItem::writeXml( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context ) const
{
  element.setAttribute( QStringLiteral( "lockAspect" ), mLockAspectRatio ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );
  element.setAttribute( QStringLiteral( "path" ), mPath );
  element.setAttribute( QStringLiteral( "format" ), qgsEnumValueToKey( mFormat ) );
  writeCommonProperties( element, document, context );
  return true;
}

QgsAnnotationPictureItem *QgsAnnotationPictureItem::create()
{
  return new QgsAnnotationPictureItem( Qgis::PictureFormat::Unknown, QString(), QgsRectangle() );
}

bool QgsAnnotationPictureItem::readXml( const QDomElement &element, const QgsReadWriteContext &context )
{
  mLockAspectRatio = element.attribute( QStringLiteral( "lockAspect" ), QStringLiteral( "1" ) ).toInt();

  const Qgis::PictureFormat format = qgsEnumKeyToValue( element.attribute( QStringLiteral( "format" ) ), Qgis::PictureFormat::Unknown );
  setPath( format, element.attribute( QStringLiteral( "path" ) ) );

  readCommonProperties( element, context );
  return true;
}

QgsAnnotationPictureItem *QgsAnnotationPictureItem::clone() const
{
  std::unique_ptr< QgsAnnotationPictureItem > item = std::make_unique< QgsAnnotationPictureItem >( mFormat, mPath, bounds() );
  item->setLockAspectRatio( mLockAspectRatio );

  item->copyCommonProperties( this );
  return item.release();
}

void QgsAnnotationPictureItem::setPath( Qgis::PictureFormat format, const QString &path )
{
  mPath = path;
  mFormat = format;

  if ( path.isEmpty() )
  {
    mFormat = Qgis::PictureFormat::Unknown;
    return;
  }
}

bool QgsAnnotationPictureItem::lockAspectRatio() const
{
  return mLockAspectRatio;
}

void QgsAnnotationPictureItem::setLockAspectRatio( bool locked )
{
  mLockAspectRatio = locked;
}
