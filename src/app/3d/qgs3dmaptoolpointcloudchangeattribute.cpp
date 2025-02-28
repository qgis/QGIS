/***************************************************************************
    qgs3dmaptoolpointcloudchangeattribute.cpp
    ---------------------
    begin                : January 2025
    copyright            : (C) 2025 by Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgs3dmaptoolpointcloudchangeattribute.h"
#include "moc_qgs3dmaptoolpointcloudchangeattribute.cpp"

Qgs3DMapToolPointCloudChangeAttribute::Qgs3DMapToolPointCloudChangeAttribute( Qgs3DMapCanvas *canvas )
  : Qgs3DMapTool( canvas )
{
}

Qgs3DMapToolPointCloudChangeAttribute::~Qgs3DMapToolPointCloudChangeAttribute() = default;

void Qgs3DMapToolPointCloudChangeAttribute::setAttribute( const QString &attribute )
{
  mAttributeName = attribute;
}

void Qgs3DMapToolPointCloudChangeAttribute::setNewValue( const double value )
{
  mNewValue = value;
}

void Qgs3DMapToolPointCloudChangeAttribute::run()
{
}

void Qgs3DMapToolPointCloudChangeAttribute::restart()
{
}
