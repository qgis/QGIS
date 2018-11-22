/***************************************************************************
  qgs3dmaptool.cpp
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

#include "qgs3dmaptool.h"

#include "qgs3dmapcanvas.h"

Qgs3DMapTool::Qgs3DMapTool( Qgs3DMapCanvas *canvas )
  : QObject( canvas )
  , mCanvas( canvas )
{
}

void Qgs3DMapTool::mousePressEvent( QMouseEvent *event )
{
  Q_UNUSED( event );
}

void Qgs3DMapTool::mouseReleaseEvent( QMouseEvent *event )
{
  Q_UNUSED( event );
}

void Qgs3DMapTool::mouseMoveEvent( QMouseEvent *event )
{
  Q_UNUSED( event );
}

void Qgs3DMapTool::activate()
{
}

void Qgs3DMapTool::deactivate()
{
}

QCursor Qgs3DMapTool::cursor() const
{
  return Qt::CrossCursor;
}
