/***************************************************************************
    qgsmaptoolselectionhandler.cpp
    ---------------------
    begin                : March 2018
    copyright            : (C) 2018 by Viktor Sklencar
    email                : vsklencar at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPTOOLSELECTIONHANDLER_H
#define QGSMAPTOOLSELECTIONHANDLER_H

#include <QObject>
#include <QWidget>

#include "qgis_gui.h"
#include "qgsgeometry.h"

class QHBoxLayout;
class QKeyEvent;

class QgisInterface;
class QgsDoubleSpinBox;
class QgsMapCanvas;
class QgsMapMouseEvent;
class QgsRubberBand;

#ifndef SIP_RUN

/// @cond private

/**
 * \ingroup gui
 * \class QgsDistanceWidget
 * \brief Spinner widget for radius selection.
 *
 * \since QGIS 3.2
 */
class QgsDistanceWidget : public QWidget
{
    Q_OBJECT

  public:

    //! Constrructor
    explicit QgsDistanceWidget( const QString &label = QString(), QWidget *parent = nullptr );

    //! distance setter
    void setDistance( double distance );
    //! distance getter
    double distance();

  signals:
    //! distance changed signal
    void distanceChanged( double distance );
    //! distanceEditingFinished signal
    void distanceEditingFinished( double distance, const Qt::KeyboardModifiers &modifiers );
    //! distanceEditingCanceled signal
    void distanceEditingCanceled();

  protected:
    virtual bool eventFilter( QObject *obj, QEvent *ev ) override;

  private:
    QHBoxLayout *mLayout = nullptr;
    QgsDoubleSpinBox *mDistanceSpinBox = nullptr;
};

/// @endcond

#endif


/**
 * \ingroup gui
 * \brief Utility class for handling various methods to create geometry for selection in layers.
 * \since QGIS 3.2
 */
class GUI_EXPORT QgsMapToolSelectionHandler : public QObject
{
    Q_OBJECT

  public:

    //! Select features to identify by:
    enum SelectionMode
    {
      //! SelectSimple - single click or drawing a rectangle, default option
      SelectSimple,
      //! SelectPolygon - drawing a polygon
      SelectPolygon,
      //! SelectFreehand - free hand selection
      SelectFreehand,
      //! SelectRadius - a circle selection
      SelectRadius
    };
    Q_ENUM( SelectionMode )

    //! constructor
    QgsMapToolSelectionHandler( QgsMapCanvas *canvas,
                                QgsMapToolSelectionHandler::SelectionMode selectionMode = QgsMapToolSelectionHandler::SelectionMode::SelectSimple,
                                QgisInterface *iface = nullptr );

    //! desctructor
    ~QgsMapToolSelectionHandler() override;

    //! mSelectedGeometry getter
    QgsGeometry selectedGeometry();
    //! mSelectedGeometry setter
    void setSelectedGeometry( const QgsGeometry &geometry, Qt::KeyboardModifiers modifiers = Qt::NoModifier );

    //! mSelectionMode getter
    SelectionMode selectionMode();
    //! mSelectionMode setter
    void setSelectionMode( SelectionMode mode );

    //! Deactivates handler (when map tool gets deactivated)
    void deactivate();

    //! Overridden mouse move event
    void canvasMoveEvent( QgsMapMouseEvent *e );
    //! Overridden mouse press event
    void canvasPressEvent( QgsMapMouseEvent *e );
    //! Overridden mouse releasae event
    void canvasReleaseEvent( QgsMapMouseEvent *e );
    //! Cancel selection - handles escape press event
    bool keyReleaseEvent( QKeyEvent *e );

  signals:
    //! emitted when mSelectedGeometry has been changed
    void geometryChanged( Qt::KeyboardModifiers modifiers = Qt::NoModifier );

  private slots:
    //! update the rubber band from the input widget
    void updateRubberband( const double &radius );

    /**
     * triggered when the user input widget has a new value
     * either programmatically (from the mouse event) or entered by the user
     */
    void radiusValueEntered( const double &radius, const Qt::KeyboardModifiers &modifiers );

    //! cancel selecting (between two click events)
    void cancel();

  private:

    //! Mouse move event handling for simple selection
    void selectFeaturesMoveEvent( QgsMapMouseEvent *e );
    //! Mouse release event handling for simple selection
    void selectFeaturesReleaseEvent( QgsMapMouseEvent *e );
    //! Mouse press event handling for simple selection
    void selectFeaturesPressEvent();

    //! Mouse move event handling for polygon selection
    void selectPolygonMoveEvent( QgsMapMouseEvent *e );
    //! Mouse press event handling for polygon selection
    void selectPolygonPressEvent( QgsMapMouseEvent *e );

    //! Mouse move event handling for freehand selection
    void selectFreehandMoveEvent( QgsMapMouseEvent *e );
    //! Mouse press event handling for freehand selection
    void selectFreehandReleaseEvent( QgsMapMouseEvent *e );

    //! Mouse move event handling for radius selection
    void selectRadiusMoveEvent( QgsMapMouseEvent *e );
    //! Mouse press event handling for radius selection
    void selectRadiusReleaseEvent( QgsMapMouseEvent *e );

    //! Initialization of the rubberband
    void initRubberBand();

    QgsPointXY toMapCoordinates( QPoint point );

    void createDistanceWidget();
    void deleteDistanceWidget();

    void updateRadiusFromEdge( QgsPointXY &radiusEdge );

  private:

    QgisInterface *mQgisInterface = nullptr;

    //! the rubberband for selection visualization
    QgsRubberBand *mSelectionRubberBand = nullptr;

    //! stores exact selection geometry
    QgsGeometry mSelectionGeometry;

    bool mSelectionActive = false;

    bool mSelectFeatures = false;

    SelectionMode mSelectionMode;

    QgsMapCanvas *mCanvas = nullptr;

    QPoint mInitDragPos;

    //! Center point for the radius
    QgsPointXY mRadiusCenter;

    //! Shows current angle value and allows numerical editing
    QgsDistanceWidget *mDistanceWidget = nullptr;

    QColor mFillColor;

    QColor mStrokeColor;

    //! to destinguish right click for finishing selection and identify extedned menu
    bool mJustFinishedSelection = false;

};

#endif
