/***************************************************************************
                          CloughTocherInterpolator.h  -  description
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

#ifndef CLOUGHTOCHERINTERPOLATOR_H
#define CLOUGHTOCHERINTERPOLATOR_H

#include "NormVecDecorator.h"
#include "TriangleInterpolator.h"
#include "Point3D.h"
#include "Vector3D.h"
#include "MathUtils.h"
#include "Bezier3D.h"

/**This is an implementation of a Clough-Tocher interpolator based on a triangular tessellation. The derivatives orthogonal to the boundary curves are interpolated linearly along a triangle edge.*/
class ANALYSIS_EXPORT CloughTocherInterpolator : public TriangleInterpolator
{
  protected:
    /**association with a triangulation object*/
    NormVecDecorator* mTIN;
    /**Tolerance of the barycentric coordinates at the borders of the triangles (to prevent errors because of very small negativ baricentric coordinates)*/
    double mEdgeTolerance;
    /**first point of the triangle in x-,y-,z-coordinates*/
    Point3D point1;
    /**second point of the triangle in x-,y-,z-coordinates*/
    Point3D point2;
    /**third point of the triangle in x-,y-,z-coordinates*/
    Point3D point3;
    Point3D cp1;
    Point3D cp2;
    Point3D cp3;
    Point3D cp4;
    Point3D cp5;
    Point3D cp6;
    Point3D cp7;
    Point3D cp8;
    Point3D cp9;
    Point3D cp10;
    Point3D cp11;
    Point3D cp12;
    Point3D cp13;
    Point3D cp14;
    Point3D cp15;
    Point3D cp16;
    /**derivative in x-direction at point1*/
    double der1X;
    /**derivative in y-direction at point1*/
    double der1Y;
    /**derivative in x-direction at point2*/
    double der2X;
    /**derivative in y-direction at point2*/
    double der2Y;
    /**derivative in x-direction at point3*/
    double der3X;
    /**derivative in y-direction at point3*/
    double der3Y;
    /**stores point1 of the last run*/
    Point3D lpoint1;
    /**stores point2 of the last run*/
    Point3D lpoint2;
    /**stores point3 of the last run*/
    Point3D lpoint3;
    /**Finds out, in which triangle the point with the coordinates x and y is*/
    void init( double x, double y );
    /**Calculates the Bernsteinpolynomials to calculate the Beziertriangle. 'n' is three in the cubical case, 'i', 'j', 'k' are the indices of the controllpoint and 'u', 'v', 'w' are the barycentric coordinates of the point*/
    double calcBernsteinPoly( int n, int i, int j, int k, double u, double v, double w );

  public:
    /**standard constructor*/
    CloughTocherInterpolator();
    /**constructor with a pointer to the triangulation as argument*/
    CloughTocherInterpolator( NormVecDecorator* tin );
    /**destructor*/
    virtual ~CloughTocherInterpolator();
    /**Calculates the normal vector and assigns it to vec (not implemented at the moment)*/
    virtual bool calcNormVec( double x, double y, Vector3D* result );
    /**Performs a linear interpolation in a triangle and assigns the x-,y- and z-coordinates to point*/
    virtual bool calcPoint( double x, double y, Point3D* result );
    virtual void setTriangulation( NormVecDecorator* tin );
};


inline CloughTocherInterpolator::CloughTocherInterpolator() : mTIN( 0 ), mEdgeTolerance( 0.00001 )
{

}

inline CloughTocherInterpolator::CloughTocherInterpolator( NormVecDecorator* tin ) : mTIN( tin ), mEdgeTolerance( 0.00001 )
{

}

inline CloughTocherInterpolator::~CloughTocherInterpolator()
{
  //nothing to do
}

inline void CloughTocherInterpolator::setTriangulation( NormVecDecorator* tin )
{
  mTIN = tin;
}

#endif



