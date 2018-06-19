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

#include "qgspoint.h"
#include <QVector>
#include "qgis_analysis.h"
#include "qgis_sip.h"

class Vector3D;

#define SIP_NO_FILE

/**
 * \ingroup analysis
 * ParametricLine is an Interface for parametric lines. It is possible, that a parametric line is composed of several parametric
 * lines (see the composite pattern in Gamma et al. 'Design Patterns'). Do not build instances of it since it is an abstract class.
 * \note Not available in Python bindings
*/
class ANALYSIS_EXPORT ParametricLine
{
  protected:
    //! Degree of the parametric Line
    int mDegree = 0;
    //! Pointer to the parent object. If there isn't one, mParent is 0
    ParametricLine *mParent = nullptr;
    //! MControlPoly stores the points of the control polygon
    QVector<QgsPoint *> *mControlPoly = nullptr;
  public:
    //! Default constructor
    ParametricLine() = default;

    /**
     * Constructor, par is a pointer to the parent object, controlpoly the controlpolygon
      */
    ParametricLine( ParametricLine *par SIP_TRANSFER, QVector<QgsPoint *> *controlpoly );
    virtual ~ParametricLine() = default;
    virtual void add( ParametricLine *pl SIP_TRANSFER ) = 0;
    virtual void calcFirstDer( float t, Vector3D *v SIP_OUT ) = 0;
    virtual void calcSecDer( float t, Vector3D *v SIP_OUT ) = 0;
    //virtual QgsPoint calcPoint(float t);
    virtual void calcPoint( float t, QgsPoint *p SIP_OUT ) = 0;
    virtual void changeDirection() = 0;
    //virtual void draw(QPainter* p);
    virtual const QgsPoint *getControlPoint( int number ) const = 0;
    virtual const QVector<QgsPoint *> *getControlPoly() const = 0;
    virtual int getDegree() const = 0;
    virtual ParametricLine *getParent() const = 0;
    //virtual bool intersects(ParametricLine* pal);
    virtual void remove( int i ) = 0;
    virtual void setControlPoly( QVector<QgsPoint *> *cp ) = 0;
    virtual void setParent( ParametricLine *paral ) = 0;
};

#ifndef SIP_RUN

//-----------------------------------------constructors and destructor----------------------

inline ParametricLine::ParametricLine( ParametricLine *par, QVector<QgsPoint *> *controlpoly )
  : mParent( par )
  , mControlPoly( controlpoly )
{

}

#endif

#endif








