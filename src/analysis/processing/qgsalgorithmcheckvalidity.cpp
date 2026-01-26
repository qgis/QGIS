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

#include "qgssettingsregistrycore.h"
#include "qgsvectorlayer.h"

///@cond PRIVATE

QString QgsCheckValidityAlgorithm::name() const
{
  return u"checkvalidity"_s;
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
  return u"vectorgeometry"_s;
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
  addParameter( new QgsProcessingParameterFeatureSource( u"INPUT_LAYER"_s, QObject::tr( "Input layer" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorAnyGeometry ) ) );

  const QStringList options = QStringList()
                              << QObject::tr( "The one selected in digitizing settings" )
                              << u"QGIS"_s
                              << u"GEOS"_s;
  auto methodParam = std::make_unique<QgsProcessingParameterEnum>( u"METHOD"_s, QObject::tr( "Method" ), options, false, 2 );
  QVariantMap methodParamMetadata;
  QVariantMap widgetMetadata;
  widgetMetadata.insert( u"useCheckBoxes"_s, true );
  widgetMetadata.insert( u"columns"_s, 3 );
  methodParamMetadata.insert( u"widget_wrapper"_s, widgetMetadata );
  methodParam->setMetadata( methodParamMetadata );
  addParameter( methodParam.release() );

  addParameter( new QgsProcessingParameterBoolean( u"IGNORE_RING_SELF_INTERSECTION"_s, QObject::tr( "Ignore ring self intersections" ), false ) );
  addParameter( new QgsProcessingParameterFeatureSink( u"VALID_OUTPUT"_s, QObject::tr( "Valid output" ), Qgis::ProcessingSourceType::VectorAnyGeometry, QVariant(), true ) );
  addParameter( new QgsProcessingParameterFeatureSink( u"INVALID_OUTPUT"_s, QObject::tr( "Invalid output" ), Qgis::ProcessingSourceType::VectorAnyGeometry, QVariant(), true ) );
  addParameter( new QgsProcessingParameterFeatureSink( u"ERROR_OUTPUT"_s, QObject::tr( "Error output" ), Qgis::ProcessingSourceType::VectorAnyGeometry, QVariant(), true ) );
  addOutput( new QgsProcessingOutputNumber( u"VALID_COUNT"_s, QObject::tr( "Count of valid features" ) ) );
  addOutput( new QgsProcessingOutputNumber( u"INVALID_COUNT"_s, QObject::tr( "Count of invalid features" ) ) );
  addOutput( new QgsProcessingOutputNumber( u"ERROR_COUNT"_s, QObject::tr( "Count of errors" ) ) );
}

QVariantMap QgsCheckValidityAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr<QgsProcessingFeatureSource> source( parameterAsSource( parameters, u"INPUT_LAYER"_s, context ) );
  if ( !source )
    throw QgsProcessingException( invalidSourceError( parameters, u"INPUT_LAYER"_s ) );

  int method = parameterAsEnum( parameters, u"METHOD"_s, context );
  if ( method == 0 )
  {
    const int methodFromSettings = QgsSettingsRegistryCore::settingsDigitizingValidateGeometries->value() - 1;
    method = methodFromSettings > 0 ? methodFromSettings : 0;
  }
  else
  {
    method--;
  }

  const bool ignoreRingSelfIntersection = parameterAsBool( parameters, u"IGNORE_RING_SELF_INTERSECTION"_s, context );

  const Qgis::GeometryValidityFlags flags = ignoreRingSelfIntersection ? Qgis::GeometryValidityFlag::AllowSelfTouchingHoles : Qgis::GeometryValidityFlags();

  QString validSinkId;
  std::unique_ptr<QgsFeatureSink> validSink( parameterAsSink( parameters, u"VALID_OUTPUT"_s, context, validSinkId, source->fields(), source->wkbType(), source->sourceCrs() ) );

  QgsFields invalidFields = source->fields();
  invalidFields.append( QgsField( u"_errors"_s, QMetaType::Type::QString, u"string"_s, 255 ) );
  QString invalidSinkId;
  std::unique_ptr<QgsFeatureSink> invalidSink( parameterAsSink( parameters, u"INVALID_OUTPUT"_s, context, invalidSinkId, invalidFields, source->wkbType(), source->sourceCrs() ) );

  QgsFields errorFields;
  errorFields.append( QgsField( u"message"_s, QMetaType::Type::QString, u"string"_s, 255 ) );
  QString errorSinkId;
  std::unique_ptr<QgsFeatureSink> errorSink( parameterAsSink( parameters, u"ERROR_OUTPUT"_s, context, errorSinkId, errorFields, Qgis::WkbType::Point, source->sourceCrs() ) );

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
              throw QgsProcessingException( writeFeatureError( errorSink.get(), parameters, u"ERROR_OUTPUT"_s ) );
            }
          }
          errorCount++;
          reasons.append( error.what() );
        }
        QString reason = reasons.join( '\n' );
        if ( reason.size() > 255 )
        {
          reason = reason.left( 252 ) + u"…"_s;
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
        throw QgsProcessingException( writeFeatureError( validSink.get(), parameters, u"VALID_OUTPUT"_s ) );
      }
      validCount++;
    }
    else
    {
      if ( invalidSink && !invalidSink->addFeature( f, QgsFeatureSink::FastInsert ) )
      {
        throw QgsProcessingException( writeFeatureError( invalidSink.get(), parameters, u"INVALID_OUTPUT"_s ) );
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
  outputs.insert( u"VALID_COUNT"_s, validCount );
  outputs.insert( u"INVALID_COUNT"_s, invalidCount );
  outputs.insert( u"ERROR_COUNT"_s, errorCount );

  if ( validSink )
  {
    outputs.insert( u"VALID_OUTPUT"_s, validSinkId );
  }
  if ( invalidSink )
  {
    outputs.insert( u"INVALID_OUTPUT"_s, invalidSinkId );
  }
  if ( errorSink )
  {
    outputs.insert( u"ERROR_OUTPUT"_s, errorSinkId );
  }

  return outputs;
}

///@endcond
