/***************************************************************************
    qgs3dmapcontroller.cpp
    ---------------------
    begin                : March 2023
    copyright            : (C) 2023 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgs3dmapcontroller.h"
#include "moc_qgs3dmapcontroller.cpp"

QgsAbstract3DMapController::QgsAbstract3DMapController( QObject *parent )
  : QgsAbstractInputController( parent )
{
}

Qgis::InputControllerType QgsAbstract3DMapController::type() const
{
  return Qgis::InputControllerType::Map3D;
}
