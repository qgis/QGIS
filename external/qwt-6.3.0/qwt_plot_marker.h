/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_PLOT_MARKER_H
#define QWT_PLOT_MARKER_H

#include "qwt_global.h"
#include "qwt_plot_item.h"

class QString;
class QRectF;
class QwtText;
class QwtSymbol;

/*!
   \brief A class for drawing markers

   A marker can be a horizontal line, a vertical line,
   a symbol, a label or any combination of them, which can
   be drawn around a center point inside a bounding rectangle.

   The setSymbol() member assigns a symbol to the marker.
   The symbol is drawn at the specified point.

   With setLabel(), a label can be assigned to the marker.
   The setLabelAlignment() member specifies where the label is
   drawn. All the Align*-constants in Qt::AlignmentFlags (see Qt documentation)
   are valid. The interpretation of the alignment depends on the marker's
   line style. The alignment refers to the center point of
   the marker, which means, for example, that the label would be printed
   left above the center point if the alignment was set to
   Qt::AlignLeft | Qt::AlignTop.

   \note QwtPlotTextLabel is intended to align a text label
        according to the geometry of canvas
        ( unrelated to plot coordinates )
 */

class QWT_EXPORT QwtPlotMarker : public QwtPlotItem
{
  public:

    /*!
        Line styles.
        \sa setLineStyle(), lineStyle()
     */
    enum LineStyle
    {
        //! No line
        NoLine,

        //! A horizontal line
        HLine,

        //! A vertical line
        VLine,

        //! A crosshair
        Cross
    };

    explicit QwtPlotMarker();
    explicit QwtPlotMarker( const QString& title );
    explicit QwtPlotMarker( const QwtText& title );

    virtual ~QwtPlotMarker();

    virtual int rtti() const QWT_OVERRIDE;

    double xValue() const;
    double yValue() const;
    QPointF value() const;

    void setXValue( double );
    void setYValue( double );
    void setValue( double, double );
    void setValue( const QPointF& );

    void setLineStyle( LineStyle );
    LineStyle lineStyle() const;

    void setLinePen( const QColor&, qreal width = 0.0, Qt::PenStyle = Qt::SolidLine );
    void setLinePen( const QPen& );
    const QPen& linePen() const;

    void setSymbol( const QwtSymbol* );
    const QwtSymbol* symbol() const;

    void setLabel( const QwtText& );
    QwtText label() const;

    void setLabelAlignment( Qt::Alignment );
    Qt::Alignment labelAlignment() const;

    void setLabelOrientation( Qt::Orientation );
    Qt::Orientation labelOrientation() const;

    void setSpacing( int );
    int spacing() const;

    virtual void draw( QPainter*,
        const QwtScaleMap& xMap, const QwtScaleMap& yMap,
        const QRectF& ) const QWT_OVERRIDE;

    virtual QRectF boundingRect() const QWT_OVERRIDE;

    virtual QwtGraphic legendIcon(
        int index, const QSizeF& ) const QWT_OVERRIDE;

  protected:
    virtual void drawLines( QPainter*,
        const QRectF&, const QPointF& ) const;

    virtual void drawSymbol( QPainter*,
        const QRectF&, const QPointF& ) const;

    virtual void drawLabel( QPainter*,
        const QRectF&, const QPointF& ) const;

  private:

    class PrivateData;
    PrivateData* m_data;
};

#endif
