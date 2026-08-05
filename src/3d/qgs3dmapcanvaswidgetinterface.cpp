/***************************************************************************
  qgs3dmapcanvaswidgetinterface.cpp
  --------------------------------------
  Date                 : July 2026
  Copyright            : (C) 2026 by Benoit De Mezzo
  Email                : benoit dot de dot mezzo at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgs3dmapcanvaswidgetinterface.h"

#include "qgs3deditingtoolbar.h"

void Qgs3DMapCanvasWidgetInterface::addEditingToolBar( Qgs3DEditingToolBar *newToolBar )
{
  ( void ) newToolBar;
};

QList<Qgs3DEditingToolBar *> Qgs3DMapCanvasWidgetInterface::editingToolBars() const
{
  return {};
}

Qgs3DMapCanvas *Qgs3DMapCanvasWidgetInterface::mapCanvas3D()
{
  return nullptr;
}
