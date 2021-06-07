/***************************************************************************
  qgsvectortilemvtutils.cpp
  --------------------------------------
  Date                 : April 2020
  Copyright            : (C) 2020 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#include "qgsvectortilemvtutils.h"

#include "qgslinestring.h"


bool QgsVectorTileMVTUtils::isExteriorRing( const QgsLineString *lineString )
{
  // Exterior rings have POSITIVE area while interior rings have NEGATIVE area
  // when calculated with https://en.wikipedia.org/wiki/Shoelace_formula
  // The orientation of axes is that X grows to the right and Y grows to the bottom.
  // the input data are expected to form a closed ring, i.e. first pt == last pt.

  double total = 0.0;
  int count = lineString->numPoints();
  const double *xData = lineString->xData();
  const double *yData = lineString->yData();

  for ( int i = 0; i < count - 1; i++ )
  {
    double val = ( xData[i + 1] - xData[i] ) * ( yData[i + 1] + yData[i] );
    //double val = xData[i] * (-yData[i+1]) - xData[i+1] * (-yData[i]);  // gives the same result
    total += val;
  }
  return total >= 0;
}
