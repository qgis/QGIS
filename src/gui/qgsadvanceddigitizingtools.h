/***************************************************************************
     qgsadvanceddigitizingtools.h
     ----------------------
     begin                : May 2024
     copyright            : (C) Mathieu Pellerin
     email                : mathieu at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSADVANCEDDIGITIZINGTOOLS
#define QGSADVANCEDDIGITIZINGTOOLS

#include <QWidget>
#include <QString>

#include "qgsadvanceddigitizingdockwidget.h"
#include "qgis_gui.h"
#include "qgis_sip.h"
#include "qgsmapmouseevent.h"

#include <QPointer>

class QgsAdvancedDigitizingDockWidget;
class QgsDoubleSpinBox;
class QgsMapCanvas;

/**
 * \ingroup gui
 * \brief An abstract class for advanced digitizing tools.
 * \since QGIS 3.40
 */
class GUI_EXPORT QgsAdvancedDigitizingTool : public QWidget
{
    Q_OBJECT

  public:

    /**
     * The advanced digitizing tool constructor.
     * \param canvas The map canvas on which the widget operates
     * \param cadDockWidget The cadDockWidget to which the floater belongs
     */
    explicit QgsAdvancedDigitizingTool( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDockWidget );

    /**
     * Returns the map canvas associated with the tool.
     */
    QgsMapCanvas *mapCanvas() const { return mMapCanvas; }

    /**
     * Returns the advanced digitizing widget associated with the tool.
     */
    QgsAdvancedDigitizingDockWidget *cadDockWidget() const { return mCadDockWidget.data(); }

    /**
     * Paints tool content onto the advanced digitizing canvas item.
     */
    virtual void paint( QPainter *painter ) { Q_UNUSED( painter ) }

    /**
     * Handles canvas press event. If TRUE is returned, the tool will have
     * blocked the event for propagating.
     */
    virtual bool canvasPressEvent( QgsMapMouseEvent *event )
    {
      Q_UNUSED( event )
      return true;
    }

    /**
     * Handles canvas press move. If TRUE is returned, the tool will have
     * blocked the event for propagating.
     */
    virtual bool canvasMoveEvent( QgsMapMouseEvent *event )
    {
      Q_UNUSED( event )
      return true;
    }

    /**
     * Handles canvas release event. If TRUE is returned, the tool will have
     * blocked the event for propagating.
     */
    virtual bool canvasReleaseEvent( QgsMapMouseEvent *event )
    {
      Q_UNUSED( event )
      return true;
    }

  signals:

    /**
     * Requests a new painting event to the advanced digitizing canvas item.
     */
    void paintRequested();

  private:

    QgsMapCanvas *mMapCanvas = nullptr;
    QPointer< QgsAdvancedDigitizingDockWidget > mCadDockWidget;
};

#ifndef SIP_RUN

/**
 * \ingroup gui
 * \brief A advanced digitizing tools to handle the selection of a point at the intersection
 * of two circles.
 * \since QGIS 3.40
 */
class GUI_EXPORT QgsAdvancedDigitizingCirclesIntersectionTool : public QgsAdvancedDigitizingTool
{
    Q_OBJECT

  public:

    /**
     * The advanced digitizing's circles intersection tool constructor.
     * \param canvas The map canvas on which the widget operates
     * \param cadDockWidget The cadDockWidget to which the floater belongs
     */
    explicit QgsAdvancedDigitizingCirclesIntersectionTool( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDockWidget );

    void paint( QPainter *painter ) override;

    bool canvasMoveEvent( QgsMapMouseEvent *event ) override;
    bool canvasReleaseEvent( QgsMapMouseEvent *event ) override;

  private:
    void processParameters();

    void drawCircle( QPainter *painter, double x, double y, double distance );
    void drawCandidate( QPainter *painter, double x, double y, bool closest );

    QToolButton *mCircle1Digitize = nullptr;
    QgsDoubleSpinBox *mCircle1X = nullptr;
    QgsDoubleSpinBox *mCircle1Y = nullptr;
    QgsDoubleSpinBox *mCircle1Distance = nullptr;

    QToolButton *mCircle2Digitize = nullptr;
    QgsDoubleSpinBox *mCircle2X = nullptr;
    QgsDoubleSpinBox *mCircle2Y = nullptr;
    QgsDoubleSpinBox *mCircle2Distance = nullptr;

    QgsPointXY mP1;
    QgsPointXY mP2;
    bool mP1Closest = false;
};

#endif

#endif // QGSADVANCEDDIGITIZINGTOOLS
