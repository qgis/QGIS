/***************************************************************************
    qgsannotationlineitem.cpp
    ----------------
    begin                : July 2020
    copyright            : (C) 2020 by Nyall Dawson
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

#include "qgsannotationlineitem.h"
#include "qgssymbol.h"
#include "qgssymbollayerutils.h"
#include "qgslinesymbol.h"
#include "qgsannotationitemnode.h"

QgsAnnotationLineItem::QgsAnnotationLineItem( QgsCurve *curve )
  : QgsAnnotationItem()
  , mCurve( curve )
  , mSymbol( std::make_unique< QgsLineSymbol >() )
{

}

QgsAnnotationLineItem::~QgsAnnotationLineItem() = default;

QString QgsAnnotationLineItem::type() const
{
  return QStringLiteral( "linestring" );
}

void QgsAnnotationLineItem::render( QgsRenderContext &context, QgsFeedback * )
{
  QPolygonF pts = mCurve->asQPolygonF();

  //transform the QPolygonF to screen coordinates
  if ( context.coordinateTransform().isValid() )
  {
    try
    {
      context.coordinateTransform().transformPolygon( pts );
    }
    catch ( QgsCsException & )
    {
      // we don't abort the rendering here, instead we remove any invalid points and just plot those which ARE valid
    }
  }

  // remove non-finite points, e.g. infinite or NaN points caused by reprojecting errors
  pts.erase( std::remove_if( pts.begin(), pts.end(),
                             []( const QPointF point )
  {
    return !std::isfinite( point.x() ) || !std::isfinite( point.y() );
  } ), pts.end() );

  QPointF *ptr = pts.data();
  for ( int i = 0; i < pts.size(); ++i, ++ptr )
  {
    context.mapToPixel().transformInPlace( ptr->rx(), ptr->ry() );
  }

  mSymbol->startRender( context );
  mSymbol->renderPolyline( pts, nullptr, context );
  mSymbol->stopRender( context );
}

bool QgsAnnotationLineItem::writeXml( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context ) const
{
  element.setAttribute( QStringLiteral( "wkt" ), mCurve->asWkt() );
  element.appendChild( QgsSymbolLayerUtils::saveSymbol( QStringLiteral( "lineSymbol" ), mSymbol.get(), document, context ) );
  writeCommonProperties( element, document, context );

  return true;
}

QList<QgsAnnotationItemNode> QgsAnnotationLineItem::nodes() const
{
  QList< QgsAnnotationItemNode > res;
  for ( auto it = mCurve->vertices_begin(); it != mCurve->vertices_end(); ++it )
  {
    res.append( QgsAnnotationItemNode( QgsPointXY( ( *it ).x(), ( *it ).y() ), Qgis::AnnotationItemNodeType::VertexHandle ) );
  }
  return res;
}

QgsGeometry QgsAnnotationLineItem::rubberBandGeometry() const
{
  return QgsGeometry( mCurve->clone() );
}

bool QgsAnnotationLineItem::transform( const QTransform &transform )
{
  mCurve->transform( transform );
  return true;
}

QgsAnnotationLineItem *QgsAnnotationLineItem::create()
{
  return new QgsAnnotationLineItem( new QgsLineString() );
}

bool QgsAnnotationLineItem::readXml( const QDomElement &element, const QgsReadWriteContext &context )
{
  const QString wkt = element.attribute( QStringLiteral( "wkt" ) );
  const QgsGeometry geometry = QgsGeometry::fromWkt( wkt );
  if ( const QgsCurve *curve = qgsgeometry_cast< const QgsCurve * >( geometry.constGet() ) )
    mCurve.reset( curve->clone() );

  const QDomElement symbolElem = element.firstChildElement( QStringLiteral( "symbol" ) );
  if ( !symbolElem.isNull() )
    setSymbol( QgsSymbolLayerUtils::loadSymbol< QgsLineSymbol >( symbolElem, context ) );

  readCommonProperties( element, context );

  return true;
}

QgsRectangle QgsAnnotationLineItem::boundingBox() const
{
  return mCurve->boundingBox();
}

QgsAnnotationLineItem *QgsAnnotationLineItem::clone()
{
  std::unique_ptr< QgsAnnotationLineItem > item = std::make_unique< QgsAnnotationLineItem >( mCurve->clone() );
  item->setSymbol( mSymbol->clone() );
  item->copyCommonProperties( this );
  return item.release();
}

const QgsLineSymbol *QgsAnnotationLineItem::symbol() const
{
  return mSymbol.get();
}

void QgsAnnotationLineItem::setSymbol( QgsLineSymbol *symbol )
{
  mSymbol.reset( symbol );
}
