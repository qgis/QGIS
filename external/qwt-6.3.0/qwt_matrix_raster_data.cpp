/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_matrix_raster_data.h"
#include "qwt_interval.h"

#include <qvector.h>
#include <qnumeric.h>
#include <qrect.h>

static inline double qwtHermiteInterpolate(
    double A, double B, double C, double D, double t )
{
    const double t2 = t * t;
    const double t3 = t2 * t;

    const double a = -A / 2.0 + ( 3.0 * B ) / 2.0 - ( 3.0 * C ) / 2.0 + D / 2.0;
    const double b = A - ( 5.0 * B ) / 2.0 + 2.0 * C - D / 2.0;
    const double c = -A / 2.0 + C / 2.0;
    const double d = B;

    return a * t3 + b * t2 + c * t + d;
}

static inline double qwtBicubicInterpolate(
    double v00, double v10, double v20, double v30,
    double v01, double v11, double v21, double v31,
    double v02, double v12, double v22, double v32,
    double v03, double v13, double v23, double v33,
    double dx, double dy )
{
    const double v0 = qwtHermiteInterpolate( v00, v10, v20, v30, dx );
    const double v1 = qwtHermiteInterpolate( v01, v11, v21, v31, dx );
    const double v2 = qwtHermiteInterpolate( v02, v12, v22, v32, dx );
    const double v3 = qwtHermiteInterpolate( v03, v13, v23, v33, dx );

    return qwtHermiteInterpolate( v0, v1, v2, v3, dy );
}

class QwtMatrixRasterData::PrivateData
{
  public:
    PrivateData()
        : resampleMode( QwtMatrixRasterData::NearestNeighbour )
        , numColumns(0)
    {
    }

    inline double value(int row, int col) const
    {
        return values.data()[ row * numColumns + col ];
    }

    QwtInterval intervals[3];
    QwtMatrixRasterData::ResampleMode resampleMode;

    QVector< double > values;
    int numColumns;
    int numRows;

    double dx;
    double dy;
};

//! Constructor
QwtMatrixRasterData::QwtMatrixRasterData()
{
    m_data = new PrivateData();
    update();
}

//! Destructor
QwtMatrixRasterData::~QwtMatrixRasterData()
{
    delete m_data;
}

/*!
   \brief Set the resampling algorithm

   \param mode Resampling mode
   \sa resampleMode(), value()
 */
void QwtMatrixRasterData::setResampleMode( ResampleMode mode )
{
    m_data->resampleMode = mode;
}

/*!
   \return resampling algorithm
   \sa setResampleMode(), value()
 */
QwtMatrixRasterData::ResampleMode QwtMatrixRasterData::resampleMode() const
{
    return m_data->resampleMode;
}

/*!
   \brief Assign the bounding interval for an axis

   Setting the bounding intervals for the X/Y axis is mandatory
   to define the positions for the values of the value matrix.
   The interval in Z direction defines the possible range for
   the values in the matrix, what is f.e used by QwtPlotSpectrogram
   to map values to colors. The Z-interval might be the bounding
   interval of the values in the matrix, but usually it isn't.
   ( f.e a interval of 0.0-100.0 for values in percentage )

   \param axis X, Y or Z axis
   \param interval Interval

   \sa QwtRasterData::interval(), setValueMatrix()
 */
void QwtMatrixRasterData::setInterval(
    Qt::Axis axis, const QwtInterval& interval )
{
    if ( axis >= 0 && axis <= 2 )
    {
        m_data->intervals[axis] = interval;
        update();
    }
}

/*!
   \return Bounding interval for an axis
   \sa setInterval
 */
QwtInterval QwtMatrixRasterData::interval( Qt::Axis axis ) const
{
    if ( axis >= 0 && axis <= 2 )
        return m_data->intervals[ axis ];

    return QwtInterval();
}

/*!
   \brief Assign a value matrix

   The positions of the values are calculated by dividing
   the bounding rectangle of the X/Y intervals into equidistant
   rectangles ( pixels ). Each value corresponds to the center of
   a pixel.

   \param values Vector of values
   \param numColumns Number of columns

   \sa valueMatrix(), numColumns(), numRows(), setInterval()()
 */
void QwtMatrixRasterData::setValueMatrix(
    const QVector< double >& values, int numColumns )
{
    m_data->values = values;
    m_data->numColumns = qMax( numColumns, 0 );
    update();
}

/*!
   \return Value matrix
   \sa setValueMatrix(), numColumns(), numRows(), setInterval()
 */
const QVector< double > QwtMatrixRasterData::valueMatrix() const
{
    return m_data->values;
}

/*!
   \brief Change a single value in the matrix

   \param row Row index
   \param col Column index
   \param value New value

   \sa value(), setValueMatrix()
 */
void QwtMatrixRasterData::setValue( int row, int col, double value )
{
    if ( row >= 0 && row < m_data->numRows &&
        col >= 0 && col < m_data->numColumns )
    {
        const int index = row * m_data->numColumns + col;
        m_data->values.data()[ index ] = value;
    }
}

/*!
   \return Number of columns of the value matrix
   \sa valueMatrix(), numRows(), setValueMatrix()
 */
int QwtMatrixRasterData::numColumns() const
{
    return m_data->numColumns;
}

/*!
   \return Number of rows of the value matrix
   \sa valueMatrix(), numColumns(), setValueMatrix()
 */
int QwtMatrixRasterData::numRows() const
{
    return m_data->numRows;
}

/*!
   \brief Calculate the pixel hint

   pixelHint() returns the geometry of a pixel, that can be used
   to calculate the resolution and alignment of the plot item, that is
   representing the data.

   - NearestNeighbour\n
     pixelHint() returns the surrounding pixel of the top left value
     in the matrix.

   - BilinearInterpolation\n
     Returns an empty rectangle recommending
     to render in target device ( f.e. screen ) resolution.

   \param area Requested area, ignored
   \return Calculated hint

   \sa ResampleMode, setMatrix(), setInterval()
 */
QRectF QwtMatrixRasterData::pixelHint( const QRectF& area ) const
{
    Q_UNUSED( area )

    QRectF rect;
    if ( m_data->resampleMode == NearestNeighbour )
    {
        const QwtInterval intervalX = interval( Qt::XAxis );
        const QwtInterval intervalY = interval( Qt::YAxis );
        if ( intervalX.isValid() && intervalY.isValid() )
        {
            rect = QRectF( intervalX.minValue(), intervalY.minValue(),
                m_data->dx, m_data->dy );
        }
    }

    return rect;
}

/*!
   \return the value at a raster position

   \param x X value in plot coordinates
   \param y Y value in plot coordinates

   \sa ResampleMode
 */
double QwtMatrixRasterData::value( double x, double y ) const
{
    const QwtInterval xInterval = interval( Qt::XAxis );
    const QwtInterval yInterval = interval( Qt::YAxis );

    if ( !( xInterval.contains(x) && yInterval.contains(y) ) )
        return qQNaN();

    double value;

    switch( m_data->resampleMode )
    {
        case BicubicInterpolation:
        {
            const double colF = ( x - xInterval.minValue() ) / m_data->dx;
            const double rowF = ( y - yInterval.minValue() ) / m_data->dy;

            const int col = qRound( colF );
            const int row = qRound( rowF );

            int col0 = col - 2;
            int col1 = col - 1;
            int col2 = col;
            int col3 = col + 1;

            if ( col1 < 0 )
                col1 = col2;

            if ( col0 < 0 )
                col0 = col1;

            if ( col2 >= m_data->numColumns )
                col2 = col1;

            if ( col3 >= m_data->numColumns )
                col3 = col2;

            int row0 = row - 2;
            int row1 = row - 1;
            int row2 = row;
            int row3 = row + 1;

            if ( row1 < 0 )
                row1 = row2;

            if ( row0 < 0 )
                row0 = row1;

            if ( row2 >= m_data->numRows )
                row2 = row1;

            if ( row3 >= m_data->numRows )
                row3 = row2;

            // First row
            const double v00 = m_data->value( row0, col0 );
            const double v10 = m_data->value( row0, col1 );
            const double v20 = m_data->value( row0, col2 );
            const double v30 = m_data->value( row0, col3 );

            // Second row
            const double v01 = m_data->value( row1, col0 );
            const double v11 = m_data->value( row1, col1 );
            const double v21 = m_data->value( row1, col2 );
            const double v31 = m_data->value( row1, col3 );

            // Third row
            const double v02 = m_data->value( row2, col0 );
            const double v12 = m_data->value( row2, col1 );
            const double v22 = m_data->value( row2, col2 );
            const double v32 = m_data->value( row2, col3 );

            // Fourth row
            const double v03 = m_data->value( row3, col0 );
            const double v13 = m_data->value( row3, col1 );
            const double v23 = m_data->value( row3, col2 );
            const double v33 = m_data->value( row3, col3 );

            value = qwtBicubicInterpolate(
                v00, v10, v20, v30, v01, v11, v21, v31,
                v02, v12, v22, v32, v03, v13, v23, v33,
                colF - col + 0.5, rowF - row + 0.5 );

            break;
        }
        case BilinearInterpolation:
        {
            int col1 = qRound( ( x - xInterval.minValue() ) / m_data->dx ) - 1;
            int row1 = qRound( ( y - yInterval.minValue() ) / m_data->dy ) - 1;
            int col2 = col1 + 1;
            int row2 = row1 + 1;

            if ( col1 < 0 )
                col1 = col2;
            else if ( col2 >= m_data->numColumns )
                col2 = col1;

            if ( row1 < 0 )
                row1 = row2;
            else if ( row2 >= m_data->numRows )
                row2 = row1;

            const double v11 = m_data->value( row1, col1 );
            const double v21 = m_data->value( row1, col2 );
            const double v12 = m_data->value( row2, col1 );
            const double v22 = m_data->value( row2, col2 );

            const double x2 = xInterval.minValue() + ( col2 + 0.5 ) * m_data->dx;
            const double y2 = yInterval.minValue() + ( row2 + 0.5 ) * m_data->dy;

            const double rx = ( x2 - x ) / m_data->dx;
            const double ry = ( y2 - y ) / m_data->dy;

            const double vr1 = rx * v11 + ( 1.0 - rx ) * v21;
            const double vr2 = rx * v12 + ( 1.0 - rx ) * v22;

            value = ry * vr1 + ( 1.0 - ry ) * vr2;

            break;
        }
        case NearestNeighbour:
        default:
        {
            int row = int( ( y - yInterval.minValue() ) / m_data->dy );
            int col = int( ( x - xInterval.minValue() ) / m_data->dx );

            // In case of intervals, where the maximum is included
            // we get out of bound for row/col, when the value for the
            // maximum is requested. Instead we return the value
            // from the last row/col

            if ( row >= m_data->numRows )
                row = m_data->numRows - 1;

            if ( col >= m_data->numColumns )
                col = m_data->numColumns - 1;

            value = m_data->value( row, col );
        }
    }

    return value;
}

void QwtMatrixRasterData::update()
{
    m_data->numRows = 0;
    m_data->dx = 0.0;
    m_data->dy = 0.0;

    if ( m_data->numColumns > 0 )
    {
        m_data->numRows = m_data->values.size() / m_data->numColumns;

        const QwtInterval xInterval = interval( Qt::XAxis );
        const QwtInterval yInterval = interval( Qt::YAxis );
        if ( xInterval.isValid() )
            m_data->dx = xInterval.width() / m_data->numColumns;
        if ( yInterval.isValid() )
            m_data->dy = yInterval.width() / m_data->numRows;
    }
}
