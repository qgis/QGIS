/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * QwtPolar Widget Library
 * Copyright (C) 2008   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_POLAR_GRID_H
#define QWT_POLAR_GRID_H

#include "qwt_polar_global.h"
#include "qwt_polar.h"
#include "qwt_polar_item.h"
#include "qwt_polar_plot.h"
#include "qwt_scale_div.h"

class QPainter;
class QPen;
class QwtScaleMap;
class QwtScaleDiv;
class QwtRoundScaleDraw;
class QwtScaleDraw;

/*!
  \brief An item which draws scales and grid lines on a polar plot.

  The QwtPolarGrid class can be used to draw a coordinate grid.
  A coordinate grid consists of major and minor gridlines.
  The locations of the gridlines are determined by the azimuth and radial
  scale divisions.

  QwtPolarGrid is also responsible for drawing the axis representing the
  scales. It is possible to display 4 radial and one azimuth axis.

  Whenever the scale divisions of the plot widget changes the grid
  is synchronized by updateScaleDiv().

  \sa QwtPolarPlot, QwtPolar::Axis
*/

class QWT_POLAR_EXPORT QwtPolarGrid: public QwtPolarItem
{
  public:
    /*!
       Mysterious flags trying to avoid conflicts, when painting the
       scales and grid lines.

      - SmartOriginLabel\n
        Try to avoid situations, where the label of the origin is
        painted over another axis.
      - HideMaxRadiusLabel\n
        Often the outermost tick of the radial scale is close to the
        canvas border. With HideMaxRadiusLabel enabled it is not painted.
      - ClipAxisBackground\n
        The tick labels of the radial scales might be hard to read, when
        they are painted on top of the radial grid lines ( or on top
        of a curve/spectrogram ). When ClipAxisBackground the bounding rect
        of each label is added to the clip region.
      - SmartScaleDraw\n
        Don't paint the backbone of the radial axes, when they are very close
        to a line of the azimuth grid.
      - ClipGridLines\n
        All grid lines are clipped against the plot area before being painted.
        When the plot is zoomed in this will have an significant impact
        on the performance of the painting cde.

      The default setting enables all flags.
     */
    enum DisplayFlag
    {
      SmartOriginLabel = 1,
      HideMaxRadiusLabel = 2,
      ClipAxisBackground = 4,
      SmartScaleDraw = 8,
      ClipGridLines = 16
    };

    /*!
      \brief Grid attributes

      - AutoScaling\n
        When AutoScaling is enabled, the radial axes will be adjusted
        to the interval, that is currently visible on the canvas plot.
     */
    enum GridAttribute
    {
      AutoScaling = 1
    };

    explicit QwtPolarGrid();
    virtual ~QwtPolarGrid();

    virtual int rtti() const;

    void setDisplayFlag( DisplayFlag, bool on = true );
    bool testDisplayFlag( DisplayFlag ) const;

    void setGridAttribute( GridAttribute, bool on = true );
    bool testGridAttribute( GridAttribute ) const;

    void showGrid( int scaleId, bool show = true );
    bool isGridVisible( int scaleId ) const;

    void showMinorGrid( int scaleId, bool show = true );
    bool isMinorGridVisible( int scaleId ) const;

    void showAxis( int axisId, bool show = true );
    bool isAxisVisible( int axisId ) const;

    void setPen( const QPen &p );
    void setFont( const QFont & );

    void setMajorGridPen( const QPen &p );
    void setMajorGridPen( int scaleId, const QPen &p );
    QPen majorGridPen( int scaleId ) const;

    void setMinorGridPen( const QPen &p );
    void setMinorGridPen( int scaleId, const QPen &p );
    QPen minorGridPen( int scaleId ) const;

    void setAxisPen( int axisId, const QPen &p );
    QPen axisPen( int axisId ) const;

    void setAxisFont( int axisId, const QFont &p );
    QFont axisFont( int axisId ) const;

    void setScaleDraw( int axisId, QwtScaleDraw * );
    const QwtScaleDraw *scaleDraw( int axisId ) const;
    QwtScaleDraw *scaleDraw( int axisId );

    void setAzimuthScaleDraw( QwtRoundScaleDraw * );
    const QwtRoundScaleDraw *azimuthScaleDraw() const;
    QwtRoundScaleDraw *azimuthScaleDraw();

    virtual void draw( QPainter *p,
                       const QwtScaleMap &azimuthMap, const QwtScaleMap &radialMap,
                       const QwtDoublePoint &pole, double radius,
                       const QwtDoubleRect &rect ) const;

    virtual void updateScaleDiv( const QwtScaleDiv &azimuthMap,
                                 const QwtScaleDiv &radialMap, const QwtDoubleInterval & );

    virtual int marginHint() const;

  protected:
    void drawRays( QPainter *, const QwtDoubleRect &,
                   const QwtDoublePoint &pole, double radius,
                   const QwtScaleMap &azimuthMap, const QwtValueList & ) const;
    void drawCircles( QPainter *, const QwtDoubleRect &,
                      const QwtDoublePoint &pole, const QwtScaleMap &radialMap,
                      const QwtValueList & ) const;

    void drawAxis( QPainter *, int axisId ) const;

  private:
    void updateScaleDraws(
      const QwtScaleMap &azimuthMap, const QwtScaleMap &radialMap,
      const QwtDoublePoint &pole, const double radius ) const;

  private:
    class GridData;
    class AxisData;
    class PrivateData;
    PrivateData *d_data;
};

#endif
