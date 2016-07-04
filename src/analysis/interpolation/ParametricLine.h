/***************************************************************************
                          ParametricLine.h  -  description
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

#ifndef PARAMETRICLINE_H
#define PARAMETRICLINE_H

#include "Point3D.h"
#include "Vector3D.h"
#include <QVector>

/** \ingroup analysis
 * ParametricLine is an Interface for parametric lines. It is possible, that a parametric line is composed of several parametric
 * lines (see the composite pattern in Gamma et al. 'Design Patterns'). Do not build instances of it since it is an abstract class.*/
class ANALYSIS_EXPORT ParametricLine
{
  protected:
    /** Degree of the parametric Line*/
    int mDegree;
    /** Pointer to the parent object. If there isn't one, mParent is 0*/
    ParametricLine* mParent;
    /** MControlPoly stores the points of the control polygon*/
    QVector<Point3D*>* mControlPoly;
  public:
    /** Default constructor*/
    ParametricLine();
    /** Constructor, par is a pointer to the parent object, controlpoly the controlpolygon
      */
    ParametricLine( ParametricLine* par, QVector<Point3D*>* controlpoly );
    /** Destructor*/
    virtual ~ParametricLine();
    virtual void add( ParametricLine* pl ) = 0;
    virtual void calcFirstDer( float t, Vector3D* v ) = 0;
    virtual void calcSecDer( float t, Vector3D* v ) = 0;
    //virtual Point3D calcPoint(float t);
    virtual void calcPoint( float t, Point3D* ) = 0;
    virtual void changeDirection() = 0;
    //virtual void draw(QPainter* p);
    virtual const Point3D* getControlPoint( int number ) const = 0;
    virtual const QVector<Point3D*>* getControlPoly() const = 0;
    virtual int getDegree() const = 0;
    virtual ParametricLine* getParent() const = 0;
    //virtual bool intersects(ParametricLine* pal);
    virtual void remove( int i ) = 0;
    virtual void setControlPoly( QVector<Point3D*>* cp ) = 0;
    virtual void setParent( ParametricLine* paral ) = 0;
};

//-----------------------------------------constructors and destructor----------------------

inline ParametricLine::ParametricLine()
    : mDegree( 0 )
    , mParent( nullptr )
    , mControlPoly( nullptr )
{

}

inline ParametricLine::ParametricLine( ParametricLine* par, QVector<Point3D*>* controlpoly )
    : mDegree( 0 )
    , mParent( par )
    , mControlPoly( controlpoly )
{

}

inline ParametricLine::~ParametricLine()
{
  //delete mParent;
}

#endif








