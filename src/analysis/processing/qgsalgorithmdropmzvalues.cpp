/***************************************************************************
                         qgsalgorithmdropmzvalues.cpp
                         ---------------------
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

#include "qgsalgorithmdropmzvalues.h"

///@cond PRIVATE

QString QgsDropMZValuesAlgorithm::name() const
{
  return QStringLiteral( "dropmzvalues" );
}

QString QgsDropMZValuesAlgorithm::displayName() const
{
  return QObject::tr( "Drop M/Z values" );
}

QStringList QgsDropMZValuesAlgorithm::tags() const
{
  return QObject::tr( "drop,set,convert,m,measure,z,25d,3d,values" ).split( ',' );
}

QString QgsDropMZValuesAlgorithm::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsDropMZValuesAlgorithm::groupId() const
{
  return QStringLiteral( "vectorgeometry" );
}

QString QgsDropMZValuesAlgorithm::outputName() const
{
  return QObject::tr( "Z/M Dropped" );
}

QString QgsDropMZValuesAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm can remove any measure (M) or Z values from input geometries." );
}

QgsDropMZValuesAlgorithm *QgsDropMZValuesAlgorithm::createInstance() const
{
  return new QgsDropMZValuesAlgorithm();
}

bool QgsDropMZValuesAlgorithm::supportInPlaceEdit( const QgsMapLayer *layer ) const
{
  Q_UNUSED( layer )
  return false;
}

void QgsDropMZValuesAlgorithm::initParameters( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterBoolean( QStringLiteral( "DROP_M_VALUES" ), QObject::tr( "Drop M Values" ), false ) );
  addParameter( new QgsProcessingParameterBoolean( QStringLiteral( "DROP_Z_VALUES" ), QObject::tr( "Drop Z Values" ), false ) );
}

Qgis::WkbType QgsDropMZValuesAlgorithm::outputWkbType( Qgis::WkbType inputWkbType ) const
{
  Qgis::WkbType wkb = inputWkbType;
  if ( mDropM )
    wkb = QgsWkbTypes::dropM( wkb );
  if ( mDropZ )
    wkb = QgsWkbTypes::dropZ( wkb );
  return wkb;
}

Qgis::ProcessingFeatureSourceFlags QgsDropMZValuesAlgorithm::sourceFlags() const
{
  return Qgis::ProcessingFeatureSourceFlag::SkipGeometryValidityChecks;
}

bool QgsDropMZValuesAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mDropM = parameterAsBoolean( parameters, QStringLiteral( "DROP_M_VALUES" ), context );
  mDropZ = parameterAsBoolean( parameters, QStringLiteral( "DROP_Z_VALUES" ), context );
  return true;
}

QgsFeatureList QgsDropMZValuesAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingContext &, QgsProcessingFeedback * )
{
  QgsFeature f = feature;
  if ( f.hasGeometry() )
  {
    std::unique_ptr<QgsAbstractGeometry> newGeom( f.geometry().constGet()->clone() );
    if ( mDropM )
      newGeom->dropMValue();
    if ( mDropZ )
      newGeom->dropZValue();
    f.setGeometry( QgsGeometry( newGeom.release() ) );
  }

  return QgsFeatureList() << f;
}

///@endcond
