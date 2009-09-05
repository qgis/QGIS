/***************************************************************************
                          LinTriangleInterpolator.h  -  description
                             -------------------
    copyright            : (C) 2004 by Marco Hugentobler
    email                : mhugent@geo.unizh.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef LINTRIANGLEINTERPOLATOR_H
#define LINTRIANGLEINTERPOLATOR_H

#include "TriangleInterpolator.h"
#include "DualEdgeTriangulation.h"

/**LinTriangleInterpolator is a class which interpolates linearly on a triangulation*/
class ANALYSIS_EXPORT LinTriangleInterpolator : public TriangleInterpolator
{
  public:
    /**Default constructor*/
    LinTriangleInterpolator();
    /**Constructor with reference to a DualEdgeTriangulation object*/
    LinTriangleInterpolator( DualEdgeTriangulation* tin );
    /** Destructor*/
    virtual ~LinTriangleInterpolator();
    /**Calculates the normal vector and assigns it to vec*/
    virtual bool calcNormVec( double x, double y, Vector3D* result );
    /**Performs a linear interpolation in a triangle and assigns the x-,y- and z-coordinates to point*/
    virtual bool calcPoint( double x, double y, Point3D* result );
    /**Returns a pointer to the current Triangulation object*/
    virtual DualEdgeTriangulation* getTriangulation() const;
    /**Sets a Triangulation*/
    virtual void setTriangulation( DualEdgeTriangulation* tin );


  protected:
    DualEdgeTriangulation* mTIN;
    /**Calculates the first derivative with respect to x for a linear surface and assigns it to vec*/
    virtual bool calcFirstDerX( double x, double y, Vector3D* result );
    /**Calculates the first derivative with respect to y for a linear surface and assigns it to vec*/
    virtual bool calcFirstDerY( double x, double y, Vector3D* result );
};

inline LinTriangleInterpolator::LinTriangleInterpolator()
{

}

inline LinTriangleInterpolator::LinTriangleInterpolator( DualEdgeTriangulation* tin ): mTIN( tin )
{

}

inline LinTriangleInterpolator::~LinTriangleInterpolator()
{

}

inline DualEdgeTriangulation* LinTriangleInterpolator::getTriangulation() const
{
  return mTIN;
}

inline void LinTriangleInterpolator::setTriangulation( DualEdgeTriangulation* tin )
{
  mTIN = tin;
}

#endif







