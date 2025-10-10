/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_compass_rose.h"
#include "qwt_point_polar.h"

#include <qpainter.h>
#include <qpainterpath.h>

static QPointF qwtIntersection(
    QPointF p11, QPointF p12, QPointF p21, QPointF p22 )
{
    const QLineF line1( p11, p12 );
    const QLineF line2( p21, p22 );

    QPointF pos;
#if QT_VERSION >= 0x050e00
    if ( line1.intersects( line2, &pos ) == QLineF::NoIntersection )
#else
    if ( line1.intersect( line2, &pos ) == QLineF::NoIntersection )
#endif
        return QPointF();

    return pos;
}

//! Constructor
QwtCompassRose::QwtCompassRose()
{
}

//! Destructor
QwtCompassRose::~QwtCompassRose()
{
}

//! Assign a palette
void QwtCompassRose::setPalette( const QPalette& p )
{
    m_palette = p;
}

//! \return Current palette
const QPalette& QwtCompassRose::palette() const
{
    return m_palette;
}

class QwtSimpleCompassRose::PrivateData
{
  public:
    PrivateData()
        : width( 0.2 )
        , numThorns( 8 )
        , numThornLevels( -1 )
        , shrinkFactor( 0.9 )
    {
    }

    double width;
    int numThorns;
    int numThornLevels;
    double shrinkFactor;
};

/*!
   Constructor

   \param numThorns Number of thorns
   \param numThornLevels Number of thorn levels
 */
QwtSimpleCompassRose::QwtSimpleCompassRose(
    int numThorns, int numThornLevels )
{
    m_data = new PrivateData();
    m_data->numThorns = numThorns;
    m_data->numThornLevels = numThornLevels;

    const QColor dark( 128, 128, 255 );
    const QColor light( 192, 255, 255 );

    QPalette palette;
    palette.setColor( QPalette::Dark, dark );
    palette.setColor( QPalette::Light, light );

    setPalette( palette );
}

//! Destructor
QwtSimpleCompassRose::~QwtSimpleCompassRose()
{
    delete m_data;
}

/*!
   Set the Factor how to shrink the thorns with each level
   The default value is 0.9.

   \param factor Shrink factor
   \sa shrinkFactor()
 */
void QwtSimpleCompassRose::setShrinkFactor( double factor )
{
    m_data->shrinkFactor = factor;
}

/*!
   \return Factor how to shrink the thorns with each level
   \sa setShrinkFactor()
 */
double QwtSimpleCompassRose::shrinkFactor() const
{
    return m_data->shrinkFactor;
}

/*!
   Draw the rose

   \param painter Painter
   \param center Center point
   \param radius Radius of the rose
   \param north Position
   \param cg Color group
 */
void QwtSimpleCompassRose::draw( QPainter* painter, const QPointF& center,
    double radius, double north, QPalette::ColorGroup cg ) const
{
    QPalette pal = palette();
    pal.setCurrentColorGroup( cg );

    drawRose( painter, pal, center, radius, north, m_data->width,
        m_data->numThorns, m_data->numThornLevels, m_data->shrinkFactor );
}

/*!
   Draw the rose

   \param painter Painter
   \param palette Palette
   \param center Center of the rose
   \param radius Radius of the rose
   \param north Position pointing to north
   \param width Width of the rose
   \param numThorns Number of thorns
   \param numThornLevels Number of thorn levels
   \param shrinkFactor Factor to shrink the thorns with each level
 */
void QwtSimpleCompassRose::drawRose(
    QPainter* painter,
    const QPalette& palette,
    const QPointF& center, double radius, double north, double width,
    int numThorns, int numThornLevels, double shrinkFactor )
{
    if ( numThorns < 4 )
        numThorns = 4;

    if ( numThorns % 4 )
        numThorns += 4 - numThorns % 4;

    if ( numThornLevels <= 0 )
        numThornLevels = numThorns / 4;

    if ( shrinkFactor >= 1.0 )
        shrinkFactor = 1.0;

    if ( shrinkFactor <= 0.5 )
        shrinkFactor = 0.5;

    painter->save();

    painter->setPen( Qt::NoPen );

    for ( int j = 1; j <= numThornLevels; j++ )
    {
        double step = std::pow( 2.0, j ) * M_PI / numThorns;
        if ( step > M_PI_2 )
            break;

        double r = radius;
        for ( int k = 0; k < 3; k++ )
        {
            if ( j + k < numThornLevels )
                r *= shrinkFactor;
        }

        double leafWidth = r * width;
        if ( 2.0 * M_PI / step > 32 )
            leafWidth = 16;

        const double origin = qwtRadians( north );
        for ( double angle = origin;
            angle < 2.0 * M_PI + origin; angle += step )
        {
            const QPointF p = qwtPolar2Pos( center, r, angle );
            const QPointF p1 = qwtPolar2Pos( center, leafWidth, angle + M_PI_2 );
            const QPointF p2 = qwtPolar2Pos( center, leafWidth, angle - M_PI_2 );
            const QPointF p3 = qwtPolar2Pos( center, r, angle + step / 2.0 );
            const QPointF p4 = qwtPolar2Pos( center, r, angle - step / 2.0 );

            QPainterPath darkPath;
            darkPath.moveTo( center );
            darkPath.lineTo( p );
            darkPath.lineTo( qwtIntersection( center, p3, p1, p ) );

            painter->setBrush( palette.brush( QPalette::Dark ) );
            painter->drawPath( darkPath );

            QPainterPath lightPath;
            lightPath.moveTo( center );
            lightPath.lineTo( p );
            lightPath.lineTo( qwtIntersection( center, p4, p2, p ) );

            painter->setBrush( palette.brush( QPalette::Light ) );
            painter->drawPath( lightPath );
        }
    }
    painter->restore();
}

/*!
   Set the width of the rose heads. Lower value make thinner heads.
   The range is limited from 0.03 to 0.4.

   \param width Width
 */
void QwtSimpleCompassRose::setWidth( double width )
{
    m_data->width = width;
    if ( m_data->width < 0.03 )
        m_data->width = 0.03;

    if ( m_data->width > 0.4 )
        m_data->width = 0.4;
}

/*!
   \return Width of the rose
   \sa setWidth()
 */
double QwtSimpleCompassRose::width() const
{
    return m_data->width;
}

/*!
   Set the number of thorns on one level
   The number is aligned to a multiple of 4, with a minimum of 4

   \param numThorns Number of thorns
   \sa numThorns(), setNumThornLevels()
 */
void QwtSimpleCompassRose::setNumThorns( int numThorns )
{
    if ( numThorns < 4 )
        numThorns = 4;

    if ( numThorns % 4 )
        numThorns += 4 - numThorns % 4;

    m_data->numThorns = numThorns;
}

/*!
   \return Number of thorns
   \sa setNumThorns(), setNumThornLevels()
 */
int QwtSimpleCompassRose::numThorns() const
{
    return m_data->numThorns;
}

/*!
   Set the of thorns levels

   \param numThornLevels Number of thorns levels
   \sa setNumThorns(), numThornLevels()
 */
void QwtSimpleCompassRose::setNumThornLevels( int numThornLevels )
{
    m_data->numThornLevels = numThornLevels;
}

/*!
   \return Number of thorn levels
   \sa setNumThorns(), setNumThornLevels()
 */
int QwtSimpleCompassRose::numThornLevels() const
{
    return m_data->numThornLevels;
}
