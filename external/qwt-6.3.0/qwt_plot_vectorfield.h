/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_PLOT_VECTOR_FIELD_H
#define QWT_PLOT_VECTOR_FIELD_H

#include "qwt_global.h"
#include "qwt_plot_seriesitem.h"

class QwtVectorFieldSymbol;
class QwtColorMap;
class QPen;
class QBrush;

/*!
    \brief A plot item, that represents a vector field

    A vector field is a representation of a points with a given magnitude and direction
    as arrows. While the direction affects the direction of the arrow, the magnitude
    might be represented as a color or by the length of the arrow.

    \sa QwtVectorFieldSymbol, QwtVectorFieldSample
 */
class QWT_EXPORT QwtPlotVectorField
    : public QwtPlotSeriesItem
    , public QwtSeriesStore< QwtVectorFieldSample >
{
  public:
    /*!
        Depending on the origin the indicator symbol ( usually an arrow )
        will be to the position of the corresponding sample.
     */
    enum IndicatorOrigin
    {
        //! symbol points to the sample position
        OriginHead,

        //! The arrow starts at the sample position
        OriginTail,

        //! The arrow is centered at the sample position
        OriginCenter
    };

    /*!
        Attributes to modify the rendering
        \sa setPaintAttribute(), testPaintAttribute()
     */
    enum PaintAttribute
    {
        /*
            FilterVectors calculates an average sample from all samples
            that lie in the same cell of a grid that is determined by
            setting the rasterSize().

            \sa setRasterSize()
         */
        FilterVectors        = 0x01
    };

    Q_DECLARE_FLAGS( PaintAttributes, PaintAttribute )

    /*!
        Depending on the MagnitudeMode the magnitude component will have
        an impact on the attributes of the symbol/arrow.

        \sa setMagnitudeMode()
     */
    enum MagnitudeMode
    {
        /*!
           The magnitude will be mapped to a color using a color map
           \sa magnitudeRange(), colorMap()
         */
        MagnitudeAsColor = 0x01,

        /*!
           The magnitude will have an impact on the length of the arrow/symbol
           \sa arrowLength(), magnitudeScaleFactor()
         */
        MagnitudeAsLength = 0x02
    };

    Q_DECLARE_FLAGS( MagnitudeModes, MagnitudeMode )

    explicit QwtPlotVectorField( const QString& title = QString() );
    explicit QwtPlotVectorField( const QwtText& title );

    virtual ~QwtPlotVectorField();

    void setPaintAttribute( PaintAttribute, bool on = true );
    bool testPaintAttribute( PaintAttribute ) const;

    void setMagnitudeMode( MagnitudeMode, bool on = true );
    bool testMagnitudeMode( MagnitudeMode ) const;

    void setSymbol( QwtVectorFieldSymbol* );
    const QwtVectorFieldSymbol* symbol() const;

    void setPen( const QPen& );
    QPen pen() const;

    void setBrush( const QBrush& );
    QBrush brush() const;

    void setRasterSize( const QSizeF& );
    QSizeF rasterSize() const;

    void setIndicatorOrigin( IndicatorOrigin );
    IndicatorOrigin indicatorOrigin() const;

    void setSamples( const QVector< QwtVectorFieldSample >& );
    void setSamples( QwtVectorFieldData* );

    void setColorMap( QwtColorMap* );
    const QwtColorMap* colorMap() const;

    void setMagnitudeRange( const QwtInterval& );
    QwtInterval magnitudeRange() const;

    void setMinArrowLength( double );
    double minArrowLength() const;

    void setMaxArrowLength( double );
    double maxArrowLength() const;

    virtual double arrowLength( double magnitude ) const;

    virtual QRectF boundingRect() const QWT_OVERRIDE;

    virtual void drawSeries( QPainter*,
        const QwtScaleMap& xMap, const QwtScaleMap& yMap,
        const QRectF& canvasRect, int from, int to ) const QWT_OVERRIDE;

    virtual int rtti() const QWT_OVERRIDE;

    virtual QwtGraphic legendIcon(
        int index, const QSizeF& ) const QWT_OVERRIDE;

    void setMagnitudeScaleFactor( double factor );
    double magnitudeScaleFactor() const;

  protected:
    virtual void drawSymbols( QPainter*,
        const QwtScaleMap& xMap, const QwtScaleMap& yMap,
        const QRectF& canvasRect, int from, int to ) const;

    virtual void drawSymbol( QPainter*,
        double x, double y, double vx, double vy ) const;

    virtual void dataChanged() QWT_OVERRIDE;

  private:
    void init();

    class PrivateData;
    PrivateData* m_data;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QwtPlotVectorField::PaintAttributes )
Q_DECLARE_OPERATORS_FOR_FLAGS( QwtPlotVectorField::MagnitudeModes )

#endif
