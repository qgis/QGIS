/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/analysis/interpolation/ParametricLine.h                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class ParametricLine
{
%Docstring
 ParametricLine is an Interface for parametric lines. It is possible, that a parametric line is composed of several parametric
 lines (see the composite pattern in Gamma et al. 'Design Patterns'). Do not build instances of it since it is an abstract class.*
%End

%TypeHeaderCode
#include "ParametricLine.h"
%End
  protected:
  public:
    ParametricLine();
%Docstring
Default constructor
%End

    ParametricLine( ParametricLine *par /Transfer/, QVector<QgsPoint *> *controlpoly );
%Docstring
 Constructor, par is a pointer to the parent object, controlpoly the controlpolygon
%End
    virtual ~ParametricLine();
    virtual void add( ParametricLine *pl /Transfer/ ) = 0;
    virtual void calcFirstDer( float t, Vector3D *v /Out/ ) = 0;
    virtual void calcSecDer( float t, Vector3D *v /Out/ ) = 0;
    virtual void calcPoint( float t, QgsPoint *p /Out/ ) = 0;
    virtual void changeDirection() = 0;
    virtual const QgsPoint *getControlPoint( int number ) const = 0;
%Docstring
 :rtype: QgsPoint
%End
    virtual const QVector<QgsPoint *> *getControlPoly() const = 0;
%Docstring
 :rtype: list of QgsPoint
%End
    virtual int getDegree() const = 0;
%Docstring
 :rtype: int
%End
    virtual ParametricLine *getParent() const = 0;
%Docstring
 :rtype: ParametricLine
%End
    virtual void remove( int i ) = 0;
    virtual void setControlPoly( QVector<QgsPoint *> *cp ) = 0;
    virtual void setParent( ParametricLine *paral ) = 0;
};










/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/analysis/interpolation/ParametricLine.h                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
