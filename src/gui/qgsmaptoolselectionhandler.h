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

#include "qgsfeature.h"
#include "qgsfields.h"
#include "qgsmaptool.h"
#include "qgspointxy.h"
#include "qgspolygon.h"
#include "qgsunittypes.h"
#include "qgsrubberband.h"

#include <QObject>
#include <QPointer>
#include "qgis_gui.h"

class QgsRasterLayer;
class QgsVectorLayer;
class QgsMapLayer;
class QgsMapCanvas;
class QgsHighlight;
class QgsDistanceArea;
class QgsDistanceWidget;

class QHBoxLayout;

class QgsDoubleSpinBox;
class QgsRubberBand;
class QgsSnapIndicator;
class QgisInterface;

/**
 * \ingroup gui
  \brief Widget used in addition with radius selection
  \since QGIS 3.2"
*/
class GUI_EXPORT QgsDistanceWidget : public QWidget
{
    Q_OBJECT

  public:

    //! Constrructor
    explicit QgsDistanceWidget( const QString &label = QString(), QWidget *parent = nullptr );

    //! distance setter
    void setDistance( double distance );
    //! distance getter
    double distance();

    //! editor getter
    QgsDoubleSpinBox *editor() {return mDistanceSpinBox;}

  signals:
    //! distance changed signal
    void distanceChanged( double distance );
    //! distanceEditingFinished signal
    void distanceEditingFinished( double distance, const Qt::KeyboardModifiers &modifiers );
    //! addDistanceWidget signal
    void addDistanceWidget();
    //! distanceEditingCanceled signal
    void distanceEditingCanceled();

  protected:
    bool eventFilter( QObject *obj, QEvent *ev ) override;

  private slots:
    void distanceSpinBoxValueChanged( double distance );

  private:
    QHBoxLayout *mLayout = nullptr;
    QgsDoubleSpinBox *mDistanceSpinBox = nullptr;
};



/**
 * \ingroup gui
  \brief Map tool for selecting geometry in layers
  \since QGIS 3.2"
*/
class GUI_EXPORT QgsMapToolSelectionHandler: public QObject
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
    QgsMapToolSelectionHandler( QgsMapCanvas *canvas, QgsMapToolSelectionHandler::SelectionMode getSelectionMode = QgsMapToolSelectionHandler::SelectionMode::SelectSimple );

    //! desctructor
    ~QgsMapToolSelectionHandler() override;

    QgisInterface *mQgisInterface = nullptr;

    //! Overridden mouse move event
    void canvasMoveEvent( QgsMapMouseEvent *e );
    //! Overridden mouse press event
    void canvasPressEvent( QgsMapMouseEvent *e );
    //! Overridden mouse releasae event
    void canvasReleaseEvent( QgsMapMouseEvent *e );
    //! Cancel selection - handles escape press event
    bool escapeSelection( QKeyEvent *e );

    //! To deactivate handler often within map tool deactivation.
    void deactivate();

    //! Mouse move event handling for simple selection
    void selectFeaturesMoveEvent( QgsMapMouseEvent *e );
    //! Mouse release event handling for simple selection
    void selectFeaturesReleaseEvent( QgsMapMouseEvent *e );
    //! Mouse press event handling for simple selection
    void selectFeaturesPressEvent( QgsMapMouseEvent *e );

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

    //! Interface variable setter - needed for messages
    void setIface( QgisInterface *iface );

    //! mSelectedGeometry getter
    QgsGeometry getSelectedGeometry();
    //! mSelectedGeometry setter
    void setSelectedGeometry( QgsGeometry geometry, Qt::KeyboardModifiers modifiers = Qt::NoModifier );

    //! mSelectionMode getter
    SelectionMode getSelectionMode();
    //! mSelectionMode setter
    void setSelectionMode( SelectionMode mode );

    //! mSelectionActive getter
    bool getSelectionActive();

    //! mSelectionRubberBand getter
    QgsRubberBand *getSelectionRubberBand();
    //! mSelectionRubberBand setter
    void setSelectionRubberBand( QgsRubberBand *selectionRubberBand );

    //! mJustFinishedSelection getter
    bool getJustFinishedSelection() const;
    //! mJustFinishedSelection setter
    void setJustFinishedSelection( bool justFinishedSelection );

    //! mRadiusCenter getter
    QgsPointXY getRadiusCenter() const;

    QPoint getInitDragPos() const;

signals:
    //! emited when mSelectedGeometry has been changed
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

    //! the rubberband for selection visualization
    QgsRubberBand *mSelectionRubberBand = nullptr;

    //! stores exact selection geometry
    QgsGeometry mSelectionGeometry;

    bool mSelectionActive = false;

    bool mSelectFeatures = false;

    SelectionMode mSelectionMode;

    QgsMapCanvas *mCanvas;

    QPoint mInitDragPos;

    //! Center point for the radius
    QgsPointXY mRadiusCenter;

    //! Shows current angle value and allows numerical editing
    QgsDistanceWidget *mDistanceWidget = nullptr;

    QColor mFillColor;

    QColor mStrokeColor;

    //! to destinguish right click for finishing selection and identify extedned menu
    bool mJustFinishedSelection = false;

    QgsPointXY toMapCoordinates( QPoint point );

    void createRotationWidget();
    void deleteRotationWidget();

    void updateRadiusFromEdge( QgsPointXY &radiusEdge );

    //! perform selection using radius from rubberband
    void selectFromRubberband( const Qt::KeyboardModifiers &modifiers );

};

#endif
