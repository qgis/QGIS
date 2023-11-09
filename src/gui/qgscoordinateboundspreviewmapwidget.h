/***************************************************************************
 *   qgscoordinateboundspreviewmapwidget.h                                 *
 *   Copyright (C) 2019 by Nyall Dawson                                    *
 *   nyall dot dawson at gmail dot com                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#ifndef QGSCOORDINATEBOUNDSPREVIEWWIDGET_H
#define QGSCOORDINATEBOUNDSPREVIEWWIDGET_H

#include "qgsmapcanvas.h"
#include "qgis_sip.h"
#include "qgis_gui.h"

class QgsVertexMarker;
class QgsMapToolPan;

/**
 * \class QgsCoordinateBoundsPreviewMapWidget
 * \ingroup gui
 * \brief A widget for showing the bounds of a rectangular region on an interactive map.
 * \since QGIS 3.8.1
 */

class GUI_EXPORT QgsCoordinateBoundsPreviewMapWidget : public QgsMapCanvas
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsCoordinateBoundsPreviewMapWidget.
     */
    QgsCoordinateBoundsPreviewMapWidget( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    ~QgsCoordinateBoundsPreviewMapWidget() override;

    /**
     * Sets the canvas bounds rectangle for the bounds overview map.
     *
     * Must be in EPSG:4326 coordinate reference system.
     * \see canvasRect()
     */
    void setCanvasRect( const QgsRectangle &rect );

    /**
     * Returns the current canvas bounds rectangle shown in the map.
     * \see setCanvasRect()
     */
    QgsRectangle canvasRect() const;

    /**
     * Sets the "preview" rectangle for the bounds overview map.
     * Must be in EPSG:4326 coordinate reference system.
     */
    void setPreviewRect( const QgsRectangle &rect );

  private:

    QgsRubberBand *mPreviewBand = nullptr;
    QgsRubberBand *mCanvasPreviewBand = nullptr;
    QgsVertexMarker *mCanvasCenterMarker = nullptr;
    QgsMapToolPan *mPanTool = nullptr;

    QList<QgsMapLayer *> mLayers;

    QgsRectangle mCanvasRect;

};

#endif // QGSCOORDINATEBOUNDSPREVIEWWIDGET_H
