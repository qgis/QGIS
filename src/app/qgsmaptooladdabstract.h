/***************************************************************************
    qgsmaptooladdabstract.h  -  abstract class for map tools of the 'add' kind
    ---------------------
    begin                : May 2017
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

#ifndef QGSMAPTOOLADDABSTRACT_H
#define QGSMAPTOOLADDABSTRACT_H

#include "qgsmaptoolcapture.h"
#include "qgsellipse.h"
#include "qgssettingsregistrycore.h"
#include "qgis_app.h"

class QgsGeometryRubberBand;
class QgsSnapIndicator;

class APP_EXPORT QgsMapToolAddAbstract: public QgsMapToolCapture
{
    Q_OBJECT
  public:
    QgsMapToolAddAbstract( QgsMapToolCapture *parentTool, QgsMapCanvas *canvas, CaptureMode mode = CaptureLine );
    ~QgsMapToolAddAbstract() override;

    void keyPressEvent( QKeyEvent *e ) override;
    void keyReleaseEvent( QKeyEvent *e ) override;

    void deactivate() override = 0;

    void activate() override;
    void clean() override;

  protected:
    explicit QgsMapToolAddAbstract( QgsMapCanvas *canvas ) = delete; //forbidden

    //! Convenient method to release (activate/deactivate) tools
    void release( QgsMapMouseEvent *e );

    /**
     * The parent map tool, e.g. the add feature tool.
     * Completed geometry will be added to this tool by calling its toLineString() method.
     */
    QPointer<QgsMapToolCapture> mParentTool;
    //! Ellipse points (in map coordinates)
    QgsPointSequence mPoints;
    //! The rubberband to show the geometry currently working on
    QgsGeometryRubberBand *mTempRubberBand = nullptr;

    //! Layer type which will be used for rubberband
    QgsWkbTypes::GeometryType mLayerType = QgsWkbTypes::LineGeometry;

    //! Snapping indicators
    std::unique_ptr<QgsSnapIndicator> mSnapIndicator;

};

#endif // QGSMAPTOOLADDABSTRACT_H
