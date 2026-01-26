/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_CURVE_FITTER_H
#define QWT_CURVE_FITTER_H

#include "qwt_global.h"

class QPainterPath;
class QPolygonF;

/*!
   \brief Abstract base class for a curve fitter
 */
class QWT_EXPORT QwtCurveFitter
{
  public:
    /*!
       \brief Preferred mode of the fitting algorithm

       Even if a QPainterPath can always be created from a QPolygonF
       the overhead of the conversion can be avoided by indicating
       the preference of the implementation to the application
       code.
     */
    enum Mode
    {
        /*!
           The fitting algorithm creates a polygon - the implementation
           of fitCurvePath() simply wraps the polygon into a path.

           \sa QwtWeedingCurveFitter
         */
        Polygon,

        /*!
           The fitting algorithm creates a painter path - the implementation
           of fitCurve() extracts a polygon from the path.

           \sa QwtSplineCurveFitter
         */
        Path
    };

    virtual ~QwtCurveFitter();

    Mode mode() const;

    /*!
        Find a curve which has the best fit to a series of data points

        \param polygon Series of data points
        \return Curve points

        \sa fitCurvePath()
     */
    virtual QPolygonF fitCurve( const QPolygonF& polygon ) const = 0;

    /*!
        Find a curve path which has the best fit to a series of data points

        \param polygon Series of data points
        \return Curve path

        \sa fitCurve()
     */
    virtual QPainterPath fitCurvePath( const QPolygonF& polygon ) const = 0;

  protected:
    explicit QwtCurveFitter( Mode mode );

  private:
    Q_DISABLE_COPY(QwtCurveFitter)

    const Mode m_mode;
};

#endif
