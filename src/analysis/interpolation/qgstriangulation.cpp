/***************************************************************************
    Triangulation.cpp
    -----------------
    begin                : September 2009
    copyright            : (C) 2009 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgstriangulation.h"
#include "qgsfields.h"

QgsFields QgsTriangulation::triangulationFields()
{
  QgsFields fields;
  fields.append( QgsField( QStringLiteral( "type" ), QMetaType::Type::QString, QStringLiteral( "String" ) ) );
  return fields;
}
