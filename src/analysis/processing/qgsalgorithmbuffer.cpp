/***************************************************************************
                         qgsalgorithmbuffer.cpp
                         ---------------------
    begin                : April 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsalgorithmbuffer.h"
#include "qgsvectorlayer.h"
#include "qgsmessagelog.h"

///@cond PRIVATE

QString QgsBufferAlgorithm::name() const
{
  return QStringLiteral( "buffer" );
}

QString QgsBufferAlgorithm::displayName() const
{
  return QObject::tr( "Buffer" );
}

QStringList QgsBufferAlgorithm::tags() const
{
  return QObject::tr( "buffer,grow,fixed,variable,distance" ).split( ',' );
}

QString QgsBufferAlgorithm::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsBufferAlgorithm::groupId() const
{
  return QStringLiteral( "vectorgeometry" );
}

void QgsBufferAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ) ) );

  auto bufferParam = std::make_unique<QgsProcessingParameterDistance>( QStringLiteral( "DISTANCE" ), QObject::tr( "Distance" ), 10, QStringLiteral( "INPUT" ) );
  bufferParam->setIsDynamic( true );
  bufferParam->setDynamicPropertyDefinition( QgsPropertyDefinition( QStringLiteral( "Distance" ), QObject::tr( "Buffer distance" ), QgsPropertyDefinition::Double ) );
  bufferParam->setDynamicLayerParameterName( QStringLiteral( "INPUT" ) );
  addParameter( bufferParam.release() );
  auto segmentParam = std::make_unique<QgsProcessingParameterNumber>( QStringLiteral( "SEGMENTS" ), QObject::tr( "Segments" ), Qgis::ProcessingNumberParameterType::Integer, 5, false, 1 );
  segmentParam->setHelp( QObject::tr( "The segments parameter controls the number of line segments to use to approximate a quarter circle when creating rounded offsets." ) );
  addParameter( segmentParam.release() );
  addParameter( new QgsProcessingParameterEnum( QStringLiteral( "END_CAP_STYLE" ), QObject::tr( "End cap style" ), QStringList() << QObject::tr( "Round" ) << QObject::tr( "Flat" ) << QObject::tr( "Square" ), false, 0 ) );
  addParameter( new QgsProcessingParameterEnum( QStringLiteral( "JOIN_STYLE" ), QObject::tr( "Join style" ), QStringList() << QObject::tr( "Round" ) << QObject::tr( "Miter" ) << QObject::tr( "Bevel" ), false, 0 ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "MITER_LIMIT" ), QObject::tr( "Miter limit" ), Qgis::ProcessingNumberParameterType::Double, 2, false, 1 ) );

  addParameter( new QgsProcessingParameterBoolean( QStringLiteral( "DISSOLVE" ), QObject::tr( "Dissolve result" ), false ) );

  auto keepDisjointParam = std::make_unique<QgsProcessingParameterBoolean>( QStringLiteral( "SEPARATE_DISJOINT" ), QObject::tr( "Keep disjoint results separate" ), false );
  keepDisjointParam->setFlags( keepDisjointParam->flags() | Qgis::ProcessingParameterFlag::Advanced );
  keepDisjointParam->setHelp( QObject::tr( "If checked, then any disjoint parts in the buffer results will be output as separate single-part features." ) );
  addParameter( keepDisjointParam.release() );

  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Buffered" ), Qgis::ProcessingSourceType::VectorPolygon, QVariant(), false, true, true ) );
}

QString QgsBufferAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm computes a buffer area for all the features in an input layer, using a fixed or dynamic distance.\n\n"
                      "The segments parameter controls the number of line segments to use to approximate a quarter circle when creating rounded offsets.\n\n"
                      "The end cap style parameter controls how line endings are handled in the buffer.\n\n"
                      "The join style parameter specifies whether round, miter or beveled joins should be used when offsetting corners in a line.\n\n"
                      "The miter limit parameter is only applicable for miter join styles, and controls the maximum distance from the offset curve to use when creating a mitered join." );
}

Qgis::ProcessingAlgorithmDocumentationFlags QgsBufferAlgorithm::documentationFlags() const
{
  return Qgis::ProcessingAlgorithmDocumentationFlag::RegeneratesPrimaryKeyInSomeScenarios;
}

QgsBufferAlgorithm *QgsBufferAlgorithm::createInstance() const
{
  return new QgsBufferAlgorithm();
}

QVariantMap QgsBufferAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr<QgsProcessingFeatureSource> source( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !source )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "INPUT" ) ) );

  // fixed parameters
  const bool dissolve = parameterAsBoolean( parameters, QStringLiteral( "DISSOLVE" ), context );
  const bool keepDisjointSeparate = parameterAsBoolean( parameters, QStringLiteral( "SEPARATE_DISJOINT" ), context );
  const int segments = parameterAsInt( parameters, QStringLiteral( "SEGMENTS" ), context );
  const Qgis::EndCapStyle endCapStyle = static_cast<Qgis::EndCapStyle>( 1 + parameterAsInt( parameters, QStringLiteral( "END_CAP_STYLE" ), context ) );
  const Qgis::JoinStyle joinStyle = static_cast<Qgis::JoinStyle>( 1 + parameterAsInt( parameters, QStringLiteral( "JOIN_STYLE" ), context ) );
  const double miterLimit = parameterAsDouble( parameters, QStringLiteral( "MITER_LIMIT" ), context );
  const double bufferDistance = parameterAsDouble( parameters, QStringLiteral( "DISTANCE" ), context );
  const bool dynamicBuffer = QgsProcessingParameters::isDynamic( parameters, QStringLiteral( "DISTANCE" ) );

  QString dest;
  std::unique_ptr<QgsFeatureSink> sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, dest, source->fields(), Qgis::WkbType::MultiPolygon, source->sourceCrs(), keepDisjointSeparate ? QgsFeatureSink::RegeneratePrimaryKey : QgsFeatureSink::SinkFlags() ) );
  if ( !sink )
    throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "OUTPUT" ) ) );

  QgsExpressionContext expressionContext = createExpressionContext( parameters, context, source.get() );
  QgsProperty bufferProperty;
  if ( dynamicBuffer )
  {
    bufferProperty = parameters.value( QStringLiteral( "DISTANCE" ) ).value<QgsProperty>();
  }

  const long count = source->featureCount();

  QgsFeature f;
  // buffer doesn't care about invalid features, and buffering can be used to repair geometries
  QgsFeatureIterator it = source->getFeatures( QgsFeatureRequest(), Qgis::ProcessingFeatureSourceFlag::SkipGeometryValidityChecks );

  const double step = count > 0 ? 100.0 / count : 1;
  int current = 0;

  QVector<QgsGeometry> bufferedGeometriesForDissolve;
  QgsAttributes dissolveAttrs;

  while ( it.nextFeature( f ) )
  {
    if ( feedback->isCanceled() )
    {
      break;
    }
    if ( dissolveAttrs.isEmpty() )
      dissolveAttrs = f.attributes();

    QgsFeature out = f;
    if ( out.hasGeometry() )
    {
      double distance = bufferDistance;
      if ( dynamicBuffer )
      {
        expressionContext.setFeature( f );
        distance = bufferProperty.valueAsDouble( expressionContext, bufferDistance );
      }

      QgsGeometry outputGeometry = f.geometry().buffer( distance, segments, endCapStyle, joinStyle, miterLimit );
      if ( outputGeometry.isNull() )
      {
        QgsMessageLog::logMessage( QObject::tr( "Error calculating buffer for feature %1" ).arg( f.id() ), QObject::tr( "Processing" ), Qgis::MessageLevel::Warning );
      }
      if ( dissolve )
      {
        bufferedGeometriesForDissolve << outputGeometry;
      }
      else
      {
        outputGeometry.convertToMultiType();

        if ( !keepDisjointSeparate )
        {
          out.setGeometry( outputGeometry );

          if ( !sink->addFeature( out, QgsFeatureSink::FastInsert ) )
            throw QgsProcessingException( writeFeatureError( sink.get(), parameters, QStringLiteral( "OUTPUT" ) ) );
        }
        else
        {
          for ( auto partIt = outputGeometry.const_parts_begin(); partIt != outputGeometry.const_parts_end(); ++partIt )
          {
            if ( const QgsAbstractGeometry *part = *partIt )
            {
              out.setGeometry( QgsGeometry( part->clone() ) );
              if ( !sink->addFeature( out, QgsFeatureSink::FastInsert ) )
                throw QgsProcessingException( writeFeatureError( sink.get(), parameters, QStringLiteral( "OUTPUT" ) ) );
            }
          }
        }
      }
    }
    else if ( !dissolve )
    {
      if ( !sink->addFeature( out, QgsFeatureSink::FastInsert ) )
        throw QgsProcessingException( writeFeatureError( sink.get(), parameters, QStringLiteral( "OUTPUT" ) ) );
    }

    feedback->setProgress( current * step );
    current++;
  }

  if ( dissolve && !bufferedGeometriesForDissolve.isEmpty() )
  {
    QgsGeometry finalGeometry = QgsGeometry::unaryUnion( bufferedGeometriesForDissolve );
    finalGeometry.convertToMultiType();
    QgsFeature f;
    f.setAttributes( dissolveAttrs );

    if ( !keepDisjointSeparate )
    {
      f.setGeometry( finalGeometry );
      if ( !sink->addFeature( f, QgsFeatureSink::FastInsert ) )
        throw QgsProcessingException( writeFeatureError( sink.get(), parameters, QStringLiteral( "OUTPUT" ) ) );
    }
    else
    {
      for ( auto partIt = finalGeometry.const_parts_begin(); partIt != finalGeometry.const_parts_end(); ++partIt )
      {
        if ( const QgsAbstractGeometry *part = *partIt )
        {
          f.setGeometry( QgsGeometry( part->clone() ) );
          if ( !sink->addFeature( f, QgsFeatureSink::FastInsert ) )
            throw QgsProcessingException( writeFeatureError( sink.get(), parameters, QStringLiteral( "OUTPUT" ) ) );
        }
      }
    }
  }

  sink->finalize();

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), dest );
  return outputs;
}

Qgis::ProcessingAlgorithmFlags QgsBufferAlgorithm::flags() const
{
  Qgis::ProcessingAlgorithmFlags f = QgsProcessingAlgorithm::flags();
  f |= Qgis::ProcessingAlgorithmFlag::SupportsInPlaceEdits;
  return f;
}

QgsProcessingAlgorithm::VectorProperties QgsBufferAlgorithm::sinkProperties( const QString &sink, const QVariantMap &parameters, QgsProcessingContext &context, const QMap<QString, QgsProcessingAlgorithm::VectorProperties> &sourceProperties ) const
{
  const bool keepDisjointSeparate = parameterAsBoolean( parameters, QStringLiteral( "SEPARATE_DISJOINT" ), context );

  QgsProcessingAlgorithm::VectorProperties result;
  if ( sink == QLatin1String( "OUTPUT" ) )
  {
    if ( sourceProperties.value( QStringLiteral( "INPUT" ) ).availability == Qgis::ProcessingPropertyAvailability::Available )
    {
      const VectorProperties inputProps = sourceProperties.value( QStringLiteral( "INPUT" ) );
      result.fields = inputProps.fields;
      result.crs = inputProps.crs;
      result.wkbType = keepDisjointSeparate ? Qgis::WkbType::Polygon : Qgis::WkbType::MultiPolygon;
      result.availability = Qgis::ProcessingPropertyAvailability::Available;
      return result;
    }
    else
    {
      std::unique_ptr<QgsProcessingFeatureSource> source( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
      if ( source )
      {
        result.fields = source->fields();
        result.crs = source->sourceCrs();
        result.wkbType = keepDisjointSeparate ? Qgis::WkbType::Polygon : Qgis::WkbType::MultiPolygon;
        result.availability = Qgis::ProcessingPropertyAvailability::Available;
        return result;
      }
    }
  }
  return result;
}

bool QgsBufferAlgorithm::supportInPlaceEdit( const QgsMapLayer *layer ) const
{
  const QgsVectorLayer *vlayer = qobject_cast<const QgsVectorLayer *>( layer );
  if ( !vlayer )
    return false;
  //Only Polygons
  return vlayer->wkbType() == Qgis::WkbType::Polygon || vlayer->wkbType() == Qgis::WkbType::MultiPolygon;
}

///@endcond
