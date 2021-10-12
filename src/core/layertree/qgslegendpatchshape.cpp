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
#include "qgsmultilinestring.h"
#include "qgslinestring.h"
#include "qgspolygon.h"
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

QPolygonF lineStringToQPolygonF( const QgsLineString *line )
{
  const double *srcX = line->xData();
  const double *srcY = line->yData();
  const int count = line->numPoints();
  QPolygonF thisRes( count );
  QPointF *dest = thisRes.data();
  for ( int i = 0; i < count; ++i )
  {
    *dest++ = QPointF( *srcX++, *srcY++ );
  }
  return thisRes;
}

QPolygonF curveToPolygonF( const QgsCurve *curve )
{
  if ( const QgsLineString *line = qgsgeometry_cast< const QgsLineString * >( curve ) )
  {
    return lineStringToQPolygonF( line );
  }
  else
  {
    const std::unique_ptr< QgsLineString > straightened( curve->curveToLine() );
    return lineStringToQPolygonF( straightened.get() );
  }
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

  switch ( mSymbolType )
  {
    case Qgis::SymbolType::Marker:
    {
      QPolygonF points;

      if ( QgsWkbTypes::flatType( mGeometry.wkbType() ) == QgsWkbTypes::MultiPoint )
      {
        const QgsGeometry patch = geom;
        for ( auto it = patch.vertices_begin(); it != patch.vertices_end(); ++it )
          points << QPointF( ( *it ).x(), ( *it ).y() );
      }
      else
      {
        points << QPointF( static_cast< int >( size.width() ) / 2, static_cast< int >( size.height() ) / 2 );
      }
      return QList< QList<QPolygonF> >() << ( QList< QPolygonF >() << points );
    }

    case Qgis::SymbolType::Line:
    {
      QList< QList<QPolygonF> > res;
      const QgsGeometry patch = geom;
      if ( QgsWkbTypes::geometryType( mGeometry.wkbType() ) == QgsWkbTypes::LineGeometry )
      {
        for ( auto it = patch.const_parts_begin(); it != patch.const_parts_end(); ++it )
        {
          res << ( QList< QPolygonF >() << curveToPolygonF( qgsgeometry_cast< const QgsCurve * >( *it ) ) );
        }
      }
      return res;
    }

    case Qgis::SymbolType::Fill:
    {
      QList< QList<QPolygonF> > res;

      const QgsGeometry patch = geom;
      for ( auto it = patch.const_parts_begin(); it != patch.const_parts_end(); ++it )
      {
        QList<QPolygonF> thisPart;
        const QgsCurvePolygon *surface = qgsgeometry_cast< const QgsCurvePolygon * >( *it );
        if ( !surface )
          continue;

        if ( !surface->exteriorRing() )
          continue;

        thisPart << curveToPolygonF( surface->exteriorRing() );

        for ( int i = 0; i < surface->numInteriorRings(); ++i )
          thisPart << curveToPolygonF( surface->interiorRing( i ) );
        res << thisPart;
      }

      return res;
    }

    case Qgis::SymbolType::Hybrid:
      return QList< QList<QPolygonF> >();
  }

  return QList< QList<QPolygonF> >();
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
