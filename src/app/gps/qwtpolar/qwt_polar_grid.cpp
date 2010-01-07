/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * QwtPolar Widget Library
 * Copyright (C) 2008   Uwe Rathmann
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include <cfloat>
#include <cmath>
#include <qpainter.h>
#include <qpen.h>
#include "qwt_painter.h"
#include "qwt_text.h"
#include "qwt_clipper.h"
#include "qwt_scale_map.h"
#include "qwt_scale_engine.h"
#include "qwt_scale_div.h"
#include "qwt_scale_draw.h"
#include "qwt_round_scale_draw.h"
#include "qwt_polar_grid.h"

static inline bool isClose(double value1, double value2 )
{
    return qwtAbs(value1 - value2) < DBL_EPSILON;
}

class QwtPolarGrid::AxisData
{
public:
    AxisData():
        isVisible(false),
        scaleDraw(NULL)
    {
    }
    ~AxisData()
    {
        delete scaleDraw;
    }

    bool isVisible;
    mutable QwtAbstractScaleDraw *scaleDraw;
    QPen pen;
    QFont font;
};

class QwtPolarGrid::GridData
{
public:
    GridData():
        isVisible(true),
        isMinorVisible(false)
    {
    }

    bool isVisible;
    bool isMinorVisible;
    QwtScaleDiv scaleDiv;

    QPen majorPen;
    QPen minorPen;
};

class QwtPolarGrid::PrivateData
{
public:
    GridData gridData[QwtPolar::ScaleCount];
    AxisData axisData[QwtPolar::AxesCount];
    int displayFlags;
    int attributes;
};

/*! 
   \brief Constructor

   Enables major and disables minor grid lines. 
   The azimuth and right radial axis are visible. all other axes
   are hidden. Autoscaling is enabled.
*/
QwtPolarGrid::QwtPolarGrid():
    QwtPolarItem(QwtText("Grid"))
{
    d_data = new PrivateData;

    for ( int axisId = 0; axisId < QwtPolar::AxesCount; axisId++ )
    {
        AxisData &axis = d_data->axisData[axisId];
        switch(axisId)
        {
            case QwtPolar::AxisAzimuth:
            {
                axis.scaleDraw = new QwtRoundScaleDraw;
                axis.scaleDraw->setTickLength(QwtScaleDiv::MinorTick, 2);
                axis.scaleDraw->setTickLength(QwtScaleDiv::MediumTick, 2);
                axis.scaleDraw->setTickLength(QwtScaleDiv::MajorTick, 4);
                axis.isVisible = true;
                break;
            }
            case QwtPolar::AxisLeft:
            {
                QwtScaleDraw *scaleDraw = new QwtScaleDraw;
                scaleDraw->setAlignment(QwtScaleDraw::BottomScale);

                axis.scaleDraw = scaleDraw;
                axis.isVisible = false;
                break;
            }
            case QwtPolar::AxisRight:
            {
                QwtScaleDraw *scaleDraw = new QwtScaleDraw;
                scaleDraw->setAlignment(QwtScaleDraw::BottomScale);

                axis.scaleDraw = scaleDraw;
                axis.isVisible = true;
                break;
            }
            case QwtPolar::AxisTop:
            {
                QwtScaleDraw *scaleDraw = new QwtScaleDraw;
                scaleDraw->setAlignment(QwtScaleDraw::LeftScale);

                axis.scaleDraw = scaleDraw;
                axis.isVisible = false;
                break;
            }
            case QwtPolar::AxisBottom:
            {
                QwtScaleDraw *scaleDraw = new QwtScaleDraw;
                scaleDraw->setAlignment(QwtScaleDraw::LeftScale);

                axis.scaleDraw = scaleDraw;
                axis.isVisible = true;
                break;
            }
            default:;
        }
    }

    d_data->attributes = AutoScaling;

    d_data->displayFlags = 0;
    d_data->displayFlags |= SmartOriginLabel;
    d_data->displayFlags |= HideMaxRadiusLabel;
    d_data->displayFlags |= ClipAxisBackground;
    d_data->displayFlags |= SmartScaleDraw;
    d_data->displayFlags |= ClipGridLines;

    setZ(10.0);
#if QT_VERSION >= 0x040000
    setRenderHint(RenderAntialiased, true);
#endif
}

//! Destructor
QwtPolarGrid::~QwtPolarGrid()
{
    delete d_data;
}

//! \return QwtPlotItem::Rtti_PolarGrid
int QwtPolarGrid::rtti() const
{
    return QwtPolarItem::Rtti_PolarGrid;
}

/*! 
   Change the display flags

   \param flag See DisplayFlag
   \param on true/false
*/
void QwtPolarGrid::setDisplayFlag(DisplayFlag flag, bool on)
{
    if ( ((d_data->displayFlags & flag) != 0) != on )
    {
        if ( on )
            d_data->displayFlags |= flag;
        else
            d_data->displayFlags &= ~flag;

        itemChanged();
    }
}

/*! 
   \return true, if flag is enabled
   \param flag See DisplayFlag
*/
bool QwtPolarGrid::testDisplayFlag(DisplayFlag flag) const
{
    return (d_data->displayFlags & flag);
}

/*!
  \brief Specify an attribute for the grid

  \param attribute Grid attribute
  \param on On/Off

  /sa GridAttribute, testGridAttribute(), updateScaleDiv(), 
      QwtPolarPlot::zoom(), QwtPolarPlot::scaleDiv()
*/
void QwtPolarGrid::setGridAttribute(GridAttribute attribute, bool on)
{
    if ( bool(d_data->attributes & attribute) == on )
        return;

    if ( on )
        d_data->attributes |= attribute;
    else
        d_data->attributes &= ~attribute;

    itemChanged();
}

/*!
    \return true, if attribute is enabled 
    \sa GridAttribute, setGridAttribute()
*/
bool QwtPolarGrid::testGridAttribute(GridAttribute attribute) const
{
    return d_data->attributes & attribute;
}

/*!
  Assign a pen for painting an axis

  \param axisId Axis id (QwtPolar::Axis)
  \param pen Pen

  \sa axisPen()
*/
void QwtPolarGrid::setAxisPen(int axisId, const QPen &pen)
{
    if ( axisId < 0 || axisId >= QwtPolar::AxesCount )
        return;

    AxisData &axisData = d_data->axisData[axisId];
    if ( axisData.pen != pen )
    {
        axisData.pen = pen;
        itemChanged();
    }
}

/*!
   Show/Hide grid lines for a scale

   \param scaleId Scale id ( QwtPolar::Scale )
   \param show true/false
*/
void QwtPolarGrid::showGrid(int scaleId, bool show)
{
    if ( scaleId < 0 || scaleId >= QwtPolar::ScaleCount )
        return;

    GridData &grid = d_data->gridData[scaleId];
    if ( grid.isVisible != show )
    {
        grid.isVisible = show;
        itemChanged();
    }
}

/*!
  \return true if grid lines are enabled
  \param scaleId Scale id ( QwtPolar::Scale )
  \sa QwtPolar::Scale, showGrid()
*/
bool QwtPolarGrid::isGridVisible(int scaleId) const
{ 
    if ( scaleId < 0 || scaleId >= QwtPolar::ScaleCount )
        return false;

    return d_data->gridData[scaleId].isVisible;
}

/*!
   Show/Hide minor grid lines for a scale

   To display minor grid lines. showGrid() needs to be enabled too.

   \param scaleId Scale id ( QwtPolar::Scale )
   \param show true/false

   \sa showGrid
*/
void QwtPolarGrid::showMinorGrid(int scaleId, bool show)
{
    if ( scaleId < 0 || scaleId >= QwtPolar::ScaleCount )
        return;

    GridData &grid = d_data->gridData[scaleId];
    if ( grid.isMinorVisible != show )
    {
        grid.isMinorVisible = show;
        itemChanged();
    }
}

/*!
  \return true if minor grid lines are enabled
  \param scaleId Scale id ( QwtPolar::Scale )
  \sa showMinorGrid()
*/
bool QwtPolarGrid::isMinorGridVisible(int scaleId) const
{ 
    if ( scaleId < 0 || scaleId >= QwtPolar::ScaleCount )
        return false;

    return d_data->gridData[scaleId].isMinorVisible;
}

/*!
  Show/Hide an axis

  \param axisId Axis id (QwtPolar::Axis)
  \param show true/false

  \sa isAxisVisible()
*/
void QwtPolarGrid::showAxis(int axisId, bool show)
{
    if ( axisId < 0 || axisId >= QwtPolar::AxesCount )
        return;

    AxisData &axisData = d_data->axisData[axisId];
    if ( axisData.isVisible != show )
    {
        axisData.isVisible = show;
        itemChanged();
    }
}

/*!
  \return true if the axis is visible
  \param axisId Axis id (QwtPolar::Axis)
  
  \sa showAxis()
*/
bool QwtPolarGrid::isAxisVisible(int axisId) const
{
    if ( axisId < 0 || axisId >= QwtPolar::AxesCount )
        return false;

    return d_data->axisData[axisId].isVisible;
}

/*!
   Assign a pen for all axes and grid lines

   \param pen Pen
   \sa setMajorGridPen(), setMinorGridPen(), setAxisPen()
*/
void QwtPolarGrid::setPen(const QPen &pen)
{
    bool isChanged = false;

    for ( int scaleId = 0; scaleId < QwtPolar::ScaleCount; scaleId++ )
    {
        GridData &grid = d_data->gridData[scaleId];
        if ( grid.majorPen != pen || grid.minorPen != pen )
        {
            grid.majorPen = pen;
            grid.minorPen = pen;
            isChanged = true;
        }
    }
    for ( int axisId = 0; axisId < QwtPolar::AxesCount; axisId++ )
    {
        AxisData &axis = d_data->axisData[axisId];
        if ( axis.pen != pen )
        {
            axis.pen = pen;
            isChanged = true;
        }
    }
    if ( isChanged )
        itemChanged();
}

/*!
   Assign a font for all scale tick labels
    
   \param font Font
   \sa setAxisFont()
*/
void QwtPolarGrid::setFont(const QFont &font)
{
    bool isChanged = false;
    for ( int axisId = 0; axisId < QwtPolar::AxesCount; axisId++ )
    {
        AxisData &axis = d_data->axisData[axisId];
        if ( axis.font != font )
        {
            axis.font = font;
            isChanged = true;
        }
    }
    if ( isChanged )
        itemChanged();
}

/*!
   Assign a pen for the major grid lines

   \param pen Pen
   \sa setPen(), setMinorGridPen(), majorGridPen
*/
void QwtPolarGrid::setMajorGridPen(const QPen &pen)
{
    bool isChanged = false;

    for ( int scaleId = 0; scaleId < QwtPolar::ScaleCount; scaleId++ )
    {
        GridData &grid = d_data->gridData[scaleId];
        if ( grid.majorPen != pen )
        {
            grid.majorPen = pen;
            isChanged = true;
        }
    }
    if ( isChanged )
        itemChanged();
}

/*!
   Assign a pen for the major grid lines of a specific scale

   \param scaleId Scale id ( QwtPolar::Scale )
   \param pen Pen
   \sa setPen(), setMinorGridPen(), majorGridPen
*/
void QwtPolarGrid::setMajorGridPen(int scaleId, const QPen &pen)
{
    if ( scaleId < 0 || scaleId >= QwtPolar::ScaleCount )
        return;

    GridData &grid = d_data->gridData[scaleId];
    if ( grid.majorPen != pen )
    {
        grid.majorPen = pen;
        itemChanged();
    }
}

/*! 
   \return Pen for painting the major grid lines of a specific scale
   \param scaleId Scale id ( QwtPolar::Scale )
   \sa setMajorGridPen(), minorGridPen()
*/
QPen QwtPolarGrid::majorGridPen(int scaleId) const
{
    if ( scaleId < 0 || scaleId >= QwtPolar::ScaleCount )
        return QPen();
    
    const GridData &grid = d_data->gridData[scaleId];
    return grid.majorPen;
}   

/*!
   Assign a pen for the minor grid lines 

   \param pen Pen
   \sa setPen(), setMajorGridPen(), minorGridPen()
*/
void QwtPolarGrid::setMinorGridPen(const QPen &pen)
{
    bool isChanged = false;

    for ( int scaleId = 0; scaleId < QwtPolar::ScaleCount; scaleId++ )
    {
        GridData &grid = d_data->gridData[scaleId];
        if ( grid.minorPen != pen )
        {
            grid.minorPen = pen;
            isChanged = true;
        }   
    }   
    if ( isChanged )
        itemChanged();
}

/*!
   Assign a pen for the minor grid lines of a specific scale

   \param scaleId Scale id ( QwtPolar::Scale )
   \param pen Pen
   \sa setPen(), setMajorGridPen(), minorGridPen
*/
void QwtPolarGrid::setMinorGridPen(int scaleId, const QPen &pen)
{
    if ( scaleId < 0 || scaleId >= QwtPolar::ScaleCount )
        return;

    GridData &grid = d_data->gridData[scaleId];
    if ( grid.minorPen != pen )
    {
        grid.minorPen = pen;
        itemChanged();
    }
}

/*! 
   \return Pen for painting the minor grid lines of a specific scale
   \param scaleId Scale id ( QwtPolar::Scale )
*/
QPen QwtPolarGrid::minorGridPen(int scaleId) const
{ 
    if ( scaleId < 0 || scaleId >= QwtPolar::ScaleCount )
        return QPen();

    const GridData &grid = d_data->gridData[scaleId];
    return grid.minorPen;
}

/*! 
   \return Pen for painting a specific axis

   \param axisId Axis id (QwtPolar::Axis)
   \sa setAxisPen()
*/
QPen QwtPolarGrid::axisPen(int axisId) const
{
    if ( axisId < 0 || axisId >= QwtPolar::AxesCount )
        return QPen();

    return d_data->axisData[axisId].pen;
}

/*!
  Assign a font for the tick labels of a specific axis

  \param axisId Axis id (QwtPolar::Axis)
  \param font new Font
*/
void QwtPolarGrid::setAxisFont(int axisId, const QFont &font)
{
    if ( axisId < 0 || axisId >= QwtPolar::AxesCount )
        return;

    AxisData &axisData = d_data->axisData[axisId];
    if ( axisData.font != font )
    {
        axisData.font = font;
        itemChanged();
    }
}

/*! 
  \return Font for the tick labels of a specific axis
  \param axisId Axis id (QwtPolar::Axis)
*/
QFont QwtPolarGrid::axisFont(int axisId) const
{
    if ( axisId < 0 || axisId >= QwtPolar::AxesCount )
        return QFont();

    return d_data->axisData[axisId].font;
}

/*!
  Draw the grid and axes

  \param painter Painter
  \param azimuthMap Maps azimuth values to values related to 0.0, M_2PI
  \param radialMap Maps radius values into painter coordinates.
  \param pole Position of the pole in painter coordinates
  \param radius Radius of the complete plot area in painter coordinates
  \param canvasRect Contents rect of the canvas in painter coordinates
*/
void QwtPolarGrid::draw(QPainter *painter, 
    const QwtScaleMap &azimuthMap, const QwtScaleMap &radialMap,
    const QwtDoublePoint &pole, double radius,
    const QwtDoubleRect &canvasRect) const
{
    updateScaleDraws(azimuthMap, radialMap, pole, radius);

    painter->save();

    if ( testDisplayFlag(ClipAxisBackground) )
    {
        QRegion clipRegion(canvasRect.toRect());
        for ( int axisId = 0; axisId < QwtPolar::AxesCount; axisId++ )
        {
            const AxisData &axis = d_data->axisData[axisId];
            if ( axisId != QwtPolar::AxisAzimuth && axis.isVisible )
            {
                QwtScaleDraw *scaleDraw = (QwtScaleDraw *)axis.scaleDraw;
                if ( scaleDraw->hasComponent(QwtScaleDraw::Labels) )
                {
                    const QwtValueList &ticks = 
                        scaleDraw->scaleDiv().ticks(QwtScaleDiv::MajorTick);
                    for ( int i = 0; i < int(ticks.size()); i++ )
                    {
                        QRect labelRect =
                            scaleDraw->boundingLabelRect(axis.font, ticks[i]);

                        const int margin = 2;
                        labelRect.setRect(
                            labelRect.x() - margin,
                            labelRect.y() - margin,
                            labelRect.width() + 2 * margin,
                            labelRect.height() + 2 * margin
                        );
                            
                        if ( labelRect.isValid() )
                            clipRegion -= QRegion(labelRect);
                    }
                }
            }
        }
        painter->setClipRegion(clipRegion);
    }

    //  draw radial grid
    
    const GridData &radialGrid = d_data->gridData[QwtPolar::Radius];
    if (radialGrid.isVisible && radialGrid.isMinorVisible)
    {
        painter->setPen(radialGrid.minorPen);
        
        drawCircles(painter, canvasRect, pole, radialMap, 
            radialGrid.scaleDiv.ticks(QwtScaleDiv::MinorTick) );
        drawCircles(painter, canvasRect, pole, radialMap, 
            radialGrid.scaleDiv.ticks(QwtScaleDiv::MediumTick) );
    }
    if (radialGrid.isVisible)
    {
        painter->setPen(radialGrid.majorPen);

        drawCircles(painter, canvasRect, pole, radialMap, 
            radialGrid.scaleDiv.ticks(QwtScaleDiv::MajorTick) );
    }

    // draw azimuth grid

    const GridData &azimuthGrid = 
        d_data->gridData[QwtPolar::Azimuth];

    if (azimuthGrid.isVisible && azimuthGrid.isMinorVisible)
    {
        painter->setPen(azimuthGrid.minorPen);

        drawRays(painter, canvasRect, pole, radius, azimuthMap, 
            azimuthGrid.scaleDiv.ticks(QwtScaleDiv::MinorTick));
        drawRays(painter, canvasRect, pole, radius, azimuthMap, 
            azimuthGrid.scaleDiv.ticks(QwtScaleDiv::MediumTick));
    }
    if (azimuthGrid.isVisible)
    {   
        painter->setPen(azimuthGrid.majorPen);

        drawRays(painter, canvasRect, pole, radius, azimuthMap,
            azimuthGrid.scaleDiv.ticks(QwtScaleDiv::MajorTick));
    }
    painter->restore();

    for ( int axisId = 0; axisId < QwtPolar::AxesCount; axisId++ )
    {
        const AxisData &axis = d_data->axisData[axisId];
        if ( axis.isVisible )
        {
            painter->save();
            drawAxis(painter, axisId);
            painter->restore();
        }
    }
}

/*!
  Draw lines from the pole 

  \param painter Painter
  \param canvasRect Contents rect of the canvas in painter coordinates
  \param pole Position of the pole in painter coordinates
  \param radius Length of the lines in painter coordinates
  \param azimuthMap Maps azimuth values to values related to 0.0, M_2PI
  \param values Azimuth values, indicating the direction of the lines
*/
void QwtPolarGrid::drawRays(
    QPainter *painter, const QwtDoubleRect &canvasRect,
    const QwtDoublePoint &pole, double radius,
    const QwtScaleMap &azimuthMap, const QwtValueList &values) const
{
    for ( int i = 0; i < int(values.size()); i++ )
    {
        double azimuth = azimuthMap.xTransform(values[i]);
        azimuth = ::fmod(azimuth, 2 * M_PI);

        bool skipLine = false;
        if ( testDisplayFlag(SmartScaleDraw) )
        {
            const QwtAbstractScaleDraw::ScaleComponent bone = 
                QwtAbstractScaleDraw::Backbone;
            if ( isClose(azimuth, 0.0) )
            {
                const AxisData &axis = d_data->axisData[QwtPolar::AxisRight];
                if ( axis.isVisible && axis.scaleDraw->hasComponent(bone) )
                    skipLine = true;
            }
            else if ( isClose(azimuth, M_PI / 2) )
            {
                const AxisData &axis = d_data->axisData[QwtPolar::AxisTop];
                if ( axis.isVisible && axis.scaleDraw->hasComponent(bone) )
                    skipLine = true;
            }
            else if ( isClose(azimuth, M_PI) )
            {
                const AxisData &axis = d_data->axisData[QwtPolar::AxisLeft];
                if ( axis.isVisible && axis.scaleDraw->hasComponent(bone) )
                    skipLine = true;
            }
            else if ( isClose(azimuth, 3 * M_PI / 2.0) )
            {
                const AxisData &axis = d_data->axisData[QwtPolar::AxisBottom];
                if ( axis.isVisible && axis.scaleDraw->hasComponent(bone) )
                    skipLine = true;
            }
        }
        if ( !skipLine )
        {
            const QwtDoublePoint pos = qwtPolar2Pos(pole, radius, azimuth);

            /*
                Qt4 is horrible slow, when painting primitives,
                with coordinates far outside the visible area.
             */

            QwtPolygon pa(2);
            pa.setPoint(0, pole.toPoint());
            pa.setPoint(1, pos.toPoint());

            if ( testDisplayFlag(ClipGridLines) )
                pa = QwtClipper::clipPolygon(canvasRect.toRect(), pa);

            QwtPainter::drawPolyline(painter, pa);
        }
    }
}

/*!
  Draw circles

  \param painter Painter
  \param canvasRect Contents rect of the canvas in painter coordinates
  \param pole Position of the pole in painter coordinates
  \param radialMap Maps radius values into painter coordinates.
  \param values Radial values, indicating the distances from the pole
*/
void QwtPolarGrid::drawCircles(
    QPainter *painter, const QwtDoubleRect &canvasRect,
    const QwtDoublePoint &pole, const QwtScaleMap &radialMap, 
    const QwtValueList &values) const
{
    for ( int i = 0; i < int(values.size()); i++ )
    {
        const double val = values[i];

        const GridData &gridData = 
            d_data->gridData[QwtPolar::Radius];

        bool skipLine = false;
        if ( testDisplayFlag(SmartScaleDraw) )
        {
            const AxisData &axis = d_data->axisData[QwtPolar::AxisAzimuth];
            if ( axis.isVisible &&
                axis.scaleDraw->hasComponent(QwtAbstractScaleDraw::Backbone) )
            {
                if ( isClose(val, gridData.scaleDiv.upperBound()) )
                    skipLine = true;
            }
        }

        if ( isClose(val, gridData.scaleDiv.lowerBound()) )
            skipLine = true;

        if ( !skipLine )
        {
            const double radius = radialMap.transform(val);

            QwtDoubleRect outerRect(0, 0, 2 * radius, 2 * radius);
            outerRect.moveCenter(pole);

#if QT_VERSION < 0x040000
            QwtPainter::drawEllipse(painter, outerRect.toRect());
#else
            if ( testDisplayFlag(ClipGridLines) )
            {

                /*
                    Qt4 is horrible slow, when painting primitives,
                    with coordinates far outside the visible area.
                    We need to clip.
                */

                const QwtArray<QwtDoubleInterval> angles = 
                    QwtClipper::clipCircle( canvasRect, pole, radius);
                for ( int i = 0; i < angles.size(); i++ )
                {
                    const QwtDoubleInterval intv = angles[i];
                    if ( intv.minValue() == 0 && intv.maxValue() == 2 * M_PI )
                        QwtPainter::drawEllipse(painter, outerRect.toRect());
                    else
                    {
                        const double from = intv.minValue() / M_PI * 180;
                        const double to = intv.maxValue() / M_PI * 180;
                        double span = to - from;
                        if ( span < 0.0 )
                            span += 360.0;
                    
                        const QwtMetricsMap &mm = QwtPainter::metricsMap();
                        const QRect r = outerRect.toRect();

                        painter->drawArc(mm.layoutToDevice(r, painter), 
                            qRound(from * 16), qRound(span * 16));
                    }
                    
                }
            }
            else
            {
                QwtPainter::drawEllipse(painter, outerRect.toRect());
            }
#endif
        }
    }
}

/*!
  Paint an axis

  \param painter Painter
  \param axisId Axis id (QwtPolar::Axis)
*/
void QwtPolarGrid::drawAxis(QPainter *painter, int axisId) const
{
    if ( axisId < 0 || axisId >= QwtPolar::AxesCount )
        return;

    AxisData &axis = d_data->axisData[axisId];

    painter->setPen(axis.pen);
    painter->setFont(axis.font);

#if QT_VERSION < 0x040000
    QColorGroup cg;
    cg.setColor(QColorGroup::Foreground, axis.pen.color());
    cg.setColor(QColorGroup::Text, axis.pen.color());

    axis.scaleDraw->draw(painter, cg);
#else
    QPalette pal;
    pal.setColor(QPalette::Foreground, axis.pen.color());
    pal.setColor(QPalette::Text, axis.pen.color());

    axis.scaleDraw->draw(painter, pal);
#endif
}

/*!
   Update the axis scale draw geometries

   \param azimuthMap Maps azimuth values to values related to 0.0, M_2PI
   \param radialMap Maps radius values into painter coordinates.
   \param pole Position of the pole in painter coordinates
   \param radius Radius of the complete plot area in painter coordinates

   \sa updateScaleDiv()
*/
void QwtPolarGrid::updateScaleDraws(
    const QwtScaleMap &azimuthMap, const QwtScaleMap &radialMap, 
    const QwtDoublePoint &pole, double radius) const
{
    const QPoint p = pole.toPoint();

    const QwtDoubleInterval interval = 
        d_data->gridData[QwtPolar::ScaleRadius].scaleDiv.interval();

    const int min = radialMap.transform(interval.minValue());
    const int max = radialMap.transform(interval.maxValue());
    const int l = max - min;

    for ( int axisId = 0; axisId < QwtPolar::AxesCount; axisId++ )
    {
        AxisData &axis = d_data->axisData[axisId];

        if ( axisId == QwtPolar::AxisAzimuth )
        {
            QwtRoundScaleDraw *scaleDraw = (QwtRoundScaleDraw *)axis.scaleDraw;

            scaleDraw->setRadius(qRound(radius));
            scaleDraw->moveCenter(p);

            double from = ::fmod(90.0 - azimuthMap.p1() * 180.0 / M_PI, 360.0);
            if ( from < 0.0 )
                from += 360.0;

            scaleDraw->setAngleRange(from, from - 360.0);
            scaleDraw->setTransformation(azimuthMap.transformation()->copy());
        }
        else
        {
            QwtScaleDraw *scaleDraw = (QwtScaleDraw *)axis.scaleDraw;
            switch(axisId)
            {
                case QwtPolar::AxisLeft:
                {
                    scaleDraw->move(p.x() - min, p.y());
                    scaleDraw->setLength(-l);
                    break;
                }
                case QwtPolar::AxisRight:
                {
                    scaleDraw->move(p.x() + min, p.y());
                    scaleDraw->setLength(l);
                    break;
                }
                case QwtPolar::AxisTop:
                {
                    scaleDraw->move(p.x(), p.y() - max);
                    scaleDraw->setLength(l);
                    break;
                }
                case QwtPolar::AxisBottom:
                {
                    scaleDraw->move(p.x(), p.y() + max);
                    scaleDraw->setLength(-l);
                    break;
                }
            }
            scaleDraw->setTransformation(radialMap.transformation()->copy());
        }
    }
}

/*!
   \brief Update the item to changes of the axes scale division

   If AutoScaling is enabled the radial scale is calculated
   from the interval, otherwise the scales are adopted to 
   the plot scales.

   \param azimuthScaleDiv Scale division of the azimuth-scale
   \param radialScaleDiv Scale division of the radius-axis
   \param interval The interval of the radius-axis, that is
                   visible on the canvas

   \sa QwtPolarPlot::setGridAttributes()
*/

void QwtPolarGrid::updateScaleDiv(const QwtScaleDiv &azimuthScaleDiv,
    const QwtScaleDiv &radialScaleDiv, const QwtDoubleInterval &interval)
{
    GridData &radialGrid = d_data->gridData[QwtPolar::Radius];

    const QwtPolarPlot *plt = plot();
    if ( plt && testGridAttribute(AutoScaling) )
    {
        const QwtScaleEngine *se = plt->scaleEngine(QwtPolar::Radius);
        radialGrid.scaleDiv = se->divideScale(
            interval.minValue(), interval.maxValue(),
            plt->scaleMaxMajor(QwtPolar::Radius),
            plt->scaleMaxMinor(QwtPolar::Radius), 0);
    }
    else
    {
        if ( radialGrid.scaleDiv != radialScaleDiv )
            radialGrid.scaleDiv = radialScaleDiv;
    }

    GridData &azimuthGrid = d_data->gridData[QwtPolar::Azimuth];
    if ( azimuthGrid.scaleDiv != azimuthScaleDiv )
    {
        azimuthGrid.scaleDiv = azimuthScaleDiv;
    }

    bool hasOrigin = false;
    for ( int axisId = 0; axisId < QwtPolar::AxesCount; axisId++ )
    {
        AxisData &axis = d_data->axisData[axisId];
        if ( axis.isVisible && axis.scaleDraw )
        {
            if ( axisId == QwtPolar::AxisAzimuth )
            {
                axis.scaleDraw->setScaleDiv(azimuthGrid.scaleDiv);
                if ( testDisplayFlag(SmartScaleDraw) )
                {
                    axis.scaleDraw->enableComponent(
                        QwtAbstractScaleDraw::Ticks, !azimuthGrid.isVisible);
                }
            }
            else
            {
                QwtScaleDiv sd = radialGrid.scaleDiv;

                QwtValueList &ticks = 
                        (QwtValueList &)sd.ticks(QwtScaleDiv::MajorTick);

                if ( testDisplayFlag(SmartOriginLabel) )
                {
                    bool skipOrigin = hasOrigin;
                    if ( !skipOrigin )
                    {
                        if ( axisId == QwtPolar::AxisLeft 
                            || axisId == QwtPolar::AxisRight )
                        {
                            if ( d_data->axisData[QwtPolar::AxisBottom].isVisible )
                                skipOrigin = true;
                        }
                        else
                        {
                            if ( d_data->axisData[QwtPolar::AxisLeft].isVisible )
                                skipOrigin = true;
                        }
                    }
                    if ( ticks.size() > 0 && ticks.first() == sd.lowerBound() )
                    {
                        if ( skipOrigin )
                        {
#if QT_VERSION < 0x040000
                            ticks.pop_front();
#else
                            ticks.removeFirst();
#endif
                        }
                        else
                            hasOrigin = true;
                    }
                }

                if ( testDisplayFlag(HideMaxRadiusLabel) )
                {
                    if ( ticks.size() > 0 && ticks.last() == sd.upperBound() )
#if QT_VERSION < 0x040000
                        ticks.pop_back();
#else
                        ticks.removeLast();
#endif
                }

                axis.scaleDraw->setScaleDiv(sd);

                if ( testDisplayFlag(SmartScaleDraw) )
                {
                    axis.scaleDraw->enableComponent(
                        QwtAbstractScaleDraw::Ticks, !radialGrid.isVisible);
                }

            }
        }
    }
}

/*! 
   \return Number of pixels, that are necessary to paint the azimuth scale
   \sa QwtRoundScaleDraw::extent()
*/
int QwtPolarGrid::marginHint() const
{
    const AxisData &axis = d_data->axisData[QwtPolar::AxisAzimuth];
    if ( axis.isVisible )
    {
        const int extent = axis.scaleDraw->extent(axis.pen, axis.font);
        return extent;
    }
    
    return 0;
}

/*!
  Returns the scale draw of a specified axis

  \param axisId axis index ( QwtPolar::AxisLeft <= axisId <= QwtPolar::AxisBottom)
  \return specified scaleDraw for axis, or NULL if axis is invalid.
  \sa azimuthScaleDraw()
*/
const QwtScaleDraw *QwtPolarGrid::scaleDraw(int axisId) const
{
    if ( axisId >= QwtPolar::AxisLeft || axisId <= QwtPolar::AxisBottom )
        return (QwtScaleDraw *)d_data->axisData[axisId].scaleDraw;

    return NULL;
}

/*!
  Returns the scale draw of a specified axis

  \param axisId axis index ( QwtPolar::AxisLeft <= axisId <= QwtPolar::AxisBottom)
  \return specified scaleDraw for axis, or NULL if axis is invalid.
  \sa setScaleDraw(), azimuthScaleDraw()
*/
QwtScaleDraw *QwtPolarGrid::scaleDraw(int axisId)
{
    if ( axisId >= QwtPolar::AxisLeft || axisId <= QwtPolar::AxisBottom )
        return (QwtScaleDraw *)d_data->axisData[axisId].scaleDraw;

    return NULL;
}

/*!
  \brief Set a scale draw

  \param axisId axis index ( QwtPolar::AxisLeft <= axisId <= QwtPolar::AxisBottom)
  \param scaleDraw object responsible for drawing scales.

  \sa scaleDraw(), setAzimuthScaleDraw()
*/
void QwtPolarGrid::setScaleDraw(int axisId, QwtScaleDraw *scaleDraw)
{
    if ( axisId < QwtPolar::AxisLeft || axisId > QwtPolar::AxisBottom )
        return;

    AxisData &axisData = d_data->axisData[axisId];
    if ( axisData.scaleDraw != scaleDraw )
    {
        delete axisData.scaleDraw;
        axisData.scaleDraw = scaleDraw;
        itemChanged();
    }
}

/*!
  Returns the scale draw of the azimuth axis
  \sa setAzimuthScaleDraw(), scaleDraw()
*/
const QwtRoundScaleDraw *QwtPolarGrid::azimuthScaleDraw() const
{
    return (QwtRoundScaleDraw *)d_data->axisData[QwtPolar::AxisAzimuth].scaleDraw;
}

/*!
  Returns the scale draw of the azimuth axis
  \sa setAzimuthScaleDraw(), scaleDraw()
*/
QwtRoundScaleDraw *QwtPolarGrid::azimuthScaleDraw() 
{
    return (QwtRoundScaleDraw *)d_data->axisData[QwtPolar::AxisAzimuth].scaleDraw;
}

/*!
  \brief Set a scale draw for the azimuth scale

  \param scaleDraw object responsible for drawing scales.
  \sa azimuthScaleDraw(), setScaleDraw()
*/
void QwtPolarGrid::setAzimuthScaleDraw(QwtRoundScaleDraw *scaleDraw)
{
    AxisData &axisData = d_data->axisData[QwtPolar::AxisAzimuth];
    if ( axisData.scaleDraw != scaleDraw )
    {
        delete axisData.scaleDraw;
        axisData.scaleDraw = scaleDraw;
        itemChanged();
    }
}
