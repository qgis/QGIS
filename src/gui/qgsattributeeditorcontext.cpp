/***************************************************************************
    qgsattributeeditorcontext.cpp
     --------------------------------------
    Date                 : 19.8.2019
    Copyright            : (C) 201 Julien Cabieces
    Email                : julien dot cabieces at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsattributeeditorcontext.h"
#include "moc_qgsattributeeditorcontext.cpp"

void QgsAttributeEditorContext::setCadDockWidget( QgsAdvancedDigitizingDockWidget *cadDockWidget )
{
  mCadDockWidget = cadDockWidget;
}
