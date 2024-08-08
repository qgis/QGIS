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
#include "qgsunittypes.h"
#include "qgscalloutsregistry.h"
#include "qgslinestring.h"
#include "qgspolygon.h"

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
  mFrameSymbol = std::make_unique< QgsFillSymbol >( QgsSymbolLayerList{ borderSymbol } );
}

QgsAnnotationPictureItem::~QgsAnnotationPictureItem() = default;

QString QgsAnnotationPictureItem::type() const
{
  return QStringLiteral( "picture" );
}

Qgis::AnnotationItemFlags QgsAnnotationPictureItem::flags() const
{
  switch ( mSizeMode )
  {
    case Qgis::AnnotationPictureSizeMode::SpatialBounds:
      return Qgis::AnnotationItemFlag::SupportsCallouts;
    case Qgis::AnnotationPictureSizeMode::FixedSize:
      return Qgis::AnnotationItemFlag::ScaleDependentBoundingBox
             | Qgis::AnnotationItemFlag::SupportsCallouts;
  }
  BUILTIN_UNREACHABLE
}

void QgsAnnotationPictureItem::render( QgsRenderContext &context, QgsFeedback *feedback )
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

  bool lockAspectRatio = mLockAspectRatio;
  QRectF painterBounds;

  switch ( mSizeMode )
  {
    case Qgis::AnnotationPictureSizeMode::SpatialBounds:
      painterBounds = context.mapToPixel().transformBounds( bounds.toRectF() );
      break;

    case Qgis::AnnotationPictureSizeMode::FixedSize:
    {
      const double widthPixels = context.convertToPainterUnits( mFixedSize.width(), mFixedSizeUnit );
      const double heightPixels = context.convertToPainterUnits( mFixedSize.height(), mFixedSizeUnit );

      if ( callout() && !calloutAnchor().isEmpty() )
      {
        QgsGeometry anchor = calloutAnchor();

        const double calloutOffsetWidthPixels = context.convertToPainterUnits( offsetFromCallout().width(), offsetFromCalloutUnit() );
        const double calloutOffsetHeightPixels = context.convertToPainterUnits( offsetFromCallout().height(), offsetFromCalloutUnit() );

        QPointF anchorPoint = anchor.asQPointF();
        if ( context.coordinateTransform().isValid() )
        {
          double x = anchorPoint.x();
          double y = anchorPoint.y();
          double z = 0.0;
          context.coordinateTransform().transformInPlace( x, y, z );
          anchorPoint = QPointF( x, y );
        }

        context.mapToPixel().transformInPlace( anchorPoint.rx(), anchorPoint.ry() );

        painterBounds = QRectF( anchorPoint.x() + calloutOffsetWidthPixels,
                                anchorPoint.y() + calloutOffsetHeightPixels, widthPixels, heightPixels );
      }
      else
      {
        QPointF center = bounds.center().toQPointF();

        context.mapToPixel().transformInPlace( center.rx(), center.ry() );
        painterBounds = QRectF( center.x() - widthPixels * 0.5,
                                center.y() - heightPixels * 0.5,
                                widthPixels, heightPixels );
      }
      break;
    }
  }

  if ( painterBounds.width() < 1 || painterBounds.height() < 1 )
    return;

  if ( mDrawBackground && mBackgroundSymbol )
  {
    mBackgroundSymbol->startRender( context );
    mBackgroundSymbol->renderPolygon( painterBounds, nullptr, nullptr, context );
    mBackgroundSymbol->stopRender( context );
  }

  if ( callout() )
  {
    QgsCallout::QgsCalloutContext calloutContext;
    renderCallout( context, painterBounds, 0, calloutContext, feedback );
  }

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

  if ( mDrawFrame && mFrameSymbol )
  {
    mFrameSymbol->startRender( context );
    mFrameSymbol->renderPolygon( painterBounds, nullptr, nullptr, context );
    mFrameSymbol->stopRender( context );
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
  element.setAttribute( QStringLiteral( "sizeMode" ), qgsEnumValueToKey( mSizeMode ) );
  element.setAttribute( QStringLiteral( "fixedWidth" ), qgsDoubleToString( mFixedSize.width() ) );
  element.setAttribute( QStringLiteral( "fixedHeight" ), qgsDoubleToString( mFixedSize.height() ) );
  element.setAttribute( QStringLiteral( "fixedSizeUnit" ), QgsUnitTypes::encodeUnit( mFixedSizeUnit ) );

  element.setAttribute( QStringLiteral( "backgroundEnabled" ), mDrawBackground ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );
  if ( mBackgroundSymbol )
  {
    QDomElement backgroundElement = document.createElement( QStringLiteral( "backgroundSymbol" ) );
    backgroundElement.appendChild( QgsSymbolLayerUtils::saveSymbol( QStringLiteral( "backgroundSymbol" ), mBackgroundSymbol.get(), document, context ) );
    element.appendChild( backgroundElement );
  }

  element.setAttribute( QStringLiteral( "frameEnabled" ), mDrawFrame ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );
  if ( mFrameSymbol )
  {
    QDomElement frameElement = document.createElement( QStringLiteral( "frameSymbol" ) );
    frameElement.appendChild( QgsSymbolLayerUtils::saveSymbol( QStringLiteral( "frameSymbol" ), mFrameSymbol.get(), document, context ) );
    element.appendChild( frameElement );
  }

  writeCommonProperties( element, document, context );
  return true;
}

QList<QgsAnnotationItemNode> QgsAnnotationPictureItem::nodesV2( const QgsAnnotationItemEditContext &context ) const
{
  QList<QgsAnnotationItemNode> res;
  switch ( mSizeMode )
  {
    case Qgis::AnnotationPictureSizeMode::SpatialBounds:
    {
      res =
      {
        QgsAnnotationItemNode( QgsVertexId( 0, 0, 0 ), QgsPointXY( mBounds.xMinimum(), mBounds.yMinimum() ), Qgis::AnnotationItemNodeType::VertexHandle ),
        QgsAnnotationItemNode( QgsVertexId( 0, 0, 1 ), QgsPointXY( mBounds.xMaximum(), mBounds.yMinimum() ), Qgis::AnnotationItemNodeType::VertexHandle ),
        QgsAnnotationItemNode( QgsVertexId( 0, 0, 2 ), QgsPointXY( mBounds.xMaximum(), mBounds.yMaximum() ), Qgis::AnnotationItemNodeType::VertexHandle ),
        QgsAnnotationItemNode( QgsVertexId( 0, 0, 3 ), QgsPointXY( mBounds.xMinimum(), mBounds.yMaximum() ), Qgis::AnnotationItemNodeType::VertexHandle ),
      };

      QgsPointXY calloutNodePoint;
      if ( !calloutAnchor().isEmpty() )
      {
        calloutNodePoint = calloutAnchor().asPoint();
      }
      else
      {
        calloutNodePoint = mBounds.center();
      }
      res.append( QgsAnnotationItemNode( QgsVertexId( 1, 0, 0 ), calloutNodePoint, Qgis::AnnotationItemNodeType::CalloutHandle ) );

      return res;
    }

    case Qgis::AnnotationPictureSizeMode::FixedSize:
    {
      res =
      {
        QgsAnnotationItemNode( QgsVertexId( 0, 0, 0 ), mBounds.center(), Qgis::AnnotationItemNodeType::VertexHandle )
      };

      QgsPointXY calloutNodePoint;
      if ( !calloutAnchor().isEmpty() )
      {
        calloutNodePoint = calloutAnchor().asPoint();
      }
      else
      {
        calloutNodePoint = QgsPointXY( context.currentItemBounds().xMinimum(), context.currentItemBounds().yMinimum() );
      }
      res.append( QgsAnnotationItemNode( QgsVertexId( 1, 0, 0 ), calloutNodePoint, Qgis::AnnotationItemNodeType::CalloutHandle ) );

      return res;
    }
  }
  BUILTIN_UNREACHABLE
}

Qgis::AnnotationItemEditOperationResult QgsAnnotationPictureItem::applyEditV2( QgsAbstractAnnotationItemEditOperation *operation, const QgsAnnotationItemEditContext &context )
{
  switch ( operation->type() )
  {
    case QgsAbstractAnnotationItemEditOperation::Type::MoveNode:
    {
      QgsAnnotationItemEditOperationMoveNode *moveOperation = dynamic_cast< QgsAnnotationItemEditOperationMoveNode * >( operation );
      if ( moveOperation->nodeId().part == 0 )
      {
        switch ( mSizeMode )
        {
          case Qgis::AnnotationPictureSizeMode::SpatialBounds:
          {
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

          case Qgis::AnnotationPictureSizeMode::FixedSize:
          {
            mBounds = QgsRectangle::fromCenterAndSize( moveOperation->after(),
                      mBounds.width(),
                      mBounds.height() );
            return Qgis::AnnotationItemEditOperationResult::Success;
          }
        }
      }
      else if ( moveOperation->nodeId().part == 1 )
      {
        setCalloutAnchor( QgsGeometry::fromPoint( moveOperation->after() ) );
        if ( !callout() )
        {
          setCallout( QgsApplication::calloutRegistry()->defaultCallout() );
        }
        return Qgis::AnnotationItemEditOperationResult::Success;
      }
      break;
    }

    case QgsAbstractAnnotationItemEditOperation::Type::TranslateItem:
    {
      QgsAnnotationItemEditOperationTranslateItem *moveOperation = qgis::down_cast< QgsAnnotationItemEditOperationTranslateItem * >( operation );
      switch ( mSizeMode )
      {

        case Qgis::AnnotationPictureSizeMode::SpatialBounds:
          mBounds = QgsRectangle( mBounds.xMinimum() + moveOperation->translationX(),
                                  mBounds.yMinimum() + moveOperation->translationY(),
                                  mBounds.xMaximum() + moveOperation->translationX(),
                                  mBounds.yMaximum() + moveOperation->translationY() );
          break;

        case Qgis::AnnotationPictureSizeMode::FixedSize:
        {
          if ( callout() && !calloutAnchor().isEmpty() )
          {
            const double xOffset = context.renderContext().convertFromPainterUnits( moveOperation->translationXPixels(), offsetFromCalloutUnit() );
            const double yOffset = context.renderContext().convertFromPainterUnits( moveOperation->translationYPixels(), offsetFromCalloutUnit() );
            setOffsetFromCallout( QSizeF( offsetFromCallout().width() + xOffset, offsetFromCallout().height() + yOffset ) );
          }
          else
          {
            mBounds = QgsRectangle( mBounds.xMinimum() + moveOperation->translationX(),
                                    mBounds.yMinimum() + moveOperation->translationY(),
                                    mBounds.xMaximum() + moveOperation->translationX(),
                                    mBounds.yMaximum() + moveOperation->translationY() );
          }
          break;
        }
      }
      return Qgis::AnnotationItemEditOperationResult::Success;
    }

    case QgsAbstractAnnotationItemEditOperation::Type::DeleteNode:
    case QgsAbstractAnnotationItemEditOperation::Type::AddNode:
      break;
  }
  return Qgis::AnnotationItemEditOperationResult::Invalid;
}

QgsAnnotationItemEditOperationTransientResults *QgsAnnotationPictureItem::transientEditResultsV2( QgsAbstractAnnotationItemEditOperation *operation, const QgsAnnotationItemEditContext &context )
{
  switch ( operation->type() )
  {
    case QgsAbstractAnnotationItemEditOperation::Type::MoveNode:
    {
      QgsAnnotationItemEditOperationMoveNode *moveOperation = dynamic_cast< QgsAnnotationItemEditOperationMoveNode * >( operation );
      if ( moveOperation->nodeId().part == 0 )
      {
        switch ( mSizeMode )
        {
          case Qgis::AnnotationPictureSizeMode::SpatialBounds:
          {
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
          case Qgis::AnnotationPictureSizeMode::FixedSize:
          {
            const QgsRectangle currentBounds = context.currentItemBounds();
            const QgsRectangle newBounds = QgsRectangle::fromCenterAndSize( moveOperation->after(), currentBounds.width(), currentBounds.height() );
            return new QgsAnnotationItemEditOperationTransientResults( QgsGeometry::fromRect( newBounds ) );
          }
        }
      }
      else
      {
        QgsAnnotationItemEditOperationMoveNode *moveOperation = dynamic_cast< QgsAnnotationItemEditOperationMoveNode * >( operation );
        return new QgsAnnotationItemEditOperationTransientResults( QgsGeometry( moveOperation->after().clone() ) );
      }
      break;
    }

    case QgsAbstractAnnotationItemEditOperation::Type::TranslateItem:
    {
      QgsAnnotationItemEditOperationTranslateItem *moveOperation = qgis::down_cast< QgsAnnotationItemEditOperationTranslateItem * >( operation );
      switch ( mSizeMode )
      {
        case Qgis::AnnotationPictureSizeMode::SpatialBounds:
        {
          const QgsRectangle modifiedBounds( mBounds.xMinimum() + moveOperation->translationX(),
                                             mBounds.yMinimum() + moveOperation->translationY(),
                                             mBounds.xMaximum() + moveOperation->translationX(),
                                             mBounds.yMaximum() + moveOperation->translationY() );
          return new QgsAnnotationItemEditOperationTransientResults( QgsGeometry::fromRect( modifiedBounds ) );
        }

        case Qgis::AnnotationPictureSizeMode::FixedSize:
        {
          if ( callout() && !calloutAnchor().isEmpty() )
          {
            QgsGeometry anchor = calloutAnchor();

            const double calloutOffsetWidthPixels = context.renderContext().convertToPainterUnits( offsetFromCallout().width(), offsetFromCalloutUnit() )
                                                    + moveOperation->translationXPixels();
            const double calloutOffsetHeightPixels = context.renderContext().convertToPainterUnits( offsetFromCallout().height(), offsetFromCalloutUnit() )
                + moveOperation->translationYPixels();

            QPointF anchorPoint = anchor.asQPointF();
            if ( context.renderContext().coordinateTransform().isValid() )
            {
              double x = anchorPoint.x();
              double y = anchorPoint.y();
              double z = 0.0;
              context.renderContext().coordinateTransform().transformInPlace( x, y, z );
              anchorPoint = QPointF( x, y );
            }

            context.renderContext().mapToPixel().transformInPlace( anchorPoint.rx(), anchorPoint.ry() );

            const double textOriginXPixels = anchorPoint.x() + calloutOffsetWidthPixels;
            const double textOriginYPixels = anchorPoint.y() + calloutOffsetHeightPixels;

            const double widthPixels = context.renderContext().convertToPainterUnits( mFixedSize.width(), mFixedSizeUnit );
            const double heightPixels = context.renderContext().convertToPainterUnits( mFixedSize.height(), mFixedSizeUnit );

            QgsLineString ls( QVector<QgsPointXY> { QgsPointXY( textOriginXPixels, textOriginYPixels ),
                                                    QgsPointXY( textOriginXPixels + widthPixels, textOriginYPixels ),
                                                    QgsPointXY( textOriginXPixels + widthPixels, textOriginYPixels + heightPixels ),
                                                    QgsPointXY( textOriginXPixels, textOriginYPixels + heightPixels ),
                                                    QgsPointXY( textOriginXPixels, textOriginYPixels )
                                                  } );

            QgsGeometry g( new QgsPolygon( ls.clone() ) );
            g.transform( context.renderContext().mapToPixel().transform().inverted() );
            g.transform( context.renderContext().coordinateTransform(), Qgis::TransformDirection::Reverse );
            return new QgsAnnotationItemEditOperationTransientResults( g );
          }
          else
          {
            const QgsRectangle currentBounds = context.currentItemBounds();
            const QgsRectangle newBounds = QgsRectangle::fromCenterAndSize( mBounds.center() + QgsVector( moveOperation->translationX(), moveOperation->translationY() ),
                                           currentBounds.width(), currentBounds.height() );
            return new QgsAnnotationItemEditOperationTransientResults( QgsGeometry::fromRect( newBounds ) );
          }
        }
      }
      break;
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

  mSizeMode = qgsEnumKeyToValue( element.attribute( QStringLiteral( "sizeMode" ) ), Qgis::AnnotationPictureSizeMode::SpatialBounds );

  mFixedSize = QSizeF(
                 element.attribute( QStringLiteral( "fixedWidth" ) ).toDouble(),
                 element.attribute( QStringLiteral( "fixedHeight" ) ).toDouble()
               );
  mFixedSizeUnit = QgsUnitTypes::decodeRenderUnit( element.attribute( QStringLiteral( "fixedSizeUnit" ) ) );

  mDrawBackground = element.attribute( QStringLiteral( "backgroundEnabled" ), QStringLiteral( "0" ) ).toInt();
  const QDomElement backgroundSymbolElem = element.firstChildElement( QStringLiteral( "backgroundSymbol" ) ).firstChildElement();
  if ( !backgroundSymbolElem.isNull() )
  {
    setBackgroundSymbol( QgsSymbolLayerUtils::loadSymbol< QgsFillSymbol >( backgroundSymbolElem, context ) );
  }

  mDrawFrame = element.attribute( QStringLiteral( "frameEnabled" ), QStringLiteral( "0" ) ).toInt();
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
  item->setSizeMode( mSizeMode );
  item->setFixedSize( mFixedSize );
  item->setFixedSizeUnit( mFixedSizeUnit );

  item->setBackgroundEnabled( mDrawBackground );
  if ( mBackgroundSymbol )
    item->setBackgroundSymbol( mBackgroundSymbol->clone() );

  item->setFrameEnabled( mDrawFrame );
  if ( mFrameSymbol )
    item->setFrameSymbol( mFrameSymbol->clone() );

  item->copyCommonProperties( this );
  return item.release();
}

QgsRectangle QgsAnnotationPictureItem::boundingBox() const
{
  QgsRectangle bounds;
  switch ( mSizeMode )
  {
    case Qgis::AnnotationPictureSizeMode::SpatialBounds:
    {
      bounds = mBounds;
      if ( callout() && !calloutAnchor().isEmpty() )
      {
        QgsGeometry anchor = calloutAnchor();
        bounds.combineExtentWith( anchor.boundingBox() );
      }
      break;
    }

    case Qgis::AnnotationPictureSizeMode::FixedSize:
      if ( callout() && !calloutAnchor().isEmpty() )
      {
        bounds = calloutAnchor().boundingBox();
      }
      else
      {
        bounds = QgsRectangle( mBounds.center(), mBounds.center() );
      }
      break;
  }

  return bounds;
}

QgsRectangle QgsAnnotationPictureItem::boundingBox( QgsRenderContext &context ) const
{
  switch ( mSizeMode )
  {
    case Qgis::AnnotationPictureSizeMode::SpatialBounds:
      return QgsAnnotationPictureItem::boundingBox();

    case Qgis::AnnotationPictureSizeMode::FixedSize:
    {
      const double widthPixels = context.convertToPainterUnits( mFixedSize.width(), mFixedSizeUnit );
      const double heightPixels = context.convertToPainterUnits( mFixedSize.height(), mFixedSizeUnit );

      QRectF boundsInPixels;
      if ( callout() && !calloutAnchor().isEmpty() )
      {
        QgsGeometry anchor = calloutAnchor();

        const double calloutOffsetWidthPixels = context.convertToPainterUnits( offsetFromCallout().width(), offsetFromCalloutUnit() );
        const double calloutOffsetHeightPixels = context.convertToPainterUnits( offsetFromCallout().height(), offsetFromCalloutUnit() );

        QPointF anchorPoint = anchor.asQPointF();
        if ( context.coordinateTransform().isValid() )
        {
          double x = anchorPoint.x();
          double y = anchorPoint.y();
          double z = 0.0;
          context.coordinateTransform().transformInPlace( x, y, z );
          anchorPoint = QPointF( x, y );
        }

        context.mapToPixel().transformInPlace( anchorPoint.rx(), anchorPoint.ry() );

        QgsRectangle textRect( anchorPoint.x() + calloutOffsetWidthPixels,
                               anchorPoint.y() + calloutOffsetHeightPixels,
                               anchorPoint.x() + calloutOffsetWidthPixels + widthPixels,
                               anchorPoint.y() + calloutOffsetHeightPixels + heightPixels );
        QgsRectangle anchorRect( anchorPoint.x(), anchorPoint.y(), anchorPoint.x(), anchorPoint.y() );
        anchorRect.combineExtentWith( textRect );

        boundsInPixels = anchorRect.toRectF();
      }
      else
      {
        QPointF center = mBounds.center().toQPointF();
        if ( context.coordinateTransform().isValid() )
        {
          double x = center.x();
          double y = center.y();
          double z = 0.0;
          context.coordinateTransform().transformInPlace( x, y, z );
          center = QPointF( x, y );
        }

        context.mapToPixel().transformInPlace( center.rx(), center.ry() );
        boundsInPixels = QRectF( center.x() - widthPixels * 0.5,
                                 center.y() - heightPixels * 0.5,
                                 widthPixels, heightPixels );
      }
      const QgsPointXY topLeft = context.mapToPixel().toMapCoordinates( boundsInPixels.left(), boundsInPixels.top() );
      const QgsPointXY topRight = context.mapToPixel().toMapCoordinates( boundsInPixels.right(), boundsInPixels.top() );
      const QgsPointXY bottomLeft = context.mapToPixel().toMapCoordinates( boundsInPixels.left(), boundsInPixels.bottom() );
      const QgsPointXY bottomRight = context.mapToPixel().toMapCoordinates( boundsInPixels.right(), boundsInPixels.bottom() );

      const QgsRectangle boundsMapUnits = QgsRectangle( topLeft.x(), bottomLeft.y(), bottomRight.x(), topRight.y() );
      QgsRectangle textRect = context.coordinateTransform().transformBoundingBox( boundsMapUnits, Qgis::TransformDirection::Reverse );
      return textRect;
    }
  }
  BUILTIN_UNREACHABLE
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
  return mFrameSymbol.get();
}

void QgsAnnotationPictureItem::setFrameSymbol( QgsFillSymbol *symbol )
{
  mFrameSymbol.reset( symbol );
}

QSizeF QgsAnnotationPictureItem::fixedSize() const
{
  return mFixedSize;
}

void QgsAnnotationPictureItem::setFixedSize( const QSizeF &size )
{
  mFixedSize = size;
}

Qgis::RenderUnit QgsAnnotationPictureItem::fixedSizeUnit() const
{
  return mFixedSizeUnit;
}

void QgsAnnotationPictureItem::setFixedSizeUnit( Qgis::RenderUnit unit )
{
  mFixedSizeUnit = unit;
}

Qgis::AnnotationPictureSizeMode QgsAnnotationPictureItem::sizeMode() const
{
  return mSizeMode;
}

void QgsAnnotationPictureItem::setSizeMode( Qgis::AnnotationPictureSizeMode mode )
{
  mSizeMode = mode;
}
