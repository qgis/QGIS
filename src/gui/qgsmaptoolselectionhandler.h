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

/**
 * \ingroup gui
  \brief Map tool for selecting geometry in layers
*/
class GUI_EXPORT QgsMapToolSelectionHandler:public QObject
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
    QgsMapToolSelectionHandler( QgsMapCanvas *canvas );

    ~QgsMapToolSelectionHandler() override;

    void canvasMoveEvent( QgsMapMouseEvent *e );
    void canvasPressEvent( QgsMapMouseEvent *e, QgsMapToolSelectionHandler::SelectionMode selectionMode );
    void canvasReleaseEvent( QgsMapMouseEvent *e );
    //    void activate() override;
    //    void deactivate() override;

    void selectFeaturesMoveEvent( QgsMapMouseEvent *e );
    void selectFeaturesReleaseEvent( QgsMapMouseEvent *e );

    void selectPolygonMoveEvent( QgsMapMouseEvent *e );
    void selectPolygonReleaseEvent( QgsMapMouseEvent *e );

    void selectFreehandMoveEvent( QgsMapMouseEvent *e );
    void selectFreehandReleaseEvent( QgsMapMouseEvent *e );

    void selectRadiusMoveEvent( QgsMapMouseEvent *e );
    void selectRadiusReleaseEvent( QgsMapMouseEvent *e );

    void initRubberBand();

    QgsGeometry selectedGeometry();
    void setSelectedGeometry(QgsGeometry geometry);

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

    QPoint mInitDragPos;

    //! Flag to indicate a map canvas drag operation is taking place
    bool mDragging;

    bool mSelectionActive = false;

    SelectionMode mSelectionMode;

    //! Center point for the radius
    QgsPointXY mRadiusCenter;

    std::unique_ptr< QgsRubberBand > mSelectionRubberBand;

    QColor mFillColor;

    QColor mStrokeColor;



    QgsPointXY toMapCoordinates( QPoint point );

    //    void keyReleaseEvent( QKeyEvent *e );

    //    void createRotationWidget();
    //    void deleteRotationWidget();

    void setRubberBand( QgsMapCanvas *canvas, QRect &selectRect, QgsRubberBand *rubberBand );

    void updateRadiusFromEdge( QgsPointXY &radiusEdge );

};

#endif
