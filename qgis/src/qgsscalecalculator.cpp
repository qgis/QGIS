/***************************************************************************
                              qgsscalecalculator.h    
                 Calculates scale based on map extent and units
                              -------------------
  begin                : May 18, 2004
  copyright            : (C) 2004 by Gary E.Sherman
  email                : sherman at mrcc.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

#include <assert.h>
#include <math.h>
#include <qstring.h>
#include "qgsrect.h"
#include "qgsscalecalculator.h"

QgsScaleCalculator::QgsScaleCalculator(int dpi, QgsScaleCalculator::units mapUnits) 
: mDpi(dpi), mMapUnits(mapUnits)
{}

QgsScaleCalculator::~QgsScaleCalculator()
{}

void QgsScaleCalculator::setDpi(int dpi)
{
  mDpi = dpi;
}

void QgsScaleCalculator::setMapUnits(units mapUnits)
{
  mMapUnits = mapUnits;
}

double QgsScaleCalculator::calculate(QgsRect &mapExtent, int canvasWidth)
{
  double conversionFactor; 
  double delta;
  // calculation is based on the map units and extent, the dpi of the
  // users display, and the canvas width
  switch(mMapUnits)
  {
    case QgsScaleCalculator::METERS:
      // convert meters to inches
      conversionFactor = 39.3700787;
      delta = mapExtent.xMax() - mapExtent.xMin();
      break;
    case QgsScaleCalculator::FEET:
      conversionFactor = 12.0;
      delta = mapExtent.xMax() - mapExtent.xMin();
      break;
    case QgsScaleCalculator::DEGREES:
      // degrees require conversion to meters first
      conversionFactor = 39.3700787;
      delta = calculateGeographicDistance(mapExtent);
      break;
    case QgsScaleCalculator::NMILE:
      // factor to convert nautical miles to inches
      conversionFactor = 72913.3857524 ;
      delta = mapExtent.xMax() - mapExtent.xMin();
    default:
      assert("bad map units");
      break; 
  }
#ifdef QGISDEBUG
  std::cerr << "Using conversionFactor of " << conversionFactor << std::endl; 
#endif
  double scale = (delta * conversionFactor)/(canvasWidth/mDpi);
  return scale;
}


double  QgsScaleCalculator::calculateGeographicDistance(QgsRect &mapExtent)
{
  // need to calculate the x distance in meters 
  // We'll use the middle latitude for the calculation
  // Note this is an approximation (although very close) but calculating scale
  // for geographic data over large extents is quasi-meaningless
  double lat1 = (mapExtent.yMax() - mapExtent.yMin())/2 + mapExtent.yMin();
  double lat2 = lat1;
  double lon1 = mapExtent.xMin();
  double lon2 = mapExtent.xMax();
  double dlon = lon2 - lon1;
  double dlat = lat2 - lat1;
  double rads = (4 * atan(1.0))/180;
  double a = pow((sin(dlat*rads/2)),2) + 
    cos(lat1 *rads) * cos(lat2 *rads) *pow(sin(dlon*rads/2),2);
  double c = 2 * atan2(sqrt(a), sqrt(1-a));
  // calculate radius of earth
  double ra = 6378;
  double rb = 6357;
  double e = .081082;
  double R = ra* sqrt(1-pow(e,2))/(1 - pow(e,2)*pow(sin(lat1*rads),2));
  double d = c *R; // kilometers;
  double meters = d * 1000.0;


#ifdef QGISDEBUG
  std::cerr << "Distance across map extent (m): " << meters << std::endl; 
#endif
  return meters;
}
