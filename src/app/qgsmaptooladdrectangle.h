/***************************************************************************
    qgsmaptooladdrectangle.h  -  map tool for adding rectangle
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

#ifndef QGSMAPTOOLADDRECTANGLE_H
#define QGSMAPTOOLADDRECTANGLE_H

#include "qgspolygon.h"
#include "qgsmaptoolcapture.h"
#include "qgsquadrilateral.h"
#include "qgis_app.h"

class QgsPolygon;
class QgsSnapIndicator;

class APP_EXPORT QgsMapToolAddRectangle: public QgsMapToolCapture
{
    Q_OBJECT

  public:
    QgsMapToolAddRectangle( QgsMapToolCapture *parentTool, QgsMapCanvas *canvas, CaptureMode mode = CaptureLine );
    ~QgsMapToolAddRectangle() override;

    void keyPressEvent( QKeyEvent *e ) override;
    void keyReleaseEvent( QKeyEvent *e ) override;

    void deactivate( ) override;

    void activate() override;
    void clean() override;

  protected:
    explicit QgsMapToolAddRectangle( QgsMapCanvas *canvas ) = delete; //forbidden

    /**
     * The parent map tool, e.g. the add feature tool.
     *  Completed regular shape will be added to this tool by calling its addCurve() method.
     **/
    QgsMapToolCapture *mParentTool = nullptr;
    //! Regular Shape points (in map coordinates)
    QgsPointSequence mPoints;
    //! The rubberband to show the rectangle currently working on
    QgsGeometryRubberBand *mTempRubberBand = nullptr;
    //! Rectangle
    QgsQuadrilateral mRectangle;

    //! Layer type which will be used for rubberband
    QgsWkbTypes::GeometryType mLayerType = QgsWkbTypes::LineGeometry;

    //! Snapping indicators
    std::unique_ptr<QgsSnapIndicator> mSnapIndicator;
};

#endif // QGSMAPTOOLADDRECTANGLE_H
