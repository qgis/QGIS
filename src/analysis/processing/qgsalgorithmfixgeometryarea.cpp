/***************************************************************************
                        qgsalgorithmfixgeometryarea.cpp
                        ---------------------
   begin                : June 2024
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

#include "qgsalgorithmfixgeometryarea.h"

#include "qgsgeometryareacheck.h"
#include "qgsgeometrycheckerror.h"
#include "qgsvectordataproviderfeaturepool.h"
#include "qgsvectorfilewriter.h"

///@cond PRIVATE

QString QgsFixGeometryAreaAlgorithm::name() const
{
  return u"fixgeometryarea"_s;
}

QString QgsFixGeometryAreaAlgorithm::displayName() const
{
  return QObject::tr( "Fix small polygons" );
}

QString QgsFixGeometryAreaAlgorithm::shortDescription() const
{
  return QObject::tr( "Merges small polygons detected with the \"Small polygons\" algorithm from the \"Check geometry\" section." );
}

QStringList QgsFixGeometryAreaAlgorithm::tags() const
{
  return QObject::tr( "merge,polygons,neighbor,fix,area" ).split( ',' );
}

QString QgsFixGeometryAreaAlgorithm::group() const
{
  return QObject::tr( "Fix geometry" );
}

QString QgsFixGeometryAreaAlgorithm::groupId() const
{
  return u"fixgeometry"_s;
}

QString QgsFixGeometryAreaAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm merges neighboring polygons according to the chosen method, "
                      "based on an error layer from the \"Small polygons\" algorithm in the \"Check geometry\" section." );
}

QgsFixGeometryAreaAlgorithm *QgsFixGeometryAreaAlgorithm::createInstance() const
{
  return new QgsFixGeometryAreaAlgorithm();
}

void QgsFixGeometryAreaAlgorithm::initAlgorithm( const QVariantMap &configuration )
{
  Q_UNUSED( configuration )

  addParameter( new QgsProcessingParameterFeatureSource(
    u"INPUT"_s, QObject::tr( "Input layer" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorPolygon )
  ) );
  addParameter( new QgsProcessingParameterFeatureSource(
    u"ERRORS"_s, QObject::tr( "Error layer" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorPoint )
  ) );

  QStringList methods;
  {
    QList<QgsGeometryCheckResolutionMethod> checkMethods = QgsGeometryAreaCheck( nullptr, QVariantMap() ).availableResolutionMethods();
    std::transform(
      checkMethods.cbegin(), checkMethods.cend() - 2, std::inserter( methods, methods.begin() ),
      []( const QgsGeometryCheckResolutionMethod &checkMethod ) { return checkMethod.name(); }
    );
  }
  addParameter( new QgsProcessingParameterEnum( u"METHOD"_s, QObject::tr( "Method" ), methods ) );
  addParameter( new QgsProcessingParameterField(
    u"MERGE_ATTRIBUTE"_s, QObject::tr( "Field to consider when merging polygons with the identical attribute method" ),
    QString(), u"INPUT"_s,
    Qgis::ProcessingFieldParameterDataType::Any, false, true
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
    u"OUTPUT"_s, QObject::tr( "Small polygons merged layer" ), Qgis::ProcessingSourceType::VectorPolygon
  ) );
  addParameter( new QgsProcessingParameterFeatureSink(
    u"REPORT"_s, QObject::tr( "Report layer from merging small polygons" ), Qgis::ProcessingSourceType::VectorPoint
  ) );

  auto tolerance = std::make_unique<QgsProcessingParameterNumber>(
    u"TOLERANCE"_s, QObject::tr( "Tolerance" ), Qgis::ProcessingNumberParameterType::Integer, 8, false, 1, 13
  );
  tolerance->setFlags( tolerance->flags() | Qgis::ProcessingParameterFlag::Advanced );
  tolerance->setHelp( QObject::tr( "The \"Tolerance\" advanced parameter defines the numerical precision of geometric operations, "
                                   "given as an integer n, meaning that any difference smaller than 10⁻ⁿ (in map units) is considered zero." ) );
  addParameter( tolerance.release() );
}

QVariantMap QgsFixGeometryAreaAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
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

  // Specific inputs for this check
  const QString mergeAttributeName = parameterAsString( parameters, u"MERGE_ATTRIBUTE"_s, context );
  const int method = parameterAsEnum( parameters, u"METHOD"_s, context );

  // Verify that input fields exists
  if ( errors->fields().indexFromName( featIdFieldName ) == -1 )
    throw QgsProcessingException( QObject::tr( "Field %1 does not exist in the error layer." ).arg( featIdFieldName ) );
  if ( errors->fields().indexFromName( partIdxFieldName ) == -1 )
    throw QgsProcessingException( QObject::tr( "Field %1 does not exist in the error layer." ).arg( partIdxFieldName ) );
  if ( errors->fields().indexFromName( ringIdxFieldName ) == -1 )
    throw QgsProcessingException( QObject::tr( "Field %1 does not exist in the error layer." ).arg( ringIdxFieldName ) );
  if ( errors->fields().indexFromName( vertexIdxFieldName ) == -1 )
    throw QgsProcessingException( QObject::tr( "Field %1 does not exist in the error layer." ).arg( vertexIdxFieldName ) );
  const int inputIdFieldIndex = input->fields().indexFromName( featIdFieldName );
  if ( inputIdFieldIndex == -1 )
    throw QgsProcessingException( QObject::tr( "Field %1 does not exist in input layer." ).arg( featIdFieldName ) );

  const QgsField inputFeatIdField = input->fields().at( inputIdFieldIndex );
  if ( inputFeatIdField.type() != errors->fields().at( errors->fields().indexFromName( featIdFieldName ) ).type() )
    throw QgsProcessingException( QObject::tr( "Field %1 does not have the same type as in the error layer." ).arg( featIdFieldName ) );

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

  QgsGeometryCheckContext checkContext = QgsGeometryCheckContext( mTolerance, input->sourceCrs(), context.transformContext(), context.project() );
  QVariantMap configurationCheck;

  // maximum limit, we know that every feature to process is an error (otherwise it is not treated and marked as obsolete)
  configurationCheck.insert( "areaThreshold", std::numeric_limits<double>::max() );
  const QgsGeometryAreaCheck check( &checkContext, configurationCheck );

  std::unique_ptr<QgsVectorLayer> fixedLayer( input->materialize( QgsFeatureRequest() ) );
  QgsVectorDataProviderFeaturePool featurePool = QgsVectorDataProviderFeaturePool( fixedLayer.get(), false );
  QMap<QString, QgsFeaturePool *> featurePools;
  featurePools.insert( fixedLayer->id(), &featurePool );

  QMap<QString, int> attributeIndex;
  if ( method == QgsGeometryAreaCheck::ResolutionMethod::MergeIdenticalAttribute )
  {
    if ( mergeAttributeName.isEmpty() )
      throw QgsProcessingException( QObject::tr( "Merge field to merge polygons with identical attribute method is empty" ) );
    if ( !fixedLayer->fields().names().contains( mergeAttributeName ) )
      throw QgsProcessingException( QObject::tr( "Merge field %1 does not exist in input layer" ).arg( mergeAttributeName ) );
    attributeIndex.insert( fixedLayer->id(), fixedLayer->fields().indexOf( mergeAttributeName ) );
  }

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

    QVariant attr = errorFeature.attribute( featIdFieldName );
    if ( !attr.isValid() || attr.isNull() )
      throw QgsProcessingException( QObject::tr( "NULL or invalid value found in unique field %1" ).arg( featIdFieldName ) );

    QString idValue = errorFeature.attribute( featIdFieldName ).toString();
    if ( inputFeatIdField.type() == QMetaType::QString )
      idValue = "'" + idValue + "'";

    QgsFeatureIterator it = fixedLayer->getFeatures( QgsFeatureRequest().setFilterExpression( "\"" + featIdFieldName + "\" = " + idValue ) );
    if ( !it.nextFeature( inputFeature ) || !inputFeature.isValid() )
      reportFeature.setAttributes( errorFeature.attributes() << QObject::tr( "Source feature not found or invalid" ) << false );

    else if ( it.nextFeature( testDuplicateIdFeature ) )
      throw QgsProcessingException( QObject::tr( "More than one feature found in input layer with value %1 in unique field %2" ).arg( idValue, featIdFieldName ) );

    else if ( inputFeature.geometry().isNull() )
      reportFeature.setAttributes( errorFeature.attributes() << QObject::tr( "Feature geometry is null" ) << false );

    else if ( QgsGeometryCheckerUtils::getGeomPart( inputFeature.geometry().constGet(), errorFeature.attribute( partIdxFieldName ).toInt() ) == nullptr )
      reportFeature.setAttributes( errorFeature.attributes() << QObject::tr( "Feature geometry part is null" ) << false );

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
      check.fixError( featurePools, &checkError, method, attributeIndex, changes );
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

bool QgsFixGeometryAreaAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mTolerance = parameterAsInt( parameters, u"TOLERANCE"_s, context );

  return true;
}

Qgis::ProcessingAlgorithmFlags QgsFixGeometryAreaAlgorithm::flags() const
{
  return QgsProcessingAlgorithm::flags() | Qgis::ProcessingAlgorithmFlag::NoThreading | Qgis::ProcessingAlgorithmFlag::RequiresProject;
}

///@endcond
