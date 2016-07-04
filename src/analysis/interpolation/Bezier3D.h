/***************************************************************************
                          Bezier3D.h  -  description
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

#ifndef BEZIER3D_H
#define BEZIER3D_H

#include "ParametricLine.h"
#include "Vector3D.h"
#include "MathUtils.h"
#include "qgslogger.h"

/** \ingroup analysis
 * Class Bezier3D represents a bezier curve, represented by control points. Parameter t is running from 0 to 1. The class is capable to calculate the curve point and the first two derivatives belonging to t.*/
class ANALYSIS_EXPORT Bezier3D: public ParametricLine
{
  protected:

  public:
    /** Default constructor*/
    Bezier3D();
    /** Constructor, par is a pointer to the parent, controlpoly a controlpolygon*/
    Bezier3D( ParametricLine* par, QVector<Point3D*>* controlpoly );
    /** Destructor*/
    virtual ~Bezier3D();
    /** Do not use this method, since a Bezier curve does not consist of other curves*/
    virtual void add( ParametricLine *pl ) override;
    /** Calculates the first derivative and assigns it to v*/
    virtual void calcFirstDer( float t, Vector3D* v ) override;
    /** Calculates the second derivative and assigns it to v*/
    virtual void calcSecDer( float t, Vector3D* v ) override;
    //virtual Point3D calcPoint(float t);
    /** Calculates the point on the curve and assigns it to p*/
    virtual void calcPoint( float t, Point3D* p ) override;
    /** Changes the order of control points*/
    virtual void changeDirection() override;
    //virtual void draw(QPainter* p);
    //virtual bool intersects(ParametricLine* pal);
    /** Do not use this method, since a Bezier curve does not consist of other curves*/
    virtual void remove( int i ) override;
    /** Returns a control point*/
    virtual const Point3D* getControlPoint( int number ) const override;
    /** Returns a pointer to the control polygon*/
    virtual const QVector<Point3D*>* getControlPoly() const override;
    /** Returns the degree of the curve*/
    virtual int getDegree() const override;
    /** Returns the parent*/
    virtual ParametricLine* getParent() const override;
    /** Sets the parent*/
    virtual void setParent( ParametricLine* par ) override;
    /** Sets the control polygon*/
    virtual void setControlPoly( QVector<Point3D*>* cp ) override;

};

//-----------------------------------------------constructors, destructor and assignment operator------------------------------

inline Bezier3D::Bezier3D() : ParametricLine()//default constructor
{

}

inline Bezier3D::Bezier3D( ParametricLine* parent, QVector<Point3D*>* controlpoly ) : ParametricLine( parent, controlpoly )
{
  mDegree = mControlPoly->count() - 1;
}

inline Bezier3D::~Bezier3D()
{

}

//----------------------------------------------invalid methods add and remove (because of inheritance from ParametricLine)

inline void Bezier3D::add( ParametricLine *pl )
{
  Q_UNUSED( pl );
  QgsDebugMsg( "Error!!!!! A Bezier-curve can not be parent of a ParametricLine." );
}

inline void Bezier3D::remove( int i )
{
  Q_UNUSED( i );
  QgsDebugMsg( "Error!!!!! A Bezier-curve has no children to remove." );
}

//-----------------------------------------------setters and getters---------------------------------------------------------------

inline const Point3D* Bezier3D::getControlPoint( int number ) const
{
  return ( *mControlPoly )[number-1];
}

inline const QVector<Point3D*>* Bezier3D::getControlPoly() const
{
  return mControlPoly;
}

inline int Bezier3D::getDegree() const
{
  return mDegree;
}

inline ParametricLine* Bezier3D::getParent() const
{
  return mParent;
}

inline void Bezier3D::setParent( ParametricLine* par )
{
  mParent = par;
}

inline void Bezier3D::setControlPoly( QVector<Point3D*>* cp )
{
  mControlPoly = cp;
  mDegree = mControlPoly->count() - 1;
}

#endif

