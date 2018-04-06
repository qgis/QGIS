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
class QgsDistanceWidgetNew;

class QHBoxLayout;

class QgsDoubleSpinBox;
class QgsRubberBand;
class QgsSnapIndicator;
class QgisInterface;

class GUI_EXPORT QgsDistanceWidgetNew : public QWidget
{
    Q_OBJECT

  public:

    explicit QgsDistanceWidgetNew( const QString &label = QString(), QWidget *parent = nullptr );

    void setDistance( double distance );

    double distance();

    QgsDoubleSpinBox *editor() {return mDistanceSpinBox;}

  signals:
    void distanceChanged( double distance );
    void distanceEditingFinished( double distance, const Qt::KeyboardModifiers &modifiers );
    void addDistanceWidget();
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
    QgsMapToolSelectionHandler( QgsMapCanvas *canvas, QgisInterface *iface = nullptr );

    ~QgsMapToolSelectionHandler() override;

    bool mSelectFeatures = false;

    QgisInterface *mQgisInterface;

    void canvasMoveEvent( QgsMapMouseEvent *e );
    void canvasPressEvent( QgsMapMouseEvent *e );
    void canvasReleaseEvent( QgsMapMouseEvent *e );
    bool escapeSelection( QKeyEvent *e );

    //    void activate() override;
    void deactivate();

    void selectFeaturesMoveEvent( QgsMapMouseEvent *e );
    void selectFeaturesReleaseEvent( QgsMapMouseEvent *e );

    void selectPolygonMoveEvent( QgsMapMouseEvent *e );
    void selectPolygonReleaseEvent( QgsMapMouseEvent *e );

    void selectFreehandMoveEvent( QgsMapMouseEvent *e );
    void selectFreehandReleaseEvent( QgsMapMouseEvent *e );

    void selectRadiusMoveEvent( QgsMapMouseEvent *e );
    void selectRadiusReleaseEvent( QgsMapMouseEvent *e );

    void initRubberBand();

    void setIface( QgisInterface *iface );

    QgsGeometry selectedGeometry();
    void setSelectedGeometry( QgsGeometry geometry );

    void setSelectionMode( SelectionMode mode );
    SelectionMode selectionMode();

    //! to destinguish right click for finishing selection and identify extedned menu
    bool mJustFinishedSelection = false;

    bool mSelectionActive = false;

    std::unique_ptr< QgsRubberBand > mSelectionRubberBand;

    void selectFeaturesPressEvent( QgsMapMouseEvent *e );

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

  signals:
    void selectionGeometryChanged();
  protected:
    //! stores exact selection geometry
    QgsGeometry mSelectionGeometry;

  private:

    // Last point in canvas CRS
    QgsPointXY mLastPoint;

    double mLastMapUnitsPerPixel;

    int mCoordinatePrecision;

    QgsMapCanvas *mCanvas;


    //friend class QgisAppInterface;

    QPoint mInitDragPos;

    //! Flag to indicate a map canvas drag operation is taking place
    bool mDragging;

    SelectionMode mSelectionMode;

    //! Center point for the radius
    QgsPointXY mRadiusCenter;

    //! Shows current angle value and allows numerical editing
    QgsDistanceWidgetNew *mDistanceWidget = nullptr;

    QColor mFillColor;

    QColor mStrokeColor;



    QgsPointXY toMapCoordinates( QPoint point );

    void createRotationWidget();
    void deleteRotationWidget();

    void updateRadiusFromEdge( QgsPointXY &radiusEdge );

    //! perform selection using radius from rubberband
    void selectFromRubberband( const Qt::KeyboardModifiers &modifiers );

};

#endif
