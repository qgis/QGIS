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
#include "qgis_gui.h"
#include "qobjectuniqueptr.h"

class QgsMapCanvas;


/**
 * \ingroup gui
 * \brief A map tool that stores clipping planes from lines drawn onto map canvas.
 */
class GUI_EXPORT QgsMapToolClippingPlanes : public QgsMapTool
{
    Q_OBJECT

  public:
    QgsMapToolClippingPlanes( QgsMapCanvas *canvas );

    Flags flags() const override { return AllowZoomRect; }
    void canvasMoveEvent( QgsMapMouseEvent *e ) override;
    void canvasPressEvent( QgsMapMouseEvent *e ) override;
    void canvasReleaseEvent( QgsMapMouseEvent *e ) override;
    void activate() override;
    void keyReleaseEvent( QKeyEvent *e ) override;
    void deactivate() override;

    //! Removes the tool's rubber band from the canvas.
    void clearRubberBand();

  signals:
    //! signal emitted on clipping planes change
    void clippingPlanesChanged( QVector<QPair<QgsVector3D, QgsVector3D>> normalVectors );

  private:
    void calculateClippingPlanes();

    QObjectUniquePtr<QgsRubberBand> mRubberBandLines;
    QObjectUniquePtr<QgsRubberBand> mRubberBandPoints;
    QVector< QgsPointXY > mPoints;
    bool mClicked = false;
};

#endif //QGSMAPTOOLCLIPPINGPLANES_H
