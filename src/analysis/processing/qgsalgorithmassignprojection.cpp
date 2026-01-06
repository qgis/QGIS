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

QString QgsAssignProjectionAlgorithm::name() const
{
  return u"assignprojection"_s;
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
  return u"vectorgeneral"_s;
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

QString QgsAssignProjectionAlgorithm::shortDescription() const
{
  return QObject::tr( "Assigns a new projection to a copy of a vector layer, with the exact same features and geometries." );
}

QgsAssignProjectionAlgorithm *QgsAssignProjectionAlgorithm::createInstance() const
{
  return new QgsAssignProjectionAlgorithm();
}

bool QgsAssignProjectionAlgorithm::supportInPlaceEdit( const QgsMapLayer *layer ) const
{
  Q_UNUSED( layer )
  return false;
}

void QgsAssignProjectionAlgorithm::initParameters( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterCrs( u"CRS"_s, QObject::tr( "Assigned CRS" ), u"EPSG:4326"_s ) );
}

Qgis::ProcessingFeatureSourceFlags QgsAssignProjectionAlgorithm::sourceFlags() const
{
  return Qgis::ProcessingFeatureSourceFlag::SkipGeometryValidityChecks;
}

bool QgsAssignProjectionAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mDestCrs = parameterAsCrs( parameters, u"CRS"_s, context );
  return true;
}

QgsFeatureList QgsAssignProjectionAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingContext &, QgsProcessingFeedback * )
{
  return QgsFeatureList() << feature;
}

///@endcond
