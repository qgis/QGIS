/***************************************************************************
                             qgsannotation.cpp
                             -----------------
    begin                : January 2017
    copyright            : (C) 2017 by Nyall Dawson
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

#include "qgsannotation.h"
#include "qgssymbollayerutils.h"
#include "qgsmaplayer.h"
#include "qgsproject.h"
#include "qgsgeometryutils.h"
#include "qgsstyleentityvisitor.h"
#include "qgsshapegenerator.h"
#include "qgssymbol.h"
#include "qgsmarkersymbol.h"
#include "qgsfillsymbol.h"

#include <QPen>
#include <QPainter>

Q_GUI_EXPORT extern int qt_defaultDpiX();

QgsAnnotation::QgsAnnotation( QObject *parent )
  : QObject( parent )
  , mMarkerSymbol( new QgsMarkerSymbol() )
{
  QVariantMap props;
  props.insert( QStringLiteral( "color" ), QStringLiteral( "white" ) );
  props.insert( QStringLiteral( "style" ), QStringLiteral( "solid" ) );
  props.insert( QStringLiteral( "style_border" ), QStringLiteral( "solid" ) );
  props.insert( QStringLiteral( "color_border" ), QStringLiteral( "black" ) );
  props.insert( QStringLiteral( "width_border" ), QStringLiteral( "0.3" ) );
  props.insert( QStringLiteral( "joinstyle" ), QStringLiteral( "miter" ) );
  mFillSymbol.reset( QgsFillSymbol::createSimple( props ) );
}

QgsAnnotation::~QgsAnnotation() = default;

void QgsAnnotation::setVisible( bool visible )
{
  if ( mVisible == visible )
    return;

  mVisible = visible;
  emit appearanceChanged();
}

void QgsAnnotation::setHasFixedMapPosition( bool fixed )
{
  if ( mHasFixedMapPosition == fixed )
    return;

  mHasFixedMapPosition = fixed;
  emit moved();
}

void QgsAnnotation::setMapPosition( const QgsPointXY &position )
{
  mMapPosition = position;
  emit moved();
}

void QgsAnnotation::setMapPositionCrs( const QgsCoordinateReferenceSystem &crs )
{
  mMapPositionCrs = crs;
  emit moved();
}

void QgsAnnotation::setRelativePosition( QPointF position )
{
  mRelativePosition = position;
  emit moved();
}

void QgsAnnotation::setFrameOffsetFromReferencePoint( QPointF offset )
{
  // convert from offset in pixels at 96 dpi to mm
  setFrameOffsetFromReferencePointMm( offset / 3.7795275 );
}

QPointF QgsAnnotation::frameOffsetFromReferencePoint() const
{
  return mOffsetFromReferencePoint / 3.7795275;
}

void QgsAnnotation::setFrameOffsetFromReferencePointMm( QPointF offset )
{
  mOffsetFromReferencePoint = offset;

  emit moved();
  emit appearanceChanged();
}

void QgsAnnotation::setFrameSize( QSizeF size )
{
  // convert from size in pixels at 96 dpi to mm
  setFrameSizeMm( size / 3.7795275 );
}

QSizeF QgsAnnotation::frameSize() const
{
  return mFrameSize / 3.7795275;
}

void QgsAnnotation::setFrameSizeMm( QSizeF size )
{
  const QSizeF frameSize = minimumFrameSize().expandedTo( size ); //don't allow frame sizes below minimum
  mFrameSize = frameSize;
  emit moved();
  emit appearanceChanged();
}

void QgsAnnotation::setContentsMargin( const QgsMargins &margins )
{
  mContentsMargins = margins;
  emit appearanceChanged();
}

void QgsAnnotation::setFillSymbol( QgsFillSymbol *symbol )
{
  mFillSymbol.reset( symbol );
  emit appearanceChanged();
}

QgsFillSymbol *QgsAnnotation::fillSymbol() const
{
  return mFillSymbol.get();
}

void QgsAnnotation::render( QgsRenderContext &context ) const
{
  QPainter *painter = context.painter();
  if ( !painter )
  {
    return;
  }

  const QgsScopedQPainterState painterState( context.painter() );
  context.setPainterFlagsUsingContext();

  drawFrame( context );
  if ( mHasFixedMapPosition )
  {
    drawMarkerSymbol( context );
  }
  if ( mHasFixedMapPosition )
  {
    painter->translate( context.convertToPainterUnits( mOffsetFromReferencePoint.x(), QgsUnitTypes::RenderMillimeters ) + context.convertToPainterUnits( mContentsMargins.left(), QgsUnitTypes::RenderMillimeters ),
                        context.convertToPainterUnits( mOffsetFromReferencePoint.y(), QgsUnitTypes::RenderMillimeters ) + context.convertToPainterUnits( mContentsMargins.top(), QgsUnitTypes::RenderMillimeters ) );
  }
  else
  {
    painter->translate( context.convertToPainterUnits( mContentsMargins.left(), QgsUnitTypes::RenderMillimeters ),
                        context.convertToPainterUnits( mContentsMargins.top(), QgsUnitTypes::RenderMillimeters ) );
  }
  const QSizeF size( context.convertToPainterUnits( mFrameSize.width(), QgsUnitTypes::RenderMillimeters ) - context.convertToPainterUnits( mContentsMargins.left() + mContentsMargins.right(), QgsUnitTypes::RenderMillimeters ),
                     context.convertToPainterUnits( mFrameSize.height(), QgsUnitTypes::RenderMillimeters ) - context.convertToPainterUnits( mContentsMargins.top() + mContentsMargins.bottom(), QgsUnitTypes::RenderMillimeters ) );

  // scale back from painter dpi to 96 dpi --
// double dotsPerMM = context.painter()->device()->logicalDpiX() / ( 25.4 * 3.78 );
// context.painter()->scale( dotsPerMM, dotsPerMM );

  renderAnnotation( context, size );
}

void QgsAnnotation::setMarkerSymbol( QgsMarkerSymbol *symbol )
{
  mMarkerSymbol.reset( symbol );
  emit appearanceChanged();
}

void QgsAnnotation::setMapLayer( QgsMapLayer *layer )
{
  mMapLayer = layer;
  emit mapLayerChanged();
}

void QgsAnnotation::setAssociatedFeature( const QgsFeature &feature )
{
  mFeature = feature;
}

bool QgsAnnotation::accept( QgsStyleEntityVisitorInterface *visitor ) const
{
  // NOTE: if visitEnter returns false it means "don't visit the annotation", not "abort all further visitations"
  if ( !visitor->visitEnter( QgsStyleEntityVisitorInterface::Node( QgsStyleEntityVisitorInterface::NodeType::Annotation, QStringLiteral( "annotation" ), tr( "Annotation" ) ) ) )
    return true;

  if ( mMarkerSymbol )
  {
    QgsStyleSymbolEntity entity( mMarkerSymbol.get() );
    if ( !visitor->visit( QgsStyleEntityVisitorInterface::StyleLeaf( &entity, QStringLiteral( "marker" ), QObject::tr( "Marker" ) ) ) )
      return false;
  }

  if ( mFillSymbol )
  {
    QgsStyleSymbolEntity entity( mFillSymbol.get() );
    if ( !visitor->visit( QgsStyleEntityVisitorInterface::StyleLeaf( &entity, QStringLiteral( "fill" ), QObject::tr( "Fill" ) ) ) )
      return false;
  }

  if ( !visitor->visitExit( QgsStyleEntityVisitorInterface::Node( QgsStyleEntityVisitorInterface::NodeType::Annotation, QStringLiteral( "annotation" ), tr( "Annotation" ) ) ) )
    return false;

  return true;
}

QSizeF QgsAnnotation::minimumFrameSize() const
{
  return QSizeF( 0, 0 );
}

void QgsAnnotation::drawFrame( QgsRenderContext &context ) const
{
  if ( !mFillSymbol )
    return;

  auto scaleSize = [&context]( double size )->double
  {
    return context.convertToPainterUnits( size, QgsUnitTypes::RenderMillimeters );
  };

  const QRectF frameRect( mHasFixedMapPosition ? scaleSize( mOffsetFromReferencePoint.x() ) : 0,
                          mHasFixedMapPosition ? scaleSize( mOffsetFromReferencePoint.y() ) : 0,
                          scaleSize( mFrameSize.width() ),
                          scaleSize( mFrameSize.height() ) );
  const QgsPointXY origin = mHasFixedMapPosition ? QgsPointXY( 0, 0 ) : QgsPointXY( frameRect.center().x(), frameRect.center().y() );

  const QPolygonF poly = QgsShapeGenerator::createBalloon( origin, frameRect, context.convertToPainterUnits( mSegmentPointWidthMm, QgsUnitTypes::RenderMillimeters ) );

  mFillSymbol->startRender( context );
  const QVector<QPolygonF> rings; //empty list
  mFillSymbol->renderPolygon( poly, &rings, nullptr, context );
  mFillSymbol->stopRender( context );
}

void QgsAnnotation::drawMarkerSymbol( QgsRenderContext &context ) const
{
  if ( !context.painter() )
  {
    return;
  }

  if ( mMarkerSymbol )
  {
    mMarkerSymbol->startRender( context );
    mMarkerSymbol->renderPoint( QPointF( 0, 0 ), nullptr, context );
    mMarkerSymbol->stopRender( context );
  }
}

void QgsAnnotation::_writeXml( QDomElement &itemElem, QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  if ( itemElem.isNull() )
  {
    return;
  }
  QDomElement annotationElem = doc.createElement( QStringLiteral( "AnnotationItem" ) );
  annotationElem.setAttribute( QStringLiteral( "mapPositionFixed" ), mHasFixedMapPosition );
  annotationElem.setAttribute( QStringLiteral( "mapPosX" ), qgsDoubleToString( mMapPosition.x() ) );
  annotationElem.setAttribute( QStringLiteral( "mapPosY" ), qgsDoubleToString( mMapPosition.y() ) );
  if ( mMapPositionCrs.isValid() )
    mMapPositionCrs.writeXml( annotationElem, doc );
  annotationElem.setAttribute( QStringLiteral( "offsetXMM" ), qgsDoubleToString( mOffsetFromReferencePoint.x() ) );
  annotationElem.setAttribute( QStringLiteral( "offsetYMM" ), qgsDoubleToString( mOffsetFromReferencePoint.y() ) );
  annotationElem.setAttribute( QStringLiteral( "frameWidthMM" ), qgsDoubleToString( mFrameSize.width() ) );
  annotationElem.setAttribute( QStringLiteral( "frameHeightMM" ), qgsDoubleToString( mFrameSize.height() ) );
  annotationElem.setAttribute( QStringLiteral( "canvasPosX" ), qgsDoubleToString( mRelativePosition.x() ) );
  annotationElem.setAttribute( QStringLiteral( "canvasPosY" ), qgsDoubleToString( mRelativePosition.y() ) );
  annotationElem.setAttribute( QStringLiteral( "contentsMargin" ), mContentsMargins.toString() );
  annotationElem.setAttribute( QStringLiteral( "visible" ), isVisible() );
  if ( mMapLayer )
  {
    annotationElem.setAttribute( QStringLiteral( "mapLayer" ), mMapLayer->id() );
  }
  if ( mMarkerSymbol )
  {
    const QDomElement symbolElem = QgsSymbolLayerUtils::saveSymbol( QStringLiteral( "marker symbol" ), mMarkerSymbol.get(), doc, context );
    if ( !symbolElem.isNull() )
    {
      annotationElem.appendChild( symbolElem );
    }
  }
  if ( mFillSymbol )
  {
    QDomElement fillElem = doc.createElement( QStringLiteral( "fillSymbol" ) );
    const QDomElement symbolElem = QgsSymbolLayerUtils::saveSymbol( QStringLiteral( "fill symbol" ), mFillSymbol.get(), doc, context );
    if ( !symbolElem.isNull() )
    {
      fillElem.appendChild( symbolElem );
      annotationElem.appendChild( fillElem );
    }
  }
  itemElem.appendChild( annotationElem );
}

void QgsAnnotation::_readXml( const QDomElement &annotationElem, const QgsReadWriteContext &context )
{
  if ( annotationElem.isNull() )
  {
    return;
  }
  QPointF pos;
  pos.setX( annotationElem.attribute( QStringLiteral( "canvasPosX" ), QStringLiteral( "0" ) ).toDouble() );
  pos.setY( annotationElem.attribute( QStringLiteral( "canvasPosY" ), QStringLiteral( "0" ) ).toDouble() );
  if ( pos.x() >= 1 || pos.x() < 0 || pos.y() < 0 || pos.y() >= 1 )
    mRelativePosition = QPointF();
  else
    mRelativePosition = pos;
  QgsPointXY mapPos;
  mapPos.setX( annotationElem.attribute( QStringLiteral( "mapPosX" ), QStringLiteral( "0" ) ).toDouble() );
  mapPos.setY( annotationElem.attribute( QStringLiteral( "mapPosY" ), QStringLiteral( "0" ) ).toDouble() );
  mMapPosition = mapPos;

  if ( !mMapPositionCrs.readXml( annotationElem ) )
  {
    mMapPositionCrs = QgsCoordinateReferenceSystem();
  }

  mContentsMargins = QgsMargins::fromString( annotationElem.attribute( QStringLiteral( "contentsMargin" ) ) );
  const double dpiScale = 25.4 / qt_defaultDpiX();
  if ( annotationElem.hasAttribute( QStringLiteral( "frameWidthMM" ) ) )
    mFrameSize.setWidth( annotationElem.attribute( QStringLiteral( "frameWidthMM" ), QStringLiteral( "5" ) ).toDouble() );
  else
    mFrameSize.setWidth( dpiScale * annotationElem.attribute( QStringLiteral( "frameWidth" ), QStringLiteral( "50" ) ).toDouble() );
  if ( annotationElem.hasAttribute( QStringLiteral( "frameHeightMM" ) ) )
    mFrameSize.setHeight( annotationElem.attribute( QStringLiteral( "frameHeightMM" ), QStringLiteral( "3" ) ).toDouble() );
  else
    mFrameSize.setHeight( dpiScale * annotationElem.attribute( QStringLiteral( "frameHeight" ), QStringLiteral( "50" ) ).toDouble() );

  if ( annotationElem.hasAttribute( QStringLiteral( "offsetXMM" ) ) )
    mOffsetFromReferencePoint.setX( annotationElem.attribute( QStringLiteral( "offsetXMM" ), QStringLiteral( "0" ) ).toDouble() );
  else
    mOffsetFromReferencePoint.setX( dpiScale * annotationElem.attribute( QStringLiteral( "offsetX" ), QStringLiteral( "0" ) ).toDouble() );
  if ( annotationElem.hasAttribute( QStringLiteral( "offsetYMM" ) ) )
    mOffsetFromReferencePoint.setY( annotationElem.attribute( QStringLiteral( "offsetYMM" ), QStringLiteral( "0" ) ).toDouble() );
  else
    mOffsetFromReferencePoint.setY( dpiScale * annotationElem.attribute( QStringLiteral( "offsetY" ), QStringLiteral( "0" ) ).toDouble() );

  mHasFixedMapPosition = annotationElem.attribute( QStringLiteral( "mapPositionFixed" ), QStringLiteral( "1" ) ).toInt();
  mVisible = annotationElem.attribute( QStringLiteral( "visible" ), QStringLiteral( "1" ) ).toInt();
  if ( annotationElem.hasAttribute( QStringLiteral( "mapLayer" ) ) )
  {
    mMapLayer = QgsProject::instance()->mapLayer( annotationElem.attribute( QStringLiteral( "mapLayer" ) ) );
  }

  //marker symbol
  {
    const QDomElement symbolElem = annotationElem.firstChildElement( QStringLiteral( "symbol" ) );
    if ( !symbolElem.isNull() )
    {
      QgsMarkerSymbol *symbol = QgsSymbolLayerUtils::loadSymbol<QgsMarkerSymbol>( symbolElem, context );
      if ( symbol )
      {
        mMarkerSymbol.reset( symbol );
      }
    }
  }

  mFillSymbol.reset( nullptr );
  const QDomElement fillElem = annotationElem.firstChildElement( QStringLiteral( "fillSymbol" ) );
  if ( !fillElem.isNull() )
  {
    const QDomElement symbolElem = fillElem.firstChildElement( QStringLiteral( "symbol" ) );
    if ( !symbolElem.isNull() )
    {
      QgsFillSymbol *symbol = QgsSymbolLayerUtils::loadSymbol<QgsFillSymbol>( symbolElem, context );
      if ( symbol )
      {
        mFillSymbol.reset( symbol );
      }
    }
  }
  if ( !mFillSymbol )
  {
    QColor frameColor;
    frameColor.setNamedColor( annotationElem.attribute( QStringLiteral( "frameColor" ), QStringLiteral( "#000000" ) ) );
    frameColor.setAlpha( annotationElem.attribute( QStringLiteral( "frameColorAlpha" ), QStringLiteral( "255" ) ).toInt() );
    QColor frameBackgroundColor;
    frameBackgroundColor.setNamedColor( annotationElem.attribute( QStringLiteral( "frameBackgroundColor" ) ) );
    frameBackgroundColor.setAlpha( annotationElem.attribute( QStringLiteral( "frameBackgroundColorAlpha" ), QStringLiteral( "255" ) ).toInt() );
    double frameBorderWidth = annotationElem.attribute( QStringLiteral( "frameBorderWidth" ), QStringLiteral( "0.5" ) ).toDouble();
    // need to roughly convert border width from pixels to mm - just assume 96 dpi
    frameBorderWidth = frameBorderWidth * 25.4 / 96.0;
    QVariantMap props;
    props.insert( QStringLiteral( "color" ), frameBackgroundColor.name() );
    props.insert( QStringLiteral( "style" ), QStringLiteral( "solid" ) );
    props.insert( QStringLiteral( "style_border" ), QStringLiteral( "solid" ) );
    props.insert( QStringLiteral( "color_border" ), frameColor.name() );
    props.insert( QStringLiteral( "width_border" ), QString::number( frameBorderWidth ) );
    props.insert( QStringLiteral( "joinstyle" ), QStringLiteral( "miter" ) );
    mFillSymbol.reset( QgsFillSymbol::createSimple( props ) );
  }

  emit mapLayerChanged();
}

void QgsAnnotation::copyCommonProperties( QgsAnnotation *target ) const
{
  target->mVisible = mVisible;
  target->mHasFixedMapPosition = mHasFixedMapPosition;
  target->mMapPosition = mMapPosition;
  target->mMapPositionCrs = mMapPositionCrs;
  target->mRelativePosition = mRelativePosition;
  target->mOffsetFromReferencePoint = mOffsetFromReferencePoint;
  target->mFrameSize = mFrameSize;
  target->mMarkerSymbol.reset( mMarkerSymbol ? mMarkerSymbol->clone() : nullptr );
  target->mContentsMargins = mContentsMargins;
  target->mFillSymbol.reset( mFillSymbol ? mFillSymbol->clone() : nullptr );
  target->mSegmentPointWidthMm = mSegmentPointWidthMm;
  target->mMapLayer = mMapLayer;
  target->mFeature = mFeature;
}

