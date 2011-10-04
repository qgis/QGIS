/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * QwtPolar Widget Library
 * Copyright (C) 2008   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_polar_spectrogram.h"
#include "qwt_polar.h"
#include "qwt_polar_plot.h"
#include <qwt_color_map.h>
#include <qwt_scale_map.h>
#include <qwt_raster_data.h>
#include <qwt_math.h>
#include <qpainter.h>
#if QT_VERSION >= 0x040400
#include <qthread.h>
#include <qfuture.h>
#include <qtconcurrentrun.h>
#endif

#if QWT_VERSION < 0x060100

static inline double qwtFastAtan( double x )
{
    if ( x < -1.0 )
        return -M_PI_2 - x / ( x * x + 0.28 );

    if ( x > 1.0 )
        return M_PI_2 - x / ( x * x + 0.28 );

    return x / ( 1.0 + x * x * 0.28 );
}

static inline double qwtFastAtan2( double y, double x )
{
    if ( x > 0 )
        return qwtFastAtan( y / x );

    if ( x < 0 )
    {
        const double d = qwtFastAtan( y / x );
        return ( y >= 0 ) ? d + M_PI : d - M_PI;
    }

    if ( y < 0.0 )
        return -M_PI_2;

    if ( y > 0.0 )
        return M_PI_2;

    return 0.0;
}

#endif // QWT_VERSION < 0x060100

#if QT_VERSION < 0x040601
#define qAtan2(y, x) ::atan2(y, x)
#endif

static bool qwtNeedsClipping( const QRectF &plotRect, const QRectF &rect )
{
    QPointF points[4];
    points[0] = rect.topLeft();
    points[1] = rect.topRight();
    points[2] = rect.bottomLeft();
    points[3] = rect.bottomRight();

    const double radius = plotRect.width() / 2.0;
    const QPointF pole = plotRect.center();

    for ( int i = 0; i < 4; i++ )
    {
        const double dx = points[i].x() - pole.x();
        const double dy = points[i].y() - pole.y();

        if ( qSqrt( dx * dx + dy * dy ) > radius )
            return true;
    }

    return false;
}

class QwtPolarSpectrogram::TileInfo
{
public:
    QPoint imagePos;
    QRect rect;
    QImage *image;
};

class QwtPolarSpectrogram::PrivateData
{
public:
    PrivateData():
        data( NULL ),
        renderThreadCount( 1 )
    {
        colorMap = new QwtLinearColorMap();
    }

    ~PrivateData()
    {
    }

    QwtRasterData *data;
    QwtColorMap *colorMap;

    uint renderThreadCount;

    QwtPolarSpectrogram::PaintAttributes paintAttributes;
};

//!  Constructor
QwtPolarSpectrogram::QwtPolarSpectrogram():
    QwtPolarItem( QwtText( "Spectrogram" ) )
{
    d_data = new PrivateData;

    setItemAttribute( QwtPolarItem::AutoScale );
    setItemAttribute( QwtPolarItem::Legend, false );

    setZ( 20.0 );
}

//! Destructor
QwtPolarSpectrogram::~QwtPolarSpectrogram()
{
    delete d_data;
}

//! \return QwtPolarItem::Rtti_PolarSpectrogram
int QwtPolarSpectrogram::rtti() const
{
    return QwtPolarItem::Rtti_PolarSpectrogram;
}

/*!
  Set the data to be displayed

  \param data Spectrogram Data
  \sa data()

  \warning QwtRasterData::initRaster() is called each time before the
           image is rendered, but without any useful parameters.
           Also QwtRasterData::rasterHint() is not used.
*/
void QwtPolarSpectrogram::setData( QwtRasterData *data )
{
    if ( data != d_data->data )
    {
        delete d_data->data;
        d_data->data = data;

        itemChanged();
    }
}

/*!
  \return Spectrogram data
  \sa setData()
*/
const QwtRasterData *QwtPolarSpectrogram::data() const
{
    return d_data->data;
}

/*!
  Change the color map

  Often it is useful to display the mapping between intensities and
  colors as an additional plot axis, showing a color bar.

  \param colorMap Color Map

  \sa colorMap(), QwtScaleWidget::setColorBarEnabled(),
      QwtScaleWidget::setColorMap()
*/
void QwtPolarSpectrogram::setColorMap( QwtColorMap *colorMap )
{
    if ( d_data->colorMap != colorMap )
    {
        delete d_data->colorMap;
        d_data->colorMap = colorMap;
    }

    itemChanged();
}

/*!
   \return Color Map used for mapping the intensity values to colors
   \sa setColorMap()
*/
const QwtColorMap *QwtPolarSpectrogram::colorMap() const
{
    return d_data->colorMap;
}

/*!
  Specify an attribute how to draw the curve

  \param attribute Paint attribute
  \param on On/Off
  \sa testPaintAttribute()
*/
void QwtPolarSpectrogram::setPaintAttribute( PaintAttribute attribute, bool on )
{
    if ( on )
        d_data->paintAttributes |= attribute;
    else
        d_data->paintAttributes &= ~attribute;
}

/*!
    \param attribute Paint attribute
    \return True, when attribute has been set
    \sa setPaintAttribute()
*/
bool QwtPolarSpectrogram::testPaintAttribute( PaintAttribute attribute ) const
{
    return ( d_data->paintAttributes & attribute );
}

/*!
   Rendering an image from the raster data can often be done
   parallel on a multicore system.

   \param numThreads Number of threads to be used for rendering.
                     If numThreads is set to 0, the system specific
                     ideal thread count is used.

   The default thread count is 1 ( = no additional threads )

   \warning Rendering in multiple threads is only supported for Qt >= 4.4
   \sa renderThreadCount(), renderImage(), renderTile()
*/
void QwtPolarSpectrogram::setRenderThreadCount( uint numThreads )
{
    d_data->renderThreadCount = numThreads;
}

/*!
   \return Number of threads to be used for rendering.
           If numThreads is set to 0, the system specific
           ideal thread count is used.

   \warning Rendering in multiple threads is only supported for Qt >= 4.4
   \sa setRenderThreadCount(), renderImage(), renderTile()
*/
uint QwtPolarSpectrogram::renderThreadCount() const
{
    return d_data->renderThreadCount;
}

/*!
  Draw the spectrogram

  \param painter Painter
  \param azimuthMap Maps azimuth values to values related to 0.0, M_2PI
  \param radialMap Maps radius values into painter coordinates.
  \param pole Position of the pole in painter coordinates
  \param radius Radius of the complete plot area in painter coordinates
  \param canvasRect Contents rect of the canvas in painter coordinates
*/
void QwtPolarSpectrogram::draw( QPainter *painter,
    const QwtScaleMap &azimuthMap, const QwtScaleMap &radialMap,
    const QPointF &pole, double,
    const QRectF &canvasRect ) const
{
    const QRectF plotRect = plot()->plotRect( canvasRect.toRect() );

    QRegion clipRegion( canvasRect.toRect() );
    if ( qwtNeedsClipping( plotRect, canvasRect ) )
    {
        // For large plotRects the ellipse becomes a huge polygon.
        // So we better clip only, when we really need to.

        clipRegion &= QRegion( plotRect.toRect(), QRegion::Ellipse );
    }

    QRect imageRect = canvasRect.toRect();

    if ( painter->hasClipping() )
        imageRect &= painter->clipRegion().boundingRect();

    const QwtInterval radialInterval =
        boundingInterval( QwtPolar::ScaleRadius );
    if ( radialInterval.isValid() )
    {
        const double radius = radialMap.transform( radialInterval.maxValue() ) -
                              radialMap.transform( radialInterval.minValue() );

        QRectF r( 0, 0, 2 * radius, 2 * radius );
        r.moveCenter( pole );

        clipRegion &= QRegion( r.toRect(), QRegion::Ellipse );;

        imageRect &= r.toRect();
    }

    const QImage image = renderImage( azimuthMap, radialMap, pole, imageRect );

    painter->save();
    painter->setClipRegion( clipRegion );

    painter->drawImage( imageRect, image );

    painter->restore();
}

/*!
   \brief Render an image from the data and color map.

   The area is translated into a rect of the paint device.
   For each pixel of this rect the intensity is mapped
   into a color.

  \param azimuthMap Maps azimuth values to values related to 0.0, M_2PI
  \param radialMap Maps radius values into painter coordinates.
  \param pole Position of the pole in painter coordinates
  \param rect Target rectangle of the image in painter coordinates

   \return A QImage::Format_Indexed8 or QImage::Format_ARGB32 depending
           on the color map.

   \sa QwtRasterData::intensity(), QwtColorMap::rgb(),
       QwtColorMap::colorIndex()
*/
QImage QwtPolarSpectrogram::renderImage(
    const QwtScaleMap &azimuthMap, const QwtScaleMap &radialMap,
    const QPointF &pole, const QRect &rect ) const
{
    if ( d_data->data == NULL || d_data->colorMap == NULL )
        return QImage();

    QImage image( rect.size(), d_data->colorMap->format() == QwtColorMap::RGB
                  ? QImage::Format_ARGB32 : QImage::Format_Indexed8 );

    const QwtInterval intensityRange = d_data->data->interval( Qt::ZAxis );
    if ( !intensityRange.isValid() )
        return image;

    if ( d_data->colorMap->format() == QwtColorMap::Indexed )
        image.setColorTable( d_data->colorMap->colorTable( intensityRange ) );

    /*
     For the moment we only announce the composition of the image by
     calling initRaster(), but we don't pass any useful parameters.
     ( How to map rect into something, that is useful to initialize a matrix
       of values in polar coordinates ? )
     */
    d_data->data->initRaster( QRectF(), QSize() );


#if QT_VERSION >= 0x040400 && !defined(QT_NO_QFUTURE)
    uint numThreads = d_data->renderThreadCount;

    if ( numThreads <= 0 )
        numThreads = QThread::idealThreadCount();

    if ( numThreads <= 0 )
        numThreads = 1;

    const int numRows = rect.height() / numThreads;


    QVector<TileInfo> tileInfos;
    for ( uint i = 0; i < numThreads; i++ )
    {
        QRect tile( rect.x(), rect.y() + i * numRows, rect.width(), numRows );
        if ( i == numThreads - 1 )
            tile.setHeight( rect.height() - i * numRows );

        TileInfo tileInfo;
        tileInfo.imagePos = rect.topLeft();
        tileInfo.rect = tile;
        tileInfo.image = &image;

        tileInfos += tileInfo;
    }

    QVector< QFuture<void> > futures;
    for ( int i = 0; i < tileInfos.size(); i++ )
    {
        if ( i == tileInfos.size() - 1 )
        {
            renderTile( azimuthMap, radialMap, pole, &tileInfos[i] );
        }
        else
        {
            futures += QtConcurrent::run( this, &QwtPolarSpectrogram::renderTile,
                azimuthMap, radialMap, pole, &tileInfos[i] );
        }
    }
    for ( int i = 0; i < futures.size(); i++ )
        futures[i].waitForFinished();

#else // QT_VERSION < 0x040400
    renderTile( azimuthMap, radialMap, pole, rect.topLeft(), rect, &image );
#endif

    d_data->data->discardRaster();

    return image;
}

void QwtPolarSpectrogram::renderTile(
    const QwtScaleMap &azimuthMap, const QwtScaleMap &radialMap,
    const QPointF &pole, TileInfo *tileInfo ) const
{
    renderTile( azimuthMap, radialMap, pole,
        tileInfo->imagePos, tileInfo->rect, tileInfo->image );
}

/*!
  \brief Render a sub-rectangle of an image 

  renderTile() is called by renderImage() to render different parts
  of the image by concurrent threads.

  \param azimuthMap Maps azimuth values to values related to 0.0, M_2PI
  \param radialMap Maps radius values into painter coordinates.
  \param pole Position of the pole in painter coordinates
  \param imagePos Top/left position of the image in painter coordinates
  \param tile Sub-rectangle of the tile in painter coordinates
  \param image Image to be rendered

   \sa setRenderThreadCount()
   \note renderTile needs to be reentrant
*/
void QwtPolarSpectrogram::renderTile(
    const QwtScaleMap &azimuthMap, const QwtScaleMap &radialMap,
    const QPointF &pole, const QPoint &imagePos,
    const QRect &tile, QImage *image ) const
{
    const QwtInterval intensityRange = d_data->data->interval( Qt::ZAxis );
    if ( !intensityRange.isValid() )
        return;

    const bool doFastAtan = testPaintAttribute( ApproximatedAtan );

    const int y0 = imagePos.y();
    const int y1 = tile.top();
    const int y2 = tile.bottom();

    const int x0 = imagePos.x();
    const int x1 = tile.left();
    const int x2 = tile.right();

    if ( d_data->colorMap->format() == QwtColorMap::RGB )
    {
        for ( int y = y1; y <= y2; y++ )
        {
            const double dy = pole.y() - y;
            const double dy2 = qwtSqr( dy );

            QRgb *line = reinterpret_cast<QRgb *>( image->scanLine( y - y0 ) );
            line += x1 - x0;

            for ( int x = x1; x <= x2; x++ )
            {
                const double dx = x - pole.x();

                double a =  doFastAtan ? qwtFastAtan2( dy, dx ) : qAtan2( dy, dx );
                if ( a < 0.0 )
                    a += 2 * M_PI;
                if ( a < azimuthMap.p1() )
                    a += 2 * M_PI;

                const double r = qSqrt( qwtSqr( dx ) + dy2 );

                const double azimuth = azimuthMap.invTransform( a );
                const double radius = radialMap.invTransform( r );

                const double value = d_data->data->value( azimuth, radius );
                *line++ = d_data->colorMap->rgb( intensityRange, value );
            }
        }
    }
    else if ( d_data->colorMap->format() == QwtColorMap::Indexed )
    {
        for ( int y = y1; y <= y2; y++ )
        {
            const double dy = pole.y() - y;
            const double dy2 = qwtSqr( dy );

            unsigned char *line = image->scanLine( y - y0 );
            line += x1 - x0;
            for ( int x = x1; x <= x2; x++ )
            {
                const double dx = x - pole.x();

                double a =  doFastAtan ? qwtFastAtan2( dy, dx ) : qAtan2( dy, dx );
                if ( a < 0.0 )
                    a += 2 * M_PI;
                if ( a < azimuthMap.p1() )
                    a += 2 * M_PI;

                const double r = qSqrt( qwtSqr( dx ) + dy2 );

                const double azimuth = azimuthMap.invTransform( a );
                const double radius = radialMap.invTransform( r );

                const double value = d_data->data->value( azimuth, radius );
                *line++ = d_data->colorMap->colorIndex( intensityRange, value );
            }
        }
    }
}

/*!
   Interval, that is necessary to display the item
   This interval can be useful for operations like clipping or autoscaling

   \param scaleId Scale index
   \return bounding interval ( == position )

   \sa position()
*/
QwtInterval QwtPolarSpectrogram::boundingInterval( int scaleId ) const
{
    if ( scaleId == QwtPolar::ScaleRadius )
        return d_data->data->interval( Qt::YAxis );

    return QwtPolarItem::boundingInterval( scaleId );
}
