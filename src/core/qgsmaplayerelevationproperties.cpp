/***************************************************************************
                         qgsmaplayerelevationproperties.cpp
                         ---------------
    begin                : November 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
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
