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

QgsSymbol::SymbolType QgsLegendPatchShape::symbolType() const
{
  return mSymbolType;
}

void QgsLegendPatchShape::setSymbolType( QgsSymbol::SymbolType type )
{
  mSymbolType = type;
}
