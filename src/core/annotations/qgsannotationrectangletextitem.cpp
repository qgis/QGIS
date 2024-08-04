/***************************************************************************
    qgsannotationrectangletextitem.cpp
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

#include "qgsannotationrectangletextitem.h"
#include "qgsgeometry.h"
#include "qgsrendercontext.h"
#include "qgsannotationitemnode.h"
#include "qgsannotationitemeditoperation.h"
#include "qgsfillsymbol.h"
#include "qgssymbollayerutils.h"
#include "qgsfillsymbollayer.h"
#include "qgslinesymbollayer.h"
#include "qgstextrenderer.h"
#include "qgsunittypes.h"
#include "qgsapplication.h"
#include "qgscalloutsregistry.h"

QgsAnnotationRectangleTextItem::QgsAnnotationRectangleTextItem( const QString &text, const QgsRectangle &bounds )
  : QgsAnnotationItem()
  , mBounds( bounds )
  , mText( text )
{
  mBackgroundSymbol = std::make_unique< QgsFillSymbol >( QgsSymbolLayerList { new QgsSimpleFillSymbolLayer( QColor( 255, 255, 255 ), Qt::BrushStyle::SolidPattern, QColor( 0, 0, 0 ), Qt::PenStyle::NoPen ) } );
  QgsSimpleLineSymbolLayer *borderSymbol = new QgsSimpleLineSymbolLayer( QColor( 0, 0, 0 ) );
  borderSymbol->setPenJoinStyle( Qt::MiterJoin );
  mFrameSymbol = std::make_unique< QgsFillSymbol >( QgsSymbolLayerList { borderSymbol } );
}

QgsAnnotationRectangleTextItem::~QgsAnnotationRectangleTextItem() = default;

QString QgsAnnotationRectangleTextItem::type() const
{
  return QStringLiteral( "recttext" );
}

void QgsAnnotationRectangleTextItem::render( QgsRenderContext &context, QgsFeedback *feedback )
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

  if ( callout() )
  {
    QgsCallout::QgsCalloutContext calloutContext;
    renderCallout( context, painterBounds, 0, calloutContext, feedback );
  }

  const double marginLeft = context.convertToPainterUnits( mMargins.left(), mMarginUnit );
  const double marginTop = context.convertToPainterUnits( mMargins.top(), mMarginUnit );
  const double marginRight = context.convertToPainterUnits( mMargins.right(), mMarginUnit );
  const double marginBottom = context.convertToPainterUnits( mMargins.bottom(), mMarginUnit );

  const QRectF innerRect(
    painterBounds.left() + marginLeft,
    painterBounds.top() + marginTop,
    painterBounds.width() - marginLeft - marginRight,
    painterBounds.height() - marginTop - marginBottom );

  const QString displayText = QgsExpression::replaceExpressionText( mText, &context.expressionContext(), &context.distanceArea() );

  const bool prevWorkaroundFlag = context.testFlag( Qgis::RenderContextFlag::ApplyScalingWorkaroundForTextRendering );
  context.setFlag( Qgis::RenderContextFlag::ApplyScalingWorkaroundForTextRendering, true );
  QgsTextRenderer::drawText( innerRect, 0,
                             QgsTextRenderer::convertQtHAlignment( mAlignment ),
                             displayText.split( '\n' ), context, mTextFormat, true,
                             QgsTextRenderer::convertQtVAlignment( mAlignment ),
                             Qgis::TextRendererFlag::WrapLines );
  context.setFlag( Qgis::RenderContextFlag::ApplyScalingWorkaroundForTextRendering, prevWorkaroundFlag );

  if ( mDrawFrame && mFrameSymbol )
  {
    mFrameSymbol->startRender( context );
    mFrameSymbol->renderPolygon( painterBounds, nullptr, nullptr, context );
    mFrameSymbol->stopRender( context );
  }
}

bool QgsAnnotationRectangleTextItem::writeXml( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context ) const
{
  element.setAttribute( QStringLiteral( "text" ), mText );
  element.setAttribute( QStringLiteral( "alignment" ), QString::number( mAlignment ) );

  QDomElement textFormatElem = document.createElement( QStringLiteral( "rectTextFormat" ) );
  textFormatElem.appendChild( mTextFormat.writeXml( document, context ) );
  element.appendChild( textFormatElem );

  element.setAttribute( QStringLiteral( "margins" ), mMargins.toString() );
  element.setAttribute( QStringLiteral( "marginUnit" ), QgsUnitTypes::encodeUnit( mMarginUnit ) );

  element.setAttribute( QStringLiteral( "xMin" ), qgsDoubleToString( mBounds.xMinimum() ) );
  element.setAttribute( QStringLiteral( "xMax" ), qgsDoubleToString( mBounds.xMaximum() ) );
  element.setAttribute( QStringLiteral( "yMin" ), qgsDoubleToString( mBounds.yMinimum() ) );
  element.setAttribute( QStringLiteral( "yMax" ), qgsDoubleToString( mBounds.yMaximum() ) );

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

QList<QgsAnnotationItemNode> QgsAnnotationRectangleTextItem::nodesV2( const QgsAnnotationItemEditContext & ) const
{
  QList<QgsAnnotationItemNode> res =
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

Qgis::AnnotationItemEditOperationResult QgsAnnotationRectangleTextItem::applyEditV2( QgsAbstractAnnotationItemEditOperation *operation, const QgsAnnotationItemEditContext & )
{
  switch ( operation->type() )
  {
    case QgsAbstractAnnotationItemEditOperation::Type::MoveNode:
    {
      QgsAnnotationItemEditOperationMoveNode *moveOperation = dynamic_cast< QgsAnnotationItemEditOperationMoveNode * >( operation );
      if ( moveOperation->nodeId().part == 0 )
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
      }
      else
      {
        setCalloutAnchor( QgsGeometry::fromPoint( moveOperation->after() ) );
        if ( !callout() )
        {
          setCallout( QgsApplication::calloutRegistry()->defaultCallout() );
        }
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

QgsAnnotationItemEditOperationTransientResults *QgsAnnotationRectangleTextItem::transientEditResultsV2( QgsAbstractAnnotationItemEditOperation *operation, const QgsAnnotationItemEditContext & )
{
  switch ( operation->type() )
  {
    case QgsAbstractAnnotationItemEditOperation::Type::MoveNode:
    {
      QgsAnnotationItemEditOperationMoveNode *moveOperation = dynamic_cast< QgsAnnotationItemEditOperationMoveNode * >( operation );
      if ( moveOperation->nodeId().part == 0 )
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
      else
      {
        QgsAnnotationItemEditOperationMoveNode *moveOperation = dynamic_cast< QgsAnnotationItemEditOperationMoveNode * >( operation );
        return new QgsAnnotationItemEditOperationTransientResults( QgsGeometry( moveOperation->after().clone() ) );
      }
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

QgsAnnotationRectangleTextItem *QgsAnnotationRectangleTextItem::create()
{
  return new QgsAnnotationRectangleTextItem( QString(), QgsRectangle() );
}

bool QgsAnnotationRectangleTextItem::readXml( const QDomElement &element, const QgsReadWriteContext &context )
{
  mText = element.attribute( QStringLiteral( "text" ) );

  const QDomElement textFormatElem = element.firstChildElement( QStringLiteral( "rectTextFormat" ) );
  if ( !textFormatElem.isNull() )
  {
    const QDomNodeList textFormatNodeList = textFormatElem.elementsByTagName( QStringLiteral( "text-style" ) );
    const QDomElement textFormatElem = textFormatNodeList.at( 0 ).toElement();
    mTextFormat.readXml( textFormatElem, context );
  }

  mBounds.setXMinimum( element.attribute( QStringLiteral( "xMin" ) ).toDouble() );
  mBounds.setXMaximum( element.attribute( QStringLiteral( "xMax" ) ).toDouble() );
  mBounds.setYMinimum( element.attribute( QStringLiteral( "yMin" ) ).toDouble() );
  mBounds.setYMaximum( element.attribute( QStringLiteral( "yMax" ) ).toDouble() );

  mMargins = QgsMargins::fromString( element.attribute( QStringLiteral( "margins" ) ) );
  mMarginUnit = QgsUnitTypes::decodeRenderUnit( element.attribute( QStringLiteral( "marginUnit" ), QgsUnitTypes::encodeUnit( Qgis::RenderUnit::Millimeters ) ) );

  mAlignment = static_cast< Qt::Alignment >( element.attribute( QStringLiteral( "alignment" ) ).toInt() );

  mDrawBackground = element.attribute( QStringLiteral( "backgroundEnabled" ), QStringLiteral( "1" ) ).toInt();
  const QDomElement backgroundSymbolElem = element.firstChildElement( QStringLiteral( "backgroundSymbol" ) ).firstChildElement();
  if ( !backgroundSymbolElem.isNull() )
  {
    setBackgroundSymbol( QgsSymbolLayerUtils::loadSymbol< QgsFillSymbol >( backgroundSymbolElem, context ) );
  }

  mDrawFrame = element.attribute( QStringLiteral( "frameEnabled" ), QStringLiteral( "1" ) ).toInt();
  const QDomElement frameSymbolElem = element.firstChildElement( QStringLiteral( "frameSymbol" ) ).firstChildElement();
  if ( !frameSymbolElem.isNull() )
  {
    setFrameSymbol( QgsSymbolLayerUtils::loadSymbol< QgsFillSymbol >( frameSymbolElem, context ) );
  }

  readCommonProperties( element, context );
  return true;
}

QgsAnnotationRectangleTextItem *QgsAnnotationRectangleTextItem::clone() const
{
  std::unique_ptr< QgsAnnotationRectangleTextItem > item = std::make_unique< QgsAnnotationRectangleTextItem >( mText, mBounds );

  item->setFormat( mTextFormat );
  item->setAlignment( mAlignment );

  item->setBackgroundEnabled( mDrawBackground );
  if ( mBackgroundSymbol )
    item->setBackgroundSymbol( mBackgroundSymbol->clone() );

  item->setFrameEnabled( mDrawFrame );
  if ( mFrameSymbol )
    item->setFrameSymbol( mFrameSymbol->clone() );

  item->setMargins( mMargins );
  item->setMarginsUnit( mMarginUnit );

  item->copyCommonProperties( this );
  return item.release();
}

QgsRectangle QgsAnnotationRectangleTextItem::boundingBox() const
{
  QgsRectangle bounds = mBounds;
  if ( callout() && !calloutAnchor().isEmpty() )
  {
    QgsGeometry anchor = calloutAnchor();
    bounds.combineExtentWith( anchor.boundingBox() );
  }
  return bounds;
}

void QgsAnnotationRectangleTextItem::setBounds( const QgsRectangle &bounds )
{
  mBounds = bounds;
}

const QgsFillSymbol *QgsAnnotationRectangleTextItem::backgroundSymbol() const
{
  return mBackgroundSymbol.get();
}

void QgsAnnotationRectangleTextItem::setBackgroundSymbol( QgsFillSymbol *symbol )
{
  mBackgroundSymbol.reset( symbol );
}

const QgsFillSymbol *QgsAnnotationRectangleTextItem::frameSymbol() const
{
  return mFrameSymbol.get();
}

void QgsAnnotationRectangleTextItem::setFrameSymbol( QgsFillSymbol *symbol )
{
  mFrameSymbol.reset( symbol );
}

Qgis::AnnotationItemFlags QgsAnnotationRectangleTextItem::flags() const
{
  return Qgis::AnnotationItemFlag::SupportsReferenceScale
         | Qgis::AnnotationItemFlag::SupportsCallouts;
}

QgsTextFormat QgsAnnotationRectangleTextItem::format() const
{
  return mTextFormat;
}

void QgsAnnotationRectangleTextItem::setFormat( const QgsTextFormat &format )
{
  mTextFormat = format;
}

Qt::Alignment QgsAnnotationRectangleTextItem::alignment() const
{
  return mAlignment;
}

void QgsAnnotationRectangleTextItem::setAlignment( Qt::Alignment alignment )
{
  mAlignment = alignment;
}
