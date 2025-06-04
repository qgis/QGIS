/***************************************************************************
    qgsmaptoolclippingplanes.h
    ---------------------
    begin                : March 2025
    copyright            : (C) 2025 by Matej Bagar
    email                : matej dot bagar at lutraconsulting dot co dot uk
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPTOOLCLIPPINGPLANES_H
#define QGSMAPTOOLCLIPPINGPLANES_H

#include "qgsmaptool.h"
#include "qgspointxy.h"
#include "qgsrubberband.h"
#include "qobjectuniqueptr.h"


class Qgs3DMapCanvasWidget;
class QgsMapCanvas;


/**
 * \ingroup app
 * \brief A map tool that defines clipping planes from rectangle drawn onto map canvas.
 */
class QgsMapToolClippingPlanes : public QgsMapTool
{
    Q_OBJECT

  public:
    QgsMapToolClippingPlanes( QgsMapCanvas *canvas, Qgs3DMapCanvasWidget *mapCanvas );

    Flags flags() const override { return AllowZoomRect; }
    void canvasMoveEvent( QgsMapMouseEvent *e ) override;
    void canvasPressEvent( QgsMapMouseEvent *e ) override;
    void canvasReleaseEvent( QgsMapMouseEvent *e ) override;
    void activate() override;
    void keyReleaseEvent( QKeyEvent *e ) override;
    void deactivate() override;

    //! Removes the tool's rubber band from the canvas.
    void clear() const;
    //! Removes the tool's rubber band from canvas, which highlights the cross-section.
    void clearHighLightedArea() const;
    //! Returns the Geometry of clipped area
    QgsGeometry clippedPolygon() const;

  private:
    void clearRubberBand() const;

    QObjectUniquePtr<QgsRubberBand> mRubberBandPolygon;
    QObjectUniquePtr<QgsRubberBand> mRubberBandLines;
    QObjectUniquePtr<QgsRubberBand> mRubberBandPoints;
    Qgs3DMapCanvasWidget *m3DCanvas = nullptr;
    bool mClicked = false;
    double mRectangleWidth = 0;
};

#endif //QGSMAPTOOLCLIPPINGPLANES_H
