/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_PLOT_SPECTROGRAM_H
#define QWT_PLOT_SPECTROGRAM_H

#include "qwt_global.h"
#include "qwt_raster_data.h"
#include "qwt_plot_rasteritem.h"

class QwtColorMap;
template< typename T > class QList;

/*!
   \brief A plot item, which displays a spectrogram

   A spectrogram displays 3-dimensional data, where the 3rd dimension
   ( the intensity ) is displayed using colors. The colors are calculated
   from the values using a color map.

   On multi-core systems the performance of the image composition
   can often be improved by dividing the area into tiles - each of them
   rendered in a different thread ( see QwtPlotItem::setRenderThreadCount() ).

   In ContourMode contour lines are painted for the contour levels.

   \sa QwtRasterData, QwtColorMap, QwtPlotItem::setRenderThreadCount()
 */

class QWT_EXPORT QwtPlotSpectrogram : public QwtPlotRasterItem
{
  public:
    /*!
       The display mode controls how the raster data will be represented.
       \sa setDisplayMode(), testDisplayMode()
     */

    enum DisplayMode
    {
        //! The values are mapped to colors using a color map.
        ImageMode = 0x01,

        //! The data is displayed using contour lines
        ContourMode = 0x02
    };

    Q_DECLARE_FLAGS( DisplayModes, DisplayMode )

    explicit QwtPlotSpectrogram( const QString& title = QString() );
    virtual ~QwtPlotSpectrogram();

    void setDisplayMode( DisplayMode, bool on = true );
    bool testDisplayMode( DisplayMode ) const;

    void setData( QwtRasterData* data );
    const QwtRasterData* data() const;
    QwtRasterData* data();

    void setColorMap( QwtColorMap* );
    const QwtColorMap* colorMap() const;

    void setColorTableSize( int numColors );
    int colorTableSize() const;

    virtual QwtInterval interval( Qt::Axis ) const QWT_OVERRIDE;
    virtual QRectF pixelHint( const QRectF& ) const QWT_OVERRIDE;

    void setDefaultContourPen( const QColor&,
        qreal width = 0.0, Qt::PenStyle = Qt::SolidLine );
    void setDefaultContourPen( const QPen& );
    QPen defaultContourPen() const;

    virtual QPen contourPen( double level ) const;

    void setConrecFlag( QwtRasterData::ConrecFlag, bool on );
    bool testConrecFlag( QwtRasterData::ConrecFlag ) const;

    void setContourLevels( const QList< double >& );
    QList< double > contourLevels() const;

    virtual int rtti() const QWT_OVERRIDE;

    virtual void draw( QPainter*,
        const QwtScaleMap& xMap, const QwtScaleMap& yMap,
        const QRectF& canvasRect ) const QWT_OVERRIDE;

  protected:
    virtual QImage renderImage(
        const QwtScaleMap& xMap, const QwtScaleMap& yMap,
        const QRectF& area, const QSize& imageSize ) const QWT_OVERRIDE;

    virtual QSize contourRasterSize(
        const QRectF&, const QRect& ) const;

    virtual QwtRasterData::ContourLines renderContourLines(
        const QRectF& rect, const QSize& raster ) const;

    virtual void drawContourLines( QPainter*,
        const QwtScaleMap& xMap, const QwtScaleMap& yMap,
        const QwtRasterData::ContourLines& ) const;

    void renderTile( const QwtScaleMap& xMap, const QwtScaleMap& yMap,
        const QRect& tile, QImage* ) const;

  private:
    class PrivateData;
    PrivateData* m_data;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QwtPlotSpectrogram::DisplayModes )

#endif
