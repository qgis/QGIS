/***************************************************************************
    qgscurveeditorwidget.h
    ----------------------
    begin                : February 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCURVEEDITORWIDGET_H
#define QGSCURVEEDITORWIDGET_H

#include <QWidget>
#include "qgis_gui.h"
#include "qgspropertytransformer.h"

class QwtPlot;
class QwtPlotCurve;
class QwtPlotMarker;
class QgsCurveEditorPlotEventFilter;

/** \ingroup gui
 * \class QgsCurveEditorWidget
 * A widget for manipulating QgsCurveTransform curves.
 * \note added in QGIS 3.0
 */
class GUI_EXPORT QgsCurveEditorWidget : public QWidget
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsCurveEditorWidget.
     */
    QgsCurveEditorWidget( QWidget* parent = nullptr, const QgsCurveTransform& curve = QgsCurveTransform() );

    /**
     * Returns a curve representing the current curve from the widget.
     * @see setCurve()
     */
    QgsCurveTransform curve() const { return mCurve; }

    /**
     * Sets the \a curve to show in the widget.
     * @see curve()
     */
    void setCurve( const QgsCurveTransform& curve );

  signals:

    //! Emitted when the widget curve changes
    void changed();

  protected:

    virtual void keyPressEvent( QKeyEvent *event ) override ;

  private slots:

    void plotMousePress( QPointF point );
    void plotMouseRelease( QPointF point );
    void plotMouseMove( QPointF point );

  private:

    QgsCurveTransform mCurve;

    QwtPlot* mPlot = nullptr;

    QwtPlotCurve* mPlotCurve = nullptr;

    QList< QwtPlotMarker* > mMarkers;
    QgsCurveEditorPlotEventFilter* mPlotFilter = nullptr;
    int mCurrentPlotMarkerIndex;

    void updatePlot();
    void addPlotMarker( double x, double y, bool isSelected = false );

    int findNearestControlPoint( QPointF point ) const;
};


//
// NOTE:
// For private only, not part of stable api or exposed to Python bindings
//
/// @cond PRIVATE
class GUI_EXPORT QgsCurveEditorPlotEventFilter: public QObject
{
    Q_OBJECT

  public:

    QgsCurveEditorPlotEventFilter( QwtPlot *plot );

    virtual bool eventFilter( QObject* object, QEvent* event ) override;

  signals:

    void mousePress( QPointF );
    void mouseRelease( QPointF );
    void mouseMove( QPointF );

  private:

    QwtPlot* mPlot;
    QPointF mapPoint( QPointF point ) const;
};
///@endcond

#endif // QGSCURVEEDITORWIDGET_H
