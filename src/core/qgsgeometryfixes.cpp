/***************************************************************************
                          qgsgeometryfixes.cpp
                             -------------------
    begin                : Aug 23, 2018
    copyright            : (C) 2018 by Matthias Kuhn
    email                : matthias@opengis.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgeometryfixes.h"

bool QgsGeometryFixes::removeDuplicateNodes() const
{
  return mRemoveDuplicateNodes;
}

void QgsGeometryFixes::setRemoveDuplicateNodes( bool value )
{
  mRemoveDuplicateNodes = value;
}

double QgsGeometryFixes::geometryPrecision() const
{
  return mGeometryPrecision;
}

void QgsGeometryFixes::setGeometryPrecision( double value )
{
  mGeometryPrecision = value;
}

bool QgsGeometryFixes::isActive() const
{
  return mGeometryPrecision != 0.0 || mRemoveDuplicateNodes;
}

void QgsGeometryFixes::apply( QgsGeometry &geometry ) const
{
  if ( mGeometryPrecision != 0.0 )
    geometry = geometry.snappedToGrid( mGeometryPrecision, mGeometryPrecision );

  if ( mRemoveDuplicateNodes )
    geometry.removeDuplicateNodes();
}
