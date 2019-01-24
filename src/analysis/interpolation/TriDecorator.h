/***************************************************************************
                          TriDecorator.h  -  description
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

#ifndef TRIDECORATOR_H
#define TRIDECORATOR_H

#include "Triangulation.h"
#include "qgis_sip.h"
#include "qgis_analysis.h"

#define SIP_NO_FILE

/**
 * \ingroup analysis
 * Decorator class for Triangulations (s. Decorator pattern in Gamma et al.).
 * \note Not available in Python bindings.
*/
class ANALYSIS_EXPORT TriDecorator : public Triangulation
{
  public:
    //! Constructor for TriDecorator
    TriDecorator() = default;
    explicit TriDecorator( Triangulation *t );
    void addLine( const QVector< QgsPoint> &points, QgsInterpolator::SourceType lineType ) override;
    int addPoint( const QgsPoint &p ) override;
    //! Adds an association to a triangulation
    virtual void addTriangulation( Triangulation *t );
    //! Performs a consistency check, remove this later
    void performConsistencyTest() override;
    bool calcNormal( double x, double y, Vector3D *result SIP_OUT ) override;
    bool calcPoint( double x, double y, QgsPoint &result SIP_OUT ) override;
    QgsPoint *getPoint( int i ) const override;
    int getNumberOfPoints() const override;
    bool getTriangle( double x, double y, QgsPoint &p1 SIP_OUT, int &n1 SIP_OUT, QgsPoint &p2 SIP_OUT, int &n2 SIP_OUT, QgsPoint &p3 SIP_OUT, int &n3 SIP_OUT )  SIP_PYNAME( getTriangleVertices ) override;
    bool getTriangle( double x, double y, QgsPoint &p1 SIP_OUT, QgsPoint &p2 SIP_OUT, QgsPoint &p3 SIP_OUT ) override;
    int getOppositePoint( int p1, int p2 ) override;
    QList<int> getSurroundingTriangles( int pointno ) override;
    double getXMax() const override;
    double getXMin() const override;
    double getYMax() const override;
    double getYMin() const override;
    void setForcedCrossBehavior( Triangulation::ForcedCrossBehavior b ) override;
    void setEdgeColor( int r, int g, int b ) override;
    void setForcedEdgeColor( int r, int g, int b ) override;
    void setBreakEdgeColor( int r, int g, int b ) override;
    void setTriangleInterpolator( TriangleInterpolator *interpolator ) override;
    void eliminateHorizontalTriangles() override;
    void ruppertRefinement() override;
    bool pointInside( double x, double y ) override;
    bool swapEdge( double x, double y ) override;
    QList<int> *getPointsAroundEdge( double x, double y ) override;
  protected:
    //! Association with a Triangulation object
    Triangulation *mTIN = nullptr;
};

#ifndef SIP_RUN

inline TriDecorator::TriDecorator( Triangulation *t )
  : mTIN( t )
{

}

inline void TriDecorator::addTriangulation( Triangulation *t )
{
  mTIN = t;
}

#endif
#endif

