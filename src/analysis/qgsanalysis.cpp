/***************************************************************************
                         qgsanalysis.cpp
                         ----------
    begin                : September 2018
    copyright            : (C) 2018 by Matthias Kuhn
    email                : matthias@opengis.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsanalysis.h"
#include "qgsgeometrycheckregistry.h"
#include "qgis.h"

QgsAnalysis *QgsAnalysis::instance()
{
  static QgsAnalysis *sInstance( new QgsAnalysis() );
  return sInstance;
}

QgsGeometryCheckRegistry *QgsAnalysis::geometryCheckRegistry()
{
  return instance()->mGeometryCheckRegistry.get();
}

QgsAnalysis::QgsAnalysis()
  : mGeometryCheckRegistry( qgis::make_unique<QgsGeometryCheckRegistry>() )
{
}

QgsAnalysis::~QgsAnalysis()
{

}
