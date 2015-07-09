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

/** Decorator class for Triangulations (s. Decorator pattern in Gamma et al.)*/
class TriDecorator : public Triangulation
{
  public:
    TriDecorator();
    explicit TriDecorator( Triangulation* t );
    virtual ~TriDecorator();
    virtual void addLine( Line3D* line, bool breakline ) override;
    virtual int addPoint( Point3D* p ) override;
    /** Adds an association to a triangulation*/
    virtual void addTriangulation( Triangulation* t );
    /** Performs a consistency check, remove this later*/
    virtual void performConsistencyTest() override;
    virtual bool calcNormal( double x, double y, Vector3D* result ) override;
    virtual bool calcPoint( double x, double y, Point3D* result ) override;
    virtual Point3D* getPoint( unsigned int i ) const override;
    virtual int getNumberOfPoints() const override;
    //! @note not available in python bindings
    bool getTriangle( double x, double y, Point3D* p1, int* n1, Point3D* p2, int* n2, Point3D* p3, int* n3 ) override;
    bool getTriangle( double x, double y, Point3D* p1, Point3D* p2, Point3D* p3 ) override;
    virtual int getOppositePoint( int p1, int p2 ) override;
    virtual QList<int>* getSurroundingTriangles( int pointno ) override;
    virtual double getXMax() const override;
    virtual double getXMin() const override;
    virtual double getYMax() const override;
    virtual double getYMin() const override;
    virtual void setForcedCrossBehaviour( Triangulation::forcedCrossBehaviour b ) override;
    virtual void setEdgeColor( int r, int g, int b ) override;
    virtual void setForcedEdgeColor( int r, int g, int b ) override;
    virtual void setBreakEdgeColor( int r, int g, int b ) override;
    virtual void setTriangleInterpolator( TriangleInterpolator* interpolator ) override;
    virtual void eliminateHorizontalTriangles() override;
    virtual void ruppertRefinement() override;
    virtual bool pointInside( double x, double y ) override;
    virtual bool swapEdge( double x, double y ) override;
    virtual QList<int>* getPointsAroundEdge( double x, double y ) override;
  protected:
    /** Association with a Triangulation object*/
    Triangulation* mTIN;
};

inline TriDecorator::TriDecorator(): mTIN( 0 )
{

}

inline TriDecorator::TriDecorator( Triangulation* t ): mTIN( t )
{

}

inline TriDecorator::~TriDecorator()
{

}

inline void TriDecorator::addTriangulation( Triangulation* t )
{
  mTIN = t;
}

#endif

