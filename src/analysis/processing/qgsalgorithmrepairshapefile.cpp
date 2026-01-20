/***************************************************************************
                         qgsalgorithmrepairshapefile.cpp
                         ---------------------
    begin                : December 2019
    copyright            : (C) 2019 by Nyall Dawson
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

#include "qgsalgorithmrepairshapefile.h"

#include <cpl_conv.h>

#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"

///@cond PRIVATE

QString QgsRepairShapefileAlgorithm::name() const
{
  return u"repairshapefile"_s;
}

QString QgsRepairShapefileAlgorithm::displayName() const
{
  return QObject::tr( "Repair Shapefile" );
}

QStringList QgsRepairShapefileAlgorithm::tags() const
{
  return QObject::tr( "fix,shp,shx,broken,missing" ).split( ',' );
}

QString QgsRepairShapefileAlgorithm::group() const
{
  return QObject::tr( "Vector general" );
}

QString QgsRepairShapefileAlgorithm::groupId() const
{
  return u"vectorgeneral"_s;
}

QString QgsRepairShapefileAlgorithm::shortHelpString() const
{
  return QObject::tr( "Repairs a broken Shapefile by recreating missing or broken SHX files." );
}

QString QgsRepairShapefileAlgorithm::shortDescription() const
{
  return QObject::tr( "Repairs broken Shapefiles by recreating SHX files." );
}

QgsRepairShapefileAlgorithm *QgsRepairShapefileAlgorithm::createInstance() const
{
  return new QgsRepairShapefileAlgorithm();
}

void QgsRepairShapefileAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFile( u"INPUT"_s, QObject::tr( "Input Shapefile" ), Qgis::ProcessingFileParameterBehavior::File, u"shp"_s, QVariant(), false, QObject::tr( "ESRI Shapefile" ) + u" (*.shp *.SHP)"_s ) );

  addOutput( new QgsProcessingOutputVectorLayer( u"OUTPUT"_s, QObject::tr( "Repaired layer" ) ) );
}

QVariantMap QgsRepairShapefileAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  const QString path = parameterAsFile( parameters, u"INPUT"_s, context );

  if ( !QFile::exists( path ) )
    throw QgsProcessingException( QObject::tr( "Could not load source layer for %1." ).arg( "INPUT"_L1 ) );

  CPLSetConfigOption( "SHAPE_RESTORE_SHX", "YES" );

  auto layer = std::make_unique<QgsVectorLayer>( path );
  if ( !layer->isValid() )
  {
    CPLSetConfigOption( "SHAPE_RESTORE_SHX", nullptr );
    throw QgsProcessingException( QObject::tr( "Could not repair %1." ).arg( path ) );
  }

  CPLSetConfigOption( "SHAPE_RESTORE_SHX", nullptr );

  feedback->pushInfo( QObject::tr( "Successfully repaired, found %n feature(s)", nullptr, layer->featureCount() ) );

  QVariantMap outputs;
  outputs.insert( u"OUTPUT"_s, path );
  return outputs;
}

///@endcond
