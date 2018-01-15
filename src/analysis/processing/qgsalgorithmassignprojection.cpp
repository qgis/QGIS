/***************************************************************************
                         qgsalgorithmassignprojection.cpp
                         --------------------------------
    begin                : April 2017
    copyright            : (C) 2017 by Nyall Dawson
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

#include "qgsalgorithmassignprojection.h"

///@cond PRIVATE

QgsProcessingAlgorithm::Flags QgsAssignProjectionAlgorithm::flags() const
{
  return QgsProcessingFeatureBasedAlgorithm::flags() | QgsProcessingAlgorithm::FlagCanRunInBackground;
}

QString QgsAssignProjectionAlgorithm::name() const
{
  return QStringLiteral( "assignprojection" );
}

QString QgsAssignProjectionAlgorithm::displayName() const
{
  return QObject::tr( "Assign projection" );
}

QStringList QgsAssignProjectionAlgorithm::tags() const
{
  return QObject::tr( "assign,set,transform,reproject,crs,srs,warp" ).split( ',' );
}

QString QgsAssignProjectionAlgorithm::group() const
{
  return QObject::tr( "Vector general" );
}

QString QgsAssignProjectionAlgorithm::groupId() const
{
  return QStringLiteral( "vectorgeneral" );
}

QString QgsAssignProjectionAlgorithm::outputName() const
{
  return QObject::tr( "Assigned CRS" );
}

QString QgsAssignProjectionAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm assigns a new projection to a vector layer. It creates a new layer with the exact same features "
                      "and geometries as the input one, but assigned to a new CRS. E.g. the geometries are not reprojected, they are just assigned "
                      "to a different CRS. This algorithm can be used to repair layers which have been assigned an incorrect projection.\n\n"
                      "Attributes are not modified by this algorithm." );
}

QgsAssignProjectionAlgorithm *QgsAssignProjectionAlgorithm::createInstance() const
{
  return new QgsAssignProjectionAlgorithm();
}

void QgsAssignProjectionAlgorithm::initParameters( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterCrs( QStringLiteral( "CRS" ), QObject::tr( "Assigned CRS" ), QStringLiteral( "EPSG:4326" ) ) );
}

bool QgsAssignProjectionAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mDestCrs = parameterAsCrs( parameters, QStringLiteral( "CRS" ), context );
  return true;
}

QgsFeature QgsAssignProjectionAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingContext &, QgsProcessingFeedback * )
{
  return feature;
}

///@endcond

