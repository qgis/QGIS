/***************************************************************************
                        qgsalgorithmfixgeometryhole.cpp
                        ---------------------
   begin                : July 2024
   copyright            : (C) 2024 by Jacky Volpes
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

#include "qgsalgorithmfixgeometryhole.h"
#include "qgsgeometrycheckerror.h"
#include "qgsgeometrycheckerutils.h"
#include "qgsgeometryholecheck.h"
#include "qgsvectordataproviderfeaturepool.h"
#include "qgsvectorlayer.h"
#include "qgsvectorfilewriter.h"

///@cond PRIVATE

auto QgsFixGeometryHoleAlgorithm::name() const -> QString
{
  return QStringLiteral( "fixgeometryhole" );
}

auto QgsFixGeometryHoleAlgorithm::displayName() const -> QString
{
  return QObject::tr( "Fix geometry (Hole)" );
}

auto QgsFixGeometryHoleAlgorithm::tags() const -> QStringList
{
  return QObject::tr( "delete,fix,hole" ).split( ',' );
}

auto QgsFixGeometryHoleAlgorithm::group() const -> QString
{
  return QObject::tr( "Fix geometry" );
}

auto QgsFixGeometryHoleAlgorithm::groupId() const -> QString
{
  return QStringLiteral( "fixgeometry" );
}

auto QgsFixGeometryHoleAlgorithm::shortHelpString() const -> QString
{
  return QObject::tr( "This algorithm deletes holes based on an error layer from the "
                      "check holes algorithm.\n" );
}

auto QgsFixGeometryHoleAlgorithm::createInstance() const -> QgsFixGeometryHoleAlgorithm *
{
  return new QgsFixGeometryHoleAlgorithm();
}

void QgsFixGeometryHoleAlgorithm::initAlgorithm( const QVariantMap &configuration )
{
  Q_UNUSED( configuration )

  // Inputs
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorPolygon ) << static_cast<int>( Qgis::ProcessingSourceType::VectorLine ) )
  );
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "ERRORS" ), QObject::tr( "Error layer" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorPoint ) )
  );
  addParameter( new QgsProcessingParameterField(
    QStringLiteral( "UNIQUE_ID" ), QObject::tr( "Field of original feature unique identifier" ),
    QStringLiteral( "id" ), QStringLiteral( "ERRORS" )
  )
  );
  addParameter( new QgsProcessingParameterField(
    QStringLiteral( "PART_IDX" ), QObject::tr( "Field of part index" ),
    QStringLiteral( "gc_partidx" ), QStringLiteral( "ERRORS" ),
    Qgis::ProcessingFieldParameterDataType::Numeric
  )
  );
  addParameter( new QgsProcessingParameterField(
    QStringLiteral( "RING_IDX" ), QObject::tr( "Field of ring index" ),
    QStringLiteral( "gc_ringidx" ), QStringLiteral( "ERRORS" ),
    Qgis::ProcessingFieldParameterDataType::Numeric
  )
  );
  addParameter( new QgsProcessingParameterField(
    QStringLiteral( "VERTEX_IDX" ), QObject::tr( "Field of vertex index" ),
    QStringLiteral( "gc_vertidx" ), QStringLiteral( "ERRORS" ),
    Qgis::ProcessingFieldParameterDataType::Numeric
  )
  );

  // Outputs
  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Output layer" ), Qgis::ProcessingSourceType::VectorAnyGeometry ) );
  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "REPORT" ), QObject::tr( "Report layer" ), Qgis::ProcessingSourceType::VectorPoint ) );

  auto tolerance = std::make_unique<QgsProcessingParameterNumber>( QStringLiteral( "TOLERANCE" ), QObject::tr( "Tolerance" ), Qgis::ProcessingNumberParameterType::Integer, 8, false, 1, 13 );
  tolerance->setFlags( tolerance->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( tolerance.release() );
}

auto QgsFixGeometryHoleAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) -> QVariantMap
{
  const std::unique_ptr<QgsProcessingFeatureSource> input( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !input )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "INPUT" ) ) );

  const std::unique_ptr<QgsProcessingFeatureSource> errors( parameterAsSource( parameters, QStringLiteral( "ERRORS" ), context ) );
  if ( !errors )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "ERRORS" ) ) );

  QgsProcessingMultiStepFeedback multiStepFeedback( 2, feedback );

  const QString featIdFieldName = parameterAsString( parameters, QStringLiteral( "UNIQUE_ID" ), context );
  const QString partIdxFieldName = parameterAsString( parameters, QStringLiteral( "PART_IDX" ), context );
  const QString ringIdxFieldName = parameterAsString( parameters, QStringLiteral( "RING_IDX" ), context );
  const QString vertexIdxFieldName = parameterAsString( parameters, QStringLiteral( "VERTEX_IDX" ), context );

  // Verify that input fields exists
  if ( errors->fields().indexFromName( featIdFieldName ) == -1 )
    throw QgsProcessingException( QObject::tr( "Field %1 does not exist in the error layer." ).arg( featIdFieldName ) );
  if ( errors->fields().indexFromName( partIdxFieldName ) == -1 )
    throw QgsProcessingException( QObject::tr( "Field %1 does not exist in the error layer." ).arg( partIdxFieldName ) );
  if ( errors->fields().indexFromName( ringIdxFieldName ) == -1 )
    throw QgsProcessingException( QObject::tr( "Field %1 does not exist in the error layer." ).arg( ringIdxFieldName ) );
  if ( errors->fields().indexFromName( vertexIdxFieldName ) == -1 )
    throw QgsProcessingException( QObject::tr( "Field %1 does not exist in the error layer." ).arg( vertexIdxFieldName ) );
  int inputIdFieldIndex = input->fields().indexFromName( featIdFieldName );
  if ( inputIdFieldIndex == -1 )
    throw QgsProcessingException( QObject::tr( "Field %1 does not exist in input layer." ).arg( featIdFieldName ) );

  QgsField inputFeatIdField = input->fields().at( inputIdFieldIndex );
  if ( inputFeatIdField.type() != errors->fields().at( errors->fields().indexFromName( featIdFieldName ) ).type() )
    throw QgsProcessingException( QObject::tr( "Field %1 does not have the same type as in the error layer." ).arg( featIdFieldName ) );

  QString dest_output;
  const std::unique_ptr<QgsFeatureSink> sink_output( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, dest_output, input->fields(), input->wkbType(), input->sourceCrs() ) );
  if ( !sink_output )
    throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "OUTPUT" ) ) );

  QString dest_report;
  QgsFields reportFields = errors->fields();
  reportFields.append( QgsField( QStringLiteral( "report" ), QMetaType::QString ) );
  reportFields.append( QgsField( QStringLiteral( "error_fixed" ), QMetaType::Bool ) );
  const std::unique_ptr<QgsFeatureSink> sink_report( parameterAsSink( parameters, QStringLiteral( "REPORT" ), context, dest_report, reportFields, errors->wkbType(), errors->sourceCrs() ) );
  if ( !sink_report )
    throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "REPORT" ) ) );

  const QgsProject *project = QgsProject::instance();
  auto checkContext = std::make_unique<QgsGeometryCheckContext>( mTolerance, input->sourceCrs(), project->transformContext(), project );
  QStringList messages;

  const QgsGeometryHoleCheck check( checkContext.get(), QVariantMap() );

  QgsVectorLayer *fixedLayer = input->materialize( QgsFeatureRequest() );
  std::unique_ptr<QgsFeaturePool> featurePool = std::make_unique<QgsVectorDataProviderFeaturePool>( fixedLayer );
  QMap<QString, QgsFeaturePool *> featurePools;
  featurePools.insert( fixedLayer->id(), featurePool.get() );

  QgsFeature errorFeature, inputFeature, testDuplicateIdFeature;
  QgsFeatureIterator errorFeaturesIt = errors->getFeatures();
  QList<QgsGeometryCheck::Changes> changesList;
  QgsFeature reportFeature;
  reportFeature.setFields( reportFields );
  long long progression = 0;
  long long totalProgression = errors->featureCount();
  multiStepFeedback.setCurrentStep( 1 );
  multiStepFeedback.setProgressText( QObject::tr( "Fixing errors..." ) );
  while ( errorFeaturesIt.nextFeature( errorFeature ) )
  {
    progression++;
    multiStepFeedback.setProgress( static_cast<double>( static_cast<long double>( progression ) / totalProgression ) * 100 );
    reportFeature.setGeometry( errorFeature.geometry() );

    QString idValue = errorFeature.attribute( featIdFieldName ).toString();
    if ( inputFeatIdField.type() == QMetaType::QString )
      idValue = "'" + idValue + "'";

    QgsFeatureIterator it = fixedLayer->getFeatures( QgsFeatureRequest().setFilterExpression( "\"" + featIdFieldName + "\" = " + idValue ) );
    if ( !it.nextFeature( inputFeature ) || !inputFeature.isValid() )
      reportFeature.setAttributes( errorFeature.attributes() << QObject::tr( "Source feature not found or invalid" ) << false );

    else if ( it.nextFeature( testDuplicateIdFeature ) )
      throw QgsProcessingException( QObject::tr( "More than one feature found in input layer with value %1 in unique field %2" ).arg( idValue ).arg( featIdFieldName ) );

    else if ( inputFeature.geometry().isNull() )
      reportFeature.setAttributes( errorFeature.attributes() << QObject::tr( "Feature geometry is null" ) << false );

    else if ( QgsGeometryCheckerUtils::getGeomPart( inputFeature.geometry().constGet(), errorFeature.attribute( partIdxFieldName ).toInt() ) == nullptr )
      reportFeature.setAttributes( errorFeature.attributes() << QObject::tr( "Feature geometry part is null" ) << false );

    else
    {
      QgsGeometryCheckError checkError = QgsGeometryCheckError(
        &check,
        QgsGeometryCheckerUtils::LayerFeature( featurePool.get(), inputFeature, checkContext.get(), false ),
        errorFeature.geometry().asPoint(),
        QgsVertexId(
          errorFeature.attribute( partIdxFieldName ).toInt(),
          errorFeature.attribute( ringIdxFieldName ).toInt(),
          errorFeature.attribute( vertexIdxFieldName ).toInt()
        )
      );
      for ( auto changes : changesList )
        checkError.handleChanges( changes );

      QgsGeometryCheck::Changes changes;
      check.fixError( featurePools, &checkError, QgsGeometryHoleCheck::ResolutionMethod::RemoveHoles, QMap<QString, int>(), changes );
      changesList << changes;
      reportFeature.setAttributes( errorFeature.attributes() << checkError.resolutionMessage() << ( checkError.status() == QgsGeometryCheckError::StatusFixed ) );
    }

    if ( !sink_report->addFeature( reportFeature, QgsFeatureSink::FastInsert ) )
      throw QgsProcessingException( writeFeatureError( sink_report.get(), parameters, QStringLiteral( "REPORT" ) ) );
  }
  multiStepFeedback.setProgress( 100 );

  progression = 0;
  totalProgression = fixedLayer->featureCount();
  multiStepFeedback.setCurrentStep( 2 );
  multiStepFeedback.setProgressText( QObject::tr( "Exporting fixed layer..." ) );
  QgsFeature fixedFeature;
  QgsFeatureIterator fixedFeaturesIt = fixedLayer->getFeatures();
  while ( fixedFeaturesIt.nextFeature( fixedFeature ) )
  {
    progression++;
    multiStepFeedback.setProgress( static_cast<double>( static_cast<long double>( progression ) / totalProgression ) * 100 );
    if ( !sink_output->addFeature( fixedFeature, QgsFeatureSink::FastInsert ) )
      throw QgsProcessingException( writeFeatureError( sink_output.get(), parameters, QStringLiteral( "OUTPUT" ) ) );
  }
  multiStepFeedback.setProgress( 100 );

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), dest_output );
  outputs.insert( QStringLiteral( "REPORT" ), dest_report );

  return outputs;
}

auto QgsFixGeometryHoleAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * ) -> bool
{
  mTolerance = parameterAsInt( parameters, QStringLiteral( "TOLERANCE" ), context );

  return true;
}

auto QgsFixGeometryHoleAlgorithm::flags() const -> Qgis::ProcessingAlgorithmFlags
{
  return QgsProcessingAlgorithm::flags() | Qgis::ProcessingAlgorithmFlag::NoThreading;
}

///@endcond
