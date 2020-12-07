/***************************************************************************
                         qgsmaplayerelevationproperties.cpp
                         ---------------
    begin                : November 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaplayerelevationproperties.h"

QgsMapLayerElevationProperties::QgsMapLayerElevationProperties( QObject *parent )
  : QObject( parent )
{
}

bool QgsMapLayerElevationProperties::hasElevation() const
{
  return false;
}

bool QgsMapLayerElevationProperties::isVisibleInZRange( const QgsDoubleRange & ) const
{
  return true;
}

QgsDoubleRange QgsMapLayerElevationProperties::calculateZRange( QgsMapLayer * ) const
{
  return QgsDoubleRange();
}
