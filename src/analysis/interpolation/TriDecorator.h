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
    virtual void addLine( Line3D* line, bool breakline ) OVERRIDE;
    virtual int addPoint( Point3D* p ) OVERRIDE;
    /**Adds an association to a triangulation*/
    virtual void addTriangulation( Triangulation* t );
    /**Performs a consistency check, remove this later*/
    virtual void performConsistencyTest() OVERRIDE;
    virtual bool calcNormal( double x, double y, Vector3D* result ) OVERRIDE;
    virtual bool calcPoint( double x, double y, Point3D* result ) OVERRIDE;
    virtual Point3D* getPoint( unsigned int i ) const OVERRIDE;
    virtual int getNumberOfPoints() const OVERRIDE;
    //! @note not available in python bindings
    bool getTriangle( double x, double y, Point3D* p1, int* n1, Point3D* p2, int* n2, Point3D* p3, int* n3 ) OVERRIDE;
    bool getTriangle( double x, double y, Point3D* p1, Point3D* p2, Point3D* p3 ) OVERRIDE;
    virtual int getOppositePoint( int p1, int p2 ) OVERRIDE;
    virtual QList<int>* getSurroundingTriangles( int pointno ) OVERRIDE;
    virtual double getXMax() const OVERRIDE;
    virtual double getXMin() const OVERRIDE;
    virtual double getYMax() const OVERRIDE;
    virtual double getYMin() const OVERRIDE;
    virtual void setForcedCrossBehaviour( Triangulation::forcedCrossBehaviour b ) OVERRIDE;
    virtual void setEdgeColor( int r, int g, int b ) OVERRIDE;
    virtual void setForcedEdgeColor( int r, int g, int b ) OVERRIDE;
    virtual void setBreakEdgeColor( int r, int g, int b ) OVERRIDE;
    virtual void setTriangleInterpolator( TriangleInterpolator* interpolator ) OVERRIDE;
    virtual void eliminateHorizontalTriangles() OVERRIDE;
    virtual void ruppertRefinement() OVERRIDE;
    virtual bool pointInside( double x, double y ) OVERRIDE;
    virtual bool swapEdge( double x, double y ) OVERRIDE;
    virtual QList<int>* getPointsAroundEdge( double x, double y ) OVERRIDE;
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

