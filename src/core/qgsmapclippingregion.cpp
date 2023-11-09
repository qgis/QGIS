/***************************************************************************
  qgsmapclippingregion.cpp
  --------------------------------------
  Date                 : June 2020
  Copyright            : (C) 2020 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmapclippingregion.h"
#include "qgsmaplayerlistutils_p.h"
#include <algorithm>

QgsGeometry QgsMapClippingRegion::geometry() const
{
  return mGeometry;
}

void QgsMapClippingRegion::setGeometry( const QgsGeometry &geometry )
{
  mGeometry = geometry;
}

void QgsMapClippingRegion::setRestrictedLayers( const QList<QgsMapLayer *> &layers )
{
  mRestrictToLayersList = _qgis_listRawToQPointer( layers );
}

QList<QgsMapLayer *> QgsMapClippingRegion::restrictedLayers() const
{
  return _qgis_listQPointerToRaw( mRestrictToLayersList );
}

bool QgsMapClippingRegion::appliesToLayer( const QgsMapLayer *layer ) const
{
  if ( !mRestrictToLayers )
    return true;

  if ( mRestrictToLayersList.empty() )
    return false;

  const auto it = std::find_if( mRestrictToLayersList.begin(), mRestrictToLayersList.end(), [layer]( const QgsWeakMapLayerPointer & item ) -> bool
  {
    return item == layer;
  } );
  return it != mRestrictToLayersList.end();
}

bool QgsMapClippingRegion::restrictToLayers() const
{
  return mRestrictToLayers;
}

void QgsMapClippingRegion::setRestrictToLayers( bool enabled )
{
  mRestrictToLayers = enabled;
}


