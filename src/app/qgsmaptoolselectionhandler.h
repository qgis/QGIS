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
#include <QTimer>

#include "qgsgeometry.h"

class QHBoxLayout;
class QKeyEvent;

class QgsDoubleSpinBox;
class QgsMapCanvas;
class QgsMapMouseEvent;
class QgsRubberBand;
class QgsSnapIndicator;
class QgsIdentifyMenu;

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
    bool eventFilter( QObject *obj, QEvent *ev ) override;

  private:
    QHBoxLayout *mLayout = nullptr;
    QgsDoubleSpinBox *mDistanceSpinBox = nullptr;
};

/// @endcond


/**
 * \ingroup gui
 * \brief Utility class for handling various methods to create geometry for selection in layers.
 * \since QGIS 3.2
 */
class QgsMapToolSelectionHandler : public QObject
{
    Q_OBJECT

  public:

    //! Select features to identify by:
    enum SelectionMode
    {
      //! SelectSimple - single click or drawing a rectangle, default option
      SelectSimple,
      //! SelectPolygon - drawing a polygon or right-click on existing polygon feature
      SelectPolygon,
      //! SelectFreehand - free hand selection
      SelectFreehand,
      //! SelectRadius - a circle selection
      SelectRadius,

      /**
       * SelectOnMouseMove - selection on mouse over
       * \since QGIS 3.30
       */
      SelectOnMouseOver
    };
    Q_ENUM( SelectionMode )

    //! constructor
    QgsMapToolSelectionHandler( QgsMapCanvas *canvas,
                                QgsMapToolSelectionHandler::SelectionMode selectionMode = QgsMapToolSelectionHandler::SelectionMode::SelectSimple );

    //! destructor
    ~QgsMapToolSelectionHandler() override;

    //! Returns most recently selected geometry (may be a point or a polygon)
    QgsGeometry selectedGeometry() const;

    //! Sets the current selection mode
    SelectionMode selectionMode() const;
    //! Returns the current selection mode
    void setSelectionMode( SelectionMode mode );

    //! Deactivates handler (when map tool gets deactivated)
    void deactivate();

    //! Handles mouse move event from map tool
    void canvasMoveEvent( QgsMapMouseEvent *e );
    //! Handles mouse press event from map tool
    void canvasPressEvent( QgsMapMouseEvent *e );
    //! Handles mouse release event from map tool
    void canvasReleaseEvent( QgsMapMouseEvent *e );
    //! Handles escape press event - returns true if the even has been processed
    bool keyReleaseEvent( QKeyEvent *e );

    void setSelectedGeometry( const QgsGeometry &geometry, Qt::KeyboardModifiers modifiers = Qt::NoModifier );

  signals:
    //! emitted when a new geometry has been picked (selectedGeometry())
    void geometryChanged( Qt::KeyboardModifiers modifiers = Qt::NoModifier );

  private slots:
    //! update the rubber band from the input widget
    void updateRadiusRubberband( double radius );

    /**
     * triggered when the user input widget has a new value
     * either programmatically (from the mouse event) or entered by the user
     */
    void radiusValueEntered( double radius, Qt::KeyboardModifiers modifiers );

    //! cancel selecting (between two click events)
    void cancel();

  private:

    void selectFeaturesMoveEvent( QgsMapMouseEvent *e );
    void selectFeaturesReleaseEvent( QgsMapMouseEvent *e );
    void selectFeaturesPressEvent( QgsMapMouseEvent *e );

    void selectPolygonMoveEvent( QgsMapMouseEvent *e );
    void selectPolygonPressEvent( QgsMapMouseEvent *e );

    void selectFreehandMoveEvent( QgsMapMouseEvent *e );
    void selectFreehandReleaseEvent( QgsMapMouseEvent *e );

    void selectRadiusMoveEvent( QgsMapMouseEvent *e );
    void selectRadiusReleaseEvent( QgsMapMouseEvent *e );

    void initRubberBand();

    QgsPointXY toMapCoordinates( QPoint point );

    void createDistanceWidget();
    void deleteDistanceWidget();

    void updateRadiusFromEdge( QgsPointXY &radiusEdge );

  private:

    QgsMapCanvas *mCanvas = nullptr;

    //! the rubberband for selection visualization
    std::unique_ptr<QgsRubberBand> mSelectionRubberBand;

    //! Stores the most recent geometry (in map coordinates)
    QgsGeometry mSelectionGeometry;

    //! Whether a geometry is being picked right now
    bool mSelectionActive = false;

    SelectionMode mSelectionMode = SelectSimple;

    //! Snap indicator used in radius selection mode
    std::unique_ptr<QgsSnapIndicator> mSnapIndicator;

    //! Initial point (in screen coordinates) when using simple selection
    QPoint mInitDragPos;

    //! Center point for the radius when using radius selection
    QgsPointXY mRadiusCenter;

    //! Shows current angle value and allows numerical editing
    QgsDistanceWidget *mDistanceWidget = nullptr;

    QColor mFillColor = QColor( 254, 178, 76, 63 );
    QColor mStrokeColor = QColor( 254, 58, 29, 100 );

    //! Shows features to select polygon from existing features
    QgsIdentifyMenu *mIdentifyMenu = nullptr; // owned by canvas

    //! Delay timer for continuous selection mode
    std::unique_ptr<QTimer> mOnMouseMoveDelayTimer;

    //! Last cursor position after a move event, used by continuous selection mode
    QPoint mMoveLastCursorPos;
};

#endif
