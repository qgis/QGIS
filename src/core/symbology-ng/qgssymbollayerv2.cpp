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

QgsExpression* QgsSymbolLayerV2::expression( const QString& property )
{
  QMap< QString, QgsExpression* >::iterator it = mDataDefinedProperties.find( property );
  if ( it != mDataDefinedProperties.end() )
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

void QgsSymbolLayerV2::prepareExpressions( const QgsVectorLayer* vl )
{
  if ( !vl )
  {
    return;
  }

  const QgsFields& fields = vl->pendingFields();
  QMap< QString, QgsExpression* >::iterator it = mDataDefinedProperties.begin();
  for ( ; it != mDataDefinedProperties.end(); ++it )
  {
    if ( it.value() )
    {
      it.value()->prepare( fields );
    }
  }
}

QSet<QString> QgsSymbolLayerV2::usedAttributes() const
{
  QSet<QString> attributes;
  QStringList columns;

  QMap< QString, QgsExpression* >::const_iterator ddIt = mDataDefinedProperties.constBegin();
  for ( ; ddIt != mDataDefinedProperties.constEnd(); ++ddIt )
  {
    if ( ddIt.value() )
    {
      columns.append( ddIt.value()->referencedColumns() );
    }
  }

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
  {
    return;
  }
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
    : QgsSymbolLayerV2( QgsSymbolV2::Marker, locked ), mSizeUnit( QgsSymbolV2::MM ),  mOffsetUnit( QgsSymbolV2::MM )
{
}

QgsLineSymbolLayerV2::QgsLineSymbolLayerV2( bool locked )
    : QgsSymbolLayerV2( QgsSymbolV2::Line, locked ), mWidthUnit( QgsSymbolV2::MM )
{
}

QgsFillSymbolLayerV2::QgsFillSymbolLayerV2( bool locked )
    : QgsSymbolLayerV2( QgsSymbolV2::Fill, locked ), mAngle( 0.0 )
{
}

void QgsMarkerSymbolLayerV2::drawPreviewIcon( QgsSymbolV2RenderContext& context, QSize size )
{
  startRender( context );
  renderPoint( QPointF( size.width() / 2, size.height() / 2 ), context );
  stopRender( context );
}

void QgsMarkerSymbolLayerV2::setOutputUnit( QgsSymbolV2::OutputUnit unit )
{
  mSizeUnit = unit;
  mOffsetUnit = unit;
}

void QgsMarkerSymbolLayerV2::markerOffset( QgsSymbolV2RenderContext& context, double& offsetX, double& offsetY )
{
  offsetX = mOffset.x();
  offsetY = mOffset.y();

  QgsExpression* offsetExpression = expression( "offset" );
  if ( offsetExpression )
  {
    QPointF offset = QgsSymbolLayerV2Utils::decodePoint( offsetExpression->evaluate( const_cast<QgsFeature*>( context.feature() ) ).toString() );
    offsetX = offset.x();
    offsetY = offset.y();
  }

  offsetX *= QgsSymbolLayerV2Utils::lineWidthScaleFactor( context.renderContext(), mOffsetUnit );
  offsetY *= QgsSymbolLayerV2Utils::lineWidthScaleFactor( context.renderContext(), mOffsetUnit );
}

QPointF QgsMarkerSymbolLayerV2::_rotatedOffset( const QPointF& offset, double angle )
{
  angle = DEG2RAD( angle );
  double c = cos( angle ), s = sin( angle );
  return QPointF( offset.x() * c - offset.y() * s, offset.x() * s + offset.y() * c );
}

QgsSymbolV2::OutputUnit QgsMarkerSymbolLayerV2::outputUnit() const
{
  QgsSymbolV2::OutputUnit unit = mSizeUnit;
  if ( mOffsetUnit != unit )
  {
    return QgsSymbolV2::Mixed;
  }
  return unit;
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


void QgsFillSymbolLayerV2::drawPreviewIcon( QgsSymbolV2RenderContext& context, QSize size )
{
  QPolygonF poly = QRectF( QPointF( 0, 0 ), QPointF( size.width() - 1, size.height() - 1 ) );
  startRender( context );
  renderPolygon( poly, NULL, context );
  stopRender( context );
}

void QgsFillSymbolLayerV2::_renderPolygon( QPainter* p, const QPolygonF& points, const QList<QPolygonF>* rings )
{
  if ( !p )
  {
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
