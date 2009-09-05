/***************************************************************************
                              qgstininterpolator.h
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

#ifndef QGSTININTERPOLATOR_H
#define QGSTININTERPOLATOR_H

#include "qgsinterpolator.h"

class Triangulation;
class TriangleInterpolator;

/**Interpolation in a triangular irregular network*/
class ANALYSIS_EXPORT QgsTINInterpolator: public QgsInterpolator
{
  public:
    QgsTINInterpolator( const QList<QgsVectorLayer*>& inputData );
    ~QgsTINInterpolator();

    /**Calculates interpolation value for map coordinates x, y
       @param x x-coordinate (in map units)
       @param y y-coordinate (in map units)
       @param result out: interpolation result
       @return 0 in case of success*/
    int interpolatePoint( double x, double y, double& result );

  private:
    Triangulation* mTriangulation;
    TriangleInterpolator* mTriangleInterpolator;
    bool mIsInitialized;

    void initialize();
};

#endif
