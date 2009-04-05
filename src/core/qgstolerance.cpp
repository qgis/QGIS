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


double QgsTolerance::toleranceInMapUnits(double tolerance, double mapUnitsPerPixel, UnitType units)
{
  if (units == MapUnits)
  {
    return tolerance;
  }
  return tolerance * mapUnitsPerPixel;
}

double QgsTolerance::vertexSearchRadius( double mapUnitsPerPixel )
{
  QSettings settings;
  double tolerance = settings.value( "/qgis/digitizing/search_radius_vertex_edit", 10 ).toDouble();
  UnitType units = (QgsTolerance::UnitType) settings.value( "/qgis/digitizing/search_radius_vertex_edit_unit", 0 ).toInt();
  return toleranceInMapUnits(tolerance, mapUnitsPerPixel, units);
}

double QgsTolerance::defaultTolerance( double mapUnitsPerPixel )
{
  QSettings settings;
  double tolerance = settings.value( "/qgis/digitizing/default_snapping_tolerance", 0 ).toDouble();
  UnitType units = (QgsTolerance::UnitType) settings.value( "/qgis/digitizing/default_snapping_tolerance_unit", 0 ).toInt();
  return toleranceInMapUnits(tolerance, mapUnitsPerPixel, units);
}
