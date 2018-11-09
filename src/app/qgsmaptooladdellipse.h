/***************************************************************************
    qgsmaptooladdellipse.h  -  map tool for adding ellipse
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

#ifndef QGSMAPTOOLADDELLIPSE_H
#define QGSMAPTOOLADDELLIPSE_H

#include "qgsmaptoolcapture.h"
#include "qgsellipse.h"
#include "qgssettings.h"
#include "qgis_app.h"

class QgsGeometryRubberBand;
class QgsSnapIndicator;

class APP_EXPORT QgsMapToolAddEllipse: public QgsMapToolCapture
{
    Q_OBJECT
  public:
    QgsMapToolAddEllipse( QgsMapToolCapture *parentTool, QgsMapCanvas *canvas, CaptureMode mode = CaptureLine );
    ~QgsMapToolAddEllipse() override;

    void keyPressEvent( QKeyEvent *e ) override;
    void keyReleaseEvent( QKeyEvent *e ) override;

    void deactivate() override;

    void activate() override;
    void clean() override;

  protected:
    explicit QgsMapToolAddEllipse( QgsMapCanvas *canvas ) = delete; //forbidden

    /**
     * The parent map tool, e.g. the add feature tool.
     *  Completed ellipse will be added to this tool by calling its toLineString() method.
     **/
    QgsMapToolCapture *mParentTool = nullptr;
    //! Ellipse points (in map coordinates)
    QgsPointSequence mPoints;
    //! The rubberband to show the ellipse currently working on
    QgsGeometryRubberBand *mTempRubberBand = nullptr;
    //! Ellipse
    QgsEllipse mEllipse;
    //! convenient method to return the number of segments
    unsigned int segments( ) { return QgsSettings().value( QStringLiteral( "/qgis/digitizing/offset_quad_seg" ), 8 ).toInt() * 12; }
    //! Layer type which will be used for rubberband
    QgsWkbTypes::GeometryType mLayerType = QgsWkbTypes::LineGeometry;

    //! Snapping indicators
    std::unique_ptr<QgsSnapIndicator> mSnapIndicator;

};

#endif // QGSMAPTOOLADDELLIPSE_H
