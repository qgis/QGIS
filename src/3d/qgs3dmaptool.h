/***************************************************************************
  qgs3dmaptool.h
  --------------------------------------
  Date                 : Sep 2018
  Copyright            : (C) 2018 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGS3DMAPTOOL_H
#define QGS3DMAPTOOL_H

#include <QObject>

#include "qgis_3d.h"

class Qgs3DMapCanvas;
class QMouseEvent;
class QKeyEvent;

#define SIP_NO_FILE


/**
 * \ingroup 3d
 * \brief Base class for map tools operating on 3D map canvas.
 * \note Not available in Python bindings
 * \since QGIS 3.36 (since QGIS 3.4 in QGIS_APP library)
 */
class _3D_EXPORT Qgs3DMapTool : public QObject
{
    Q_OBJECT

  public:
    //! Base constructor for a Qgs3DMapTool for the specified \a canvas
    Qgs3DMapTool( Qgs3DMapCanvas *canvas );

    //! Reimplement to handle mouse \a event forwarded by the parent Qgs3DMapCanvas
    virtual void mousePressEvent( QMouseEvent *event );
    //! Reimplement to handle mouse release \a event forwarded by the parent Qgs3DMapCanvas
    virtual void mouseReleaseEvent( QMouseEvent *event );
    //! Reimplement to handle mouse move \a event forwarded by the parent Qgs3DMapCanvas
    virtual void mouseMoveEvent( QMouseEvent *event );
    //! Reimplement to handle key press \a event forwarded by the parent Qgs3DMapCanvas
    virtual void keyPressEvent( QKeyEvent *event );

    //! Called when set as currently active map tool
    virtual void activate();

    //! Called when map tool is being deactivated
    virtual void deactivate();

    //! Mouse cursor to be used when the tool is active
    virtual QCursor cursor() const;

    /**
     * Whether the default mouse controls to zoom/pan/rotate camera can stay enabled
     * while the tool is active. This may be useful for some basic tools using just
     * mouse clicks (e.g. identify, measure), but it could be creating conflicts when used
     * with more advanced tools. Default implementation returns TRUE.
     */
    virtual bool allowsCameraControls() const { return true; }

    //! Returns the parent Qgs3DMapCanvas
    Qgs3DMapCanvas *canvas();

  private slots:
    //! Called when canvas's map setting is changed
    virtual void onMapSettingsChanged();

  protected:
    Qgs3DMapCanvas *mCanvas = nullptr;
};

#endif // QGS3DMAPTOOL_H
