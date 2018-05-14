/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/analysis/interpolation/Bezier3D.h                                *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class Bezier3D: ParametricLine
{
%Docstring
 Class Bezier3D represents a bezier curve, represented by control points. Parameter t is running from 0 to 1. The class is capable to calculate the curve point and the first two derivatives belonging to t.*
%End

%TypeHeaderCode
#include "Bezier3D.h"
%End
  protected:

  public:
    Bezier3D();
%Docstring
Default constructor
%End
    Bezier3D( ParametricLine *par, QVector<QgsPoint *> *controlpoly );
%Docstring
Constructor, par is a pointer to the parent, controlpoly a controlpolygon
%End

    virtual ~Bezier3D();
    virtual void add( ParametricLine *pl /Transfer/ );
%Docstring
Do not use this method, since a Bezier curve does not consist of other curves
%End
    virtual void calcFirstDer( float t, Vector3D *v /Out/ );
%Docstring
Calculates the first derivative and assigns it to v
%End
    virtual void calcSecDer( float t, Vector3D *v /Out/ );
%Docstring
Calculates the second derivative and assigns it to v
%End
    virtual void calcPoint( float t, QgsPoint *p /Out/ );
%Docstring
Calculates the point on the curve and assigns it to p
%End
    virtual void changeDirection();
%Docstring
Changes the order of control points
%End
    virtual void remove( int i );
%Docstring
Do not use this method, since a Bezier curve does not consist of other curves
%End
    virtual const QgsPoint *getControlPoint( int number ) const;
%Docstring
Returns a control point
 :rtype: QgsPoint
%End
    virtual const QVector<QgsPoint *> *getControlPoly() const;
%Docstring
Returns a pointer to the control polygon
 :rtype: list of QgsPoint
%End
    virtual int getDegree() const;
%Docstring
Returns the degree of the curve
 :rtype: int
%End
    virtual ParametricLine *getParent() const;
%Docstring
Returns the parent
 :rtype: ParametricLine
%End
    virtual void setParent( ParametricLine *par );
%Docstring
Sets the parent
%End
    virtual void setControlPoly( QVector<QgsPoint *> *cp );
%Docstring
Sets the control polygon
%End

};



/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/analysis/interpolation/Bezier3D.h                                *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
