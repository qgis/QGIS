/***************************************************************************
                        qgsalgorithmfixgeometrygap.cpp
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

#include "qgsalgorithmfixgeometrygap.h"

#include "qgsgeometrycheckcontext.h"
#include "qgsgeometrycheckerror.h"
#include "qgsgeometrygapcheck.h"
#include "qgsvectordataproviderfeaturepool.h"

///@cond PRIVATE

QString QgsFixGeometryGapAlgorithm::name() const
{
  return u"fixgeometrygap"_s;
}

QString QgsFixGeometryGapAlgorithm::displayName() const
{
  return QObject::tr( "Fill gaps" );
}

QString QgsFixGeometryGapAlgorithm::shortDescription() const
{
  return QObject::tr( "Fills gaps detected with the \"Small gaps\" algorithm from the \"Check geometry\" section." );
}

QStringList QgsFixGeometryGapAlgorithm::tags() const
{
  return QObject::tr( "fix,fill,gap" ).split( ',' );
}

QString QgsFixGeometryGapAlgorithm::group() const
{
  return QObject::tr( "Fix geometry" );
}

QString QgsFixGeometryGapAlgorithm::groupId() const
{
  return u"fixgeometry"_s;
}

QString QgsFixGeometryGapAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm fills the gaps based on a gap and neighbors layer from the \"Small gaps\" algorithm in the \"Check geometry\" section.\n\n"
                      "3 different fixing methods are available, which will give different results." );
}

QgsFixGeometryGapAlgorithm *QgsFixGeometryGapAlgorithm::createInstance() const
{
  return new QgsFixGeometryGapAlgorithm();
}

void QgsFixGeometryGapAlgorithm::initAlgorithm( const QVariantMap &configuration )
{
  Q_UNUSED( configuration )

  addParameter( new QgsProcessingParameterFeatureSource(
    u"INPUT"_s, QObject::tr( "Input layer" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorPolygon )
  ) );
  addParameter( new QgsProcessingParameterFeatureSource(
    u"NEIGHBORS"_s, QObject::tr( "Neighbors layer" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::Vector )
  ) );
  addParameter( new QgsProcessingParameterFeatureSource(
    u"GAPS"_s, QObject::tr( "Gaps layer" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorPolygon )
  ) );

  QStringList methods;
  {
    QList<QgsGeometryCheckResolutionMethod> checkMethods = QgsGeometryGapCheck( nullptr, QVariantMap() ).availableResolutionMethods();
    std::transform(
      checkMethods.cbegin(), checkMethods.cend() - 1, std::inserter( methods, methods.begin() ),
      []( const QgsGeometryCheckResolutionMethod &checkMethod ) { return checkMethod.name(); }
    );
  }
  addParameter( new QgsProcessingParameterEnum( u"METHOD"_s, QObject::tr( "Method" ), methods ) );

  addParameter( new QgsProcessingParameterField(
    u"UNIQUE_ID"_s, QObject::tr( "Field of original feature unique identifier" ),
    QString(), u"INPUT"_s
  ) );
  addParameter( new QgsProcessingParameterField(
    u"ERROR_ID_IDX"_s, QObject::tr( "Field of error id" ),
    u"gc_errorid"_s, u"GAPS"_s,
    Qgis::ProcessingFieldParameterDataType::Numeric
  ) );

  addParameter( new QgsProcessingParameterFeatureSink(
    u"OUTPUT"_s, QObject::tr( "Gaps-filled layer" ), Qgis::ProcessingSourceType::VectorPolygon
  ) );
  addParameter( new QgsProcessingParameterFeatureSink(
    u"REPORT"_s, QObject::tr( "Report layer from fixing gaps" ), Qgis::ProcessingSourceType::VectorPoint
  ) );

  auto tolerance = std::make_unique<QgsProcessingParameterNumber>(
    u"TOLERANCE"_s, QObject::tr( "Tolerance" ), Qgis::ProcessingNumberParameterType::Integer, 8, false, 1, 13
  );
  tolerance->setFlags( tolerance->flags() | Qgis::ProcessingParameterFlag::Advanced );
  tolerance->setHelp( QObject::tr( "The \"Tolerance\" advanced parameter defines the numerical precision of geometric operations, "
                                   "given as an integer n, meaning that any difference smaller than 10⁻ⁿ (in map units) is considered zero." ) );
  addParameter( tolerance.release() );
}

QVariantMap QgsFixGeometryGapAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  const std::unique_ptr<QgsProcessingFeatureSource> input( parameterAsSource( parameters, u"INPUT"_s, context ) );
  if ( !input )
    throw QgsProcessingException( invalidSourceError( parameters, u"INPUT"_s ) );

  const std::unique_ptr<QgsProcessingFeatureSource> neighbors( parameterAsSource( parameters, u"NEIGHBORS"_s, context ) );
  if ( !neighbors )
    throw QgsProcessingException( invalidSourceError( parameters, u"NEIGHBORS"_s ) );

  const std::unique_ptr<QgsProcessingFeatureSource> gaps( parameterAsSource( parameters, u"GAPS"_s, context ) );
  if ( !gaps )
    throw QgsProcessingException( invalidSourceError( parameters, u"GAPS"_s ) );

  QgsProcessingMultiStepFeedback multiStepFeedback( 3, feedback );

  const QString featIdFieldName = parameterAsString( parameters, u"UNIQUE_ID"_s, context );
  const QString errorIdFieldName = parameterAsString( parameters, u"ERROR_ID_IDX"_s, context );

  // Specific inputs for this check
  int method = parameterAsEnum( parameters, u"METHOD"_s, context );
  switch ( method )
  {
    case 0:
      method = QgsGeometryGapCheck::ResolutionMethod::MergeLongestEdge;
      break;
    case 1:
      method = QgsGeometryGapCheck::ResolutionMethod::CreateNewFeature;
      break;
    case 2:
      method = QgsGeometryGapCheck::ResolutionMethod::MergeLargestArea;
      break;
    default:
      throw QgsProcessingException( QObject::tr( "Unknown resolution method" ) );
  }

  // Verify that input fields exists
  if ( gaps->fields().indexFromName( errorIdFieldName ) == -1 )
    throw QgsProcessingException( QObject::tr( "Field \"%1\" does not exist in the gaps layer." ).arg( errorIdFieldName ) );
  if ( neighbors->fields().indexFromName( featIdFieldName ) == -1 )
    throw QgsProcessingException( QObject::tr( "Field \"%1\" does not exist in the neighbors layer." ).arg( featIdFieldName ) );
  const int inputIdFieldIndex = input->fields().indexFromName( featIdFieldName );
  if ( inputIdFieldIndex == -1 )
    throw QgsProcessingException( QObject::tr( "Field \"%1\" does not exist in input layer." ).arg( featIdFieldName ) );

  const QgsField inputFeatIdField = input->fields().at( inputIdFieldIndex );
  const QMetaType::Type inputFeatIdFieldType = inputFeatIdField.type();
  if ( inputFeatIdFieldType != neighbors->fields().at( neighbors->fields().indexFromName( featIdFieldName ) ).type() )
    throw QgsProcessingException( QObject::tr( "Field \"%1\" does not have the same type as in the neighbors layer." ).arg( featIdFieldName ) );

  QString dest_output;
  const std::unique_ptr<QgsFeatureSink> sink_output( parameterAsSink(
    parameters, u"OUTPUT"_s, context, dest_output, input->fields(), input->wkbType(), input->sourceCrs()
  ) );
  if ( !sink_output )
    throw QgsProcessingException( invalidSinkError( parameters, u"OUTPUT"_s ) );

  QString dest_report;
  QgsFields reportFields = gaps->fields();
  reportFields.append( QgsField( u"report"_s, QMetaType::QString ) );
  reportFields.append( QgsField( u"error_fixed"_s, QMetaType::Bool ) );
  const std::unique_ptr<QgsFeatureSink> sink_report( parameterAsSink(
    parameters, u"REPORT"_s, context, dest_report, reportFields, Qgis::WkbType::Point, gaps->sourceCrs()
  ) );
  if ( !sink_report )
    throw QgsProcessingException( invalidSinkError( parameters, u"REPORT"_s ) );

  QgsGeometryCheckContext checkContext = QgsGeometryCheckContext( mTolerance, input->sourceCrs(), context.transformContext(), context.project() );

  const QgsGeometryGapCheck check( &checkContext, QVariantMap() );

  multiStepFeedback.setCurrentStep( 1 );
  multiStepFeedback.setProgressText( QObject::tr( "Preparing features..." ) );
  std::unique_ptr<QgsVectorLayer> fixedLayer( input->materialize( QgsFeatureRequest() ) );
  QgsVectorDataProviderFeaturePool featurePool = QgsVectorDataProviderFeaturePool( fixedLayer.get(), false );
  QMap<QString, QgsFeaturePool *> featurePools;
  featurePools.insert( fixedLayer->id(), &featurePool );

  // To add features into the layer, the geometry checker looks for the layer in the project
  if ( method == QgsGeometryGapCheck::ResolutionMethod::CreateNewFeature )
  {
    context.project()->addMapLayer( fixedLayer.get(), false, false );
    fixedLayer->startEditing();
  }

  QgsFeature gapFeature;
  QgsFeatureIterator gapsFeaturesIt = gaps->getFeatures();
  QgsFeature reportFeature;
  reportFeature.setFields( reportFields );
  long long progression = 0;
  long long totalProgression = gaps->featureCount();
  multiStepFeedback.setCurrentStep( 2 );
  multiStepFeedback.setProgressText( QObject::tr( "Fixing errors..." ) );
  while ( gapsFeaturesIt.nextFeature( gapFeature ) )
  {
    if ( feedback->isCanceled() )
      break;

    progression++;
    multiStepFeedback.setProgress( static_cast<double>( static_cast<long double>( progression ) / totalProgression ) * 100 );
    reportFeature.setGeometry( gapFeature.geometry().centroid() );

    QVariant gapId = gapFeature.attribute( errorIdFieldName );
    if ( !gapId.isValid() || gapId.isNull() )
      throw QgsProcessingException( QObject::tr( "NULL or invalid value found in field \"%1\"" ).arg( errorIdFieldName ) );

    // Get the neighbor features of the current gap to fill the neighbors ids
    QgsFeature f;
    QgsFeatureIds neighborIds;
    const QString errorIdValue = gapFeature.attribute( errorIdFieldName ).toString();
    QgsFeatureIterator it = neighbors->getFeatures( QgsFeatureRequest().setFilterExpression( "\"" + errorIdFieldName + "\" = " + errorIdValue ) );
    while ( it.nextFeature( f ) )
    {
      QString neighborIdValue = f.attribute( featIdFieldName ).toString();
      if ( inputFeatIdFieldType == QMetaType::QString )
        neighborIdValue = "'" + neighborIdValue + "'";
      QgsFeature neighborFeature;
      if ( fixedLayer->getFeatures( QgsFeatureRequest().setFilterExpression( "\"" + featIdFieldName + "\" = " + neighborIdValue ) ).nextFeature( neighborFeature ) )
        neighborIds << neighborFeature.id();
    }

    QMap<QString, QgsFeatureIds> neighborsMap;
    neighborsMap.insert( fixedLayer->id(), neighborIds );
    QgsGeometryGapCheckError gapError = QgsGeometryGapCheckError(
      &check,
      fixedLayer->id(),
      gapFeature.geometry(),
      neighborsMap,
      0,
      QgsRectangle(),
      QgsRectangle()
    );

    QgsGeometryCheck::Changes changes;
    check.fixError( featurePools, &gapError, method, QMap<QString, int>(), changes );

    QString resolutionMessage = gapError.resolutionMessage();
    if ( gapError.status() == QgsGeometryCheckError::StatusObsolete )
      resolutionMessage = QObject::tr( "Error is obsolete" );

    reportFeature.setAttributes( gapFeature.attributes() << resolutionMessage << ( gapError.status() == QgsGeometryCheckError::StatusFixed ) );

    if ( !sink_report->addFeature( reportFeature, QgsFeatureSink::FastInsert ) )
      throw QgsProcessingException( writeFeatureError( sink_report.get(), parameters, u"REPORT"_s ) );
  }
  multiStepFeedback.setProgress( 100 );

  if ( method == QgsGeometryGapCheck::ResolutionMethod::CreateNewFeature )
  {
    if ( !fixedLayer->commitChanges() )
      throw QgsProcessingException( QObject::tr( "Unable to add gap features" ) );
    context.project()->removeMapLayer( fixedLayer.get() );
  }

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

bool QgsFixGeometryGapAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mTolerance = parameterAsInt( parameters, u"TOLERANCE"_s, context );

  return true;
}

Qgis::ProcessingAlgorithmFlags QgsFixGeometryGapAlgorithm::flags() const
{
  return QgsProcessingAlgorithm::flags() | Qgis::ProcessingAlgorithmFlag::NoThreading | Qgis::ProcessingAlgorithmFlag::RequiresProject;
}

///@endcond
