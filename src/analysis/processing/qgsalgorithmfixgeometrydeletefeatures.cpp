/***************************************************************************
                        qgsalgorithmfixgeometrydeletefeatures.cpp
                        ---------------------
   begin                : April 2025
   copyright            : (C) 2025 by Jacky Volpes
   email                : jacky dot volpes at oslandia dot com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsalgorithmfixgeometrydeletefeatures.h"

///@cond PRIVATE

QString QgsFixGeometryDeleteFeaturesAlgorithm::name() const
{
  return u"fixgeometrydeletefeatures"_s;
}

QString QgsFixGeometryDeleteFeaturesAlgorithm::displayName() const
{
  return QObject::tr( "Delete features" );
}

QString QgsFixGeometryDeleteFeaturesAlgorithm::shortDescription() const
{
  return QObject::tr( "Deletes features detected with an algorithm from the \"Check geometry\" section." );
}

QStringList QgsFixGeometryDeleteFeaturesAlgorithm::tags() const
{
  return QObject::tr( "fix,delete,features" ).split( ',' );
}

QString QgsFixGeometryDeleteFeaturesAlgorithm::group() const
{
  return QObject::tr( "Fix geometry" );
}

QString QgsFixGeometryDeleteFeaturesAlgorithm::groupId() const
{
  return u"fixgeometry"_s;
}

QString QgsFixGeometryDeleteFeaturesAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm deletes error features listed in the errors layer from an algorithm in the \"Check geometry\" section.\n"
                      "The required inputs are the original layer used in the check algorithm, its unique id field, and its corresponding errors layer.\n\n"
                      "For instance, it can be used after the following check algorithms to delete error features:"
                      "<html><ul><li>Feature inside polygon</li>"
                      "<li>Degenerate polygons</li>"
                      "<li>Small segments</li>"
                      "<li>Duplicated geometries</li>"
                      "<li>etc.</li></ul></html>" );
}

QgsFixGeometryDeleteFeaturesAlgorithm *QgsFixGeometryDeleteFeaturesAlgorithm::createInstance() const
{
  return new QgsFixGeometryDeleteFeaturesAlgorithm();
}

void QgsFixGeometryDeleteFeaturesAlgorithm::initAlgorithm( const QVariantMap &configuration )
{
  Q_UNUSED( configuration )

  addParameter( new QgsProcessingParameterFeatureSource(
    u"INPUT"_s, QObject::tr( "Input layer" ),
    QList<int>()
      << static_cast<int>( Qgis::ProcessingSourceType::VectorPoint )
      << static_cast<int>( Qgis::ProcessingSourceType::VectorLine )
      << static_cast<int>( Qgis::ProcessingSourceType::VectorPolygon )
  ) );
  addParameter( new QgsProcessingParameterFeatureSource(
    u"ERRORS"_s, QObject::tr( "Error layer" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorPoint )
  ) );
  addParameter( new QgsProcessingParameterField(
    u"UNIQUE_ID"_s, QObject::tr( "Field of original feature unique identifier" ),
    QString(), u"ERRORS"_s
  ) );

  addParameter( new QgsProcessingParameterFeatureSink(
    u"OUTPUT"_s, QObject::tr( "Cleaned layer" ), Qgis::ProcessingSourceType::VectorAnyGeometry
  ) );
  addParameter( new QgsProcessingParameterFeatureSink(
    u"REPORT"_s, QObject::tr( "Report layer from deleting features" ), Qgis::ProcessingSourceType::VectorPoint
  ) );
}

QVariantMap QgsFixGeometryDeleteFeaturesAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  const std::unique_ptr<QgsProcessingFeatureSource> input( parameterAsSource( parameters, u"INPUT"_s, context ) );
  if ( !input )
    throw QgsProcessingException( invalidSourceError( parameters, u"INPUT"_s ) );

  const std::unique_ptr<QgsProcessingFeatureSource> errors( parameterAsSource( parameters, u"ERRORS"_s, context ) );
  if ( !errors )
    throw QgsProcessingException( invalidSourceError( parameters, u"ERRORS"_s ) );

  const QString featIdFieldName = parameterAsString( parameters, u"UNIQUE_ID"_s, context );

  // Verify that input fields exists
  const int errorsIdFieldIndex = errors->fields().indexFromName( featIdFieldName );
  if ( errorsIdFieldIndex == -1 )
    throw QgsProcessingException( QObject::tr( "Field %1 does not exist in the error layer." ).arg( featIdFieldName ) );
  const int inputIdFieldIndex = input->fields().indexFromName( featIdFieldName );
  if ( inputIdFieldIndex == -1 )
    throw QgsProcessingException( QObject::tr( "Field %1 does not exist in input layer." ).arg( featIdFieldName ) );

  const QgsField inputFeatIdField = input->fields().at( inputIdFieldIndex );
  const QMetaType::Type inputFeatIdFieldType = inputFeatIdField.type();
  if ( inputFeatIdFieldType != errors->fields().at( errorsIdFieldIndex ).type() )
    throw QgsProcessingException( QObject::tr( "Input field %1 does not have the same type as in the error layer." ).arg( featIdFieldName ) );

  QString dest_output;
  const std::unique_ptr<QgsFeatureSink> sink_output( parameterAsSink( parameters, u"OUTPUT"_s, context, dest_output, input->fields(), input->wkbType(), input->sourceCrs() ) );
  if ( !sink_output )
    throw QgsProcessingException( invalidSinkError( parameters, u"OUTPUT"_s ) );

  QString dest_report;
  QgsFields reportFields = errors->fields();
  reportFields.append( QgsField( u"report"_s, QMetaType::QString ) );
  reportFields.append( QgsField( u"error_fixed"_s, QMetaType::Bool ) );
  const std::unique_ptr<QgsFeatureSink> sink_report( parameterAsSink( parameters, u"REPORT"_s, context, dest_report, reportFields, errors->wkbType(), errors->sourceCrs() ) );
  if ( !sink_report )
    throw QgsProcessingException( invalidSinkError( parameters, u"REPORT"_s ) );

  QgsFeature inputFeature, errorFeature, reportFeature;
  reportFeature.setFields( reportFields );
  long long progression = 0;
  long long totalProgression = input->featureCount();
  feedback->setProgressText( QObject::tr( "Fixing errors..." ) );

  QgsFeatureIterator inputFeaturesIt = input->getFeatures();
  while ( inputFeaturesIt.nextFeature( inputFeature ) )
  {
    if ( feedback->isCanceled() )
      break;

    progression++;
    feedback->setProgress( static_cast<double>( static_cast<long double>( progression ) / totalProgression ) * 100 );

    QString idValue = inputFeature.attribute( featIdFieldName ).toString();
    if ( inputFeatIdFieldType == QMetaType::QString )
      idValue = "'" + idValue + "'";

    QgsFeatureIterator errorFeaturesIt = errors->getFeatures(
      QgsFeatureRequest()
        .setFlags( Qgis::FeatureRequestFlag::SubsetOfAttributes ) // the request will select only the fields referenced in the expression
        .setFilterExpression( "\"" + featIdFieldName + "\" = " + idValue )
    );
    if ( errorFeaturesIt.nextFeature( errorFeature ) && errorFeature.isValid() )
    {
      // We found an error feature corresponding to the input feature we are iterating
      // so it means that we want to delete this feature.
      // Just don't add this feature to the output sink, and add a report feature saying
      // that everything went fine.
      reportFeature.setGeometry( errorFeature.geometry() );
      reportFeature.setAttributes( errorFeature.attributes() << u"Feature deleted"_s << true );
      if ( !sink_report->addFeature( reportFeature, QgsFeatureSink::FastInsert ) )
        throw QgsProcessingException( writeFeatureError( sink_report.get(), parameters, u"REPORT"_s ) );
    }
    else
    {
      // We didn't find an error corresponding to this feature, so we must keep this feature.
      // Just add it to the output sink.
      if ( !sink_output->addFeature( inputFeature, QgsFeatureSink::FastInsert ) )
        throw QgsProcessingException( writeFeatureError( sink_output.get(), parameters, u"OUTPUT"_s ) );
    }
  }
  feedback->setProgress( 100 );

  QVariantMap outputs;
  outputs.insert( u"OUTPUT"_s, dest_output );
  outputs.insert( u"REPORT"_s, dest_report );

  return outputs;
}

Qgis::ProcessingAlgorithmFlags QgsFixGeometryDeleteFeaturesAlgorithm::flags() const
{
  return QgsProcessingAlgorithm::flags() | Qgis::ProcessingAlgorithmFlag::NoThreading;
}

///@endcond
