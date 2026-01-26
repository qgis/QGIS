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

#include "qgsfillsymbol.h"
#include "qgsmaplayer.h"
#include "qgsmarkersymbol.h"
#include "qgspainting.h"
#include "qgsproject.h"
#include "qgsshapegenerator.h"
#include "qgsstyleentityvisitor.h"
#include "qgssymbol.h"
#include "qgssymbollayerutils.h"

#include <QPainter>
#include <QPen>

#include "moc_qgsannotation.cpp"

QgsAnnotation::QgsAnnotation( QObject *parent )
  : QObject( parent )
  , mMarkerSymbol( new QgsMarkerSymbol() )
{
  QVariantMap props;
  props.insert( u"color"_s, u"white"_s );
  props.insert( u"style"_s, u"solid"_s );
  props.insert( u"style_border"_s, u"solid"_s );
  props.insert( u"color_border"_s, u"black"_s );
  props.insert( u"width_border"_s, u"0.3"_s );
  props.insert( u"joinstyle"_s, u"miter"_s );
  mFillSymbol = QgsFillSymbol::createSimple( props );
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
  if ( !painter || ( context.feedback() && context.feedback()->isCanceled() ) )
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
    painter->translate( context.convertToPainterUnits( mOffsetFromReferencePoint.x(), Qgis::RenderUnit::Millimeters ) + context.convertToPainterUnits( mContentsMargins.left(), Qgis::RenderUnit::Millimeters ),
                        context.convertToPainterUnits( mOffsetFromReferencePoint.y(), Qgis::RenderUnit::Millimeters ) + context.convertToPainterUnits( mContentsMargins.top(), Qgis::RenderUnit::Millimeters ) );
  }
  else
  {
    painter->translate( context.convertToPainterUnits( mContentsMargins.left(), Qgis::RenderUnit::Millimeters ),
                        context.convertToPainterUnits( mContentsMargins.top(), Qgis::RenderUnit::Millimeters ) );
  }
  const QSizeF size( context.convertToPainterUnits( mFrameSize.width(), Qgis::RenderUnit::Millimeters ) - context.convertToPainterUnits( mContentsMargins.left() + mContentsMargins.right(), Qgis::RenderUnit::Millimeters ),
                     context.convertToPainterUnits( mFrameSize.height(), Qgis::RenderUnit::Millimeters ) - context.convertToPainterUnits( mContentsMargins.top() + mContentsMargins.bottom(), Qgis::RenderUnit::Millimeters ) );

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
  if ( !visitor->visitEnter( QgsStyleEntityVisitorInterface::Node( QgsStyleEntityVisitorInterface::NodeType::Annotation, u"annotation"_s, tr( "Annotation" ) ) ) )
    return true;

  if ( mMarkerSymbol )
  {
    QgsStyleSymbolEntity entity( mMarkerSymbol.get() );
    if ( !visitor->visit( QgsStyleEntityVisitorInterface::StyleLeaf( &entity, u"marker"_s, QObject::tr( "Marker" ) ) ) )
      return false;
  }

  if ( mFillSymbol )
  {
    QgsStyleSymbolEntity entity( mFillSymbol.get() );
    if ( !visitor->visit( QgsStyleEntityVisitorInterface::StyleLeaf( &entity, u"fill"_s, QObject::tr( "Fill" ) ) ) )
      return false;
  }

  if ( !visitor->visitExit( QgsStyleEntityVisitorInterface::Node( QgsStyleEntityVisitorInterface::NodeType::Annotation, u"annotation"_s, tr( "Annotation" ) ) ) )
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
    return context.convertToPainterUnits( size, Qgis::RenderUnit::Millimeters );
  };

  const QRectF frameRect( mHasFixedMapPosition ? scaleSize( mOffsetFromReferencePoint.x() ) : 0,
                          mHasFixedMapPosition ? scaleSize( mOffsetFromReferencePoint.y() ) : 0,
                          scaleSize( mFrameSize.width() ),
                          scaleSize( mFrameSize.height() ) );
  const QgsPointXY origin = mHasFixedMapPosition ? QgsPointXY( 0, 0 ) : QgsPointXY( frameRect.center().x(), frameRect.center().y() );

  const QPolygonF poly = QgsShapeGenerator::createBalloon( origin, frameRect, context.convertToPainterUnits( mSegmentPointWidthMm, Qgis::RenderUnit::Millimeters ) );

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
  QDomElement annotationElem = doc.createElement( u"AnnotationItem"_s );
  annotationElem.setAttribute( u"mapPositionFixed"_s, mHasFixedMapPosition );
  annotationElem.setAttribute( u"mapPosX"_s, qgsDoubleToString( mMapPosition.x() ) );
  annotationElem.setAttribute( u"mapPosY"_s, qgsDoubleToString( mMapPosition.y() ) );
  if ( mMapPositionCrs.isValid() )
    mMapPositionCrs.writeXml( annotationElem, doc );
  annotationElem.setAttribute( u"offsetXMM"_s, qgsDoubleToString( mOffsetFromReferencePoint.x() ) );
  annotationElem.setAttribute( u"offsetYMM"_s, qgsDoubleToString( mOffsetFromReferencePoint.y() ) );
  annotationElem.setAttribute( u"frameWidthMM"_s, qgsDoubleToString( mFrameSize.width() ) );
  annotationElem.setAttribute( u"frameHeightMM"_s, qgsDoubleToString( mFrameSize.height() ) );
  annotationElem.setAttribute( u"canvasPosX"_s, qgsDoubleToString( mRelativePosition.x() ) );
  annotationElem.setAttribute( u"canvasPosY"_s, qgsDoubleToString( mRelativePosition.y() ) );
  annotationElem.setAttribute( u"contentsMargin"_s, mContentsMargins.toString() );
  annotationElem.setAttribute( u"visible"_s, isVisible() );
  if ( mMapLayer )
  {
    annotationElem.setAttribute( u"mapLayer"_s, mMapLayer->id() );
  }
  if ( mMarkerSymbol )
  {
    const QDomElement symbolElem = QgsSymbolLayerUtils::saveSymbol( u"marker symbol"_s, mMarkerSymbol.get(), doc, context );
    if ( !symbolElem.isNull() )
    {
      annotationElem.appendChild( symbolElem );
    }
  }
  if ( mFillSymbol )
  {
    QDomElement fillElem = doc.createElement( u"fillSymbol"_s );
    const QDomElement symbolElem = QgsSymbolLayerUtils::saveSymbol( u"fill symbol"_s, mFillSymbol.get(), doc, context );
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
  pos.setX( annotationElem.attribute( u"canvasPosX"_s, u"0"_s ).toDouble() );
  pos.setY( annotationElem.attribute( u"canvasPosY"_s, u"0"_s ).toDouble() );
  if ( pos.x() >= 1 || pos.x() < 0 || pos.y() < 0 || pos.y() >= 1 )
    mRelativePosition = QPointF();
  else
    mRelativePosition = pos;
  QgsPointXY mapPos;
  mapPos.setX( annotationElem.attribute( u"mapPosX"_s, u"0"_s ).toDouble() );
  mapPos.setY( annotationElem.attribute( u"mapPosY"_s, u"0"_s ).toDouble() );
  mMapPosition = mapPos;

  if ( !mMapPositionCrs.readXml( annotationElem ) )
  {
    mMapPositionCrs = QgsCoordinateReferenceSystem();
  }

  mContentsMargins = QgsMargins::fromString( annotationElem.attribute( u"contentsMargin"_s ) );
  const double dpiScale = 25.4 / QgsPainting::qtDefaultDpiX();
  if ( annotationElem.hasAttribute( u"frameWidthMM"_s ) )
    mFrameSize.setWidth( annotationElem.attribute( u"frameWidthMM"_s, u"5"_s ).toDouble() );
  else
    mFrameSize.setWidth( dpiScale * annotationElem.attribute( u"frameWidth"_s, u"50"_s ).toDouble() );
  if ( annotationElem.hasAttribute( u"frameHeightMM"_s ) )
    mFrameSize.setHeight( annotationElem.attribute( u"frameHeightMM"_s, u"3"_s ).toDouble() );
  else
    mFrameSize.setHeight( dpiScale * annotationElem.attribute( u"frameHeight"_s, u"50"_s ).toDouble() );

  if ( annotationElem.hasAttribute( u"offsetXMM"_s ) )
    mOffsetFromReferencePoint.setX( annotationElem.attribute( u"offsetXMM"_s, u"0"_s ).toDouble() );
  else
    mOffsetFromReferencePoint.setX( dpiScale * annotationElem.attribute( u"offsetX"_s, u"0"_s ).toDouble() );
  if ( annotationElem.hasAttribute( u"offsetYMM"_s ) )
    mOffsetFromReferencePoint.setY( annotationElem.attribute( u"offsetYMM"_s, u"0"_s ).toDouble() );
  else
    mOffsetFromReferencePoint.setY( dpiScale * annotationElem.attribute( u"offsetY"_s, u"0"_s ).toDouble() );

  mHasFixedMapPosition = annotationElem.attribute( u"mapPositionFixed"_s, u"1"_s ).toInt();
  mVisible = annotationElem.attribute( u"visible"_s, u"1"_s ).toInt();
  if ( annotationElem.hasAttribute( u"mapLayer"_s ) )
  {
    mMapLayer = QgsProject::instance()->mapLayer( annotationElem.attribute( u"mapLayer"_s ) ); // skip-keyword-check
  }

  //marker symbol
  {
    const QDomElement symbolElem = annotationElem.firstChildElement( u"symbol"_s );
    if ( !symbolElem.isNull() )
    {
      std::unique_ptr< QgsMarkerSymbol > symbol = QgsSymbolLayerUtils::loadSymbol<QgsMarkerSymbol>( symbolElem, context );
      if ( symbol )
      {
        mMarkerSymbol = std::move( symbol );
      }
    }
  }

  mFillSymbol.reset( nullptr );
  const QDomElement fillElem = annotationElem.firstChildElement( u"fillSymbol"_s );
  if ( !fillElem.isNull() )
  {
    const QDomElement symbolElem = fillElem.firstChildElement( u"symbol"_s );
    if ( !symbolElem.isNull() )
    {
      std::unique_ptr< QgsFillSymbol  >symbol = QgsSymbolLayerUtils::loadSymbol<QgsFillSymbol>( symbolElem, context );
      if ( symbol )
      {
        mFillSymbol = std::move( symbol );
      }
    }
  }
  if ( !mFillSymbol )
  {
    QColor frameColor;
    frameColor.setNamedColor( annotationElem.attribute( u"frameColor"_s, u"#000000"_s ) );
    frameColor.setAlpha( annotationElem.attribute( u"frameColorAlpha"_s, u"255"_s ).toInt() );
    QColor frameBackgroundColor;
    frameBackgroundColor.setNamedColor( annotationElem.attribute( u"frameBackgroundColor"_s ) );
    frameBackgroundColor.setAlpha( annotationElem.attribute( u"frameBackgroundColorAlpha"_s, u"255"_s ).toInt() );
    double frameBorderWidth = annotationElem.attribute( u"frameBorderWidth"_s, u"0.5"_s ).toDouble();
    // need to roughly convert border width from pixels to mm - just assume 96 dpi
    frameBorderWidth = frameBorderWidth * 25.4 / 96.0;
    QVariantMap props;
    props.insert( u"color"_s, frameBackgroundColor.name() );
    props.insert( u"style"_s, u"solid"_s );
    props.insert( u"style_border"_s, u"solid"_s );
    props.insert( u"color_border"_s, frameColor.name() );
    props.insert( u"width_border"_s, QString::number( frameBorderWidth ) );
    props.insert( u"joinstyle"_s, u"miter"_s );
    mFillSymbol = QgsFillSymbol::createSimple( props );
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

