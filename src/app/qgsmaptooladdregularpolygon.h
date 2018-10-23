/***************************************************************************
    qgsmaptooladdregularpolygon.h  -  map tool for adding regular polygon
    ---------------------
    begin                : July 2017
    copyright            : (C) 2017
    email                : lbartoletti at tuxfamily dot org
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPTOOLADDREGULARPOLYGON_H
#define QGSMAPTOOLADDREGULARPOLYGON_H

#include "qgsregularpolygon.h"
#include "qgsmaptoolcapture.h"
#include "qgsspinbox.h"
#include "qgis_app.h"

class QSpinBox;
class QgsSnapIndicator;

class APP_EXPORT QgsMapToolAddRegularPolygon: public QgsMapToolCapture
{
    Q_OBJECT

  public:
    QgsMapToolAddRegularPolygon( QgsMapToolCapture *parentTool, QgsMapCanvas *canvas, CaptureMode mode = CaptureLine );
    ~QgsMapToolAddRegularPolygon() override;

    void keyPressEvent( QKeyEvent *e ) override;
    void keyReleaseEvent( QKeyEvent *e ) override;

    void deactivate() override;

    void activate() override;
    void clean() override;

  protected:
    explicit QgsMapToolAddRegularPolygon( QgsMapCanvas *canvas ) = delete; //forbidden

    std::unique_ptr<QgsSpinBox> mNumberSidesSpinBox;
    int mNumberSides = 6;

    //! (re-)create the spin box to enter the number of sides
    void createNumberSidesSpinBox();
    //! delete the spin box to enter the number of sides, if it exists
    void deleteNumberSidesSpinBox();

    /**
     * The parent map tool, e.g. the add feature tool.
     *  Completed regular polygon will be added to this tool by calling its addCurve() method.
     **/
    QgsMapToolCapture *mParentTool = nullptr;
    //! Regular Shape points (in map coordinates)
    QgsPointSequence mPoints;
    //! The rubberband to show the regular polygon currently working on
    QgsGeometryRubberBand *mTempRubberBand = nullptr;
    //! Regular shape as a regular polygon
    QgsRegularPolygon mRegularPolygon;

    //! Layer type which will be used for rubberband
    QgsWkbTypes::GeometryType mLayerType = QgsWkbTypes::LineGeometry;

    //! Snapping indicators
    std::unique_ptr<QgsSnapIndicator> mSnapIndicator;

};

#endif // QGSMAPTOOLADDREGULARPOLYGON_H
