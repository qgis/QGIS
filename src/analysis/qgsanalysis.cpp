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
#include "qgsgeometrycheckfactory.h"
#include "qgis.h"

#include "qgsgeometryselfintersectioncheck.h"
#include "qgsgeometrygapcheck.h"
#include "qgsgeometrymissingvertexcheck.h"
#include "qgsgeometryoverlapcheck.h"

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
  qRegisterMetaType< QList<std::shared_ptr<QgsGeometryCheckError> > >( "QList<std::shared_ptr<QgsGeometryCheckError>>" );

  mGeometryCheckRegistry->registerGeometryCheck( new QgsGeometryCheckFactoryT<QgsGeometrySelfIntersectionCheck>() );
  mGeometryCheckRegistry->registerGeometryCheck( new QgsGeometryCheckFactoryT<QgsGeometryGapCheck>() );
  mGeometryCheckRegistry->registerGeometryCheck( new QgsGeometryCheckFactoryT<QgsGeometryOverlapCheck>() );
  mGeometryCheckRegistry->registerGeometryCheck( new QgsGeometryCheckFactoryT<QgsGeometryMissingVertexCheck>() );
}

QgsAnalysis::~QgsAnalysis()
{

}
