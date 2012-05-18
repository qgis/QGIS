/***************************************************************************
    qgssymbollayerv2.cpp
    ---------------------
    begin                : November 2009
    copyright            : (C) 2009 by Martin Dobias
    email                : wonder.sk at gmail.com
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
#include "qgsrenderer.h"
#include "qgsrendercontext.h"

#include <QSize>
#include <QPainter>
#include <QPointF>
#include <QPolygonF>



QgsMarkerSymbolLayerV2::QgsMarkerSymbolLayerV2( bool locked )
    : QgsSymbolLayerV2( QgsSymbolV2::Marker, locked )
{
}

QgsLineSymbolLayerV2::QgsLineSymbolLayerV2( bool locked )
    : QgsSymbolLayerV2( QgsSymbolV2::Line, locked )
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
    foreach( const QPolygonF& ring, *rings )
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
