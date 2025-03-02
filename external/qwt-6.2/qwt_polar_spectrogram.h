/******************************************************************************
 * QwtPolar Widget Library
 * Copyright (C) 2008   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_POLAR_SPECTROGRAM_H
#define QWT_POLAR_SPECTROGRAM_H

#include "qwt_global.h"
#include "qwt_polar_item.h"
#include <qimage.h>

class QwtRasterData;
class QwtColorMap;

/*!
   \brief An item, which displays a spectrogram

   A spectrogram displays 3-dimensional data, where the 3rd dimension
   ( the intensity ) is displayed using colors. The colors are calculated
   from the values using a color map.

   \sa QwtRasterData, QwtColorMap
 */
class QWT_EXPORT QwtPolarSpectrogram : public QwtPolarItem
{
  public:
    /*!
        Attributes to modify the drawing algorithm.
        The default setting disables ApproximatedAtan

        \sa setPaintAttribute(), testPaintAttribute()
     */
    enum PaintAttribute
    {
        /*!
           Use qwtFastAtan2 instead of atan2 for translating
           widget into polar coordinates.
         */

        ApproximatedAtan = 0x01
    };

    Q_DECLARE_FLAGS( PaintAttributes, PaintAttribute )

    explicit QwtPolarSpectrogram();
    virtual ~QwtPolarSpectrogram();

    void setData( QwtRasterData* data );
    const QwtRasterData* data() const;

    void setColorMap( QwtColorMap* );
    const QwtColorMap* colorMap() const;

    void setPaintAttribute( PaintAttribute, bool on = true );
    bool testPaintAttribute( PaintAttribute ) const;

    virtual int rtti() const QWT_OVERRIDE;

    virtual void draw( QPainter* painter,
        const QwtScaleMap& azimuthMap, const QwtScaleMap& radialMap,
        const QPointF& pole, double radius,
        const QRectF& canvasRect ) const QWT_OVERRIDE;

    virtual QwtInterval boundingInterval( int scaleId ) const QWT_OVERRIDE;

  protected:
    virtual QImage renderImage(
        const QwtScaleMap& azimuthMap, const QwtScaleMap& radialMap,
        const QPointF& pole, const QRect& rect ) const;

    virtual void renderTile(
        const QwtScaleMap& azimuthMap, const QwtScaleMap& radialMap,
        const QPointF& pole, const QPoint& imagePos,
        const QRect& tile, QImage* image ) const;

  private:
    class TileInfo;
    void renderTileInfo( const QwtScaleMap&, const QwtScaleMap&,
        const QPointF& pole, TileInfo* ) const;

    class PrivateData;
    PrivateData* m_data;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QwtPolarSpectrogram::PaintAttributes )

#endif
