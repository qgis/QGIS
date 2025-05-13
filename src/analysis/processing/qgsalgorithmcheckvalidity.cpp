/***************************************************************************
                         qgsalgorithmcheckvalidity.cpp
                         ---------------------
    begin                : February 2025
    copyright            : (C) 2025 by Alexander Bruy
    email                : alexander dot bruy at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsalgorithmcheckvalidity.h"
#include "qgsvectorlayer.h"
#include "qgssettingsregistrycore.h"

///@cond PRIVATE

QString QgsCheckValidityAlgorithm::name() const
{
  return QStringLiteral( "checkvalidity" );
}

QString QgsCheckValidityAlgorithm::displayName() const
{
  return QObject::tr( "Check validity" );
}

QStringList QgsCheckValidityAlgorithm::tags() const
{
  return QObject::tr( "valid,invalid,detect,error" ).split( ',' );
}

QString QgsCheckValidityAlgorithm::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsCheckValidityAlgorithm::groupId() const
{
  return QStringLiteral( "vectorgeometry" );
}

QString QgsCheckValidityAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm performs a validity check on the geometries of a vector layer.\n\n"
                      "The geometries are classified in three groups (valid, invalid and error), and a vector layer "
                      "is generated with the features in each of these categories.\n\n"
                      "By default the algorithm uses the strict OGC definition of polygon validity, where a polygon "
                      "is marked as invalid if a self-intersecting ring causes an interior hole. If the 'Ignore "
                      "ring self intersections' option is checked, then this rule will be ignored and a more "
                      "lenient validity check will be performed.\n\n"
                      "The GEOS method is faster and performs better on larger geometries, but is limited to only "
                      "returning the first error encountered in a geometry. The QGIS method will be slower but "
                      "reports all errors encountered in the geometry, not just the first." );
}

QString QgsCheckValidityAlgorithm::shortDescription() const
{
  return QObject::tr( "Performs a validity check on the geometries of a vector layer "
                      "and classifies them in three groups (valid, invalid and error)." );
}

QgsCheckValidityAlgorithm *QgsCheckValidityAlgorithm::createInstance() const
{
  return new QgsCheckValidityAlgorithm();
}

void QgsCheckValidityAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT_LAYER" ), QObject::tr( "Input layer" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorAnyGeometry ) ) );

  const QStringList options = QStringList()
                              << QObject::tr( "The one selected in digitizing settings" )
                              << QStringLiteral( "QGIS" )
                              << QStringLiteral( "GEOS" );
  auto methodParam = std::make_unique<QgsProcessingParameterEnum>( QStringLiteral( "METHOD" ), QObject::tr( "Method" ), options, false, 2 );
  QVariantMap methodParamMetadata;
  QVariantMap widgetMetadata;
  widgetMetadata.insert( QStringLiteral( "useCheckBoxes" ), true );
  widgetMetadata.insert( QStringLiteral( "columns" ), 3 );
  methodParamMetadata.insert( QStringLiteral( "widget_wrapper" ), widgetMetadata );
  methodParam->setMetadata( methodParamMetadata );
  addParameter( methodParam.release() );

  addParameter( new QgsProcessingParameterBoolean( QStringLiteral( "IGNORE_RING_SELF_INTERSECTION" ), QObject::tr( "Ignore ring self intersections" ), false ) );
  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "VALID_OUTPUT" ), QObject::tr( "Valid output" ), Qgis::ProcessingSourceType::VectorAnyGeometry, QVariant(), true ) );
  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "INVALID_OUTPUT" ), QObject::tr( "Invalid output" ), Qgis::ProcessingSourceType::VectorAnyGeometry, QVariant(), true ) );
  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "ERROR_OUTPUT" ), QObject::tr( "Error output" ), Qgis::ProcessingSourceType::VectorAnyGeometry, QVariant(), true ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "VALID_COUNT" ), QObject::tr( "Count of valid features" ) ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "INVALID_COUNT" ), QObject::tr( "Count of invalid features" ) ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "ERROR_COUNT" ), QObject::tr( "Count of errors" ) ) );
}

QVariantMap QgsCheckValidityAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr<QgsProcessingFeatureSource> source( parameterAsSource( parameters, QStringLiteral( "INPUT_LAYER" ), context ) );
  if ( !source )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "INPUT_LAYER" ) ) );

  int method = parameterAsEnum( parameters, QStringLiteral( "METHOD" ), context );
  if ( method == 0 )
  {
    const int methodFromSettings = QgsSettingsRegistryCore::settingsDigitizingValidateGeometries->value() - 1;
    method = methodFromSettings > 0 ? methodFromSettings : 0;
  }
  else
  {
    method--;
  }

  const bool ignoreRingSelfIntersection = parameterAsBool( parameters, QStringLiteral( "IGNORE_RING_SELF_INTERSECTION" ), context );

  const Qgis::GeometryValidityFlags flags = ignoreRingSelfIntersection ? Qgis::GeometryValidityFlag::AllowSelfTouchingHoles : Qgis::GeometryValidityFlags();

  QString validSinkId;
  std::unique_ptr<QgsFeatureSink> validSink( parameterAsSink( parameters, QStringLiteral( "VALID_OUTPUT" ), context, validSinkId, source->fields(), source->wkbType(), source->sourceCrs() ) );

  QgsFields invalidFields = source->fields();
  invalidFields.append( QgsField( QStringLiteral( "_errors" ), QMetaType::Type::QString, QStringLiteral( "string" ), 255 ) );
  QString invalidSinkId;
  std::unique_ptr<QgsFeatureSink> invalidSink( parameterAsSink( parameters, QStringLiteral( "INVALID_OUTPUT" ), context, invalidSinkId, invalidFields, source->wkbType(), source->sourceCrs() ) );

  QgsFields errorFields;
  errorFields.append( QgsField( QStringLiteral( "message" ), QMetaType::Type::QString, QStringLiteral( "string" ), 255 ) );
  QString errorSinkId;
  std::unique_ptr<QgsFeatureSink> errorSink( parameterAsSink( parameters, QStringLiteral( "ERROR_OUTPUT" ), context, errorSinkId, errorFields, Qgis::WkbType::Point, source->sourceCrs() ) );

  int validCount = 0;
  int invalidCount = 0;
  int errorCount = 0;

  const long count = source->featureCount();
  const double step = count > 0 ? 100.0 / count : 1;
  long long current = 0;

  QgsFeatureIterator it = source->getFeatures( QgsFeatureRequest(), Qgis::ProcessingFeatureSourceFlag::SkipGeometryValidityChecks );
  QgsFeature f;
  while ( it.nextFeature( f ) )
  {
    if ( feedback->isCanceled() )
    {
      break;
    }

    const QgsGeometry geom = f.geometry();
    QgsAttributes attrs = f.attributes();

    bool isValid = true;

    if ( !geom.isNull() && !geom.isEmpty() )
    {
      QVector< QgsGeometry::Error > errors;
      geom.validateGeometry( errors, Qgis::GeometryValidationEngine( method ), flags );
      if ( errors.count() > 0 )
      {
        isValid = false;
        QStringList reasons;
        reasons.reserve( errors.count() );
        for ( const QgsGeometry::Error &error : std::as_const( errors ) )
        {
          if ( errorSink )
          {
            QgsFeature f;
            f.setGeometry( QgsGeometry::fromPointXY( error.where() ) );
            f.setAttributes( QVector< QVariant >() << error.what() );
            if ( !errorSink->addFeature( f, QgsFeatureSink::FastInsert ) )
            {
              throw QgsProcessingException( writeFeatureError( errorSink.get(), parameters, QStringLiteral( "ERROR_OUTPUT" ) ) );
            }
          }
          errorCount++;
          reasons.append( error.what() );
        }
        QString reason = reasons.join( '\n' );
        if ( reason.size() > 255 )
        {
          reason = reason.left( 252 ) + QStringLiteral( "…" );
        }
        attrs.append( reason );
      }
    }

    QgsFeature f;
    f.setGeometry( geom );
    f.setAttributes( attrs );

    if ( isValid )
    {
      if ( validSink && !validSink->addFeature( f, QgsFeatureSink::FastInsert ) )
      {
        throw QgsProcessingException( writeFeatureError( validSink.get(), parameters, QStringLiteral( "VALID_OUTPUT" ) ) );
      }
      validCount++;
    }
    else
    {
      if ( invalidSink && !invalidSink->addFeature( f, QgsFeatureSink::FastInsert ) )
      {
        throw QgsProcessingException( writeFeatureError( invalidSink.get(), parameters, QStringLiteral( "INVALID_OUTPUT" ) ) );
      }
      invalidCount++;
    }

    feedback->setProgress( current * step );
    current++;
  }

  if ( validSink )
  {
    validSink->finalize();
  }
  if ( invalidSink )
  {
    invalidSink->finalize();
  }
  if ( errorSink )
  {
    errorSink->finalize();
  }

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "VALID_COUNT" ), validCount );
  outputs.insert( QStringLiteral( "INVALID_COUNT" ), invalidCount );
  outputs.insert( QStringLiteral( "ERROR_COUNT" ), errorCount );

  if ( validSink )
  {
    outputs.insert( QStringLiteral( "VALID_OUTPUT" ), validSinkId );
  }
  if ( invalidSink )
  {
    outputs.insert( QStringLiteral( "INVALID_OUTPUT" ), invalidSinkId );
  }
  if ( errorSink )
  {
    outputs.insert( QStringLiteral( "ERROR_OUTPUT" ), errorSinkId );
  }

  return outputs;
}

///@endcond
