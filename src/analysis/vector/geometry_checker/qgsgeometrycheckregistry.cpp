/***************************************************************************
    qgsgeometrycheckregistry.cpp
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

#include "qgsgeometrycheckregistry.h"

#include "qgis.h"

QgsGeometryCheckRegistry::QgsGeometryCheckRegistry()
{

}

QgsGeometryCheckRegistry::~QgsGeometryCheckRegistry()
{
  qDeleteAll( mGeometryCheckFactories.values() );
}

QgsGeometryCheck *QgsGeometryCheckRegistry::geometryCheck( const QString &checkId )
{

}

bool QgsGeometryCheckRegistry::registerGeometryCheck( const QString &checkId, QgsGeometryCheckFactory *checkFactory )
{
  mGeometryCheckFactories.insert( checkId, checkFactory );
}
