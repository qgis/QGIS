/***************************************************************************
    qgsmaptoolzoom.h  -  map tool for zooming
    ----------------------
    begin                : January 2006
    copyright            : (C) 2006 by Martin Dobias
    email                : wonder.sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPTOOLZOOM_H
#define QGSMAPTOOLZOOM_H

#include "qgsmaptool.h"
#include <QRect>
#include "qgis_gui.h"

class QgsRubberBand;

/**
 * \ingroup gui
 * \brief A map tool for zooming into the map.
 * \see QgsMapTool
 */
class GUI_EXPORT QgsMapToolZoom : public QgsMapTool
{
    Q_OBJECT

  public:
    //! constructor
    QgsMapToolZoom( QgsMapCanvas *canvas, bool zoomOut );
    ~QgsMapToolZoom() override;

    Flags flags() const override;
    void canvasMoveEvent( QgsMapMouseEvent *e ) override;
    void canvasPressEvent( QgsMapMouseEvent *e ) override;
    void canvasReleaseEvent( QgsMapMouseEvent *e ) override;
    void keyPressEvent( QKeyEvent *e ) override;
    void keyReleaseEvent( QKeyEvent *e ) override;
    void deactivate() override;

  protected:
    //! stores actual zoom rect
    QRect mZoomRect;
    // minimum pixel size of diagonal of the zoom rectangle
    int mMinPixelZoom = 20;

    //! indicates whether we're zooming in or out
    bool mZoomOut;
    //! native tool
    bool mNativeZoomOut;

    //! Flag to indicate a map canvas drag operation is taking place
    bool mDragging;

    //! Flag to indicate the user has canceled the current zoom operation
    bool mCanceled = false;

    QgsRubberBand *mRubberBand = nullptr;

    QCursor mZoomOutCursor;
    QCursor mZoomInCursor;

  private:
    void setZoomMode( bool zoomOut, bool force = false );
};

#endif
