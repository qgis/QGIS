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

#ifdef WITH_SFCGAL
#include "qgssfcgalgeometry.h"
#endif

///@cond PRIVATE

QString QgsApproximateMedialAxisAlgorithm::name() const
{
  return u"approximatemedialaxis"_s;
}

QString QgsApproximateMedialAxisAlgorithm::displayName() const
{
  return QObject::tr( "Approximate medial axis" );
}

QStringList QgsApproximateMedialAxisAlgorithm::tags() const
{
  return QObject::tr( "medial,axis,create,lines,straight,skeleton" ).split( ',' );
}

QString QgsApproximateMedialAxisAlgorithm::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsApproximateMedialAxisAlgorithm::groupId() const
{
  return u"vectorgeometry"_s;
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
  return QObject::tr( "Returns an approximate medial axis for a polygon layer input based on its straight skeleton." );
}

QgsApproximateMedialAxisAlgorithm *QgsApproximateMedialAxisAlgorithm::createInstance() const
{
  return new QgsApproximateMedialAxisAlgorithm();
}

QgsFields QgsApproximateMedialAxisAlgorithm::outputFields( const QgsFields &inputFields ) const
{
  QgsFields newFields;
  newFields.append( QgsField( u"length"_s, QMetaType::Type::Double, QString(), 20, 6 ) );
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

bool QgsApproximateMedialAxisAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_UNUSED( parameters )
  Q_UNUSED( context )
  Q_UNUSED( feedback )

#ifdef WITH_SFCGAL
  return true;
#else
  throw QgsProcessingException( QObject::tr( "This processing algorithm requires a QGIS installation with SFCGAL support enabled. Please use a version of QGIS that includes SFCGAL." ) );
#endif
}

QgsFeatureList QgsApproximateMedialAxisAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_UNUSED( context )

#ifdef WITH_SFCGAL
  QgsFeature modifiedFeature = feature;
  if ( modifiedFeature.hasGeometry() )
  {
    QgsGeometry outputGeometry;
    QgsSfcgalGeometry inputSfcgalGeometry;
    try
    {
      inputSfcgalGeometry = QgsSfcgalGeometry( modifiedFeature.geometry() );
    }
    catch ( const QgsSfcgalException &exception )
    {
      feedback->reportError( QObject::tr( "Cannot calculate approximate medial axis for feature %1: %2" ).arg( feature.id() ).arg( exception.what() ) );
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
      catch ( const QgsSfcgalException &medialAxisException )
      {
        feedback->reportError( QObject::tr( "Cannot calculate approximate medial axis for feature %1: %2" ).arg( feature.id() ).arg( medialAxisException.what() ) );
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
#else
  Q_UNUSED( feature )
  Q_UNUSED( feedback )
  throw QgsProcessingException( QObject::tr( "This processing algorithm requires a QGIS installation with SFCGAL support enabled. Please use a version of QGIS that includes SFCGAL." ) );
#endif
}

///@endcond PRIVATE
