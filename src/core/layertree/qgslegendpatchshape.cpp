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


QgsLegendPatchShape::QgsLegendPatchShape( QgsSymbol::SymbolType type, const QgsGeometry &geometry, bool preserveAspectRatio )
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

QVector<QPolygonF> QgsLegendPatchShape::defaultPatch( QgsSymbol::SymbolType type, QSizeF size )
{
  switch ( type )
  {
    case QgsSymbol::Marker:
      return QVector< QPolygonF >() << ( QPolygonF() << QPointF( size.width() / 2, size.height() / 2 ) );

    case QgsSymbol::Line:
      // we're adding 0.5 to get rid of blurred preview:
      // drawing antialiased lines of width 1 at (x,0)-(x,100) creates 2px line
      return QVector< QPolygonF >() << ( QPolygonF()  << QPointF( 0, int( size.height() / 2 ) + 0.5 ) << QPointF( size.width(), int( size.height() / 2 ) + 0.5 ) );

    case QgsSymbol::Fill:
      return QVector< QPolygonF >() << QRectF( QPointF( 0, 0 ), QPointF( size.width(), size.height() ) );

    case QgsSymbol::Hybrid:
      return QVector<QPolygonF>();
  }

  return QVector<QPolygonF>();
}

QgsSymbol::SymbolType QgsLegendPatchShape::symbolType() const
{
  return mSymbolType;
}

void QgsLegendPatchShape::setSymbolType( QgsSymbol::SymbolType type )
{
  mSymbolType = type;
}
