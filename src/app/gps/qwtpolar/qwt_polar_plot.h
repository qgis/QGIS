/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * QwtPolar Widget Library
 * Copyright (C) 2008   Uwe Rathmann
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_POLAR_PLOT_H
#define QWT_POLAR_PLOT_H 1

#include <qframe.h>
#include "qwt_polar_global.h"
#include "qwt_double_interval.h"
#include "qwt_scale_map.h"
#include "qwt_polar.h"
#include "qwt_polar_point.h"
#include "qwt_polar_itemdict.h"

class QwtRoundScaleDraw;
class QwtScaleEngine;
class QwtScaleDiv;
class QwtTextLabel;
class QwtPolarCanvas;
class QwtPolarLayout;

/*!
  \brief A plotting widget, displaying a polar coordinate system

  An unlimited number of plot items can be displayed on
  its canvas. Plot items might be curves (QwtPolarCurve), markers
  (QwtPolarMarker), the grid (QwtPolarGrid), or anything else derived
  from QwtPolarItem.

  The coordinate system is defined by a radial and a azimuth scale.
  The scales at the axes can be explicitely set (QwtScaleDiv), or
  are calculated from the plot items, using algorithms (QwtScaleEngine) which
  can be configured separately for each axis. Autoscaling is supported 
  for the radial scale.

  In opposite to QwtPlot the scales might be different from the
  view, that is displayed on the canvas. The view can be changed by 
  zooming - f.e. by using QwtPolarPanner or QwtPolarMaginfier.
*/
class QWT_POLAR_EXPORT QwtPolarPlot: public QFrame, public QwtPolarItemDict
{
    Q_OBJECT

    Q_PROPERTY(QBrush plotBackground READ plotBackground WRITE setPlotBackground)
    Q_PROPERTY(double azimuthOrigin READ azimuthOrigin WRITE setAzimuthOrigin)


public:
    /*!
        Position of the legend, relative to the canvas.

        - LeftLegend\n
          The legend will be left from the canvas.
        - RightLegend\n
          The legend will be right from the canvas.
        - BottomLegend\n
          The legend will be below the canvas.
        - TopLegend\n
          The legend will be between canvas and title.
        - ExternalLegend\n
          External means that only the content of the legend
          will be handled by QwtPlot, but not its geometry.
          This might be interesting if an application wants to
          have a legend in an external window ( or on the canvas ).

        \note In case of ExternalLegend, the legend is not
              painted by renderTo().

        \sa insertLegend()
     */
    enum LegendPosition
    {
        LeftLegend,
        RightLegend,
        BottomLegend,
        TopLegend,

        ExternalLegend
    };

    explicit QwtPolarPlot( QWidget *parent = NULL);
    QwtPolarPlot(const QwtText &title, QWidget *parent = NULL);

    virtual ~QwtPolarPlot();

    void setTitle(const QString &);
    void setTitle(const QwtText &);

    QwtText title() const;

    QwtTextLabel *titleLabel();
    const QwtTextLabel *titleLabel() const;

    void setAutoReplot(bool tf = true); 
    bool autoReplot() const;

    void setAutoScale(int scaleId);
    bool hasAutoScale(int scaleId) const;

    void setScaleMaxMinor(int scaleId, int maxMinor);
    int scaleMaxMinor(int scaleId) const;

    int scaleMaxMajor(int scaleId) const;
    void setScaleMaxMajor(int scaleId, int maxMajor);

    QwtScaleEngine *scaleEngine(int scaleId);
    const QwtScaleEngine *scaleEngine(int scaleId) const;
    void setScaleEngine(int scaleId, QwtScaleEngine *);

    void setScale(int scaleId, double min, double max, double step = 0);

    void setScaleDiv(int scaleId, const QwtScaleDiv &);
    const QwtScaleDiv *scaleDiv(int scaleId) const;
    QwtScaleDiv *scaleDiv(int scaleId);

    QwtScaleMap scaleMap(int scaleId, double radius) const;
    QwtScaleMap scaleMap(int scaleId) const;

    void updateScale(int scaleId);

    double azimuthOrigin() const;

    void zoom(const QwtPolarPoint&, double factor);
    void unzoom();

    QwtPolarPoint zoomPos() const;
    double zoomFactor() const;

    virtual void polish();

    // Canvas

    QwtPolarCanvas *canvas();
    const QwtPolarCanvas *canvas() const;

    void setPlotBackground (const QBrush &c);
    const QBrush& plotBackground() const;

    virtual void drawCanvas(QPainter *, const QwtDoubleRect &) const;


    // Legend

    void insertLegend(QwtLegend *, LegendPosition = QwtPolarPlot::RightLegend,
        double ratio = -1.0);

    QwtLegend *legend();
    const QwtLegend *legend() const;

    // Layout
    QwtPolarLayout *plotLayout();
    const QwtPolarLayout *plotLayout() const;

    QwtDoubleInterval visibleInterval() const;
    QwtDoubleRect plotRect() const;
    QwtDoubleRect plotRect(const QRect &) const;

    int plotMarginHint() const;

    void renderTo(QPaintDevice &) const;
    virtual void renderTo(QPainter *, const QRect &) const;

signals:
    /*!
      A signal which is emitted when the user has clicked on
      a legend item, which is in QwtLegend::ClickableItem mode.

      \param plotItem Corresponding plot item of the
                 selected legend item

      \note clicks are disabled as default
      \sa QwtLegend::setItemMode, QwtLegend::itemMode
     */
    void legendClicked(QwtPolarItem *plotItem);

    /*!
      A signal which is emitted when the user has clicked on
      a legend item, which is in QwtLegend::CheckableItem mode

      \param plotItem Corresponding plot item of the
                 selected legend item
      \param on True when the legen item is checked

      \note clicks are disabled as default
      \sa QwtLegend::setItemMode, QwtLegend::itemMode
     */
    void legendChecked(QwtPolarItem *plotItem, bool on);

    /*!
      A signal that is emitted, whenever the layout of the plot
      has been recalculated.
     */
    void layoutChanged();

public slots:
    virtual void replot();
    void autoRefresh();
    void setAzimuthOrigin(double);

protected slots:
    virtual void legendItemClicked();
    virtual void legendItemChecked(bool);

protected:
    virtual bool event(QEvent *);
    virtual void resizeEvent(QResizeEvent *);

    virtual void updateLayout();

    virtual void drawItems(QPainter *painter, 
        const QwtScaleMap &radialMap, const QwtScaleMap &azimuthMap,
        const QwtDoublePoint &pole, double radius,
        const QwtDoubleRect &canvasRect) const;

    virtual void renderTitle(QPainter *, const QRect &) const;
    virtual void renderLegend(QPainter *, const QRect &) const;
    virtual void renderLegendItem(QPainter *,
        const QWidget *, const QRect &) const;

private:
    void initPlot(const QwtText &);

    class ScaleData;
    class PrivateData;
    PrivateData *d_data;
};

#endif
