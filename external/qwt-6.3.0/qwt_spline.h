/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_SPLINE_H
#define QWT_SPLINE_H

#include "qwt_global.h"
#include "qwt_spline.h"

class QwtSplineParametrization;
class QwtSplinePolynomial;
class QPainterPath;
class QLineF;
class QPolygonF;

#if QT_VERSION < 0x060000
template< typename T > class QVector;
#endif

/*!
   \brief Base class for all splines

   A spline is a curve represented by a sequence of polynomials. Spline approximation
   is the process of finding polynomials for a given set of points.
   When the algorithm preserves the initial points it is called interpolating.

   Splines can be classified according to conditions of the polynomials that
   are met at the start/endpoints of the pieces:

   - Geometric Continuity

    - G0: polynomials are joined
    - G1: first derivatives are proportional at the join point
          The curve tangents thus have the same direction, but not necessarily the
          same magnitude. i.e., C1'(1) = (a,b,c) and C2'(0) = (k*a, k*b, k*c).
    - G2: first and second derivatives are proportional at join point

   - Parametric Continuity

    - C0: curves are joined
    - C1: first derivatives equal
    - C2: first and second derivatives are equal

   Geometric continuity requires the geometry to be continuous, while parametric
   continuity requires that the underlying parameterization be continuous as well.
   Parametric continuity of order n implies geometric continuity of order n,
   but not vice-versa.

   QwtSpline is the base class for spline approximations of any continuity.
 */
class QWT_EXPORT QwtSpline
{
  public:
    /*!
       Boundary type specifying the spline at its endpoints

       \sa setBoundaryType(), boundaryType()
     */
    enum BoundaryType
    {
        /*!
           The polynomials at the start/endpoint depend on specific conditions

           \sa QwtSpline::BoundaryCondition
         */
        ConditionalBoundaries,

        /*!
           The polynomials at the start/endpoint are found by using
           imaginary additional points. Additional points at the end
           are found by translating points from the beginning or v.v.
         */
        PeriodicPolygon,

        /*!
           ClosedPolygon is similar to PeriodicPolygon beside, that
           the interpolation includes the connection between the last
           and the first control point.

           \note Only works for parametrizations, where the parameter increment
                for the the final closing line is positive.
                This excludes QwtSplineParametrization::ParameterX and
                QwtSplineParametrization::ParameterY
         */

        ClosedPolygon
    };

    /*!
       position of a boundary condition
       \sa boundaryCondition(), boundaryValue()
     */
    enum BoundaryPosition
    {
        //! the condition is at the beginning of the polynomial
        AtBeginning,

        //! the condition is at the end of the polynomial
        AtEnd
    };

    /*!
       \brief Boundary condition

       A spline algorithm calculates polynomials by looking
       a couple of points back/ahead ( locality() ). At the ends
       additional rules are necessary to compensate the missing
       points.

       \sa boundaryCondition(), boundaryValue()
       \sa QwtSplineC2::BoundaryConditionC2
     */
    enum BoundaryCondition
    {
        /*!
           The first derivative at the end point is given
           \sa boundaryValue()
         */
        Clamped1,

        /*!
           The second derivative at the end point is given

           \sa boundaryValue()
           \note a condition having a second derivative of 0
                is also called "natural".
         */
        Clamped2,

        /*!
           The third derivative at the end point is given

           \sa boundaryValue()
           \note a condition having a third derivative of 0
                is also called "parabolic runout".
         */
        Clamped3,

        /*!
           The first derivate at the endpoint is related to the first derivative
           at its neighbour by the boundary value. F,e when the boundary
           value at the end is 1.0 then the slope at the last 2 points is
           the same.

           \sa boundaryValue().
         */
        LinearRunout
    };

    QwtSpline();
    virtual ~QwtSpline();

    void setParametrization( int type );
    void setParametrization( QwtSplineParametrization* );
    const QwtSplineParametrization* parametrization() const;

    void setBoundaryType( BoundaryType );
    BoundaryType boundaryType() const;

    void setBoundaryValue( BoundaryPosition, double value );
    double boundaryValue( BoundaryPosition ) const;

    void setBoundaryCondition( BoundaryPosition, int condition );
    int boundaryCondition( BoundaryPosition ) const;

    void setBoundaryConditions( int condition,
        double valueBegin = 0.0, double valueEnd = 0.0 );

    virtual QPolygonF polygon( const QPolygonF&, double tolerance ) const;
    virtual QPainterPath painterPath( const QPolygonF& ) const = 0;

    virtual uint locality() const;

  private:
    Q_DISABLE_COPY(QwtSpline)

    class PrivateData;
    PrivateData* m_data;
};

/*!
   \brief Base class for a spline interpolation

   Spline interpolation is the process of interpolating a set of points
   piecewise with polynomials. The initial set of points is preserved.
 */
class QWT_EXPORT QwtSplineInterpolating : public QwtSpline
{
  public:
    QwtSplineInterpolating();
    virtual ~QwtSplineInterpolating();

    virtual QPolygonF equidistantPolygon( const QPolygonF&,
        double distance, bool withNodes ) const;

    virtual QPolygonF polygon(
        const QPolygonF&, double tolerance ) const QWT_OVERRIDE;

    virtual QPainterPath painterPath( const QPolygonF& ) const QWT_OVERRIDE;
    virtual QVector< QLineF > bezierControlLines( const QPolygonF& ) const = 0;

  private:
    Q_DISABLE_COPY(QwtSplineInterpolating)
};

/*!
   \brief Base class for spline interpolations providing a
         first order geometric continuity ( G1 ) between adjoining curves
 */
class QWT_EXPORT QwtSplineG1 : public QwtSplineInterpolating
{
  public:
    QwtSplineG1();
    virtual ~QwtSplineG1();
};

/*!
   \brief Base class for spline interpolations providing a
         first order parametric continuity ( C1 ) between adjoining curves

   All interpolations with C1 continuity are based on rules for finding
   the 1. derivate at some control points.

   In case of non parametric splines those points are the curve points, while
   for parametric splines the calculation is done twice using a parameter value t.

   \sa QwtSplineParametrization
 */
class QWT_EXPORT QwtSplineC1 : public QwtSplineG1
{
  public:
    QwtSplineC1();
    virtual ~QwtSplineC1();

    virtual QPainterPath painterPath( const QPolygonF& ) const QWT_OVERRIDE;
    virtual QVector< QLineF > bezierControlLines( const QPolygonF& ) const QWT_OVERRIDE;

    virtual QPolygonF equidistantPolygon( const QPolygonF&,
        double distance, bool withNodes ) const QWT_OVERRIDE;

    // these methods are the non parametric part
    virtual QVector< QwtSplinePolynomial > polynomials( const QPolygonF& ) const;
    virtual QVector< double > slopes( const QPolygonF& ) const = 0;

    virtual double slopeAtBeginning( const QPolygonF&, double slopeNext ) const;
    virtual double slopeAtEnd( const QPolygonF&, double slopeBefore ) const;
};

/*!
   \brief Base class for spline interpolations providing a
         second order parametric continuity ( C2 ) between adjoining curves

   All interpolations with C2 continuity are based on rules for finding
   the 2. derivate at some control points.

   In case of non parametric splines those points are the curve points, while
   for parametric splines the calculation is done twice using a parameter value t.

   \sa QwtSplineParametrization
 */
class QWT_EXPORT QwtSplineC2 : public QwtSplineC1
{
  public:
    /*!
       Boundary condition that requires C2 continuity

       \sa QwtSpline::boundaryCondition, QwtSpline::BoundaryCondition
     */
    enum BoundaryConditionC2
    {
        /*!
           The second derivate at the endpoint is related to the second derivatives
           at the 2 neighbours: cv[0] := 2.0 * cv[1] - cv[2].

           \note boundaryValue() is ignored
         */
        CubicRunout = LinearRunout + 1,

        /*!
           The 3rd derivate at the endpoint matches the 3rd derivate at its neighbours.
           Or in other words: the first/last curve segment extents the polynomial of its
           neighboured polynomial

           \note boundaryValue() is ignored
         */
        NotAKnot
    };

    QwtSplineC2();
    virtual ~QwtSplineC2();

    virtual QPainterPath painterPath( const QPolygonF& ) const QWT_OVERRIDE;
    virtual QVector< QLineF > bezierControlLines( const QPolygonF& ) const QWT_OVERRIDE;

    virtual QPolygonF equidistantPolygon( const QPolygonF&,
        double distance, bool withNodes ) const QWT_OVERRIDE;

    // calculating the parametric equations
    virtual QVector< QwtSplinePolynomial > polynomials( const QPolygonF& ) const QWT_OVERRIDE;
    virtual QVector< double > slopes( const QPolygonF& ) const QWT_OVERRIDE;
    virtual QVector< double > curvatures( const QPolygonF& ) const = 0;
};

#endif
