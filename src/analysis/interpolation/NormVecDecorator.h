/***************************************************************************
                          NormVecDecorator.h  -  description
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

#ifndef NORMVECDECORATOR_H
#define NORMVECDECORATOR_H

#include "TriDecorator.h"
#include "qgis_sip.h"
#include "qgis.h"
#include "TriangleInterpolator.h"
#include "MathUtils.h"
#include "qgslogger.h"
#include "qgis_analysis.h"

#define SIP_NO_FILE

class QgsFeedback;

/**
 * \ingroup analysis
 * Decorator class which adds the functionality of estimating normals at the data points.
 * \note Not available in Python bindings.
*/
class ANALYSIS_EXPORT NormVecDecorator: public TriDecorator
{
  public:
    //! Enumeration for the state of a point. Normal means, that the point is not on a BreakLine, BreakLine means that the point is on a breakline (but not an end point of it) and EndPoint means, that it is an endpoint of a breakline.
    enum PointState {Normal, BreakLine, EndPoint};
    NormVecDecorator();
    NormVecDecorator( Triangulation *tin );
    ~NormVecDecorator() override;
    int addPoint( const QgsPoint &p ) override;
    //! Calculates the normal at a point on the surface and assigns it to 'result'. Returns true in case of success and false in case of failure
    bool calcNormal( double x, double y, Vector3D *result SIP_OUT ) override;
    //! Calculates the normal of a triangle-point for the point with coordinates x and y. This is needed, if a point is on a break line and there is no unique normal stored in 'mNormVec'. Returns false, it something went wrong and true otherwise
    bool calcNormalForPoint( double x, double y, int point, Vector3D *result SIP_OUT );
    bool calcPoint( double x, double y, QgsPoint &result SIP_OUT ) override;
    //! Eliminates the horizontal triangles by swapping or by insertion of new points. If alreadyestimated is true, a re-estimation of the normals will be done
    void eliminateHorizontalTriangles() override;
    //! Estimates the first derivative a point. Return true in case of success and false otherwise
    bool estimateFirstDerivative( int pointno );
    //! This method adds the functionality of estimating normals at the data points. Return true in the case of success and false otherwise
    bool estimateFirstDerivatives( QgsFeedback *feedback = nullptr );
    //! Returns a pointer to the normal vector for the point with the number n
    Vector3D *getNormal( int n ) const;
    //! Finds out, in which triangle a point with coordinates x and y is and assigns the triangle points to p1, p2, p3 and the estimated normals to v1, v2, v3. The vectors are normally taken from 'mNormVec', except if p1, p2 or p3 is a point on a breakline. In this case, the normal is calculated on-the-fly. Returns false, if something went wrong and true otherwise
    bool getTriangle( double x, double y, QgsPoint &p1 SIP_OUT, Vector3D *v1 SIP_OUT, QgsPoint &p2 SIP_OUT, Vector3D *v2 SIP_OUT, QgsPoint &p3 SIP_OUT, Vector3D *v3 SIP_OUT )  SIP_PYNAME( getTriangleVertices );
    //! This function behaves similar to the one above. Additionally, the numbers of the points are returned (ptn1, ptn2, ptn3) as well as the PointStates of the triangle points (state1, state2, state3)
    bool getTriangle( double x, double y, QgsPoint &p1 SIP_OUT, int &ptn1 SIP_OUT, Vector3D *v1 SIP_OUT, PointState *state1 SIP_OUT, QgsPoint &p2 SIP_OUT, int &ptn2 SIP_OUT, Vector3D *v2 SIP_OUT, PointState *state2 SIP_OUT, QgsPoint &p3 SIP_OUT, int &ptn3 SIP_OUT, Vector3D *v3 SIP_OUT, PointState *state3 SIP_OUT );
    //! Returns the state of the point with the number 'pointno'
    PointState getState( int pointno ) const;
    //! Sets an interpolator
    void setTriangleInterpolator( TriangleInterpolator *inter ) override;
    //! Swaps the edge which is closest to the point with x and y coordinates (if this is possible) and forces recalculation of the concerned normals (if alreadyestimated is true)
    bool swapEdge( double x, double y ) override;

    bool saveTriangulation( QgsFeatureSink *sink, QgsFeedback *feedback = nullptr ) const override;

  protected:
    //! Is true, if the normals already have been estimated
    bool alreadyestimated;
    static const unsigned int DEFAULT_STORAGE_FOR_NORMALS = 100000;
    //! Association with an interpolator object
    TriangleInterpolator *mInterpolator = nullptr;
    //! Vector that stores the normals for the points. If 'estimateFirstDerivatives()' was called and there is a null pointer, this means, that the triangle point is on a breakline
    QVector<Vector3D *> *mNormVec;
    //! Vector who stores, it a point is not on a breakline, if it is a normal point of the breakline or if it is an endpoint of a breakline
    QVector<PointState> *mPointState;
    //! Sets the state (BreakLine, Normal, EndPoint) of a point
    void setState( int pointno, PointState s );
};

#ifndef SIP_RUN

inline NormVecDecorator::NormVecDecorator()
  : mNormVec( new QVector<Vector3D*>( DEFAULT_STORAGE_FOR_NORMALS ) )
  , mPointState( new QVector<PointState>( DEFAULT_STORAGE_FOR_NORMALS ) )
{
  alreadyestimated = false;
}

inline NormVecDecorator::NormVecDecorator( Triangulation *tin )
  : TriDecorator( tin )
  , mNormVec( new QVector<Vector3D*>( DEFAULT_STORAGE_FOR_NORMALS ) )
  , mPointState( new QVector<PointState>( DEFAULT_STORAGE_FOR_NORMALS ) )
{
  alreadyestimated = false;
}

inline void NormVecDecorator::setTriangleInterpolator( TriangleInterpolator *inter )
{
  mInterpolator = inter;
}

inline Vector3D *NormVecDecorator::getNormal( int n ) const
{
  if ( mNormVec )
  {
    return mNormVec->at( n );
  }
  else
  {
    QgsDebugMsg( QStringLiteral( "warning, null pointer" ) );
    return nullptr;
  }
}

#endif

#endif
