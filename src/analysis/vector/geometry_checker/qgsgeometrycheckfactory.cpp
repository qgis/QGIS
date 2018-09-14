/***************************************************************************
    qgsgeometrycheckfactory.cpp
     --------------------------------------
    Date                 : September 2018
    Copyright            : (C) 2018 Matthias Kuhn
    Email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgeometrycheckfactory.h"

#include "qgis.h"

QgsGeometryCheck::CheckType QgsGeometryCheckFactory::flags() const
{
  return QgsGeometryCheck::FeatureNodeCheck;
}
