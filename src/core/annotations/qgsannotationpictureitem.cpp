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
#include "qgsannotationitemnode.h"
#include "qgsannotationitemeditoperation.h"
#include "qgspainting.h"
#include "qgsfillsymbol.h"
#include "qgssymbollayerutils.h"
#include "qgsfillsymbollayer.h"
#include "qgslinesymbollayer.h"

#include <QFileInfo>

QgsAnnotationPictureItem::QgsAnnotationPictureItem( Qgis::PictureFormat format, const QString &path, const QgsRectangle &bounds )
  : QgsAnnotationItem()
  , mBounds( bounds )
{
  setPath( format, path );

  mBackgroundSymbol = std::make_unique< QgsFillSymbol >(
                        QgsSymbolLayerList
  {
    new QgsSimpleFillSymbolLayer( QColor( 255, 255, 255 ), Qt::BrushStyle::SolidPattern, QColor( 0, 0, 0 ), Qt::PenStyle::NoPen )
  }
                      );
  QgsSimpleLineSymbolLayer *borderSymbol = new QgsSimpleLineSymbolLayer( QColor( 0, 0, 0 ) );
  borderSymbol->setPenJoinStyle( Qt::MiterJoin );
  mBorderSymbol = std::make_unique< QgsFillSymbol >(
                    QgsSymbolLayerList
  {
    borderSymbol
  }
                  );
}

QgsAnnotationPictureItem::~QgsAnnotationPictureItem() = default;

QString QgsAnnotationPictureItem::type() const
{
  return QStringLiteral( "picture" );
}

void QgsAnnotationPictureItem::render( QgsRenderContext &context, QgsFeedback * )
{
  QgsRectangle bounds = mBounds;
  if ( context.coordinateTransform().isValid() )
  {
    try
    {
      bounds = context.coordinateTransform().transformBoundingBox( mBounds );
    }
    catch ( QgsCsException & )
    {
      return;
    }
  }

  const QRectF painterBounds = context.mapToPixel().transformBounds( bounds.toRectF() );
  if ( painterBounds.width() < 1 || painterBounds.height() < 1 )
    return;

  if ( mDrawBackground && mBackgroundSymbol )
  {
    mBackgroundSymbol->startRender( context );
    mBackgroundSymbol->renderPolygon( painterBounds, nullptr, nullptr, context );
    mBackgroundSymbol->stopRender( context );
  }

  bool fitsInCache = false;
  switch ( mFormat )
  {
    case Qgis::PictureFormat::SVG:
    {
      const QSizeF svgViewbox = QgsApplication::svgCache()->svgViewboxSize( mPath, painterBounds.width(), QColor(), QColor(), 1, context.scaleFactor() );
      double aspectRatio = painterBounds.height() / painterBounds.width();
      double svgWidth = painterBounds.width();
      if ( mLockAspectRatio )
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
      if ( mLockAspectRatio && static_cast< int >( painterBounds.width() ) > pictureWidth )
      {
        xOffset = ( painterBounds.width() - pictureWidth ) * 0.5;
      }
      double yOffset = 0;
      if ( mLockAspectRatio && static_cast< int >( painterBounds.height() ) > pictureHeight )
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
                        mLockAspectRatio, 1, fitsInCache );
      double xOffset = 0;
      if ( mLockAspectRatio && static_cast< int >( painterBounds.width() ) > im.width() )
      {
        xOffset = ( painterBounds.width() - im.width() ) * 0.5;
      }
      double yOffset = 0;
      if ( mLockAspectRatio && static_cast< int >( painterBounds.height() ) > im.height() )
      {
        yOffset = ( painterBounds.height() - im.height() ) * 0.5;
      }
      context.painter()->drawImage( QPointF( painterBounds.left() + xOffset, painterBounds.top() + yOffset ), im );
      break;
    }

    case Qgis::PictureFormat::Unknown:
      break;
  }

  if ( mDrawBorder && mBorderSymbol )
  {
    mBorderSymbol->startRender( context );
    mBorderSymbol->renderPolygon( painterBounds, nullptr, nullptr, context );
    mBorderSymbol->stopRender( context );
  }

}

bool QgsAnnotationPictureItem::writeXml( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context ) const
{
  element.setAttribute( QStringLiteral( "lockAspect" ), mLockAspectRatio ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );
  element.setAttribute( QStringLiteral( "xMin" ), qgsDoubleToString( mBounds.xMinimum() ) );
  element.setAttribute( QStringLiteral( "xMax" ), qgsDoubleToString( mBounds.xMaximum() ) );
  element.setAttribute( QStringLiteral( "yMin" ), qgsDoubleToString( mBounds.yMinimum() ) );
  element.setAttribute( QStringLiteral( "yMax" ), qgsDoubleToString( mBounds.yMaximum() ) );
  element.setAttribute( QStringLiteral( "path" ), mPath );
  element.setAttribute( QStringLiteral( "format" ), qgsEnumValueToKey( mFormat ) );

  element.setAttribute( QStringLiteral( "backgroundEnabled" ), mDrawBackground ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );
  if ( mBackgroundSymbol )
  {
    QDomElement backgroundElement = document.createElement( QStringLiteral( "backgroundSymbol" ) );
    backgroundElement.appendChild( QgsSymbolLayerUtils::saveSymbol( QStringLiteral( "backgroundSymbol" ), mBackgroundSymbol.get(), document, context ) );
    element.appendChild( backgroundElement );
  }

  element.setAttribute( QStringLiteral( "frameEnabled" ), mDrawBorder ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );
  if ( mBorderSymbol )
  {
    QDomElement frameElement = document.createElement( QStringLiteral( "frameSymbol" ) );
    frameElement.appendChild( QgsSymbolLayerUtils::saveSymbol( QStringLiteral( "frameSymbol" ), mBorderSymbol.get(), document, context ) );
    element.appendChild( frameElement );
  }

  writeCommonProperties( element, document, context );
  return true;
}

QList<QgsAnnotationItemNode> QgsAnnotationPictureItem::nodes() const
{
  return
  {
    QgsAnnotationItemNode( QgsVertexId( 0, 0, 0 ), QgsPointXY( mBounds.xMinimum(), mBounds.yMinimum() ), Qgis::AnnotationItemNodeType::VertexHandle ),
    QgsAnnotationItemNode( QgsVertexId( 0, 0, 1 ), QgsPointXY( mBounds.xMaximum(), mBounds.yMinimum() ), Qgis::AnnotationItemNodeType::VertexHandle ),
    QgsAnnotationItemNode( QgsVertexId( 0, 0, 2 ), QgsPointXY( mBounds.xMaximum(), mBounds.yMaximum() ), Qgis::AnnotationItemNodeType::VertexHandle ),
    QgsAnnotationItemNode( QgsVertexId( 0, 0, 3 ), QgsPointXY( mBounds.xMinimum(), mBounds.yMaximum() ), Qgis::AnnotationItemNodeType::VertexHandle ),
  };
}

Qgis::AnnotationItemEditOperationResult QgsAnnotationPictureItem::applyEdit( QgsAbstractAnnotationItemEditOperation *operation )
{
  switch ( operation->type() )
  {
    case QgsAbstractAnnotationItemEditOperation::Type::MoveNode:
    {
      QgsAnnotationItemEditOperationMoveNode *moveOperation = dynamic_cast< QgsAnnotationItemEditOperationMoveNode * >( operation );
      switch ( moveOperation->nodeId().vertex )
      {
        case 0:
          mBounds = QgsRectangle( moveOperation->after().x(),
                                  moveOperation->after().y(),
                                  mBounds.xMaximum(),
                                  mBounds.yMaximum() );
          break;
        case 1:
          mBounds = QgsRectangle( mBounds.xMinimum(),
                                  moveOperation->after().y(),
                                  moveOperation->after().x(),
                                  mBounds.yMaximum() );
          break;
        case 2:
          mBounds = QgsRectangle( mBounds.xMinimum(),
                                  mBounds.yMinimum(),
                                  moveOperation->after().x(),
                                  moveOperation->after().y() );
          break;
        case 3:
          mBounds = QgsRectangle( moveOperation->after().x(),
                                  mBounds.yMinimum(),
                                  mBounds.xMaximum(),
                                  moveOperation->after().y() );
          break;
        default:
          break;
      }
      return Qgis::AnnotationItemEditOperationResult::Success;
    }

    case QgsAbstractAnnotationItemEditOperation::Type::TranslateItem:
    {
      QgsAnnotationItemEditOperationTranslateItem *moveOperation = qgis::down_cast< QgsAnnotationItemEditOperationTranslateItem * >( operation );
      mBounds = QgsRectangle( mBounds.xMinimum() + moveOperation->translationX(),
                              mBounds.yMinimum() + moveOperation->translationY(),
                              mBounds.xMaximum() + moveOperation->translationX(),
                              mBounds.yMaximum() + moveOperation->translationY() );
      return Qgis::AnnotationItemEditOperationResult::Success;
    }

    case QgsAbstractAnnotationItemEditOperation::Type::DeleteNode:
    case QgsAbstractAnnotationItemEditOperation::Type::AddNode:
      break;
  }
  return Qgis::AnnotationItemEditOperationResult::Invalid;
}

QgsAnnotationItemEditOperationTransientResults *QgsAnnotationPictureItem::transientEditResults( QgsAbstractAnnotationItemEditOperation *operation )
{
  switch ( operation->type() )
  {
    case QgsAbstractAnnotationItemEditOperation::Type::MoveNode:
    {
      QgsAnnotationItemEditOperationMoveNode *moveOperation = dynamic_cast< QgsAnnotationItemEditOperationMoveNode * >( operation );
      QgsRectangle modifiedBounds = mBounds;
      switch ( moveOperation->nodeId().vertex )
      {
        case 0:
          modifiedBounds.setXMinimum( moveOperation->after().x() );
          modifiedBounds.setYMinimum( moveOperation->after().y() );
          break;
        case 1:
          modifiedBounds.setXMaximum( moveOperation->after().x() );
          modifiedBounds.setYMinimum( moveOperation->after().y() );
          break;
        case 2:
          modifiedBounds.setXMaximum( moveOperation->after().x() );
          modifiedBounds.setYMaximum( moveOperation->after().y() );
          break;
        case 3:
          modifiedBounds.setXMinimum( moveOperation->after().x() );
          modifiedBounds.setYMaximum( moveOperation->after().y() );
          break;
        default:
          break;
      }

      return new QgsAnnotationItemEditOperationTransientResults( QgsGeometry::fromRect( modifiedBounds ) );
    }

    case QgsAbstractAnnotationItemEditOperation::Type::TranslateItem:
    {
      QgsAnnotationItemEditOperationTranslateItem *moveOperation = qgis::down_cast< QgsAnnotationItemEditOperationTranslateItem * >( operation );
      const QgsRectangle modifiedBounds( mBounds.xMinimum() + moveOperation->translationX(),
                                         mBounds.yMinimum() + moveOperation->translationY(),
                                         mBounds.xMaximum() + moveOperation->translationX(),
                                         mBounds.yMaximum() + moveOperation->translationY() );
      return new QgsAnnotationItemEditOperationTransientResults( QgsGeometry::fromRect( modifiedBounds ) );
    }

    case QgsAbstractAnnotationItemEditOperation::Type::DeleteNode:
    case QgsAbstractAnnotationItemEditOperation::Type::AddNode:
      break;
  }
  return nullptr;
}

QgsAnnotationPictureItem *QgsAnnotationPictureItem::create()
{
  return new QgsAnnotationPictureItem( Qgis::PictureFormat::Unknown, QString(), QgsRectangle() );
}

bool QgsAnnotationPictureItem::readXml( const QDomElement &element, const QgsReadWriteContext &context )
{
  mLockAspectRatio = element.attribute( QStringLiteral( "lockAspect" ), QStringLiteral( "1" ) ).toInt();
  mBounds.setXMinimum( element.attribute( QStringLiteral( "xMin" ) ).toDouble() );
  mBounds.setXMaximum( element.attribute( QStringLiteral( "xMax" ) ).toDouble() );
  mBounds.setYMinimum( element.attribute( QStringLiteral( "yMin" ) ).toDouble() );
  mBounds.setYMaximum( element.attribute( QStringLiteral( "yMax" ) ).toDouble() );

  const Qgis::PictureFormat format = qgsEnumKeyToValue( element.attribute( QStringLiteral( "format" ) ), Qgis::PictureFormat::Unknown );
  setPath( format, element.attribute( QStringLiteral( "path" ) ) );

  mDrawBackground = element.attribute( QStringLiteral( "backgroundEnabled" ), QStringLiteral( "0" ) ).toInt();
  const QDomElement backgroundSymbolElem = element.firstChildElement( QStringLiteral( "backgroundSymbol" ) ).firstChildElement();
  if ( !backgroundSymbolElem.isNull() )
  {
    setBackgroundSymbol( QgsSymbolLayerUtils::loadSymbol< QgsFillSymbol >( backgroundSymbolElem, context ) );
  }

  mDrawBorder = element.attribute( QStringLiteral( "frameEnabled" ), QStringLiteral( "0" ) ).toInt();
  const QDomElement frameSymbolElem = element.firstChildElement( QStringLiteral( "frameSymbol" ) ).firstChildElement();
  if ( !frameSymbolElem.isNull() )
  {
    setFrameSymbol( QgsSymbolLayerUtils::loadSymbol< QgsFillSymbol >( frameSymbolElem, context ) );
  }

  readCommonProperties( element, context );
  return true;
}

QgsAnnotationPictureItem *QgsAnnotationPictureItem::clone() const
{
  std::unique_ptr< QgsAnnotationPictureItem > item = std::make_unique< QgsAnnotationPictureItem >( mFormat, mPath, mBounds );
  item->setLockAspectRatio( mLockAspectRatio );

  item->setBackgroundEnabled( mDrawBackground );
  if ( mBackgroundSymbol )
    item->setBackgroundSymbol( mBackgroundSymbol->clone() );

  item->setFrameEnabled( mDrawBorder );
  if ( mBorderSymbol )
    item->setFrameSymbol( mBorderSymbol->clone() );

  item->copyCommonProperties( this );
  return item.release();
}

QgsRectangle QgsAnnotationPictureItem::boundingBox() const
{
  return mBounds;
}

void QgsAnnotationPictureItem::setBounds( const QgsRectangle &bounds )
{
  mBounds = bounds;
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

const QgsFillSymbol *QgsAnnotationPictureItem::backgroundSymbol() const
{
  return mBackgroundSymbol.get();
}

void QgsAnnotationPictureItem::setBackgroundSymbol( QgsFillSymbol *symbol )
{
  mBackgroundSymbol.reset( symbol );
}

const QgsFillSymbol *QgsAnnotationPictureItem::frameSymbol() const
{
  return mBorderSymbol.get();
}

void QgsAnnotationPictureItem::setFrameSymbol( QgsFillSymbol *symbol )
{
  mBorderSymbol.reset( symbol );
}
