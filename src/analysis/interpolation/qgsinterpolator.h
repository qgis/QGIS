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
class QgsGeometry;

struct ANALYSIS_EXPORT vertexData
{
  double x;
  double y;
  double z;
};

/**Interface class for interpolations. Interpolators take
the vertices of a vector layer as base data. The z-Value
can be an attribute or the z-coordinates in case of 25D types*/
class ANALYSIS_EXPORT QgsInterpolator
{
  public:
    /**Describes the type of input data*/
    enum InputType
    {
      POINTS,
      STRUCTURE_LINES,
      BREAK_LINES
    };

    QgsInterpolator( const QList<QgsVectorLayer*>& vlayers );

    virtual ~QgsInterpolator();

    /**Calculates interpolation value for map coordinates x, y
       @param x x-coordinate (in map units)
       @param y y-coordinate (in map units)
       @param result out: interpolation result
       @return 0 in case of success*/
    virtual int interpolatePoint( double x, double y, double& result ) = 0;

    /**Use the z-coord of 25D for interpolation*/
    void enableZCoordInterpolation() {zCoordInterpolation = true;}

    /**Use a vector attribute as interpolation value*/
    void enableAttributeValueInterpolation( int attribute );

    /**Creates a vector layer that can be added to the main map, e.g. TIN edges for triangle interpolation.
     Mouse clicks in the main map can be tracked and used for configuration (e.g. edge swaping). Default implementation does
    nothing.*/
    virtual QgsVectorLayer* createVectorLayer() const {return 0;}

  protected:
    /**Caches the vertex and value data from the provider. All the vertex data
     will be held in virtual memory
    @return 0 in case of success*/
    int cacheBaseData();

    QVector<vertexData> mCachedBaseData;

    /**Flag that tells if the cache already has been filled*/
    bool mDataIsCached;

  private:
    QgsInterpolator(); //forbidden
    /**Helper method that adds the vertices of a geometry to the mCachedBaseData
       @param geom the geometry
       @param attributeValue the attribute value for interpolation (if not interpolated from z-coordinate)
     @return 0 in case of success*/
    int addVerticesToCache( QgsGeometry* geom, double attributeValue );

    QList<QgsVectorLayer*> mVectorLayers;

    bool zCoordInterpolation;
    int mValueAttribute;
};

#endif
