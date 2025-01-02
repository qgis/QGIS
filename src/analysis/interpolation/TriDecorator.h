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

#include "qgstriangulation.h"
#include "qgis_sip.h"
#include "qgis_analysis.h"

#define SIP_NO_FILE

/**
 * \ingroup analysis
 * \brief Decorator class for Triangulations (s. Decorator pattern in Gamma et al.).
 * \note Not available in Python bindings.
*/
class ANALYSIS_EXPORT TriDecorator : public QgsTriangulation
{
  public:
    TriDecorator() = default;
    //! Constructor for TriDecorator with an existing triangulation
    explicit TriDecorator( QgsTriangulation *t );
    void addLine( const QVector<QgsPoint> &points, QgsInterpolator::SourceType lineType ) override;
    int addPoint( const QgsPoint &p ) override;
    //! Adds an association to a triangulation
    virtual void addTriangulation( QgsTriangulation *t );
    //! Performs a consistency check, remove this later
    void performConsistencyTest() override;
    bool calcNormal( double x, double y, QgsPoint &result SIP_OUT ) override;
    bool calcPoint( double x, double y, QgsPoint &result SIP_OUT ) override;
    QgsPoint *point( int i ) const override;
    int pointsCount() const override;
    bool triangleVertices( double x, double y, QgsPoint &p1 SIP_OUT, int &n1 SIP_OUT, QgsPoint &p2 SIP_OUT, int &n2 SIP_OUT, QgsPoint &p3 SIP_OUT, int &n3 SIP_OUT ) override;
    bool triangleVertices( double x, double y, QgsPoint &p1 SIP_OUT, QgsPoint &p2 SIP_OUT, QgsPoint &p3 SIP_OUT ) override;
    int oppositePoint( int p1, int p2 ) override;
    QList<int> surroundingTriangles( int pointno ) override;
    double xMax() const override;
    double xMin() const override;
    double yMax() const override;
    double yMin() const override;
    void setForcedCrossBehavior( QgsTriangulation::ForcedCrossBehavior b ) override;
    void setTriangleInterpolator( TriangleInterpolator *interpolator ) override;
    void eliminateHorizontalTriangles() override;
    void ruppertRefinement() override;
    bool pointInside( double x, double y ) override;
    bool swapEdge( double x, double y ) override;
    QList<int> pointsAroundEdge( double x, double y ) override;

  protected:
    //! Association with a Triangulation object
    QgsTriangulation *mTIN = nullptr;
};

#ifndef SIP_RUN

inline TriDecorator::TriDecorator( QgsTriangulation *t )
  : mTIN( t )
{
}

inline void TriDecorator::addTriangulation( QgsTriangulation *t )
{
  mTIN = t;
}

#endif
#endif
