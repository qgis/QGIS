/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_PLOT_CURVE_3D_H
#define QWT_PLOT_CURVE_3D_H

#include "qwt_global.h"
#include "qwt_plot_seriesitem.h"

class QwtColorMap;

/*!
    \brief Curve that displays 3D points as dots, where the z coordinate is
           mapped to a color.
 */
class QWT_EXPORT QwtPlotSpectroCurve
    : public QwtPlotSeriesItem
    , public QwtSeriesStore< QwtPoint3D >
{
  public:
    //! Paint attributes
    enum PaintAttribute
    {
        //! Clip points outside the canvas rectangle
        ClipPoints = 1
    };

    Q_DECLARE_FLAGS( PaintAttributes, PaintAttribute )

    explicit QwtPlotSpectroCurve( const QString& title = QString() );
    explicit QwtPlotSpectroCurve( const QwtText& title );

    virtual ~QwtPlotSpectroCurve();

    virtual int rtti() const QWT_OVERRIDE;

    void setPaintAttribute( PaintAttribute, bool on = true );
    bool testPaintAttribute( PaintAttribute ) const;

    void setSamples( const QVector< QwtPoint3D >& );
    void setSamples( QwtSeriesData< QwtPoint3D >* );


    void setColorMap( QwtColorMap* );
    const QwtColorMap* colorMap() const;

    void setColorRange( const QwtInterval& );
    QwtInterval& colorRange() const;

    virtual void drawSeries( QPainter*,
        const QwtScaleMap& xMap, const QwtScaleMap& yMap,
        const QRectF& canvasRect, int from, int to ) const QWT_OVERRIDE;

    void setPenWidth( double );
    double penWidth() const;

  protected:
    virtual void drawDots( QPainter*,
        const QwtScaleMap& xMap, const QwtScaleMap& yMap,
        const QRectF& canvasRect, int from, int to ) const;

  private:
    void init();

    class PrivateData;
    PrivateData* m_data;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QwtPlotSpectroCurve::PaintAttributes )

#endif
