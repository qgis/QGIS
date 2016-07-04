/***************************************************************************
    qgsmapcanvassnappingutils.h
    ---------------------
    begin                : December 2014
    copyright            : (C) 2014 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSMAPCANVASSNAPPINGUTILS_H
#define QGSMAPCANVASSNAPPINGUTILS_H

#include "qgssnappingutils.h"

class QgsMapCanvas;

class QProgressDialog;

/** \ingroup gui
 * Snapping utils instance that is connected to a canvas and updates the configuration
 *  (map settings + current layer) whenever that is changed in the canvas.
 *  @note added in 2.8
 */
class GUI_EXPORT QgsMapCanvasSnappingUtils : public QgsSnappingUtils
{
    Q_OBJECT
  public:
    QgsMapCanvasSnappingUtils( QgsMapCanvas* canvas, QObject* parent = nullptr );

  protected:
    virtual void prepareIndexStarting( int count ) override;
    virtual void prepareIndexProgress( int index ) override;

  private slots:
    void canvasMapSettingsChanged();
    void canvasCurrentLayerChanged();

  private:
    QgsMapCanvas* mCanvas;
    QProgressDialog* mProgress;
};


#endif // QGSMAPCANVASSNAPPINGUTILS_H
