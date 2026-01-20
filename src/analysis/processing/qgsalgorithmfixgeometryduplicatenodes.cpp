/***************************************************************************
                        qgsalgorithmfixgeometryduplicatenodes.cpp
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

#include "qgsalgorithmfixgeometryduplicatenodes.h"

#include "qgsgeometrycheckerror.h"
#include "qgsgeometrycheckerutils.h"
#include "qgsgeometryduplicatenodescheck.h"
#include "qgsvectordataproviderfeaturepool.h"
#include "qgsvectorfilewriter.h"
#include "qgsvectorlayer.h"

///@cond PRIVATE

QString QgsFixGeometryDuplicateNodesAlgorithm::name() const
{
  return u"fixgeometryduplicatenodes"_s;
}

QString QgsFixGeometryDuplicateNodesAlgorithm::displayName() const
{
  return QObject::tr( "Delete duplicated vertices" );
}

QString QgsFixGeometryDuplicateNodesAlgorithm::shortDescription() const
{
  return QObject::tr( "Deletes duplicated vertices detected with the \"Duplicated vertices\" algorithm from the \"Check geometry\" section." );
}

QStringList QgsFixGeometryDuplicateNodesAlgorithm::tags() const
{
  return QObject::tr( "fix,duplicated,vertex,delete" ).split( ',' );
}

QString QgsFixGeometryDuplicateNodesAlgorithm::group() const
{
  return QObject::tr( "Fix geometry" );
}

QString QgsFixGeometryDuplicateNodesAlgorithm::groupId() const
{
  return u"fixgeometry"_s;
}

QString QgsFixGeometryDuplicateNodesAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm delete duplicate vertices based on an error layer from the \"Duplicated vertices\" algorithm in the \"Check geometry\" section." );
}

QgsFixGeometryDuplicateNodesAlgorithm *QgsFixGeometryDuplicateNodesAlgorithm::createInstance() const
{
  return new QgsFixGeometryDuplicateNodesAlgorithm();
}

void QgsFixGeometryDuplicateNodesAlgorithm::initAlgorithm( const QVariantMap &configuration )
{
  Q_UNUSED( configuration )

  addParameter( new QgsProcessingParameterFeatureSource(
    u"INPUT"_s, QObject::tr( "Input layer" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorPolygon ) << static_cast<int>( Qgis::ProcessingSourceType::VectorLine )
  ) );
  addParameter( new QgsProcessingParameterFeatureSource(
    u"ERRORS"_s, QObject::tr( "Error layer" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorPoint )
  ) );
  addParameter( new QgsProcessingParameterField(
    u"UNIQUE_ID"_s, QObject::tr( "Field of original feature unique identifier" ),
    u"id"_s, u"ERRORS"_s
  ) );
  addParameter( new QgsProcessingParameterField(
    u"PART_IDX"_s, QObject::tr( "Field of part index" ),
    u"gc_partidx"_s, u"ERRORS"_s,
    Qgis::ProcessingFieldParameterDataType::Numeric
  ) );
  addParameter( new QgsProcessingParameterField(
    u"RING_IDX"_s, QObject::tr( "Field of ring index" ),
    u"gc_ringidx"_s, u"ERRORS"_s,
    Qgis::ProcessingFieldParameterDataType::Numeric
  ) );
  addParameter( new QgsProcessingParameterField(
    u"VERTEX_IDX"_s, QObject::tr( "Field of vertex index" ),
    u"gc_vertidx"_s, u"ERRORS"_s,
    Qgis::ProcessingFieldParameterDataType::Numeric
  ) );

  addParameter( new QgsProcessingParameterFeatureSink(
    u"OUTPUT"_s, QObject::tr( "Fixed duplicate vertices layer" ), Qgis::ProcessingSourceType::VectorAnyGeometry
  ) );
  addParameter( new QgsProcessingParameterFeatureSink(
    u"REPORT"_s, QObject::tr( "Report layer from fixing duplicate vertices" ), Qgis::ProcessingSourceType::VectorPoint
  ) );

  auto tolerance = std::make_unique<QgsProcessingParameterNumber>(
    u"TOLERANCE"_s, QObject::tr( "Tolerance" ), Qgis::ProcessingNumberParameterType::Integer, 8, false, 1, 13
  );
  tolerance->setFlags( tolerance->flags() | Qgis::ProcessingParameterFlag::Advanced );
  tolerance->setHelp( QObject::tr( "The \"Tolerance\" advanced parameter defines the numerical precision of geometric operations, "
                                   "given as an integer n, meaning that any difference smaller than 10⁻ⁿ (in map units) is considered zero." ) );
  addParameter( tolerance.release() );
}

QVariantMap QgsFixGeometryDuplicateNodesAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  const std::unique_ptr<QgsProcessingFeatureSource> input( parameterAsSource( parameters, u"INPUT"_s, context ) );
  if ( !input )
    throw QgsProcessingException( invalidSourceError( parameters, u"INPUT"_s ) );

  const std::unique_ptr<QgsProcessingFeatureSource> errors( parameterAsSource( parameters, u"ERRORS"_s, context ) );
  if ( !errors )
    throw QgsProcessingException( invalidSourceError( parameters, u"ERRORS"_s ) );

  QgsProcessingMultiStepFeedback multiStepFeedback( 2, feedback );

  const QString featIdFieldName = parameterAsString( parameters, u"UNIQUE_ID"_s, context );
  const QString partIdxFieldName = parameterAsString( parameters, u"PART_IDX"_s, context );
  const QString ringIdxFieldName = parameterAsString( parameters, u"RING_IDX"_s, context );
  const QString vertexIdxFieldName = parameterAsString( parameters, u"VERTEX_IDX"_s, context );

  // Verify that input fields exists
  if ( errors->fields().indexFromName( featIdFieldName ) == -1 )
    throw QgsProcessingException( QObject::tr( "Field \"%1\" does not exist in the error layer." ).arg( featIdFieldName ) );
  if ( errors->fields().indexFromName( partIdxFieldName ) == -1 )
    throw QgsProcessingException( QObject::tr( "Field %1 does not exist in the error layer." ).arg( partIdxFieldName ) );
  if ( errors->fields().indexFromName( ringIdxFieldName ) == -1 )
    throw QgsProcessingException( QObject::tr( "Field %1 does not exist in the error layer." ).arg( ringIdxFieldName ) );
  if ( errors->fields().indexFromName( vertexIdxFieldName ) == -1 )
    throw QgsProcessingException( QObject::tr( "Field %1 does not exist in the error layer." ).arg( vertexIdxFieldName ) );
  int inputIdFieldIndex = input->fields().indexFromName( featIdFieldName );
  if ( inputIdFieldIndex == -1 )
    throw QgsProcessingException( QObject::tr( "Field \"%1\" does not exist in input layer." ).arg( featIdFieldName ) );

  const QgsField inputFeatIdField = input->fields().at( inputIdFieldIndex );
  if ( inputFeatIdField.type() != errors->fields().at( errors->fields().indexFromName( featIdFieldName ) ).type() )
    throw QgsProcessingException( QObject::tr( "Field \"%1\" does not have the same type as in the error layer." ).arg( featIdFieldName ) );

  QString dest_output;
  const std::unique_ptr<QgsFeatureSink> sink_output( parameterAsSink(
    parameters, u"OUTPUT"_s, context, dest_output, input->fields(), input->wkbType(), input->sourceCrs()
  ) );
  if ( !sink_output )
    throw QgsProcessingException( invalidSinkError( parameters, u"OUTPUT"_s ) );

  QString dest_report;
  QgsFields reportFields = errors->fields();
  reportFields.append( QgsField( u"report"_s, QMetaType::QString ) );
  reportFields.append( QgsField( u"error_fixed"_s, QMetaType::Bool ) );
  const std::unique_ptr<QgsFeatureSink> sink_report( parameterAsSink(
    parameters, u"REPORT"_s, context, dest_report, reportFields, errors->wkbType(), errors->sourceCrs()
  ) );
  if ( !sink_report )
    throw QgsProcessingException( invalidSinkError( parameters, u"REPORT"_s ) );

  QgsGeometryCheckContext checkContext = QgsGeometryCheckContext( mTolerance, input->sourceCrs(), context.transformContext(), context.project() );

  const QgsGeometryDuplicateNodesCheck check( &checkContext, QVariantMap() );

  std::unique_ptr<QgsVectorLayer> fixedLayer( input->materialize( QgsFeatureRequest() ) );
  QgsVectorDataProviderFeaturePool featurePool = QgsVectorDataProviderFeaturePool( fixedLayer.get() );
  QMap<QString, QgsFeaturePool *> featurePools;
  featurePools.insert( fixedLayer->id(), &featurePool );

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
    if ( feedback->isCanceled() )
      break;

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
      throw QgsProcessingException( QObject::tr( "More than one feature found in input layer with value \"%1\" in unique field \"%2\"" ).arg( idValue, featIdFieldName ) );

    else if ( inputFeature.geometry().isNull() )
      reportFeature.setAttributes( errorFeature.attributes() << QObject::tr( "Feature geometry is null" ) << false );

    else
    {
      QgsGeometryCheckError checkError = QgsGeometryCheckError(
        &check,
        QgsGeometryCheckerUtils::LayerFeature( &featurePool, inputFeature, &checkContext, false ),
        errorFeature.geometry().asPoint(),
        QgsVertexId(
          errorFeature.attribute( partIdxFieldName ).toInt(),
          errorFeature.attribute( ringIdxFieldName ).toInt(),
          errorFeature.attribute( vertexIdxFieldName ).toInt()
        )
      );
      for ( const QgsGeometryCheck::Changes &changes : std::as_const( changesList ) )
        checkError.handleChanges( changes );

      QgsGeometryCheck::Changes changes;
      check.fixError( featurePools, &checkError, QgsGeometryDuplicateNodesCheck::ResolutionMethod::RemoveDuplicates, QMap<QString, int>(), changes );
      changesList << changes;
      QString resolutionMessage = checkError.resolutionMessage();
      if ( checkError.status() == QgsGeometryCheckError::StatusObsolete )
        resolutionMessage = QObject::tr( "Error is obsolete" );
      reportFeature.setAttributes( errorFeature.attributes() << resolutionMessage << ( checkError.status() == QgsGeometryCheckError::StatusFixed ) );
    }

    if ( !sink_report->addFeature( reportFeature, QgsFeatureSink::FastInsert ) )
      throw QgsProcessingException( writeFeatureError( sink_report.get(), parameters, u"REPORT"_s ) );
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
    if ( feedback->isCanceled() )
      break;

    progression++;
    multiStepFeedback.setProgress( static_cast<double>( static_cast<long double>( progression ) / totalProgression ) * 100 );
    if ( !sink_output->addFeature( fixedFeature, QgsFeatureSink::FastInsert ) )
      throw QgsProcessingException( writeFeatureError( sink_output.get(), parameters, u"OUTPUT"_s ) );
  }
  multiStepFeedback.setProgress( 100 );

  QVariantMap outputs;
  outputs.insert( u"OUTPUT"_s, dest_output );
  outputs.insert( u"REPORT"_s, dest_report );

  return outputs;
}

bool QgsFixGeometryDuplicateNodesAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mTolerance = parameterAsInt( parameters, u"TOLERANCE"_s, context );

  return true;
}

Qgis::ProcessingAlgorithmFlags QgsFixGeometryDuplicateNodesAlgorithm::flags() const
{
  return QgsProcessingAlgorithm::flags() | Qgis::ProcessingAlgorithmFlag::NoThreading | Qgis::ProcessingAlgorithmFlag::RequiresProject;
}

///@endcond
