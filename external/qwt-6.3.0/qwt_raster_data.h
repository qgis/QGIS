/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_RASTER_DATA_H
#define QWT_RASTER_DATA_H

#include "qwt_global.h"
#include <qnamespace.h>

class QwtInterval;
class QPolygonF;
class QRectF;
class QSize;
template< typename T > class QList;
template< class Key, class T > class QMap;

/*!
   \brief QwtRasterData defines an interface to any type of raster data.

   QwtRasterData is an abstract interface, that is used by
   QwtPlotRasterItem to find the values at the pixels of its raster.

   Gaps inside the bounding rectangle of the data can be indicated by NaN
   values ( when WithoutGaps is disabled ).

   Often a raster item is used to display values from a matrix. Then the
   derived raster data class needs to implement some sort of resampling,
   that maps the raster of the matrix into the requested raster of
   the raster item ( depending on resolution and scales of the canvas ).

   QwtMatrixRasterData implements raster data, that returns values from
   a given 2D matrix.

   \sa QwtMatrixRasterData
 */
class QWT_EXPORT QwtRasterData
{
  public:
    //! Contour lines
    typedef QMap< double, QPolygonF > ContourLines;

    /*!
       \brief Raster data attributes

       Additional information that is used to improve processing
       of the data.
     */
    enum Attribute
    {
        /*!
           The bounding rectangle of the data is spanned by
           the interval(Qt::XAxis) and interval(Qt::YAxis).

           WithoutGaps indicates, that the data has no gaps
           ( unknown values ) in this area and the result of
           value() does not need to be checked for NaN values.

           Enabling this flag will have an positive effect on
           the performance of rendering a QwtPlotSpectrogram.

           The default setting is false.

           \note NaN values indicate an undefined value
         */
        WithoutGaps = 0x01
    };

    Q_DECLARE_FLAGS( Attributes, Attribute )

    //! Flags to modify the contour algorithm
    enum ConrecFlag
    {
        //! Ignore all vertices on the same level
        IgnoreAllVerticesOnLevel = 0x01,

        //! Ignore all values, that are out of range
        IgnoreOutOfRange = 0x02
    };

    Q_DECLARE_FLAGS( ConrecFlags, ConrecFlag )

    QwtRasterData();
    virtual ~QwtRasterData();

    void setAttribute( Attribute, bool on = true );
    bool testAttribute( Attribute ) const;

    /*!
       \return Bounding interval for an axis
       \sa setInterval
     */
    virtual QwtInterval interval( Qt::Axis ) const = 0;

    virtual QRectF pixelHint( const QRectF& ) const;

    virtual void initRaster( const QRectF&, const QSize& raster );
    virtual void discardRaster();

    /*!
       \return the value at a raster position
       \param x X value in plot coordinates
       \param y Y value in plot coordinates
     */
    virtual double value( double x, double y ) const = 0;

    virtual ContourLines contourLines( const QRectF& rect,
        const QSize& raster, const QList< double >& levels,
        ConrecFlags ) const;

    class Contour3DPoint;
    class ContourPlane;

  private:
    Q_DISABLE_COPY(QwtRasterData)

    class PrivateData;
    PrivateData* m_data;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QwtRasterData::ConrecFlags )
Q_DECLARE_OPERATORS_FOR_FLAGS( QwtRasterData::Attributes )

#endif
