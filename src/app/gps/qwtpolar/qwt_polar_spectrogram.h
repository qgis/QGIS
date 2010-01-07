/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * QwtPolar Widget Library
 * Copyright (C) 2008   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_POLAR_SPECTROGRAM_H
#define QWT_POLAR_SPECTROGRAM_H

#include "qwt_polar_global.h"
#include "qwt_polar_item.h"
#include <qimage.h>

class QwtRasterData;
class QwtColorMap;

/*!
  \brief An item, which displays a spectrogram

  A spectrogram displays threedimenional data, where the 3rd dimension
  ( the intensity ) is displayed using colors. The colors are calculated
  from the values using a color map.

  \sa QwtRasterData, QwtColorMap
*/
class QWT_POLAR_EXPORT QwtPolarSpectrogram: public QwtPolarItem
{
public:
    explicit QwtPolarSpectrogram();
    virtual ~QwtPolarSpectrogram();

    void setData(const QwtRasterData &data);
    const QwtRasterData &data() const;

    void setColorMap(const QwtColorMap &);
    const QwtColorMap &colorMap() const;

    virtual int rtti() const;

    virtual void draw(QPainter *painter,
        const QwtScaleMap &azimuthMap, const QwtScaleMap &radialMap,
        const QwtDoublePoint &pole, double radius,
        const QwtDoubleRect &canvasRect) const;

    virtual QwtDoubleInterval boundingInterval(int scaleId) const;

protected:
    virtual QImage renderImage(
        const QwtScaleMap &azimuthMap, const QwtScaleMap &radialMap, 
        const QwtDoublePoint &pole, const QRect &rect) const;

private:
    class PrivateData;
    PrivateData *d_data;
};

#endif
