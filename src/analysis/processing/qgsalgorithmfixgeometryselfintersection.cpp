/***************************************************************************
                        qgsalgorithmfixgeometryselfintersection.cpp
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

#include "qgsalgorithmfixgeometryselfintersection.h"

#include "qgsgeometrycheckcontext.h"
#include "qgsgeometrycheckerror.h"
#include "qgsgeometryselfintersectioncheck.h"
#include "qgsvectordataproviderfeaturepool.h"
#include "qgsvectorfilewriter.h"

///@cond PRIVATE

QString QgsFixGeometrySelfIntersectionAlgorithm::name() const
{
  return u"fixgeometryselfintersection"_s;
}

QString QgsFixGeometrySelfIntersectionAlgorithm::displayName() const
{
  return QObject::tr( "Split self-intersecting geometries" );
}

QString QgsFixGeometrySelfIntersectionAlgorithm::shortDescription() const
{
  return QObject::tr( "Splits features detected with the \"Self-intersections\" algorithm from the \"Check geometry\" section." );
}

QStringList QgsFixGeometrySelfIntersectionAlgorithm::tags() const
{
  return QObject::tr( "fix,self,intersection,split,multipart" ).split( ',' );
}

QString QgsFixGeometrySelfIntersectionAlgorithm::group() const
{
  return QObject::tr( "Fix geometry" );
}

QString QgsFixGeometrySelfIntersectionAlgorithm::groupId() const
{
  return u"fixgeometry"_s;
}

QString QgsFixGeometrySelfIntersectionAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm splits self intersecting lines or polygons according to the chosen method, "
                      "based on an error layer from the \"Self-intersections\" algorithm in the \"Check geometry\" section." );
}

QgsFixGeometrySelfIntersectionAlgorithm *QgsFixGeometrySelfIntersectionAlgorithm::createInstance() const
{
  return new QgsFixGeometrySelfIntersectionAlgorithm();
}

void QgsFixGeometrySelfIntersectionAlgorithm::initAlgorithm( const QVariantMap &configuration )
{
  Q_UNUSED( configuration )

  addParameter( new QgsProcessingParameterFeatureSource(
    u"INPUT"_s, QObject::tr( "Input layer" ),
    QList<int>()
      << static_cast<int>( Qgis::ProcessingSourceType::VectorLine )
      << static_cast<int>( Qgis::ProcessingSourceType::VectorPolygon )
  ) );
  addParameter( new QgsProcessingParameterFeatureSource(
    u"ERRORS"_s, QObject::tr( "Error layer" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorPoint )
  ) );

  QStringList methods;
  {
    QList<QgsGeometryCheckResolutionMethod> checkMethods = QgsGeometrySelfIntersectionCheck( nullptr, QVariantMap() ).availableResolutionMethods();
    std::transform(
      checkMethods.cbegin(), checkMethods.cend() - 1, std::inserter( methods, methods.begin() ),
      []( const QgsGeometryCheckResolutionMethod &checkMethod ) { return checkMethod.name(); }
    );
  }
  addParameter( new QgsProcessingParameterEnum( u"METHOD"_s, QObject::tr( "Method" ), methods ) );

  addParameter( new QgsProcessingParameterField(
    u"UNIQUE_ID"_s, QObject::tr( "Field of original feature unique identifier" ),
    QString(), u"ERRORS"_s
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
  addParameter( new QgsProcessingParameterField(
    u"SEGMENT_1"_s, QObject::tr( "Field of segment 1" ),
    u"gc_segment_1"_s, u"ERRORS"_s,
    Qgis::ProcessingFieldParameterDataType::Numeric
  ) );
  addParameter( new QgsProcessingParameterField(
    u"SEGMENT_2"_s, QObject::tr( "Field of segment 2" ),
    u"gc_segment_2"_s, u"ERRORS"_s,
    Qgis::ProcessingFieldParameterDataType::Numeric
  ) );

  addParameter( new QgsProcessingParameterFeatureSink(
    u"OUTPUT"_s, QObject::tr( "Self-intersections fixed layer" ), Qgis::ProcessingSourceType::VectorPolygon
  ) );
  addParameter( new QgsProcessingParameterFeatureSink(
    u"REPORT"_s, QObject::tr( "Report layer from fixing self-intersections" ), Qgis::ProcessingSourceType::VectorPoint
  ) );

  auto tolerance = std::make_unique<QgsProcessingParameterNumber>(
    u"TOLERANCE"_s, QObject::tr( "Tolerance" ), Qgis::ProcessingNumberParameterType::Integer, 8, false, 1, 13
  );
  tolerance->setFlags( tolerance->flags() | Qgis::ProcessingParameterFlag::Advanced );
  tolerance->setHelp( QObject::tr( "The \"Tolerance\" advanced parameter defines the numerical precision of geometric operations, "
                                   "given as an integer n, meaning that any difference smaller than 10⁻ⁿ (in map units) is considered zero." ) );
  addParameter( tolerance.release() );
}

QVariantMap QgsFixGeometrySelfIntersectionAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  const std::unique_ptr<QgsProcessingFeatureSource> input( parameterAsSource( parameters, u"INPUT"_s, context ) );
  if ( !input )
    throw QgsProcessingException( invalidSourceError( parameters, u"INPUT"_s ) );

  const std::unique_ptr<QgsProcessingFeatureSource> errors( parameterAsSource( parameters, u"ERRORS"_s, context ) );
  if ( !errors )
    throw QgsProcessingException( invalidSourceError( parameters, u"ERRORS"_s ) );

  QgsProcessingMultiStepFeedback multiStepFeedback( 3, feedback );

  const QString featIdFieldName = parameterAsString( parameters, u"UNIQUE_ID"_s, context );
  const QString partIdxFieldName = parameterAsString( parameters, u"PART_IDX"_s, context );
  const QString ringIdxFieldName = parameterAsString( parameters, u"RING_IDX"_s, context );
  const QString vertexIdxFieldName = parameterAsString( parameters, u"VERTEX_IDX"_s, context );
  const QString segment1FieldName = parameterAsString( parameters, u"SEGMENT_1"_s, context );
  const QString segment2FieldName = parameterAsString( parameters, u"SEGMENT_2"_s, context );

  const int method = parameterAsEnum( parameters, u"METHOD"_s, context );

  // Verify that input fields exists
  if ( errors->fields().indexFromName( featIdFieldName ) == -1 )
    throw QgsProcessingException( QObject::tr( "Field \"%1\" does not exist in the error layer." ).arg( featIdFieldName ) );
  if ( errors->fields().indexFromName( partIdxFieldName ) == -1 )
    throw QgsProcessingException( QObject::tr( "Field \"%1\" does not exist in the error layer." ).arg( partIdxFieldName ) );
  if ( errors->fields().indexFromName( ringIdxFieldName ) == -1 )
    throw QgsProcessingException( QObject::tr( "Field \"%1\" does not exist in the error layer." ).arg( ringIdxFieldName ) );
  if ( errors->fields().indexFromName( vertexIdxFieldName ) == -1 )
    throw QgsProcessingException( QObject::tr( "Field \"%1\" does not exist in the error layer." ).arg( vertexIdxFieldName ) );
  if ( errors->fields().indexFromName( segment1FieldName ) == -1 )
    throw QgsProcessingException( QObject::tr( "Field \"%1\" does not exist in the error layer." ).arg( segment1FieldName ) );
  if ( errors->fields().indexFromName( segment2FieldName ) == -1 )
    throw QgsProcessingException( QObject::tr( "Field \"%1\" does not exist in the error layer." ).arg( segment2FieldName ) );
  const int inputIdFieldIndex = input->fields().indexFromName( featIdFieldName );
  if ( inputIdFieldIndex == -1 )
    throw QgsProcessingException( QObject::tr( "Field \"%1\" does not exist in input layer." ).arg( featIdFieldName ) );

  const QgsField inputFeatIdField = input->fields().at( inputIdFieldIndex );
  const QMetaType::Type inputFeatIdFieldType = inputFeatIdField.type();
  if ( inputFeatIdFieldType != errors->fields().at( errors->fields().indexFromName( featIdFieldName ) ).type() )
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

  const QgsGeometrySelfIntersectionCheck check( &checkContext, QVariantMap() );

  multiStepFeedback.setCurrentStep( 1 );
  multiStepFeedback.setProgressText( QObject::tr( "Preparing features..." ) );
  std::unique_ptr<QgsVectorLayer> fixedLayer( input->materialize( QgsFeatureRequest() ) );
  QgsVectorDataProviderFeaturePool featurePool = QgsVectorDataProviderFeaturePool( fixedLayer.get(), false );
  QMap<QString, QgsFeaturePool *> featurePools;
  featurePools.insert( fixedLayer->id(), &featurePool );

  QgsFeature errorFeature, inputFeature, testDuplicateIdFeature;
  QgsFeatureIterator errorFeaturesIt = errors->getFeatures();
  QList<QgsGeometryCheck::Changes> changesList;
  QgsFeature reportFeature;
  reportFeature.setFields( reportFields );
  long long progression = 0;
  QgsFeatureIds fixedFeatures;
  long long totalProgression = errors->featureCount();
  multiStepFeedback.setCurrentStep( 2 );
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
      throw QgsProcessingException( QObject::tr( "NULL or invalid value found in unique field \"%1\"" ).arg( featIdFieldName ) );

    QString idValue = errorFeature.attribute( featIdFieldName ).toString();
    if ( inputFeatIdFieldType == QMetaType::QString )
      idValue = "'" + idValue + "'";

    QgsFeatureIterator it = fixedLayer->getFeatures( QgsFeatureRequest().setFilterExpression( "\"" + featIdFieldName + "\" = " + idValue ) );
    if ( !it.nextFeature( inputFeature ) || !inputFeature.isValid() )
    {
      reportFeature.setAttributes( errorFeature.attributes() << QObject::tr( "Source feature not found or invalid" ) << false );
      if ( !sink_report->addFeature( reportFeature, QgsFeatureSink::FastInsert ) )
        throw QgsProcessingException( writeFeatureError( sink_report.get(), parameters, u"REPORT"_s ) );
      continue;
    }

    // If a previous input feature has been fixed with the ToSingleObjects method, it means
    // that we have split the input feature into two features, with the same attribute value for
    // the uniqiue id field...
    // As a result, we must keep track of the internal feature ids of already fixed features,
    // which IS unique within the layer even after the split.
    // Here we get the next feature with the user unique id which has not been already fixed.
    bool skip = false;
    if ( method == QgsGeometrySelfIntersectionCheck::ResolutionMethod::ToSingleObjects )
    {
      while ( true )
      {
        if ( fixedFeatures.contains( inputFeature.id() ) )
        {
          if ( !it.nextFeature( inputFeature ) )
            skip = true; // should not happen if the errors layer and the input layer are coherent.
        }
        else
          break;
      }
    }
    else if ( method == QgsGeometrySelfIntersectionCheck::ResolutionMethod::ToMultiObject )
    {
      if ( it.nextFeature( testDuplicateIdFeature ) )
        throw QgsProcessingException( QObject::tr( "More than one feature found in input layer with value %1 in unique field %2" ).arg( idValue, featIdFieldName ) );
    }
    if ( skip )
      continue;

    if ( inputFeature.geometry().isNull() )
      reportFeature.setAttributes( errorFeature.attributes() << QObject::tr( "Feature geometry is null" ) << false );

    else if ( QgsGeometryCheckerUtils::getGeomPart( inputFeature.geometry().constGet(), errorFeature.attribute( partIdxFieldName ).toInt() ) == nullptr )
      reportFeature.setAttributes( errorFeature.attributes() << QObject::tr( "Feature geometry part is null" ) << false );

    else
    {
      QgsGeometryUtils::SelfIntersection intersection;
      intersection.segment1 = errorFeature.attribute( segment1FieldName ).toInt();
      intersection.segment2 = errorFeature.attribute( segment2FieldName ).toInt();
      QgsGeometrySelfIntersectionCheckError intersectionError = QgsGeometrySelfIntersectionCheckError(
        &check,
        inputFeature.geometry(),
        errorFeature.geometry(),
        QgsVertexId(
          errorFeature.attribute( partIdxFieldName ).toInt(),
          errorFeature.attribute( ringIdxFieldName ).toInt(),
          errorFeature.attribute( vertexIdxFieldName ).toInt()
        ),
        intersection
      );
      QgsGeometryCheckErrorSingle checkError = QgsGeometryCheckErrorSingle(
        &intersectionError,
        QgsGeometryCheckerUtils::LayerFeature( &featurePool, inputFeature, &checkContext, false )
      );
      for ( const QgsGeometryCheck::Changes &changes : std::as_const( changesList ) )
        checkError.handleChanges( changes );

      QgsGeometryCheck::Changes changes;

      check.fixError( featurePools, &checkError, method, QMap<QString, int>(), changes );
      changesList << changes;

      QString resolutionMessage = checkError.resolutionMessage();
      if ( checkError.status() == QgsGeometryCheckError::StatusObsolete )
        resolutionMessage = QObject::tr( "Error is obsolete" );
      else if ( checkError.status() == QgsGeometryCheckError::StatusFixed )
        fixedFeatures << inputFeature.id();

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

bool QgsFixGeometrySelfIntersectionAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mTolerance = parameterAsInt( parameters, u"TOLERANCE"_s, context );

  return true;
}

Qgis::ProcessingAlgorithmFlags QgsFixGeometrySelfIntersectionAlgorithm::flags() const
{
  return QgsProcessingAlgorithm::flags() | Qgis::ProcessingAlgorithmFlag::NoThreading | Qgis::ProcessingAlgorithmFlag::RequiresProject;
}

///@endcond
