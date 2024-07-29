/***************************************************************************
                         qgslegendpatchshape.cpp
                         -------------------
begin                : April 2020
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

#include "qgslegendpatchshape.h"
#include "qgsstyle.h"

QgsLegendPatchShape::QgsLegendPatchShape( Qgis::SymbolType type, const QgsGeometry &geometry, bool preserveAspectRatio )
  : mSymbolType( type )
  , mGeometry( geometry )
  , mPreserveAspectRatio( preserveAspectRatio )
{

}

bool QgsLegendPatchShape::isNull() const
{
  return mGeometry.isNull() || mGeometry.isEmpty();
}

QgsGeometry QgsLegendPatchShape::geometry() const
{
  return mGeometry;
}

void QgsLegendPatchShape::setGeometry( const QgsGeometry &geometry )
{
  mGeometry = geometry;
}

bool QgsLegendPatchShape::preserveAspectRatio() const
{
  return mPreserveAspectRatio;
}

void QgsLegendPatchShape::setPreserveAspectRatio( bool preserveAspectRatio )
{
  mPreserveAspectRatio = preserveAspectRatio;
}

bool QgsLegendPatchShape::scaleToOutputSize() const
{
  return mScaleToTargetSize;
}

void QgsLegendPatchShape::setScaleToOutputSize( bool scale )
{
  mScaleToTargetSize = scale;
}

QgsGeometry QgsLegendPatchShape::scaledGeometry( QSizeF size ) const
{
  QgsGeometry geom = mGeometry;
  if ( mScaleToTargetSize )
  {
    // scale and translate to desired size

    const QRectF bounds = mGeometry.boundingBox().toRectF();

    double dx = 0;
    double dy = 0;
    if ( mPreserveAspectRatio && bounds.height() > 0 && bounds.width() > 0 )
    {
      const double scaling = std::min( size.width() / bounds.width(), size.height() / bounds.height() );
      const QSizeF scaledSize = bounds.size() * scaling;
      dx = ( size.width() - scaledSize.width() ) / 2.0;
      dy = ( size.height() - scaledSize.height() ) / 2.0;
      size = scaledSize;
    }

    // important -- the transform needs to flip from north-up to painter style "increasing y down" coordinates
    const QPolygonF targetRectPoly = QPolygonF() << QPointF( dx, dy + size.height() )
                                     << QPointF( dx + size.width(), dy + size.height() )
                                     << QPointF( dx + size.width(), dy )
                                     << QPointF( dx, dy );
    QTransform t;

    if ( bounds.width() > 0 && bounds.height() > 0 )
    {
      QPolygonF patchRectPoly = QPolygonF( bounds );
      //workaround QT Bug #21329
      patchRectPoly.pop_back();

      QTransform::quadToQuad( patchRectPoly, targetRectPoly, t );
    }
    else if ( bounds.width() > 0 )
    {
      t = QTransform::fromScale( size.width() / bounds.width(), 1 ).translate( -bounds.left(), size.height() / 2 - bounds.y() );
    }
    else if ( bounds.height() > 0 )
    {
      t = QTransform::fromScale( 1, size.height() / bounds.height() ).translate( size.width() / 2 - bounds.x(), -bounds.top() );
    }

    geom.transform( t );
  }
  return geom;
}

QList<QList<QPolygonF> > QgsLegendPatchShape::toQPolygonF( Qgis::SymbolType type, QSizeF size ) const
{
  if ( isNull() || type != mSymbolType )
    return QgsStyle::defaultStyle()->defaultPatchAsQPolygonF( type, size );

  const QgsGeometry geom = scaledGeometry( size );
  if ( type == Qgis::SymbolType::Marker && ( QgsWkbTypes::flatType( geom.wkbType() ) != Qgis::WkbType::MultiPoint ) )
  {
    QPolygonF points;
    points << QPointF( size.width() / 2, size.height() / 2 );
    return QList< QList<QPolygonF> >() << ( QList< QPolygonF >() << points );
  }

  return QgsSymbolLayerUtils::toQPolygonF( geom, type );
}

void QgsLegendPatchShape::readXml( const QDomElement &element, const QgsReadWriteContext & )
{
  mGeometry = QgsGeometry::fromWkt( element.attribute( QStringLiteral( "wkt" ) ) );
  mPreserveAspectRatio = element.attribute( QStringLiteral( "preserveAspect" ) ).toInt();
  mSymbolType = static_cast< Qgis::SymbolType >( element.attribute( QStringLiteral( "type" ) ).toInt() );
}

void QgsLegendPatchShape::writeXml( QDomElement &element, QDomDocument &, const QgsReadWriteContext & ) const
{
  element.setAttribute( QStringLiteral( "wkt" ), mGeometry.isNull() ? QString() : mGeometry.asWkt( ) );
  element.setAttribute( QStringLiteral( "preserveAspect" ), mPreserveAspectRatio ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );
  element.setAttribute( QStringLiteral( "type" ), QString::number( static_cast< int >( mSymbolType ) ) );
}

Qgis::SymbolType QgsLegendPatchShape::symbolType() const
{
  return mSymbolType;
}

void QgsLegendPatchShape::setSymbolType( Qgis::SymbolType type )
{
  mSymbolType = type;
}
