/***************************************************************************
  qgs3dmaptoolmeasureline.cpp
  --------------------------------------
  Date                 : Jun 2019
  Copyright            : (C) 2019 by Ismail Sunni
  Email                : imajimatika at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QDebug>

#include "qgs3dmaptoolmeasureline.h"
#include "qgs3dmapcanvas.h"

Qgs3DMapToolMeasureLine::Qgs3DMapToolMeasureLine( Qgs3DMapCanvas *canvas )
  : Qgs3DMapTool( canvas )
{
  qInfo() << "Constructed";
}

Qgs3DMapToolMeasureLine::~Qgs3DMapToolMeasureLine() = default;


void Qgs3DMapToolMeasureLine::mousePressEvent( QMouseEvent *event )
{
  Q_UNUSED( event )
  qInfo() << "Mouse clicked";

}

void Qgs3DMapToolMeasureLine::activate()
{
  qInfo() << "Measure line activated";
}

void Qgs3DMapToolMeasureLine::deactivate()
{
  qInfo() << "Measure line deactivated";
}
