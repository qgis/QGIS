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
class GUI_EXPORT QgsAdvancedDigitizingTool : public QObject
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
     * Returns a widget to control the tool.
     * \note The caller gets the ownership.
     */
    virtual QWidget *createWidget() { return nullptr; }

    /**
     * Paints tool content onto the advanced digitizing canvas item.
     */
    virtual void paint( QPainter *painter ) { Q_UNUSED( painter ) }

    /**
     * Handles canvas press event.
     * \note To stop propagation, set the event's accepted property to FALSE.
     */
    virtual void canvasPressEvent( QgsMapMouseEvent *event )
    {
      Q_UNUSED( event )
    }

    /**
     * Handles canvas press move.
     * \note To stop propagation, set the event's accepted property to FALSE.
     */
    virtual void canvasMoveEvent( QgsMapMouseEvent *event )
    {
      Q_UNUSED( event )
    }

    /**
     * Handles canvas release event.
     * \note To stop propagation, set the event's accepted property to FALSE.
     */
    virtual void canvasReleaseEvent( QgsMapMouseEvent *event )
    {
      Q_UNUSED( event )
    }

  signals:

    /**
     * Requests a new painting event to the advanced digitizing canvas item.
     */
    void paintRequested();

  protected:
    QgsMapCanvas *mMapCanvas = nullptr;
    QPointer<QgsAdvancedDigitizingDockWidget> mCadDockWidget;
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
    ~QgsAdvancedDigitizingCirclesIntersectionTool();

    QWidget *createWidget() override;
    void paint( QPainter *painter ) override;

    void canvasMoveEvent( QgsMapMouseEvent *event ) override;
    void canvasReleaseEvent( QgsMapMouseEvent *event ) override;

  private:
    bool eventFilter( QObject *obj, QEvent *event ) override;

    void processParameters();

    void drawCircle( QPainter *painter, double x, double y, double distance );
    void drawCandidate( QPainter *painter, double x, double y, bool closest );

    QPointer<QWidget> mToolWidget;
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
