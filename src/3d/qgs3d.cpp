/***************************************************************************
                         qgs3d.cpp
                         ----------
    begin                : July 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgs3d.h"

Qgs3D *Qgs3D::instance()
{
  static Qgs3D *sInstance( new Qgs3D() );
  return sInstance;
}

Qgs3D::~Qgs3D()
{
}

Qgs3D::Qgs3D()
{
}
