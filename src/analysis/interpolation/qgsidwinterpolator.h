/***************************************************************************
                              qgsidwinterpolator.h
                              --------------------
  begin                : March 10, 2008
  copyright            : (C) 2008 by Marco Hugentobler
  email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSIDWINTERPOLATOR_H
#define QGSIDWINTERPOLATOR_H

#include "qgsinterpolator.h"

class ANALYSIS_EXPORT QgsIDWInterpolator: public QgsInterpolator
{
  public:
    QgsIDWInterpolator( const QList<LayerData>& layerData );
    ~QgsIDWInterpolator();

    /**Calculates interpolation value for map coordinates x, y
       @param x x-coordinate (in map units)
       @param y y-coordinate (in map units)
       @param result out: interpolation result
       @return 0 in case of success*/
    int interpolatePoint( double x, double y, double& result );

    void setDistanceCoefficient( double p ) {mDistanceCoefficient = p;}

  private:

    QgsIDWInterpolator(); //forbidden

    /**The parameter that sets how the values are weighted with distance.
       Smaller values mean sharper peaks at the data points. The default is a
       value of 2*/
    double mDistanceCoefficient;
};

#endif
