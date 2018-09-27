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

class Qgs3DMapCanvas;
class QMouseEvent;

/**
 * Base class for map tools operating on 3D map canvas.
 */
class Qgs3DMapTool : public QObject
{
    Q_OBJECT

  public:
    Qgs3DMapTool( Qgs3DMapCanvas *canvas );

    virtual void mousePressEvent( QMouseEvent *event );
    virtual void mouseReleaseEvent( QMouseEvent *event );
    virtual void mouseMoveEvent( QMouseEvent *event );

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
     * with more advanced tools. Default implementation returns true.
     */
    virtual bool allowsCameraControls() const { return true; }

  protected:
    Qgs3DMapCanvas *mCanvas = nullptr;
};

#endif // QGS3DMAPTOOL_H
