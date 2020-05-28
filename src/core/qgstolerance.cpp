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
#include "qgssettings.h"
#include "qgspointxy.h"

#include <QPoint>
#include <cmath>


// return ratio [mu/lu] between map units and layer units
// this is of course only an approximation
double _ratioMU2LU( const QgsMapSettings &mapSettings, QgsMapLayer *layer )
{
  double distMU = mapSettings.mapUnitsPerPixel();
  QgsPointXY ptMapCenterMU = mapSettings.visibleExtent().center();
  QgsPointXY ptMapCenterRightMU( ptMapCenterMU.x() + distMU, ptMapCenterMU.y() );
  QgsPointXY ptMapCenterLU = mapSettings.mapToLayerCoordinates( layer, ptMapCenterMU );
  QgsPointXY ptMapCenterRightLU = mapSettings.mapToLayerCoordinates( layer, ptMapCenterRightMU );
  double distLU = std::sqrt( ptMapCenterLU.sqrDist( ptMapCenterRightLU ) );
  double ratio = distMU / distLU;
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
    double layerUnitsPerPixel = computeMapUnitPerPixel( layer, mapSettings );
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
  QgsSettings settings;
  double tolerance = settings.value( QStringLiteral( "qgis/digitizing/search_radius_vertex_edit" ), 10 ).toDouble();
  UnitType units = settings.enumValue( QStringLiteral( "qgis/digitizing/search_radius_vertex_edit_unit" ),  QgsTolerance::Pixels );
  if ( units == LayerUnits )
    units = ProjectUnits;
  return toleranceInProjectUnits( tolerance, nullptr, mapSettings, units );
}

double QgsTolerance::vertexSearchRadius( QgsMapLayer *layer, const QgsMapSettings &mapSettings )
{
  QgsSettings settings;
  double tolerance = settings.value( QStringLiteral( "qgis/digitizing/search_radius_vertex_edit" ), 10 ).toDouble();
  UnitType units = settings.enumValue( QStringLiteral( "qgis/digitizing/search_radius_vertex_edit_unit" ),  QgsTolerance::Pixels );
  return toleranceInMapUnits( tolerance, layer, mapSettings, units );
}

double QgsTolerance::defaultTolerance( QgsMapLayer *layer, const QgsMapSettings &mapSettings )
{
  QgsSettings settings;
  double tolerance = settings.value( QStringLiteral( "qgis/digitizing/default_snapping_tolerance" ), Qgis::DEFAULT_SNAP_TOLERANCE ).toDouble();
  UnitType units = settings.enumValue( QStringLiteral( "qgis/digitizing/default_snapping_tolerance_unit" ),  Qgis::DEFAULT_SNAP_UNITS );
  return toleranceInMapUnits( tolerance, layer, mapSettings, units );
}


double QgsTolerance::computeMapUnitPerPixel( QgsMapLayer *layer, const QgsMapSettings &mapSettings )
{
  // the layer is projected. Find out how many pixels are in one map unit - either horizontal and vertical direction
  // this check might not work correctly in some cases
  // (on a large area the pixels projected around "0,0" can have different properties from the actual point)
  QgsPointXY p1 = toLayerCoordinates( layer, mapSettings, QPoint( 0, 1 ) );
  QgsPointXY p2 = toLayerCoordinates( layer, mapSettings, QPoint( 0, 2 ) );
  QgsPointXY p3 = toLayerCoordinates( layer, mapSettings, QPoint( 1, 0 ) );
  QgsPointXY p4 = toLayerCoordinates( layer, mapSettings, QPoint( 2, 0 ) );
  double x = p1.sqrDist( p2 );
  double y = p3.sqrDist( p4 );
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
  QgsPointXY pt = mapSettings.mapToPixel().toMapCoordinates( point );
  return mapSettings.mapToLayerCoordinates( layer, pt );
}
