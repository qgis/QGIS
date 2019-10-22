/***************************************************************************
    qgsannotationitem.cpp
    ------------------
    copyright            : (C) 2019 by Sandro Mani
    email                : smani at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsannotationitem.h"
#include "qgssymbol.h"
#include "qgssymbollayerutils.h"


QgsAnnotationItem::QgsAnnotationItem( const QgsCoordinateReferenceSystem &crs )
  : mCrs( crs )
{

}

QgsMarkerItem::QgsMarkerItem( QgsPointXY point, const QgsCoordinateReferenceSystem &crs )
  : QgsAnnotationItem( crs )
  , mPoint( point )
  , mSymbol( qgis::make_unique< QgsMarkerSymbol >() )
{

}

QgsMarkerItem::~QgsMarkerItem() = default;

QString QgsMarkerItem::type() const
{
  return QStringLiteral( "marker" );
}

void QgsMarkerItem::render( QgsRenderContext &context, QgsFeedback * )
{
  QPointF pt;
  if ( context.coordinateTransform().isValid() )
  {
    double x = mPoint.x();
    double y = mPoint.y();
    double z = 0.0;
    context.coordinateTransform().transformInPlace( x, y, z );
    pt = QPointF( x, y );
  }
  else
    pt = mPoint.toQPointF();

  context.mapToPixel().transformInPlace( pt.rx(), pt.ry() );

  mSymbol->startRender( context );
  mSymbol->renderPoint( pt, nullptr, context );
  mSymbol->stopRender( context );
}

bool QgsMarkerItem::writeXml( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context ) const
{
  element.setAttribute( QStringLiteral( "x" ), qgsDoubleToString( mPoint.x() ) );
  element.setAttribute( QStringLiteral( "y" ), qgsDoubleToString( mPoint.y() ) );
  crs().writeXml( element, document );

  element.appendChild( QgsSymbolLayerUtils::saveSymbol( QStringLiteral( "markerSymbol" ), mSymbol.get(), document, context ) );

  return true;
}

QgsMarkerItem *QgsMarkerItem::create()
{
  return new QgsMarkerItem( QgsPointXY(), QgsCoordinateReferenceSystem() );
}

QgsMarkerItem *QgsMarkerItem::createFromElement( const QDomElement &element, const QgsReadWriteContext &context )
{
  const double x = element.attribute( QStringLiteral( "x" ) ).toDouble();
  const double y = element.attribute( QStringLiteral( "y" ) ).toDouble();

  QgsCoordinateReferenceSystem crs;
  crs.readXml( element );

  std::unique_ptr< QgsMarkerItem > item = qgis::make_unique< QgsMarkerItem >( QgsPointXY( x, y ), crs );

  const QDomElement symbolElem = element.firstChildElement( QStringLiteral( "symbol" ) );
  if ( !symbolElem.isNull() )
    item->setSymbol( QgsSymbolLayerUtils::loadSymbol< QgsMarkerSymbol >( symbolElem, context ) );

  return item.release();
}

QgsMarkerItem *QgsMarkerItem::clone()
{
  std::unique_ptr< QgsMarkerItem > item = qgis::make_unique< QgsMarkerItem >( mPoint, crs() );
  item->setSymbol( mSymbol->clone() );
  return item.release();
}

const QgsMarkerSymbol *QgsMarkerItem::symbol() const
{
  return mSymbol.get();
}

void QgsMarkerItem::setSymbol( QgsMarkerSymbol *symbol )
{
  mSymbol.reset( symbol );
}




QgsLineStringItem::QgsLineStringItem( const QgsLineString &linestring, const QgsCoordinateReferenceSystem &crs )
  : QgsAnnotationItem( crs )
  , mLineString( linestring )
  , mSymbol( qgis::make_unique< QgsLineSymbol >() )
{

}

QgsLineStringItem::~QgsLineStringItem() = default;

QString QgsLineStringItem::type() const
{
  return QStringLiteral( "linestring" );
}

void QgsLineStringItem::render( QgsRenderContext &context, QgsFeedback * )
{
  QPolygonF pts = mLineString.asQPolygonF();

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

bool QgsLineStringItem::writeXml( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context ) const
{
  element.setAttribute( QStringLiteral( "wkt" ), mLineString.asWkt() );
  crs().writeXml( element, document );

  element.appendChild( QgsSymbolLayerUtils::saveSymbol( QStringLiteral( "lineSymbol" ), mSymbol.get(), document, context ) );

  return true;
}

QgsLineStringItem *QgsLineStringItem::create()
{
  return new QgsLineStringItem( QgsLineString(), QgsCoordinateReferenceSystem() );
}

QgsLineStringItem *QgsLineStringItem::createFromElement( const QDomElement &element, const QgsReadWriteContext &context )
{
  const QString wkt = element.attribute( QStringLiteral( "wkt" ) );
  QgsLineString ls;
  ls.fromWkt( wkt );

  QgsCoordinateReferenceSystem crs;
  crs.readXml( element );

  std::unique_ptr< QgsLineStringItem > item = qgis::make_unique< QgsLineStringItem >( ls, crs );

  const QDomElement symbolElem = element.firstChildElement( QStringLiteral( "symbol" ) );
  if ( !symbolElem.isNull() )
    item->setSymbol( QgsSymbolLayerUtils::loadSymbol< QgsLineSymbol >( symbolElem, context ) );

  return item.release();
}

QgsLineStringItem *QgsLineStringItem::clone()
{
  std::unique_ptr< QgsLineStringItem > item = qgis::make_unique< QgsLineStringItem >( mLineString, crs() );
  item->setSymbol( mSymbol->clone() );
  return item.release();
}

const QgsLineSymbol *QgsLineStringItem::symbol() const
{
  return mSymbol.get();
}

void QgsLineStringItem::setSymbol( QgsLineSymbol *symbol )
{
  mSymbol.reset( symbol );
}



QgsPolygonItem::QgsPolygonItem( const QgsPolygon &polygon, const QgsCoordinateReferenceSystem &crs )
  : QgsAnnotationItem( crs )
  , mPolygon( polygon )
  , mSymbol( qgis::make_unique< QgsFillSymbol >() )
{

}

QgsPolygonItem::~QgsPolygonItem() = default;

QString QgsPolygonItem::type() const
{
  return QStringLiteral( "polygon" );
}

void QgsPolygonItem::render( QgsRenderContext &context, QgsFeedback * )
{

  auto transformRing = [&context]( QPolygonF & pts )
  {
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
  };

  QPolygonF exterior = mPolygon.exteriorRing()->asQPolygonF();
  transformRing( exterior );
  QList<QPolygonF> rings;
  rings.reserve( mPolygon.numInteriorRings() );
  for ( int i = 0; i < mPolygon.numInteriorRings(); ++i )
  {
    QPolygonF ring = mPolygon.interiorRing( i )->asQPolygonF();
    transformRing( ring );
    rings.append( ring );
  }

  mSymbol->startRender( context );
  mSymbol->renderPolygon( exterior, rings.empty() ? nullptr : &rings, nullptr, context );
  mSymbol->stopRender( context );
}

bool QgsPolygonItem::writeXml( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context ) const
{
  element.setAttribute( QStringLiteral( "wkt" ), mPolygon.asWkt() );
  crs().writeXml( element, document );

  element.appendChild( QgsSymbolLayerUtils::saveSymbol( QStringLiteral( "lineSymbol" ), mSymbol.get(), document, context ) );

  return true;
}

QgsPolygonItem *QgsPolygonItem::create()
{
  return new QgsPolygonItem( QgsPolygon(), QgsCoordinateReferenceSystem() );
}

QgsPolygonItem *QgsPolygonItem::createFromElement( const QDomElement &element, const QgsReadWriteContext &context )
{
  const QString wkt = element.attribute( QStringLiteral( "wkt" ) );
  QgsPolygon poly;
  poly.fromWkt( wkt );

  QgsCoordinateReferenceSystem crs;
  crs.readXml( element );

  std::unique_ptr< QgsPolygonItem > item = qgis::make_unique< QgsPolygonItem >( poly, crs );

  const QDomElement symbolElem = element.firstChildElement( QStringLiteral( "symbol" ) );
  if ( !symbolElem.isNull() )
    item->setSymbol( QgsSymbolLayerUtils::loadSymbol< QgsFillSymbol >( symbolElem, context ) );

  return item.release();
}

QgsPolygonItem *QgsPolygonItem::clone()
{
  std::unique_ptr< QgsPolygonItem > item = qgis::make_unique< QgsPolygonItem >( mPolygon, crs() );
  item->setSymbol( mSymbol->clone() );
  return item.release();
}

const QgsFillSymbol *QgsPolygonItem::symbol() const
{
  return mSymbol.get();
}

void QgsPolygonItem::setSymbol( QgsFillSymbol *symbol )
{
  mSymbol.reset( symbol );
}
