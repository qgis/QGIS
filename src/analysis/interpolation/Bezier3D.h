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
#include "qgslogger.h"
#include "qgis_analysis.h"

#define SIP_NO_FILE

/**
 * \ingroup analysis
 * Class Bezier3D represents a bezier curve, represented by control points. Parameter t is running from 0 to 1. The class is capable to calculate the curve point and the first two derivatives belonging to it.
 * \note Not available in Python bindings
*/
class ANALYSIS_EXPORT Bezier3D: public ParametricLine
{
  protected:

  public:
    //! Default constructor
    Bezier3D() = default;
    //! Constructor, par is a pointer to the parent, controlpoly a controlpolygon
    Bezier3D( ParametricLine *par, QVector<QgsPoint *> *controlpoly );

    //! Do not use this method, since a Bezier curve does not consist of other curves
    void add( ParametricLine *pl SIP_TRANSFER ) override;
    //! Calculates the first derivative and assigns it to v
    void calcFirstDer( float t, Vector3D *v SIP_OUT ) override;
    //! Calculates the second derivative and assigns it to v
    void calcSecDer( float t, Vector3D *v SIP_OUT ) override;
    //virtual QgsPoint calcPoint(float t);
    //! Calculates the point on the curve and assigns it to p
    void calcPoint( float t, QgsPoint *p SIP_OUT ) override;
    //! Changes the order of control points
    void changeDirection() override;
    //virtual void draw(QPainter* p);
    //virtual bool intersects(ParametricLine* pal);
    //! Do not use this method, since a Bezier curve does not consist of other curves
    void remove( int i ) override;
    //! Returns a control point
    const QgsPoint *getControlPoint( int number ) const override;
    //! Returns a pointer to the control polygon
    const QVector<QgsPoint *> *getControlPoly() const override;
    //! Returns the degree of the curve
    int getDegree() const override;
    //! Returns the parent
    ParametricLine *getParent() const override;
    //! Sets the parent
    void setParent( ParametricLine *par ) override;
    //! Sets the control polygon
    void setControlPoly( QVector<QgsPoint *> *cp ) override;

};

#ifndef SIP_RUN

//-----------------------------------------------constructors, destructor and assignment operator------------------------------

inline Bezier3D::Bezier3D( ParametricLine *parent, QVector<QgsPoint *> *controlpoly ) : ParametricLine( parent, controlpoly )
{
  mDegree = mControlPoly->count() - 1;
}

//----------------------------------------------invalid methods add and remove (because of inheritance from ParametricLine)

inline void Bezier3D::add( ParametricLine *pl )
{
  Q_UNUSED( pl );
  QgsDebugMsg( QStringLiteral( "Error!!!!! A Bezier-curve can not be parent of a ParametricLine." ) );
}

inline void Bezier3D::remove( int i )
{
  Q_UNUSED( i );
  QgsDebugMsg( QStringLiteral( "Error!!!!! A Bezier-curve has no children to remove." ) );
}

//-----------------------------------------------setters and getters---------------------------------------------------------------

inline const QgsPoint *Bezier3D::getControlPoint( int number ) const
{
  return ( *mControlPoly )[number - 1];
}

inline const QVector<QgsPoint *> *Bezier3D::getControlPoly() const
{
  return mControlPoly;
}

inline int Bezier3D::getDegree() const
{
  return mDegree;
}

inline ParametricLine *Bezier3D::getParent() const
{
  return mParent;
}

inline void Bezier3D::setParent( ParametricLine *par )
{
  mParent = par;
}

inline void Bezier3D::setControlPoly( QVector<QgsPoint *> *cp )
{
  mControlPoly = cp;
  mDegree = mControlPoly->count() - 1;
}

#endif

#endif

