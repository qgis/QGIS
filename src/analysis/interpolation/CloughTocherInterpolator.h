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

#define SIP_NO_FILE

/**
 * \ingroup analysis
 * This is an implementation of a Clough-Tocher interpolator based on a triangular tessellation. The derivatives orthogonal to the boundary curves are interpolated linearly along a triangle edge.
 * \note Not available in Python bindings
*/
class ANALYSIS_EXPORT CloughTocherInterpolator : public TriangleInterpolator
{
  protected:
    //! Association with a triangulation object
    NormVecDecorator *mTIN = nullptr;
    //! Tolerance of the barycentric coordinates at the borders of the triangles (to prevent errors because of very small negativ baricentric coordinates)
    double mEdgeTolerance = 0.00001;
    //! First point of the triangle in x-,y-,z-coordinates
    QgsPoint point1 = QgsPoint( 0, 0, 0 );
    //! Second point of the triangle in x-,y-,z-coordinates
    QgsPoint point2 = QgsPoint( 0, 0, 0 );
    //! Third point of the triangle in x-,y-,z-coordinates
    QgsPoint point3 = QgsPoint( 0, 0, 0 );
    //! Control point 1
    QgsPoint cp1 = QgsPoint( 0, 0, 0 );
    //! Control point 2
    QgsPoint cp2 = QgsPoint( 0, 0, 0 );
    //! Control point 3
    QgsPoint cp3 = QgsPoint( 0, 0, 0 );
    //! Control point 4
    QgsPoint cp4 = QgsPoint( 0, 0, 0 );
    //! Control point 5
    QgsPoint cp5 = QgsPoint( 0, 0, 0 );
    //! Control point 6
    QgsPoint cp6 = QgsPoint( 0, 0, 0 );
    //! Control point 7
    QgsPoint cp7 = QgsPoint( 0, 0, 0 );
    //! Control point 8
    QgsPoint cp8 = QgsPoint( 0, 0, 0 );
    //! Control point 9
    QgsPoint cp9 = QgsPoint( 0, 0, 0 );
    //! Control point 10
    QgsPoint cp10 = QgsPoint( 0, 0, 0 );
    //! Control point 11
    QgsPoint cp11 = QgsPoint( 0, 0, 0 );
    //! Control point 12
    QgsPoint cp12 = QgsPoint( 0, 0, 0 );
    //! Control point 13
    QgsPoint cp13 = QgsPoint( 0, 0, 0 );
    //! Control point 14
    QgsPoint cp14 = QgsPoint( 0, 0, 0 );
    //! Control point 15
    QgsPoint cp15 = QgsPoint( 0, 0, 0 );
    //! Control point 16
    QgsPoint cp16 = QgsPoint( 0, 0, 0 );
    //! Derivative in x-direction at point1
    double der1X = 0.0;
    //! Derivative in y-direction at point1
    double der1Y = 0.0;
    //! Derivative in x-direction at point2
    double der2X = 0.0;
    //! Derivative in y-direction at point2
    double der2Y = 0.0;
    //! Derivative in x-direction at point3
    double der3X = 0.0;
    //! Derivative in y-direction at point3
    double der3Y = 0.0;
    //! Stores point1 of the last run
    QgsPoint lpoint1 = QgsPoint( 0, 0, 0 );
    //! Stores point2 of the last run
    QgsPoint lpoint2 = QgsPoint( 0, 0, 0 );
    //! Stores point3 of the last run
    QgsPoint lpoint3 = QgsPoint( 0, 0, 0 );
    //! Finds out, in which triangle the point with the coordinates x and y is
    void init( double x, double y );
    //! Calculates the Bernsteinpolynomials to calculate the Beziertriangle. 'n' is three in the cubical case, 'i', 'j', 'k' are the indices of the controllpoint and 'u', 'v', 'w' are the barycentric coordinates of the point
    double calcBernsteinPoly( int n, int i, int j, int k, double u, double v, double w );

  public:
    //! Standard constructor
    CloughTocherInterpolator() = default;

    //! Constructor with a pointer to the triangulation as argument
    CloughTocherInterpolator( NormVecDecorator *tin );

    //! Calculates the normal vector and assigns it to vec (not implemented at the moment)
    bool calcNormVec( double x, double y, Vector3D *result SIP_OUT ) override;
    bool calcPoint( double x, double y, QgsPoint &result SIP_OUT ) override;
    virtual void setTriangulation( NormVecDecorator *tin );
};

#endif



