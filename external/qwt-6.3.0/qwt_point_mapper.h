/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_POINT_MAPPER_H
#define QWT_POINT_MAPPER_H

#include "qwt_global.h"

class QwtScaleMap;
template< typename T > class QwtSeriesData;
class QPolygonF;
class QPointF;
class QRectF;
class QPolygon;
class QPen;
class QImage;

/*!
   \brief A helper class for translating a series of points

   QwtPointMapper is a collection of methods and optimizations
   for translating a series of points into paint device coordinates.
   It is used by QwtPlotCurve but might also be useful for
   similar plot items displaying a QwtSeriesData<QPointF>.
 */
class QWT_EXPORT QwtPointMapper
{
  public:
    /*!
       \brief Flags affecting the transformation process
       \sa setFlag(), setFlags()
     */
    enum TransformationFlag
    {
        //! Round points to integer values
        RoundPoints = 0x01,

        /*!
           Try to remove points, that are translated to the
           same position.
         */
        WeedOutPoints = 0x02,

        /*!
           An even more aggressive weeding algorithm, that
           can be used in toPolygon().

           A consecutive chunk of points being mapped to the
           same x coordinate is reduced to 4 points:

              - first point
              - point with the minimum y coordinate
              - point with the maximum y coordinate
              - last point

           In the worst case ( first and last points are never one of the extremes )
           the number of points will be 4 times the width.

           As the algorithm is fast it can be used inside of
           a polyline render cycle.
         */
        WeedOutIntermediatePoints = 0x04
    };

    Q_DECLARE_FLAGS( TransformationFlags, TransformationFlag )

    QwtPointMapper();
    ~QwtPointMapper();

    void setFlags( TransformationFlags );
    TransformationFlags flags() const;

    void setFlag( TransformationFlag, bool on = true );
    bool testFlag( TransformationFlag ) const;

    void setBoundingRect( const QRectF& );
    QRectF boundingRect() const;

    QPolygonF toPolygonF( const QwtScaleMap& xMap, const QwtScaleMap& yMap,
        const QwtSeriesData< QPointF >* series, int from, int to ) const;

    QPolygon toPolygon( const QwtScaleMap& xMap, const QwtScaleMap& yMap,
        const QwtSeriesData< QPointF >* series, int from, int to ) const;

    QPolygon toPoints( const QwtScaleMap& xMap, const QwtScaleMap& yMap,
        const QwtSeriesData< QPointF >* series, int from, int to ) const;

    QPolygonF toPointsF( const QwtScaleMap& xMap, const QwtScaleMap& yMap,
        const QwtSeriesData< QPointF >* series, int from, int to ) const;

    QImage toImage( const QwtScaleMap& xMap, const QwtScaleMap& yMap,
        const QwtSeriesData< QPointF >* series, int from, int to,
        const QPen&, bool antialiased, uint numThreads ) const;

  private:
    Q_DISABLE_COPY(QwtPointMapper)

    class PrivateData;
    PrivateData* m_data;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QwtPointMapper::TransformationFlags )

#endif
