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
#include <QSettings>
#include <QPoint>
#include <cmath>


// return ratio [mu/lu] between map units and layer units
// this is of course only an approximation
double _ratioMU2LU( const QgsMapSettings& mapSettings, QgsMapLayer* layer )
{
  double distMU = mapSettings.mapUnitsPerPixel();
  QgsPoint ptMapCenterMU = mapSettings.visibleExtent().center();
  QgsPoint ptMapCenterRightMU( ptMapCenterMU.x() + distMU, ptMapCenterMU.y() );
  QgsPoint ptMapCenterLU = mapSettings.mapToLayerCoordinates( layer, ptMapCenterMU );
  QgsPoint ptMapCenterRightLU = mapSettings.mapToLayerCoordinates( layer, ptMapCenterRightMU );
  double distLU = sqrt( ptMapCenterLU.sqrDist( ptMapCenterRightLU ) );
  double ratio = distMU / distLU;
  return ratio;
}

double QgsTolerance::toleranceInProjectUnits( double tolerance, QgsMapLayer* layer, const QgsMapSettings& mapSettings, QgsTolerance::UnitType units )
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


double QgsTolerance::toleranceInMapUnits( double tolerance, QgsMapLayer *layer, const QgsMapSettings& mapSettings, QgsTolerance::UnitType units )
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

double QgsTolerance::toleranceInMapUnits( double tolerance, QgsMapLayer* layer, QgsMapRenderer* renderer, UnitType units )
{
  return toleranceInMapUnits( tolerance, layer, renderer->mapSettings(), units );
}

double QgsTolerance::vertexSearchRadius( const QgsMapSettings& mapSettings )
{
  QSettings settings;
  double tolerance = settings.value( "/qgis/digitizing/search_radius_vertex_edit", 10 ).toDouble();
  UnitType units = static_cast< QgsTolerance::UnitType >( settings.value( "/qgis/digitizing/search_radius_vertex_edit_unit", QgsTolerance::Pixels ).toInt() );
  if ( units == LayerUnits )
    units = ProjectUnits;
  return toleranceInProjectUnits( tolerance, nullptr, mapSettings, units );
}

double QgsTolerance::vertexSearchRadius( QgsMapLayer *layer, const QgsMapSettings &mapSettings )
{
  QSettings settings;
  double tolerance = settings.value( "/qgis/digitizing/search_radius_vertex_edit", 10 ).toDouble();
  UnitType units = static_cast< QgsTolerance::UnitType >( settings.value( "/qgis/digitizing/search_radius_vertex_edit_unit", QgsTolerance::Pixels ).toInt() );
  return toleranceInMapUnits( tolerance, layer, mapSettings, units );
}

double QgsTolerance::vertexSearchRadius( QgsMapLayer* layer, QgsMapRenderer* renderer )
{
  return vertexSearchRadius( layer, renderer->mapSettings() );
}

double QgsTolerance::defaultTolerance( QgsMapLayer *layer, const QgsMapSettings& mapSettings )
{
  QSettings settings;
  double tolerance = settings.value( "/qgis/digitizing/default_snapping_tolerance", 0 ).toDouble();
  UnitType units = static_cast< QgsTolerance::UnitType >( settings.value( "/qgis/digitizing/default_snapping_tolerance_unit", ProjectUnits ).toInt() );
  return toleranceInMapUnits( tolerance, layer, mapSettings, units );
}


double QgsTolerance::defaultTolerance( QgsMapLayer* layer, QgsMapRenderer* renderer )
{
  return defaultTolerance( layer, renderer->mapSettings() );
}


double QgsTolerance::computeMapUnitPerPixel( QgsMapLayer* layer, const QgsMapSettings& mapSettings )
{
  if ( ! mapSettings.hasCrsTransformEnabled() )
  {
    // if the on-the-fly projections are not enabled, layer units pre pixel are the same as map units per pixel
    return mapSettings.mapUnitsPerPixel();
  }

  // the layer is projected. Find out how many pixels are in one map unit - either horizontal and vertical direction
  // this check might not work correctly in some cases
  // (on a large area the pixels projected around "0,0" can have different properties from the actual point)
  QgsPoint p1 = toLayerCoordinates( layer, mapSettings, QPoint( 0, 1 ) );
  QgsPoint p2 = toLayerCoordinates( layer, mapSettings, QPoint( 0, 2 ) );
  QgsPoint p3 = toLayerCoordinates( layer, mapSettings, QPoint( 1, 0 ) );
  QgsPoint p4 = toLayerCoordinates( layer, mapSettings, QPoint( 2, 0 ) );
  double x = p1.sqrDist( p2 );
  double y = p3.sqrDist( p4 );
  if ( x > y )
  {
    return sqrt( x );
  }
  else
  {
    return sqrt( y );
  }
}


QgsPoint QgsTolerance::toLayerCoordinates( QgsMapLayer* layer, const QgsMapSettings& mapSettings, QPoint point )
{
  QgsPoint pt = mapSettings.mapToPixel().toMapCoordinates( point );
  return mapSettings.mapToLayerCoordinates( layer, pt );
}
