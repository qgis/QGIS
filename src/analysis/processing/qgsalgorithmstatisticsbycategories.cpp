/***************************************************************************
                         qgsalgorithmstatisticsbycategories.cpp
                         ------------------------------
    begin                : September 2026
    copyright            : (C) 2026 by Alexander Bruy
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

#include "qgsalgorithmstatisticsbycategories.h"

#include "qgsdatetimestatisticalsummary.h"
#include "qgsstatisticalsummary.h"
#include "qgsstringstatisticalsummary.h"

#include <QString>

using namespace Qt::StringLiterals;

///@cond PRIVATE

QString QgsStatisticsByCategoriesAlgorithm::name() const
{
  return u"statisticsbycategories"_s;
}

QString QgsStatisticsByCategoriesAlgorithm::displayName() const
{
  return QObject::tr( "Statistics by categories" );
}

QStringList QgsStatisticsByCategoriesAlgorithm::tags() const
{
  return QObject::tr(
           "groups,stats,statistics,table,layer,sum,maximum,minimum,mean,average,standard,deviation,count,distinct,unique,variance,median,quartile,range,majority,minority,histogram,distinct,summary"
  )
    .split( ',' );
}

QString QgsStatisticsByCategoriesAlgorithm::group() const
{
  return QObject::tr( "Vector analysis" );
}

QString QgsStatisticsByCategoriesAlgorithm::groupId() const
{
  return u"vectoranalysis"_s;
}

QString QgsStatisticsByCategoriesAlgorithm::shortHelpString() const
{
  return QObject::tr(
    "This algorithm calculates statistics of fields depending on a parent class. Numeric, date, time and string fields are supported. The statistics "
    "returned will depend on the field type."
  );
}

QString QgsStatisticsByCategoriesAlgorithm::shortDescription() const
{
  return QObject::tr( "Calculates statistics of fields depending on a parent class." );
}

QgsStatisticsByCategoriesAlgorithm *QgsStatisticsByCategoriesAlgorithm::createInstance() const
{
  return new QgsStatisticsByCategoriesAlgorithm();
}

void QgsStatisticsByCategoriesAlgorithm::initAlgorithm( const QVariantMap & )
{
  auto inputParam = std::make_unique<QgsProcessingParameterFeatureSource>( u"INPUT"_s, QObject::tr( "Input layer" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::Vector ) );
  inputParam->setHelp( QObject::tr( "Input vector layer containing values to categorize and analyze." ) );
  addParameter( inputParam.release() );

  auto valueFieldParam = std::make_unique<
    QgsProcessingParameterField>( u"VALUES_FIELD_NAME"_s, QObject::tr( "Field to calculate statistics on" ), QVariant(), u"INPUT"_s, Qgis::ProcessingFieldParameterDataType::Any, false, true );
  valueFieldParam->setHelp( QObject::tr( "Feld containing the values to analyze. If left empty, the algorithm will only compute the total feature count for each unique category." ) );
  addParameter( valueFieldParam.release() );

  auto categoryFieldsParam
    = std::make_unique<QgsProcessingParameterField>( u"CATEGORIES_FIELD_NAME"_s, QObject::tr( "Field(s) with categories" ), QVariant(), u"INPUT"_s, Qgis::ProcessingFieldParameterDataType::Any, true, false );
  categoryFieldsParam->setHelp(
    QObject::tr( "One or more fields to group features by. A separate row of statistical summary values will be generated for every unique combination of these category values." )
  );
  addParameter( categoryFieldsParam.release() );

  auto outputParam = std::make_unique<QgsProcessingParameterFeatureSink>( u"OUTPUT"_s, QObject::tr( "Statistics by category" ), Qgis::ProcessingSourceType::Vector, QVariant() );
  outputParam->setHelp( QObject::tr( "Output table where the calculated summary statistics per category will be saved." ) );
  addParameter( outputParam.release() );
}

QVariantMap QgsStatisticsByCategoriesAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QGS_MARK_ALGORITHM_SOURCE

  std::unique_ptr<QgsProcessingFeatureSource> source( parameterAsSource( parameters, u"INPUT"_s, context ) );
  if ( !source )
  {
    throw QgsProcessingException( invalidSourceError( parameters, u"INPUT"_s ) );
  }

  const QString valueFieldName = parameterAsString( parameters, u"VALUES_FIELD_NAME"_s, context );
  const int valueFieldIndex = source->fields().lookupField( valueFieldName );
  const QgsField valueField = valueFieldIndex >= 0 ? source->fields().at( valueFieldIndex ) : QgsField();

  const QStringList categoryFieldNames = parameterAsStrings( parameters, u"CATEGORIES_FIELD_NAME"_s, context );

  QList<int> categoryFieldIndexes;
  categoryFieldIndexes.reserve( categoryFieldNames.size() );
  QgsFields outputFields;
  for ( const QString &fieldName : categoryFieldNames )
  {
    const int fieldIndex = source->fields().lookupField( fieldName );
    if ( fieldIndex < 0 )
    {
      throw QgsProcessingException( QObject::tr( "Field “%1” does not exist." ).arg( fieldName ) );
    }
    categoryFieldIndexes.append( fieldIndex );
    outputFields.append( source->fields().at( fieldIndex ) );
  }

  // Adds a field to the output, keeping the same data type as the valueField
  auto addFieldWithSameType = [&]( const QString &name ) {
    QgsField field( valueField );
    field.setName( name );
    outputFields.append( field );
  };

  if ( valueFieldIndex < 0 )
  {
    outputFields.append( QgsField( u"count"_s, QMetaType::Type::Int ) );
  }
  else if ( valueField.isNumeric() )
  {
    outputFields.append( QgsField( u"count"_s, QMetaType::Type::Int ) );
    outputFields.append( QgsField( u"unique"_s, QMetaType::Type::Int ) );
    outputFields.append( QgsField( u"min"_s, QMetaType::Type::Double ) );
    outputFields.append( QgsField( u"max"_s, QMetaType::Type::Double ) );
    outputFields.append( QgsField( u"range"_s, QMetaType::Type::Double ) );
    outputFields.append( QgsField( u"sum"_s, QMetaType::Type::Double ) );
    outputFields.append( QgsField( u"mean"_s, QMetaType::Type::Double ) );
    outputFields.append( QgsField( u"median"_s, QMetaType::Type::Double ) );
    outputFields.append( QgsField( u"stddev"_s, QMetaType::Type::Double ) );
    outputFields.append( QgsField( u"minority"_s, QMetaType::Type::Double ) );
    outputFields.append( QgsField( u"majority"_s, QMetaType::Type::Double ) );
    outputFields.append( QgsField( u"q1"_s, QMetaType::Type::Double ) );
    outputFields.append( QgsField( u"q3"_s, QMetaType::Type::Double ) );
    outputFields.append( QgsField( u"iqr"_s, QMetaType::Type::Double ) );
  }
  else if ( valueField.isDateOrTime() )
  {
    outputFields.append( QgsField( u"count"_s, QMetaType::Type::Int ) );
    outputFields.append( QgsField( u"unique"_s, QMetaType::Type::Int ) );
    outputFields.append( QgsField( u"empty"_s, QMetaType::Type::Int ) );
    outputFields.append( QgsField( u"filled"_s, QMetaType::Type::Int ) );
    // min and max fields should have the same data type as valueField
    addFieldWithSameType( u"min"_s );
    addFieldWithSameType( u"max"_s );
  }
  else
  {
    outputFields.append( QgsField( u"count"_s, QMetaType::Type::Int ) );
    outputFields.append( QgsField( u"unique"_s, QMetaType::Type::Int ) );
    outputFields.append( QgsField( u"empty"_s, QMetaType::Type::Int ) );
    outputFields.append( QgsField( u"filled"_s, QMetaType::Type::Int ) );
    // min and max fields should have the same data type as valueField
    addFieldWithSameType( u"min"_s );
    addFieldWithSameType( u"max"_s );
    outputFields.append( QgsField( u"min_length"_s, QMetaType::Type::Int ) );
    outputFields.append( QgsField( u"max_length"_s, QMetaType::Type::Int ) );
    outputFields.append( QgsField( u"mean_length"_s, QMetaType::Type::Double ) );
  }

  QgsFeatureRequest request;
  request.setFlags( Qgis::FeatureRequestFlag::NoGeometry );

  QList<int> attributeIndexes;
  if ( valueFieldIndex >= 0 )
  {
    attributeIndexes.append( valueFieldIndex );
  }
  attributeIndexes.append( categoryFieldIndexes );
  request.setSubsetOfAttributes( attributeIndexes );
  QgsFeatureIterator it = source->getFeatures( request, Qgis::ProcessingFeatureSourceFlag::SkipGeometryValidityChecks );
  long long count = 0;
  const double step = source->featureCount() > 0 ? 50.0 / source->featureCount() : 0.0;

  QMap<QVariantList, int> countValues;
  QMap<QVariantList, QList<double>> numericValues;
  QMap<QVariantList, QVariantList> dateTimeValues;
  QMap<QVariantList, QStringList> stringValues;

  QgsFeature f;
  while ( it.nextFeature( f ) )
  {
    if ( feedback->isCanceled() )
    {
      break;
    }

    feedback->setProgress( static_cast<int>( count * step ) );

    QVariantList categoryKey;
    categoryKey.reserve( categoryFieldIndexes.size() );
    for ( int i : std::as_const( categoryFieldIndexes ) )
    {
      categoryKey.append( f.attribute( i ) );
    }

    if ( valueFieldIndex < 0 )
    {
      countValues[categoryKey]++;
    }
    else if ( valueField.isNumeric() )
    {
      const QVariant value = f.attribute( valueFieldIndex );
      if ( !QgsVariantUtils::isNull( value ) )
      {
        numericValues[categoryKey].append( value.toDouble() );
      }
    }
    else if ( valueField.isDateOrTime() )
    {
      const QVariant value = f.attribute( valueFieldIndex );
      dateTimeValues[categoryKey].append( QgsVariantUtils::isNull( value ) ? QVariant() : value );
    }
    else
    {
      const QVariant value = f.attribute( valueFieldIndex );
      stringValues[categoryKey].append( QgsVariantUtils::isNull( value ) ? QString() : value.toString() );
    }

    count++;
  }

  QString destId;
  std::unique_ptr<QgsFeatureSink> sink( parameterAsSink( parameters, u"OUTPUT"_s, context, destId, outputFields, Qgis::WkbType::NoGeometry, QgsCoordinateReferenceSystem() ) );
  if ( !sink )
  {
    throw QgsProcessingException( invalidSinkError( parameters, u"OUTPUT"_s ) );
  }

  if ( valueFieldIndex < 0 )
  {
    saveCounts( parameters, countValues, sink.get(), feedback );
  }
  else if ( valueField.isNumeric() )
  {
    calculateNumericStatistics( parameters, numericValues, sink.get(), feedback );
  }
  else if ( valueField.isDateOrTime() )
  {
    calculateDateTimeStatistics( parameters, dateTimeValues, sink.get(), feedback );
  }
  else
  {
    calculateStringStatistics( parameters, stringValues, sink.get(), feedback );
  }

  sink->finalize();
  feedback->featureSinkFinalized( u"OUTPUT"_s );

  QVariantMap outputs;
  outputs.insert( u"OUTPUT"_s, destId );
  return outputs;
}

void QgsStatisticsByCategoriesAlgorithm::saveCounts( const QVariantMap &parameters, const QMap<QVariantList, int> &values, QgsFeatureSink *sink, QgsProcessingFeedback *feedback )
{
  const double step = values.isEmpty() ? 0 : 50.0 / values.size();
  long long count = 0;

  for ( auto it = values.constBegin(); it != values.constEnd(); ++it )
  {
    if ( feedback->isCanceled() )
    {
      break;
    }

    feedback->setProgress( static_cast<int>( count * step ) + 50 );

    QgsFeature f;
    QVariantList attrs = it.key();
    attrs.append( it.value() );
    f.setAttributes( attrs );
    if ( !sink->addFeature( f, QgsFeatureSink::FastInsert ) )
    {
      throw QgsProcessingException( writeFeatureError( sink, parameters, u"OUTPUT"_s ) );
    }
    else
    {
      feedback->featureAddedToSink( u"OUTPUT"_s );
    }

    count++;
  }
}

void QgsStatisticsByCategoriesAlgorithm::calculateNumericStatistics( const QVariantMap &parameters, const QMap<QVariantList, QList<double>> &values, QgsFeatureSink *sink, QgsProcessingFeedback *feedback )
{
  QgsStatisticalSummary stat;
  const double step = values.isEmpty() ? 0 : 50.0 / values.size();
  long long count = 0;

  for ( auto it = values.constBegin(); it != values.constEnd(); ++it )
  {
    if ( feedback->isCanceled() )
    {
      break;
    }

    feedback->setProgress( static_cast<int>( count * step ) + 50 );

    stat.calculate( it.value() );

    QgsFeature f;
    QVariantList attrs = it.key();
    attrs
      << stat.count()
      << stat.variety()
      << stat.min()
      << stat.max()
      << stat.range()
      << stat.sum()
      << stat.mean()
      << stat.median()
      << stat.stDev()
      << stat.minority()
      << stat.majority()
      << stat.firstQuartile()
      << stat.thirdQuartile()
      << stat.interQuartileRange();

    f.setAttributes( attrs );
    if ( !sink->addFeature( f, QgsFeatureSink::FastInsert ) )
    {
      throw QgsProcessingException( writeFeatureError( sink, parameters, u"OUTPUT"_s ) );
    }
    else
    {
      feedback->featureAddedToSink( u"OUTPUT"_s );
    }

    count++;
  }
}

void QgsStatisticsByCategoriesAlgorithm::calculateDateTimeStatistics( const QVariantMap &parameters, const QMap<QVariantList, QVariantList> &values, QgsFeatureSink *sink, QgsProcessingFeedback *feedback )
{
  QgsDateTimeStatisticalSummary stat;
  const double step = values.isEmpty() ? 0 : 50.0 / values.size();
  long long count = 0;

  for ( auto it = values.constBegin(); it != values.constEnd(); ++it )
  {
    if ( feedback->isCanceled() )
    {
      break;
    }

    feedback->setProgress( static_cast<int>( count * step ) + 50 );

    stat.calculate( it.value() );

    QgsFeature f;
    QVariantList attrs = it.key();
    attrs << stat.count() << stat.countDistinct() << stat.countMissing() << stat.count() - stat.countMissing() << stat.min() << stat.max();

    f.setAttributes( attrs );
    if ( !sink->addFeature( f, QgsFeatureSink::FastInsert ) )
    {
      throw QgsProcessingException( writeFeatureError( sink, parameters, u"OUTPUT"_s ) );
    }
    else
    {
      feedback->featureAddedToSink( u"OUTPUT"_s );
    }

    count++;
  }
}

void QgsStatisticsByCategoriesAlgorithm::calculateStringStatistics( const QVariantMap &parameters, const QMap<QVariantList, QStringList> &values, QgsFeatureSink *sink, QgsProcessingFeedback *feedback )
{
  QgsStringStatisticalSummary stat;
  const double step = values.isEmpty() ? 0 : 50.0 / values.size();
  long long count = 0;

  for ( auto it = values.constBegin(); it != values.constEnd(); ++it )
  {
    if ( feedback->isCanceled() )
    {
      break;
    }

    feedback->setProgress( static_cast<int>( count * step ) + 50 );

    stat.calculate( it.value() );

    QgsFeature f;
    QVariantList attrs = it.key();
    attrs << stat.count() << stat.countDistinct() << stat.countMissing() << stat.count() - stat.countMissing() << stat.min() << stat.max() << stat.minLength() << stat.maxLength() << stat.meanLength();

    f.setAttributes( attrs );
    if ( !sink->addFeature( f, QgsFeatureSink::FastInsert ) )
    {
      throw QgsProcessingException( writeFeatureError( sink, parameters, u"OUTPUT"_s ) );
    }
    else
    {
      feedback->featureAddedToSink( u"OUTPUT"_s );
    }

    count++;
  }
}

///@endcond
