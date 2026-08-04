/***************************************************************************
    qgs3deditingtoolbar.cpp
    -------------------
    begin                : November 2025
    copyright            : (C) 2025 Oslandia
    email                : benoit dot de dot mezzo at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgs3deditingtoolbar.h"

Qgs3DEditingToolBar::Qgs3DEditingToolBar( const QString &title, QWidget *parent )
  : QToolBar( title, parent )
  , mParentWidget( parent )
{}
