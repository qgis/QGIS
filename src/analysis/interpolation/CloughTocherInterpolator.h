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

#include "TriangleInterpolator.h"
#include "qgspoint.h"
#include "qgis_analysis.h"

class NormVecDecorator;

/** \ingroup analysis
 * This is an implementation of a Clough-Tocher interpolator based on a triangular tessellation. The derivatives orthogonal to the boundary curves are interpolated linearly along a triangle edge.*/
class ANALYSIS_EXPORT CloughTocherInterpolator : public TriangleInterpolator
{
  protected:
    //! Association with a triangulation object
    NormVecDecorator *mTIN = nullptr;
    //! Tolerance of the barycentric coordinates at the borders of the triangles (to prevent errors because of very small negativ baricentric coordinates)
    double mEdgeTolerance;
    //! First point of the triangle in x-,y-,z-coordinates
    QgsPoint point1;
    //! Second point of the triangle in x-,y-,z-coordinates
    QgsPoint point2;
    //! Third point of the triangle in x-,y-,z-coordinates
    QgsPoint point3;
    QgsPoint cp1;
    QgsPoint cp2;
    QgsPoint cp3;
    QgsPoint cp4;
    QgsPoint cp5;
    QgsPoint cp6;
    QgsPoint cp7;
    QgsPoint cp8;
    QgsPoint cp9;
    QgsPoint cp10;
    QgsPoint cp11;
    QgsPoint cp12;
    QgsPoint cp13;
    QgsPoint cp14;
    QgsPoint cp15;
    QgsPoint cp16;
    //! Derivative in x-direction at point1
    double der1X;
    //! Derivative in y-direction at point1
    double der1Y;
    //! Derivative in x-direction at point2
    double der2X;
    //! Derivative in y-direction at point2
    double der2Y;
    //! Derivative in x-direction at point3
    double der3X;
    //! Derivative in y-direction at point3
    double der3Y;
    //! Stores point1 of the last run
    QgsPoint lpoint1;
    //! Stores point2 of the last run
    QgsPoint lpoint2;
    //! Stores point3 of the last run
    QgsPoint lpoint3;
    //! Finds out, in which triangle the point with the coordinates x and y is
    void init( double x, double y );
    //! Calculates the Bernsteinpolynomials to calculate the Beziertriangle. 'n' is three in the cubical case, 'i', 'j', 'k' are the indices of the controllpoint and 'u', 'v', 'w' are the barycentric coordinates of the point
    double calcBernsteinPoly( int n, int i, int j, int k, double u, double v, double w );

  public:
    //! Standard constructor
    CloughTocherInterpolator();
    //! Constructor with a pointer to the triangulation as argument
    CloughTocherInterpolator( NormVecDecorator *tin );
    virtual ~CloughTocherInterpolator();
    //! Calculates the normal vector and assigns it to vec (not implemented at the moment)
    virtual bool calcNormVec( double x, double y, Vector3D *result ) override;
    //! Performs a linear interpolation in a triangle and assigns the x-,y- and z-coordinates to point
    virtual bool calcPoint( double x, double y, QgsPoint *result ) override;
    virtual void setTriangulation( NormVecDecorator *tin );
};

#endif



