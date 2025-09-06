/***************************************************************************
                         qgsalgorithmapproximatemedialaxis.cpp
                         ---------------------
    begin                : September 2025
    copyright            : (C) 2025 by Jean Felder
    email                : jean dot felder at oslandia dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgsalgorithmapproximatemedialaxis.h"
#include "qgsexception.h"
#include "qgssfcgalgeometry.h"

///@cond PRIVATE

QString QgsApproximateMedialAxisAlgorithm::name() const
{
  return QStringLiteral( "approximatemedialaxis" );
}

QString QgsApproximateMedialAxisAlgorithm::displayName() const
{
  return QObject::tr( "Approximate medial axis" );
}

QStringList QgsApproximateMedialAxisAlgorithm::tags() const
{
  return QObject::tr( "medial,axis,create,lines" ).split( ',' );
}

QString QgsApproximateMedialAxisAlgorithm::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsApproximateMedialAxisAlgorithm::groupId() const
{
  return QStringLiteral( "vectorgeometry" );
}

QString QgsApproximateMedialAxisAlgorithm::shortHelpString() const
{
  return QObject::tr( "The Approximate Medial Axis algorithm generates a simplified skeleton of a shape by approximating its medial axis. \n\n"
                      "The output is a collection of lines that follow the central structure of the shape. The result is a thin, stable set "
                      "of curves that capture the main topology while ignoring noise.\n\n"
                      "This algorithm ignores the Z dimensions. If the geometry is 3D, the approximate medial axis will be calculated from "
                      "its 2D projection." );
}

QString QgsApproximateMedialAxisAlgorithm::shortDescription() const
{
  return QObject::tr( "Returns an approximate medial axis for the areal input based on its straight skeleton." );
}

Qgis::ProcessingAlgorithmFlags QgsApproximateMedialAxisAlgorithm::flags() const
{
  return QgsProcessingFeatureBasedAlgorithm::flags() | Qgis::ProcessingAlgorithmFlag::NoThreading;
}

QgsApproximateMedialAxisAlgorithm *QgsApproximateMedialAxisAlgorithm::createInstance() const
{
  return new QgsApproximateMedialAxisAlgorithm();
}

QgsFields QgsApproximateMedialAxisAlgorithm::outputFields( const QgsFields &inputFields ) const
{
  QgsFields newFields;
  newFields.append( QgsField( QStringLiteral( "length" ), QMetaType::Type::Double, QString(), 20, 6 ) );
  return QgsProcessingUtils::combineFields( inputFields, newFields );
}

QList<int> QgsApproximateMedialAxisAlgorithm::inputLayerTypes() const
{
  return QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorPolygon );
}

QString QgsApproximateMedialAxisAlgorithm::outputName() const
{
  return QObject::tr( "Medial axis" );
}

Qgis::ProcessingSourceType QgsApproximateMedialAxisAlgorithm::outputLayerType() const
{
  return Qgis::ProcessingSourceType::VectorLine;
}

Qgis::WkbType QgsApproximateMedialAxisAlgorithm::outputWkbType( Qgis::WkbType inputWkbType ) const
{
  Q_UNUSED( inputWkbType )
  return Qgis::WkbType::MultiLineString;
}

QgsFeatureList QgsApproximateMedialAxisAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_UNUSED( context )
  Q_UNUSED( feedback )

  QgsFeature modifiedFeature = feature;
  if ( modifiedFeature.hasGeometry() )
  {
    QgsGeometry outputGeometry;
    QgsSfcgalGeometry inputSfcgalGeometry;
    try
    {
      inputSfcgalGeometry = QgsSfcgalGeometry( modifiedFeature.geometry() );
    }
    catch ( QgsSfcgalException const & )
    {
      feedback->reportError( QObject::tr( "Wrong geometry, cannot calculate approximate medial axis." ) );
      modifiedFeature.clearGeometry();
    }

    if ( !inputSfcgalGeometry.isEmpty() )
    {
      try
      {
        std::unique_ptr<QgsSfcgalGeometry> outputSfcgalGeometry = inputSfcgalGeometry.approximateMedialAxis();
        outputGeometry = QgsGeometry( outputSfcgalGeometry->asQgisGeometry() );
        modifiedFeature.setGeometry( outputGeometry );
      }
      catch ( QgsSfcgalException const &medialAxisException )
      {
        feedback->reportError( medialAxisException.what() );
      }
    }

    if ( !outputGeometry.isNull() )
    {
      QgsAttributes attrs = modifiedFeature.attributes();
      attrs << outputGeometry.constGet()->length();
      modifiedFeature.setAttributes( attrs );
    }
    else
    {
      QgsAttributes attrs = modifiedFeature.attributes();
      attrs << QVariant();
      modifiedFeature.setAttributes( attrs );
    }
  }
  return QgsFeatureList() << modifiedFeature;
}

///@endcond PRIVATE
