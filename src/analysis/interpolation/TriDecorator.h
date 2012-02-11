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

/**Decorator class for Triangulations (s. Decorator pattern in Gamma et al.)*/
class TriDecorator: public Triangulation
{
  public:
    TriDecorator();
    TriDecorator( Triangulation* t );
    virtual ~TriDecorator();
    virtual void addLine( Line3D* line, bool breakline );
    virtual int addPoint( Point3D* p );
    /**Adds an association to a triangulation*/
    virtual void addTriangulation( Triangulation* t );
    /**Performs a consistency check, remove this later*/
    virtual void performConsistencyTest();
    virtual bool calcNormal( double x, double y, Vector3D* result );
    virtual bool calcPoint( double x, double y, Point3D* result );
    virtual Point3D* getPoint( unsigned int i ) const;
    virtual int getNumberOfPoints() const;
    bool getTriangle( double x, double y, Point3D* p1, int* n1, Point3D* p2, int* n2, Point3D* p3, int* n3 );
    bool getTriangle( double x, double y, Point3D* p1, Point3D* p2, Point3D* p3 );
    virtual int getOppositePoint( int p1, int p2 );
    virtual QList<int>* getSurroundingTriangles( int pointno );
    virtual double getXMax() const;
    virtual double getXMin() const;
    virtual double getYMax() const;
    virtual double getYMin() const;
    virtual void setForcedCrossBehaviour( Triangulation::forcedCrossBehaviour b );
    virtual void setEdgeColor( int r, int g, int b );
    virtual void setForcedEdgeColor( int r, int g, int b );
    virtual void setBreakEdgeColor( int r, int g, int b );
    virtual void setTriangleInterpolator( TriangleInterpolator* interpolator );
    virtual void eliminateHorizontalTriangles();
    virtual void ruppertRefinement();
    virtual bool pointInside( double x, double y );
    virtual bool swapEdge( double x, double y );
    virtual QList<int>* getPointsAroundEdge( double x, double y );
  protected:
    /**Association with a Triangulation object*/
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

