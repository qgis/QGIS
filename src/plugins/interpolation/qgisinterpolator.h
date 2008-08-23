/***************************************************************************
                              qgsinterpolator.h
                              ------------------------
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

#ifndef QGSINTERPOLATOR_H
#define QGSINTERPOLATOR_H

#include <QVector>

class QgsVectorLayer;

struct vertexData
{
  double x;
  double y;
  double z;
};

/**Interface class for interpolations. Interpolators take
the vertices of a vector layer as base data. The z-Value
can be an attribute or the z-coordinates in case of 25D types*/
class QgsInterpolator
{
  public:
    QgsInterpolator( QgsVectorLayer* vlayer );

    virtual ~QgsInterpolator();

    /**Calculates interpolation value for map coordinates x, y
       @param x x-coordinate (in map units)
       @param y y-coordinate (in map units)
       @param result out: interpolation result
       @return 0 in case of success*/
    virtual int interpolatePoint( double x, double y, double& result ) = 0;

    /**Use the z-coord of 25D for interpolation*/
    void enableZCoordInterpolation() {useZCoord = true;}

    /**Use a vector attribute as interpolation value*/
    void enableAttributeValueInterpolation( int attribute );

  protected:
    /**Caches the vertex and value data from the provider. All the vertex data
     will be held in virtual memory
    @return 0 in case of success*/
    int cacheBaseData();

    QVector<vertexData> mCachedBaseData;

  private:
    QgsInterpolator(); //forbidden
    QgsVectorLayer* mVectorLayer;

    bool zCoordInterpolation;
    int mValueAttribute;
};

#endif
