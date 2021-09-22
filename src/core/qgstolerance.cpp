/***************************************************************************
    qgstolerance.cpp  -  wrapper for tolerance handling
    ----------------------
    begin                : March 2009
    copyright            : (C) 2009 by Richard Kostecky
    email                : csf.kostej at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstolerance.h"
#include "qgsmapsettings.h"
#include "qgssettingsregistrycore.h"
#include "qgspointxy.h"

#include <QPoint>
#include <cmath>


// return ratio [mu/lu] between map units and layer units
// this is of course only an approximation
double _ratioMU2LU( const QgsMapSettings &mapSettings, QgsMapLayer *layer )
{
  const double distMU = mapSettings.mapUnitsPerPixel();
  const QgsPointXY ptMapCenterMU = mapSettings.visibleExtent().center();
  const QgsPointXY ptMapCenterRightMU( ptMapCenterMU.x() + distMU, ptMapCenterMU.y() );
  const QgsPointXY ptMapCenterLU = mapSettings.mapToLayerCoordinates( layer, ptMapCenterMU );
  const QgsPointXY ptMapCenterRightLU = mapSettings.mapToLayerCoordinates( layer, ptMapCenterRightMU );
  const double distLU = std::sqrt( ptMapCenterLU.sqrDist( ptMapCenterRightLU ) );
  const double ratio = distMU / distLU;
  return ratio;
}

double QgsTolerance::toleranceInProjectUnits( double tolerance, QgsMapLayer *layer, const QgsMapSettings &mapSettings, QgsTolerance::UnitType units )
{
  // converts to map units
  if ( units == ProjectUnits )
    return tolerance;
  else if ( units == Pixels )
    return tolerance * mapSettings.mapUnitsPerPixel();
  else // units == LayerUnits
  {
    // [mu] = [lu] * [mu/lu]
    return tolerance * _ratioMU2LU( mapSettings, layer );
  }
}


double QgsTolerance::toleranceInMapUnits( double tolerance, QgsMapLayer *layer, const QgsMapSettings &mapSettings, QgsTolerance::UnitType units )
{
  // converts to layer units
  if ( units == LayerUnits )
  {
    return tolerance;
  }
  else if ( units == Pixels )
  {
    const double layerUnitsPerPixel = computeMapUnitPerPixel( layer, mapSettings );
    return tolerance * layerUnitsPerPixel;
  }
  else // ProjectUnits
  {
    // [lu] = [mu] / [mu/lu]
    return tolerance / _ratioMU2LU( mapSettings, layer );
  }
}

double QgsTolerance::vertexSearchRadius( const QgsMapSettings &mapSettings )
{
  const double tolerance = QgsSettingsRegistryCore::settingsDigitizingSearchRadiusVertexEdit.value();
  UnitType units = QgsSettingsRegistryCore::settingsDigitizingSearchRadiusVertexEditUnit.value();
  if ( units == LayerUnits )
    units = ProjectUnits;
  return toleranceInProjectUnits( tolerance, nullptr, mapSettings, units );
}

double QgsTolerance::vertexSearchRadius( QgsMapLayer *layer, const QgsMapSettings &mapSettings )
{
  const double tolerance = QgsSettingsRegistryCore::settingsDigitizingSearchRadiusVertexEdit.value();
  const UnitType units = QgsSettingsRegistryCore::settingsDigitizingSearchRadiusVertexEditUnit.value();
  return toleranceInMapUnits( tolerance, layer, mapSettings, units );
}

double QgsTolerance::defaultTolerance( QgsMapLayer *layer, const QgsMapSettings &mapSettings )
{
  const double tolerance = QgsSettingsRegistryCore::settingsDigitizingDefaultSnappingTolerance.value();
  const UnitType units = QgsSettingsRegistryCore::settingsDigitizingDefaultSnappingToleranceUnit.value();
  return toleranceInMapUnits( tolerance, layer, mapSettings, units );
}


double QgsTolerance::computeMapUnitPerPixel( QgsMapLayer *layer, const QgsMapSettings &mapSettings )
{
  // the layer is projected. Find out how many pixels are in one map unit - either horizontal and vertical direction
  // this check might not work correctly in some cases
  // (on a large area the pixels projected around "0,0" can have different properties from the actual point)
  const QgsPointXY p1 = toLayerCoordinates( layer, mapSettings, QPoint( 0, 1 ) );
  const QgsPointXY p2 = toLayerCoordinates( layer, mapSettings, QPoint( 0, 2 ) );
  const QgsPointXY p3 = toLayerCoordinates( layer, mapSettings, QPoint( 1, 0 ) );
  const QgsPointXY p4 = toLayerCoordinates( layer, mapSettings, QPoint( 2, 0 ) );
  const double x = p1.sqrDist( p2 );
  const double y = p3.sqrDist( p4 );
  if ( x > y )
  {
    return std::sqrt( x );
  }
  else
  {
    return std::sqrt( y );
  }
}


QgsPointXY QgsTolerance::toLayerCoordinates( QgsMapLayer *layer, const QgsMapSettings &mapSettings, QPoint point )
{
  const QgsPointXY pt = mapSettings.mapToPixel().toMapCoordinates( point );
  return mapSettings.mapToLayerCoordinates( layer, pt );
}
