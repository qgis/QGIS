/***************************************************************************
 qgssymbollayerv2.cpp
 ---------------------
 begin                : November 2009
 copyright            : (C) 2009 by Martin Dobias
 email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgssymbollayerv2.h"
#include "qgsclipper.h"
#include "qgsexpression.h"
#include "qgsrendercontext.h"
#include "qgsvectorlayer.h"
#include "qgsdxfexport.h"
#include "qgsgeometrysimplifier.h"

#include <QSize>
#include <QPainter>
#include <QPointF>
#include <QPolygonF>

const QgsExpression* QgsSymbolLayerV2::dataDefinedProperty( const QString& property ) const
{
  QMap< QString, QgsExpression* >::const_iterator it = mDataDefinedProperties.find( property );
  if ( it != mDataDefinedProperties.constEnd() )
  {
    return it.value();
  }
  return 0;
}

QgsExpression* QgsSymbolLayerV2::expression( const QString& property ) const
{
  QMap< QString, QgsExpression* >::const_iterator it = mDataDefinedProperties.find( property );
  if ( it != mDataDefinedProperties.constEnd() )
  {
    return it.value();
  }
  return 0;
}

QString QgsSymbolLayerV2::dataDefinedPropertyString( const QString& property ) const
{
  const QgsExpression* ex = dataDefinedProperty( property );
  return ex ? ex->expression() : QString();
}

void QgsSymbolLayerV2::setDataDefinedProperty( const QString& property, const QString& expressionString )
{
  removeDataDefinedProperty( property );
  mDataDefinedProperties.insert( property, new QgsExpression( expressionString ) );
}

void QgsSymbolLayerV2::removeDataDefinedProperty( const QString& property )
{
  QMap< QString, QgsExpression* >::iterator it = mDataDefinedProperties.find( property );
  if ( it != mDataDefinedProperties.end() )
  {
    delete( it.value() );
    mDataDefinedProperties.erase( it );
  }
}

void QgsSymbolLayerV2::removeDataDefinedProperties()
{
  QMap< QString, QgsExpression* >::iterator it = mDataDefinedProperties.begin();
  for ( ; it != mDataDefinedProperties.constEnd(); ++it )
  {
    delete( it.value() );
  }
  mDataDefinedProperties.clear();
}

bool QgsSymbolLayerV2::writeDxf( QgsDxfExport& e,
                                 double mmMapUnitScaleFactor,
                                 const QString& layerName,
                                 const QgsSymbolV2RenderContext* context,
                                 const QgsFeature* f,
                                 const QPointF& shift ) const
{
  Q_UNUSED( e );
  Q_UNUSED( mmMapUnitScaleFactor );
  Q_UNUSED( layerName );
  Q_UNUSED( context );
  Q_UNUSED( f );
  Q_UNUSED( shift );
  return false;
}

double QgsSymbolLayerV2::dxfWidth( const QgsDxfExport& e, const QgsSymbolV2RenderContext& context ) const
{
  Q_UNUSED( e );
  Q_UNUSED( context );
  return 1.0;
}

QColor QgsSymbolLayerV2::dxfColor( const QgsSymbolV2RenderContext& context ) const
{
  Q_UNUSED( context );
  return color();
}

QVector<qreal> QgsSymbolLayerV2::dxfCustomDashPattern( QgsSymbolV2::OutputUnit& unit ) const
{
  Q_UNUSED( unit );
  return QVector<qreal>();
}

Qt::PenStyle QgsSymbolLayerV2::dxfPenStyle() const
{
  return Qt::SolidLine;
}

QColor QgsSymbolLayerV2::dxfBrushColor( const QgsSymbolV2RenderContext& context ) const
{
  Q_UNUSED( context );
  return color();
}

Qt::BrushStyle QgsSymbolLayerV2::dxfBrushStyle() const
{
  return Qt::NoBrush;
}

void QgsSymbolLayerV2::prepareExpressions( const QgsFields* fields, double scale )
{
  if ( !fields )
  {
    return;
  }

  QMap< QString, QgsExpression* >::iterator it = mDataDefinedProperties.begin();
  for ( ; it != mDataDefinedProperties.end(); ++it )
  {
    if ( it.value() )
    {
      it.value()->prepare( *fields );
      if ( scale > 0 )
      {
        it.value()->setScale( scale );
      }
    }
  }
}

QSet<QString> QgsSymbolLayerV2::usedAttributes() const
{
  QStringList columns;

  QMap< QString, QgsExpression* >::const_iterator ddIt = mDataDefinedProperties.constBegin();
  for ( ; ddIt != mDataDefinedProperties.constEnd(); ++ddIt )
  {
    if ( ddIt.value() )
    {
      columns.append( ddIt.value()->referencedColumns() );
    }
  }

  QSet<QString> attributes;
  QStringList::const_iterator it = columns.constBegin();
  for ( ; it != columns.constEnd(); ++it )
  {
    attributes.insert( *it );
  }

  return attributes;
}

void QgsSymbolLayerV2::saveDataDefinedProperties( QgsStringMap& stringMap ) const
{
  QMap< QString, QgsExpression* >::const_iterator ddIt = mDataDefinedProperties.constBegin();
  for ( ; ddIt != mDataDefinedProperties.constEnd(); ++ddIt )
  {
    if ( ddIt.value() )
    {
      stringMap.insert( ddIt.key() + "_expression", ddIt.value()->expression() );
    }
  }
}

void QgsSymbolLayerV2::copyDataDefinedProperties( QgsSymbolLayerV2* destLayer ) const
{
  if ( !destLayer )
    return;

  destLayer->removeDataDefinedProperties();

  QMap< QString, QgsExpression* >::const_iterator ddIt = mDataDefinedProperties.constBegin();
  for ( ; ddIt != mDataDefinedProperties.constEnd(); ++ddIt )
  {
    if ( ddIt.value() )
    {
      destLayer->setDataDefinedProperty( ddIt.key(), ddIt.value()->expression() );
    }
  }
}


QgsMarkerSymbolLayerV2::QgsMarkerSymbolLayerV2( bool locked )
    : QgsSymbolLayerV2( QgsSymbolV2::Marker, locked )
    , mSizeUnit( QgsSymbolV2::MM )
    , mOffsetUnit( QgsSymbolV2::MM )
    , mHorizontalAnchorPoint( HCenter )
    , mVerticalAnchorPoint( VCenter )
{
  mOffsetExpression = NULL;
  mHorizontalAnchorExpression = NULL;
  mVerticalAnchorExpression = NULL;
}

QgsLineSymbolLayerV2::QgsLineSymbolLayerV2( bool locked )
    : QgsSymbolLayerV2( QgsSymbolV2::Line, locked )
    , mWidthUnit( QgsSymbolV2::MM )
{
}

QgsFillSymbolLayerV2::QgsFillSymbolLayerV2( bool locked )
    : QgsSymbolLayerV2( QgsSymbolV2::Fill, locked )
    , mAngle( 0.0 )
{
}

void QgsMarkerSymbolLayerV2::startRender( QgsSymbolV2RenderContext& context )
{
  Q_UNUSED( context );
  mOffsetExpression = expression( "offset" );
  mHorizontalAnchorExpression = expression( "horizontal_anchor_point" );
  mVerticalAnchorExpression = expression( "vertical_anchor_point" );
}

void QgsMarkerSymbolLayerV2::drawPreviewIcon( QgsSymbolV2RenderContext& context, QSize size )
{
  startRender( context );
  renderPoint( QPointF( size.width() / 2, size.height() / 2 ), context );
  stopRender( context );
}

void QgsMarkerSymbolLayerV2::markerOffset( const QgsSymbolV2RenderContext& context, double& offsetX, double& offsetY ) const
{
  markerOffset( context, mSize, mSize, mSizeUnit, mSizeUnit, offsetX, offsetY, mSizeMapUnitScale, mSizeMapUnitScale );
}

void QgsMarkerSymbolLayerV2::markerOffset( const QgsSymbolV2RenderContext& context, double width, double height, double& offsetX, double& offsetY ) const
{
  markerOffset( context, width, height, mSizeUnit, mSizeUnit, offsetX, offsetY, mSizeMapUnitScale, mSizeMapUnitScale );
}

void QgsMarkerSymbolLayerV2::markerOffset( const QgsSymbolV2RenderContext& context, double width, double height,
    QgsSymbolV2::OutputUnit widthUnit, QgsSymbolV2::OutputUnit heightUnit,
    double& offsetX, double& offsetY, const QgsMapUnitScale& widthMapUnitScale, const QgsMapUnitScale& heightMapUnitScale ) const
{
  offsetX = mOffset.x();
  offsetY = mOffset.y();

  if ( mOffsetExpression )
  {
    QPointF offset = QgsSymbolLayerV2Utils::decodePoint( mOffsetExpression->evaluate( const_cast<QgsFeature*>( context.feature() ) ).toString() );
    offsetX = offset.x();
    offsetY = offset.y();
  }

  offsetX *= QgsSymbolLayerV2Utils::lineWidthScaleFactor( context.renderContext(), mOffsetUnit, mOffsetMapUnitScale );
  offsetY *= QgsSymbolLayerV2Utils::lineWidthScaleFactor( context.renderContext(), mOffsetUnit, mOffsetMapUnitScale );

  HorizontalAnchorPoint horizontalAnchorPoint = mHorizontalAnchorPoint;
  VerticalAnchorPoint verticalAnchorPoint = mVerticalAnchorPoint;
  if ( mHorizontalAnchorExpression )
  {
    horizontalAnchorPoint = decodeHorizontalAnchorPoint( mHorizontalAnchorExpression->evaluate( const_cast<QgsFeature*>( context.feature() ) ).toString() );
  }
  if ( mVerticalAnchorExpression )
  {
    verticalAnchorPoint = decodeVerticalAnchorPoint( mVerticalAnchorExpression->evaluate( const_cast<QgsFeature*>( context.feature() ) ).toString() );
  }

  //correct horizontal position according to anchor point
  if ( horizontalAnchorPoint == HCenter && verticalAnchorPoint == VCenter )
  {
    return;
  }

  double anchorPointCorrectionX = width * QgsSymbolLayerV2Utils::lineWidthScaleFactor( context.renderContext(), widthUnit, widthMapUnitScale ) / 2.0;
  double anchorPointCorrectionY = height * QgsSymbolLayerV2Utils::lineWidthScaleFactor( context.renderContext(), heightUnit, heightMapUnitScale ) / 2.0;
  if ( horizontalAnchorPoint == Left )
  {
    offsetX += anchorPointCorrectionX;
  }
  else if ( horizontalAnchorPoint == Right )
  {
    offsetX -= anchorPointCorrectionX;
  }

  //correct vertical position according to anchor point
  if ( verticalAnchorPoint == Top )
  {
    offsetY += anchorPointCorrectionY;
  }
  else if ( verticalAnchorPoint == Bottom )
  {
    offsetY -= anchorPointCorrectionY;
  }
}

QPointF QgsMarkerSymbolLayerV2::_rotatedOffset( const QPointF& offset, double angle )
{
  angle = DEG2RAD( angle );
  double c = cos( angle ), s = sin( angle );
  return QPointF( offset.x() * c - offset.y() * s, offset.x() * s + offset.y() * c );
}

QgsMarkerSymbolLayerV2::HorizontalAnchorPoint QgsMarkerSymbolLayerV2::decodeHorizontalAnchorPoint( const QString& str )
{
  if ( str.compare( "left", Qt::CaseInsensitive ) == 0 )
  {
    return QgsMarkerSymbolLayerV2::Left;
  }
  else if ( str.compare( "right", Qt::CaseInsensitive ) == 0 )
  {
    return QgsMarkerSymbolLayerV2::Right;
  }
  else
  {
    return QgsMarkerSymbolLayerV2::HCenter;
  }
}

QgsMarkerSymbolLayerV2::VerticalAnchorPoint QgsMarkerSymbolLayerV2::decodeVerticalAnchorPoint( const QString& str )
{
  if ( str.compare( "top", Qt::CaseInsensitive ) == 0 )
  {
    return QgsMarkerSymbolLayerV2::Top;
  }
  else if ( str.compare( "bottom", Qt::CaseInsensitive ) == 0 )
  {
    return QgsMarkerSymbolLayerV2::Bottom;
  }
  else
  {
    return QgsMarkerSymbolLayerV2::VCenter;
  }
}

void QgsMarkerSymbolLayerV2::setOutputUnit( QgsSymbolV2::OutputUnit unit )
{
  mSizeUnit = unit;
  mOffsetUnit = unit;
}

QgsSymbolV2::OutputUnit QgsMarkerSymbolLayerV2::outputUnit() const
{
  if ( mOffsetUnit != mSizeUnit )
  {
    return QgsSymbolV2::Mixed;
  }
  return mOffsetUnit;
}

void QgsMarkerSymbolLayerV2::setMapUnitScale( const QgsMapUnitScale &scale )
{
  mSizeMapUnitScale = scale;
  mOffsetMapUnitScale = scale;
}

QgsMapUnitScale QgsMarkerSymbolLayerV2::mapUnitScale() const
{
  if ( mSizeMapUnitScale == mOffsetMapUnitScale )
  {
    return mSizeMapUnitScale;
  }
  return QgsMapUnitScale();
}

void QgsLineSymbolLayerV2::setOutputUnit( QgsSymbolV2::OutputUnit unit )
{
  mWidthUnit = unit;
}

QgsSymbolV2::OutputUnit QgsLineSymbolLayerV2::outputUnit() const
{
  return mWidthUnit;
}

void QgsLineSymbolLayerV2::setMapUnitScale( const QgsMapUnitScale& scale )
{
  mWidthMapUnitScale = scale;
}

QgsMapUnitScale QgsLineSymbolLayerV2::mapUnitScale() const
{
  return mWidthMapUnitScale;
}


void QgsLineSymbolLayerV2::drawPreviewIcon( QgsSymbolV2RenderContext& context, QSize size )
{
  QPolygonF points;
  // we're adding 0.5 to get rid of blurred preview:
  // drawing antialiased lines of width 1 at (x,0)-(x,100) creates 2px line
  points << QPointF( 0, size.height() / 2 + 0.5 ) << QPointF( size.width(), size.height() / 2 + 0.5 );

  startRender( context );
  renderPolyline( points, context );
  stopRender( context );
}

void QgsLineSymbolLayerV2::renderPolygonOutline( const QPolygonF& points, QList<QPolygonF>* rings, QgsSymbolV2RenderContext& context )
{
  renderPolyline( points, context );
  if ( rings )
  {
    foreach ( const QPolygonF& ring, *rings )
      renderPolyline( ring, context );
  }
}

double QgsLineSymbolLayerV2::dxfWidth( const QgsDxfExport& e, const QgsSymbolV2RenderContext& context ) const
{
  Q_UNUSED( context );
  return ( width() * e.mapUnitScaleFactor( e.symbologyScaleDenominator(), widthUnit(), e.mapUnits() ) );
}


void QgsFillSymbolLayerV2::drawPreviewIcon( QgsSymbolV2RenderContext& context, QSize size )
{
  QPolygonF poly = QRectF( QPointF( 0, 0 ), QPointF( size.width(), size.height() ) );
  startRender( context );
  renderPolygon( poly, NULL, context );
  stopRender( context );
}

void QgsFillSymbolLayerV2::_renderPolygon( QPainter* p, const QPolygonF& points, const QList<QPolygonF>* rings, QgsSymbolV2RenderContext& context )
{
  if ( !p )
  {
    return;
  }

  // Disable 'Antialiasing' if the geometry was generalized in the current RenderContext (We known that it must have least #5 points).
  if ( points.size() <= 5 &&
       ( context.renderContext().vectorSimplifyMethod().simplifyHints() & QgsVectorSimplifyMethod::AntialiasingSimplification ) &&
       QgsAbstractGeometrySimplifier::isGeneralizableByDeviceBoundingBox( points, context.renderContext().vectorSimplifyMethod().threshold() ) &&
       ( p->renderHints() & QPainter::Antialiasing ) )
  {
    p->setRenderHint( QPainter::Antialiasing, false );
    p->drawRect( points.boundingRect() );
    p->setRenderHint( QPainter::Antialiasing, true );
    return;
  }

  if ( rings == NULL )
  {
    // simple polygon without holes
    p->drawPolygon( points );
  }
  else
  {
    // polygon with holes must be drawn using painter path
    QPainterPath path;
    QPolygonF outerRing = points;
    path.addPolygon( outerRing );

    QList<QPolygonF>::const_iterator it = rings->constBegin();
    for ( ; it != rings->constEnd(); ++it )
    {
      QPolygonF ring = *it;
      path.addPolygon( ring );
    }

    p->drawPath( path );
  }
}

void QgsMarkerSymbolLayerV2::toSld( QDomDocument &doc, QDomElement &element, QgsStringMap props ) const
{
  QDomElement symbolizerElem = doc.createElement( "se:PointSymbolizer" );
  if ( !props.value( "uom", "" ).isEmpty() )
    symbolizerElem.setAttribute( "uom", props.value( "uom", "" ) );
  element.appendChild( symbolizerElem );

  // <Geometry>
  QgsSymbolLayerV2Utils::createGeometryElement( doc, symbolizerElem, props.value( "geom", "" ) );

  writeSldMarker( doc, symbolizerElem, props );
}


