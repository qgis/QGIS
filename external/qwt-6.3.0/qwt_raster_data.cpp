/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_raster_data.h"
#include "qwt_point_3d.h"
#include "qwt_interval.h"

#include <qrect.h>
#include <qpolygon.h>
#include <qnumeric.h>
#include <qlist.h>
#include <qmap.h>

class QwtRasterData::ContourPlane
{
  public:
    explicit inline ContourPlane( double z ):
        m_z( z )
    {
    }

    inline bool intersect( const QwtPoint3D vertex[3],
        QPointF line[2], bool ignoreOnPlane ) const;

    inline double z() const { return m_z; }

  private:
    inline int compare( double z ) const;
    inline QPointF intersection(
        const QwtPoint3D& p1, const QwtPoint3D& p2 ) const;

    double m_z;
};

inline bool QwtRasterData::ContourPlane::intersect(
    const QwtPoint3D vertex[3], QPointF line[2],
    bool ignoreOnPlane ) const
{
    bool found = true;

    // Are the vertices below (-1), on (0) or above (1) the plan ?
    const int eq1 = compare( vertex[0].z() );
    const int eq2 = compare( vertex[1].z() );
    const int eq3 = compare( vertex[2].z() );

    /*
        (a) All the vertices lie below the contour level.
        (b) Two vertices lie below and one on the contour level.
        (c) Two vertices lie below and one above the contour level.
        (d) One vertex lies below and two on the contour level.
        (e) One vertex lies below, one on and one above the contour level.
        (f) One vertex lies below and two above the contour level.
        (g) Three vertices lie on the contour level.
        (h) Two vertices lie on and one above the contour level.
        (i) One vertex lies on and two above the contour level.
        (j) All the vertices lie above the contour level.
     */

    static const int tab[3][3][3] =
    {
        // jump table to avoid nested case statements
        { { 0, 0, 8 }, { 0, 2, 5 }, { 7, 6, 9 } },
        { { 0, 3, 4 }, { 1, 10, 1 }, { 4, 3, 0 } },
        { { 9, 6, 7 }, { 5, 2, 0 }, { 8, 0, 0 } }
    };

    const int edgeType = tab[eq1 + 1][eq2 + 1][eq3 + 1];
    switch ( edgeType )
    {
        case 1:
            // d(0,0,-1), h(0,0,1)
            line[0] = vertex[0].toPoint();
            line[1] = vertex[1].toPoint();
            break;
        case 2:
            // d(-1,0,0), h(1,0,0)
            line[0] = vertex[1].toPoint();
            line[1] = vertex[2].toPoint();
            break;
        case 3:
            // d(0,-1,0), h(0,1,0)
            line[0] = vertex[2].toPoint();
            line[1] = vertex[0].toPoint();
            break;
        case 4:
            // e(0,-1,1), e(0,1,-1)
            line[0] = vertex[0].toPoint();
            line[1] = intersection( vertex[1], vertex[2] );
            break;
        case 5:
            // e(-1,0,1), e(1,0,-1)
            line[0] = vertex[1].toPoint();
            line[1] = intersection( vertex[2], vertex[0] );
            break;
        case 6:
            // e(-1,1,0), e(1,0,-1)
            line[0] = vertex[2].toPoint();
            line[1] = intersection( vertex[0], vertex[1] );
            break;
        case 7:
            // c(-1,1,-1), f(1,1,-1)
            line[0] = intersection( vertex[0], vertex[1] );
            line[1] = intersection( vertex[1], vertex[2] );
            break;
        case 8:
            // c(-1,-1,1), f(1,1,-1)
            line[0] = intersection( vertex[1], vertex[2] );
            line[1] = intersection( vertex[2], vertex[0] );
            break;
        case 9:
            // f(-1,1,1), c(1,-1,-1)
            line[0] = intersection( vertex[2], vertex[0] );
            line[1] = intersection( vertex[0], vertex[1] );
            break;
        case 10:
            // g(0,0,0)
            // The CONREC algorithm has no satisfying solution for
            // what to do, when all vertices are on the plane.

            if ( ignoreOnPlane )
                found = false;
            else
            {
                line[0] = vertex[2].toPoint();
                line[1] = vertex[0].toPoint();
            }
            break;
        default:
            found = false;
    }

    return found;
}

inline int QwtRasterData::ContourPlane::compare( double z ) const
{
    if ( z > m_z )
        return 1;

    if ( z < m_z )
        return -1;

    return 0;
}

inline QPointF QwtRasterData::ContourPlane::intersection(
    const QwtPoint3D& p1, const QwtPoint3D& p2 ) const
{
    const double h1 = p1.z() - m_z;
    const double h2 = p2.z() - m_z;

    const double x = ( h2 * p1.x() - h1 * p2.x() ) / ( h2 - h1 );
    const double y = ( h2 * p1.y() - h1 * p2.y() ) / ( h2 - h1 );

    return QPointF( x, y );
}

class QwtRasterData::PrivateData
{
  public:
    QwtRasterData::Attributes attributes;
};

//! Constructor
QwtRasterData::QwtRasterData()
{
    m_data = new PrivateData();
}

//! Destructor
QwtRasterData::~QwtRasterData()
{
    delete m_data;
}

/*!
   Specify an attribute of the data

   \param attribute Attribute
   \param on On/Off
   /sa Attribute, testAttribute()
 */
void QwtRasterData::setAttribute( Attribute attribute, bool on )
{
    if ( on )
        m_data->attributes |= attribute;
    else
        m_data->attributes &= ~attribute;
}

/*!
    \return True, when attribute is enabled
    \sa Attribute, setAttribute()
 */
bool QwtRasterData::testAttribute( Attribute attribute ) const
{
    return m_data->attributes & attribute;
}

/*!
   \brief Initialize a raster

   Before the composition of an image QwtPlotSpectrogram calls initRaster(),
   announcing the area and its resolution that will be requested.

   The default implementation does nothing, but for data sets that
   are stored in files, it might be good idea to reimplement initRaster(),
   where the data is resampled and loaded into memory.

   \param area Area of the raster
   \param raster Number of horizontal and vertical pixels

   \sa initRaster(), value()
 */
void QwtRasterData::initRaster( const QRectF& area, const QSize& raster )
{
    Q_UNUSED( area );
    Q_UNUSED( raster );
}

/*!
   \brief Discard a raster

   After the composition of an image QwtPlotSpectrogram calls discardRaster().

   The default implementation does nothing, but if data has been loaded
   in initRaster(), it could deleted now.

   \sa initRaster(), value()
 */
void QwtRasterData::discardRaster()
{
}

/*!
   \brief Pixel hint

   pixelHint() returns the geometry of a pixel, that can be used
   to calculate the resolution and alignment of the plot item, that is
   representing the data.

   Width and height of the hint need to be the horizontal
   and vertical distances between 2 neighbored points.
   The center of the hint has to be the position of any point
   ( it doesn't matter which one ).

   An empty hint indicates, that there are values for any detail level.

   Limiting the resolution of the image might significantly improve
   the performance and heavily reduce the amount of memory when rendering
   a QImage from the raster data.

   The default implementation returns an empty rectangle recommending
   to render in target device ( f.e. screen ) resolution.

   \param area In most implementations the resolution of the data doesn't
               depend on the requested area.

   \return Bounding rectangle of a pixel
 */
QRectF QwtRasterData::pixelHint( const QRectF& area ) const
{
    Q_UNUSED( area );
    return QRectF();
}

/*!
   Calculate contour lines

   \param rect Bounding rectangle for the contour lines
   \param raster Number of data pixels of the raster data
   \param levels List of limits, where to insert contour lines
   \param flags Flags to customize the contouring algorithm

   \return Calculated contour lines

   An adaption of CONREC, a simple contouring algorithm.
   http://local.wasp.uwa.edu.au/~pbourke/papers/conrec/
 */
QwtRasterData::ContourLines QwtRasterData::contourLines(
    const QRectF& rect, const QSize& raster,
    const QList< double >& levels, ConrecFlags flags ) const
{
    ContourLines contourLines;

    if ( levels.size() == 0 || !rect.isValid() || !raster.isValid() )
        return contourLines;

    const double dx = rect.width() / raster.width();
    const double dy = rect.height() / raster.height();

    const bool ignoreOnPlane =
        flags & QwtRasterData::IgnoreAllVerticesOnLevel;

    const QwtInterval range = interval( Qt::ZAxis );
    bool ignoreOutOfRange = false;
    if ( range.isValid() )
        ignoreOutOfRange = flags & IgnoreOutOfRange;

    QwtRasterData* that = const_cast< QwtRasterData* >( this );
    that->initRaster( rect, raster );

    for ( int y = 0; y < raster.height() - 1; y++ )
    {
        enum Position
        {
            Center,

            TopLeft,
            TopRight,
            BottomRight,
            BottomLeft,

            NumPositions
        };

        QwtPoint3D xy[NumPositions];

        for ( int x = 0; x < raster.width() - 1; x++ )
        {
            const QPointF pos( rect.x() + x * dx, rect.y() + y * dy );

            if ( x == 0 )
            {
                xy[TopRight].setX( pos.x() );
                xy[TopRight].setY( pos.y() );
                xy[TopRight].setZ(
                    value( xy[TopRight].x(), xy[TopRight].y() )
                    );

                xy[BottomRight].setX( pos.x() );
                xy[BottomRight].setY( pos.y() + dy );
                xy[BottomRight].setZ(
                    value( xy[BottomRight].x(), xy[BottomRight].y() )
                    );
            }

            xy[TopLeft] = xy[TopRight];
            xy[BottomLeft] = xy[BottomRight];

            xy[TopRight].setX( pos.x() + dx );
            xy[TopRight].setY( pos.y() );
            xy[BottomRight].setX( pos.x() + dx );
            xy[BottomRight].setY( pos.y() + dy );

            xy[TopRight].setZ(
                value( xy[TopRight].x(), xy[TopRight].y() )
                );
            xy[BottomRight].setZ(
                value( xy[BottomRight].x(), xy[BottomRight].y() )
                );

            double zMin = xy[TopLeft].z();
            double zMax = zMin;
            double zSum = zMin;

            for ( int i = TopRight; i <= BottomLeft; i++ )
            {
                const double z = xy[i].z();

                zSum += z;
                if ( z < zMin )
                    zMin = z;
                if ( z > zMax )
                    zMax = z;
            }

            if ( qIsNaN( zSum ) )
            {
                // one of the points is NaN
                continue;
            }

            if ( ignoreOutOfRange )
            {
                if ( !range.contains( zMin ) || !range.contains( zMax ) )
                    continue;
            }

            if ( zMax < levels[0] ||
                zMin > levels[levels.size() - 1] )
            {
                continue;
            }

            xy[Center].setX( pos.x() + 0.5 * dx );
            xy[Center].setY( pos.y() + 0.5 * dy );
            xy[Center].setZ( 0.25 * zSum );

            const int numLevels = levels.size();
            for ( int l = 0; l < numLevels; l++ )
            {
                const double level = levels[l];
                if ( level < zMin || level > zMax )
                    continue;
                QPolygonF& lines = contourLines[level];
                const ContourPlane plane( level );

                QPointF line[2];
                QwtPoint3D vertex[3];

                for ( int m = TopLeft; m < NumPositions; m++ )
                {
                    vertex[0] = xy[m];
                    vertex[1] = xy[0];
                    vertex[2] = xy[m != BottomLeft ? m + 1 : TopLeft];

                    const bool intersects =
                        plane.intersect( vertex, line, ignoreOnPlane );
                    if ( intersects )
                    {
                        lines += line[0];
                        lines += line[1];
                    }
                }
            }
        }
    }

    that->discardRaster();

    return contourLines;
}
