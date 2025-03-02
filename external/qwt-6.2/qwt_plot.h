/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_PLOT_H
#define QWT_PLOT_H

#include "qwt_global.h"
#include "qwt_axis_id.h"
#include "qwt_plot_dict.h"

#include <qframe.h>

class QwtPlotLayout;
class QwtAbstractLegend;
class QwtScaleWidget;
class QwtScaleEngine;
class QwtScaleDiv;
class QwtScaleMap;
class QwtScaleDraw;
class QwtTextLabel;
class QwtInterval;
class QwtText;
template< typename T > class QList;

// 6.1 compatibility definitions
#define QWT_AXIS_COMPAT 1

/*!
   \brief A 2-D plotting widget

   QwtPlot is a widget for plotting two-dimensional graphs.
   An unlimited number of plot items can be displayed on
   its canvas. Plot items might be curves (QwtPlotCurve), markers
   (QwtPlotMarker), the grid (QwtPlotGrid), or anything else derived
   from QwtPlotItem.
   A plot can have up to four axes, with each plot item attached to an x- and
   a y axis. The scales at the axes can be explicitly set (QwtScaleDiv), or
   are calculated from the plot items, using algorithms (QwtScaleEngine) which
   can be configured separately for each axis.

   The simpleplot example is a good starting point to see how to set up a
   plot widget.

   \image html plot.png

   \par Example
    The following example shows (schematically) the most simple
    way to use QwtPlot. By default, only the left and bottom axes are
    visible and their scales are computed automatically.
    \code
 #include <qwt_plot.h>
 #include <qwt_plot_curve.h>

      QwtPlot *myPlot = new QwtPlot( "Two Curves", parent );

      // add curves
      QwtPlotCurve *curve1 = new QwtPlotCurve( "Curve 1" );
      QwtPlotCurve *curve2 = new QwtPlotCurve( "Curve 2" );

      // connect or copy the data to the curves
      curve1->setData( ... );
      curve2->setData( ... );

      curve1->attach( myPlot );
      curve2->attach( myPlot );

      // finally, refresh the plot
      myPlot->replot();
    \endcode
 */

class QWT_EXPORT QwtPlot : public QFrame, public QwtPlotDict
{
    Q_OBJECT

    Q_PROPERTY( QBrush canvasBackground
        READ canvasBackground WRITE setCanvasBackground )

    Q_PROPERTY( bool autoReplot READ autoReplot WRITE setAutoReplot )

  public:
    /*!
        Position of the legend, relative to the canvas.

        \sa insertLegend()
     */
    enum LegendPosition
    {
        //! The legend will be left from the QwtAxis::YLeft axis.
        LeftLegend,

        //! The legend will be right from the QwtAxis::YRight axis.
        RightLegend,

        //! The legend will be below the footer
        BottomLegend,

        //! The legend will be above the title
        TopLegend
    };

    explicit QwtPlot( QWidget* = NULL );
    explicit QwtPlot( const QwtText& title, QWidget* = NULL );

    virtual ~QwtPlot();

    void setAutoReplot( bool = true );
    bool autoReplot() const;

    // Layout

    void setPlotLayout( QwtPlotLayout* );

    QwtPlotLayout* plotLayout();
    const QwtPlotLayout* plotLayout() const;

    // Title

    void setTitle( const QString& );
    void setTitle( const QwtText& );
    QwtText title() const;

    QwtTextLabel* titleLabel();
    const QwtTextLabel* titleLabel() const;

    // Footer

    void setFooter( const QString& );
    void setFooter( const QwtText& );
    QwtText footer() const;

    QwtTextLabel* footerLabel();
    const QwtTextLabel* footerLabel() const;

    // Canvas

    void setCanvas( QWidget* );

    QWidget* canvas();
    const QWidget* canvas() const;

    void setCanvasBackground( const QBrush& );
    QBrush canvasBackground() const;

    virtual QwtScaleMap canvasMap( QwtAxisId ) const;

    double invTransform( QwtAxisId, double pos ) const;
    double transform( QwtAxisId, double value ) const;

    // Axes

    bool isAxisValid( QwtAxisId ) const;

    void setAxisVisible( QwtAxisId, bool on = true );
    bool isAxisVisible( QwtAxisId ) const;

    // Axes data

    QwtScaleEngine* axisScaleEngine( QwtAxisId );
    const QwtScaleEngine* axisScaleEngine( QwtAxisId ) const;
    void setAxisScaleEngine( QwtAxisId, QwtScaleEngine* );

    void setAxisAutoScale( QwtAxisId, bool on = true );
    bool axisAutoScale( QwtAxisId ) const;

    void setAxisFont( QwtAxisId, const QFont& );
    QFont axisFont( QwtAxisId ) const;

    void setAxisScale( QwtAxisId, double min, double max, double stepSize = 0 );
    void setAxisScaleDiv( QwtAxisId, const QwtScaleDiv& );
    void setAxisScaleDraw( QwtAxisId, QwtScaleDraw* );

    double axisStepSize( QwtAxisId ) const;
    QwtInterval axisInterval( QwtAxisId ) const;
    const QwtScaleDiv& axisScaleDiv( QwtAxisId ) const;

    const QwtScaleDraw* axisScaleDraw( QwtAxisId ) const;
    QwtScaleDraw* axisScaleDraw( QwtAxisId );

    const QwtScaleWidget* axisWidget( QwtAxisId ) const;
    QwtScaleWidget* axisWidget( QwtAxisId );

    void setAxisLabelAlignment( QwtAxisId, Qt::Alignment );
    void setAxisLabelRotation( QwtAxisId, double rotation );

    void setAxisTitle( QwtAxisId, const QString& );
    void setAxisTitle( QwtAxisId, const QwtText& );
    QwtText axisTitle( QwtAxisId ) const;

    void setAxisMaxMinor( QwtAxisId, int maxMinor );
    int axisMaxMinor( QwtAxisId ) const;

    void setAxisMaxMajor( QwtAxisId, int maxMajor );
    int axisMaxMajor( QwtAxisId ) const;

    // Legend

    void insertLegend( QwtAbstractLegend*,
        LegendPosition = QwtPlot::RightLegend, double ratio = -1.0 );

    QwtAbstractLegend* legend();
    const QwtAbstractLegend* legend() const;

    void updateLegend();
    void updateLegend( const QwtPlotItem* );

    // Misc

    virtual QSize sizeHint() const QWT_OVERRIDE;
    virtual QSize minimumSizeHint() const QWT_OVERRIDE;

    virtual void updateLayout();
    virtual void drawCanvas( QPainter* );

    void updateAxes();
    void updateCanvasMargins();

    virtual void getCanvasMarginsHint(
        const QwtScaleMap maps[], const QRectF& canvasRect,
        double& left, double& top, double& right, double& bottom) const;

    virtual bool event( QEvent* ) QWT_OVERRIDE;
    virtual bool eventFilter( QObject*, QEvent* ) QWT_OVERRIDE;

    virtual void drawItems( QPainter*, const QRectF&,
        const QwtScaleMap maps[ QwtAxis::AxisPositions ] ) const;

    virtual QVariant itemToInfo( QwtPlotItem* ) const;
    virtual QwtPlotItem* infoToItem( const QVariant& ) const;

#if QWT_AXIS_COMPAT
    enum Axis
    {
        yLeft   = QwtAxis::YLeft,
        yRight  = QwtAxis::YRight,
        xBottom = QwtAxis::XBottom,
        xTop    = QwtAxis::XTop,

        axisCnt = QwtAxis::AxisPositions
    };

    void enableAxis( int axisId, bool on = true )
    {
        setAxisVisible( axisId, on );
    }

    bool axisEnabled( int axisId ) const
    {
        return isAxisVisible( axisId );
    }
#endif

  Q_SIGNALS:
    /*!
       A signal indicating, that an item has been attached/detached

       \param plotItem Plot item
       \param on Attached/Detached
     */
    void itemAttached( QwtPlotItem* plotItem, bool on );

    /*!
       A signal with the attributes how to update
       the legend entries for a plot item.

       \param itemInfo Info about a plot item, build from itemToInfo()
       \param data Attributes of the entries ( usually <= 1 ) for
                  the plot item.

       \sa itemToInfo(), infoToItem(), QwtAbstractLegend::updateLegend()
     */
    void legendDataChanged( const QVariant& itemInfo,
        const QList< QwtLegendData >& data );

  public Q_SLOTS:
    virtual void replot();
    void autoRefresh();

  protected:

    virtual void resizeEvent( QResizeEvent* ) QWT_OVERRIDE;

  private Q_SLOTS:
    void updateLegendItems( const QVariant& itemInfo,
        const QList< QwtLegendData >& legendData );

  private:
    friend class QwtPlotItem;
    void attachItem( QwtPlotItem*, bool );

    void initAxesData();
    void deleteAxesData();
    void updateScaleDiv();

    void initPlot( const QwtText& title );

    class ScaleData;
    ScaleData* m_scaleData;

    class PrivateData;
    PrivateData* m_data;
};

#endif
