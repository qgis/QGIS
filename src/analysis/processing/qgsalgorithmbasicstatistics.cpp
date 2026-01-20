/***************************************************************************
                         qgsalgorithmbasicstatistics.cpp
                         ------------------------------
    begin                : June 2024
    copyright            : (C) 2024 by Alexander Bruy
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

#include "qgsalgorithmbasicstatistics.h"

#include "qgsdatetimestatisticalsummary.h"
#include "qgsstatisticalsummary.h"
#include "qgsstringstatisticalsummary.h"

///@cond PRIVATE

QString QgsBasicStatisticsAlgorithm::name() const
{
  return u"basicstatisticsforfields"_s;
}

QString QgsBasicStatisticsAlgorithm::displayName() const
{
  return QObject::tr( "Basic statistics for fields" );
}

QStringList QgsBasicStatisticsAlgorithm::tags() const
{
  return QObject::tr( "stats,statistics,date,time,datetime,string,number,text,table,layer,sum,maximum,minimum,mean,average,standard,deviation,count,distinct,unique,variance,median,quartile,range,majority,minority,summary" ).split( ',' );
}

QString QgsBasicStatisticsAlgorithm::group() const
{
  return QObject::tr( "Vector analysis" );
}

QString QgsBasicStatisticsAlgorithm::groupId() const
{
  return u"vectoranalysis"_s;
}

QString QgsBasicStatisticsAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm generates basic statistics from the analysis of values in a field in the attribute table of a vector layer. Numeric, date, time and string fields are supported. The statistics returned will depend on the field type." );
}

QString QgsBasicStatisticsAlgorithm::shortDescription() const
{
  return QObject::tr( "Generates basic statistics from the values in a field of a vector layer." );
}

QgsBasicStatisticsAlgorithm *QgsBasicStatisticsAlgorithm::createInstance() const
{
  return new QgsBasicStatisticsAlgorithm();
}

void QgsBasicStatisticsAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( u"INPUT_LAYER"_s, QObject::tr( "Input layer" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::Vector ) ) );
  addParameter( new QgsProcessingParameterField( u"FIELD_NAME"_s, QObject::tr( "Field to calculate statistics on" ), QVariant(), u"INPUT_LAYER"_s ) );
  addParameter( new QgsProcessingParameterFeatureSink( u"OUTPUT"_s, QObject::tr( "Statistics" ), Qgis::ProcessingSourceType::Vector, QVariant(), true ) );
  addParameter( new QgsProcessingParameterFileDestination( u"OUTPUT_HTML_FILE"_s, QObject::tr( "Statistics report" ), QObject::tr( "'HTML files (*.html)" ), QVariant(), true ) );

  addOutput( new QgsProcessingOutputNumber( u"COUNT"_s, QObject::tr( "Count" ) ) );
  addOutput( new QgsProcessingOutputNumber( u"UNIQUE"_s, QObject::tr( "Number of unique values" ) ) );
  addOutput( new QgsProcessingOutputNumber( u"EMPTY"_s, QObject::tr( "Number of empty (null) values" ) ) );
  addOutput( new QgsProcessingOutputNumber( u"FILLED"_s, QObject::tr( "Number of non-empty values" ) ) );
  addOutput( new QgsProcessingOutputNumber( u"MIN"_s, QObject::tr( "Minimum value" ) ) );
  addOutput( new QgsProcessingOutputNumber( u"MAX"_s, QObject::tr( "Maximum value" ) ) );
  addOutput( new QgsProcessingOutputNumber( u"MIN_LENGTH"_s, QObject::tr( "Minimum length" ) ) );
  addOutput( new QgsProcessingOutputNumber( u"MAX_LENGTH"_s, QObject::tr( "Maximum length" ) ) );
  addOutput( new QgsProcessingOutputNumber( u"MEAN_LENGTH"_s, QObject::tr( "Mean length" ) ) );
  addOutput( new QgsProcessingOutputNumber( u"CV"_s, QObject::tr( "Coefficient of Variation" ) ) );
  addOutput( new QgsProcessingOutputNumber( u"SUM"_s, QObject::tr( "Sum" ) ) );
  addOutput( new QgsProcessingOutputNumber( u"MEAN"_s, QObject::tr( "Mean value" ) ) );
  addOutput( new QgsProcessingOutputNumber( u"STD_DEV"_s, QObject::tr( "Standard deviation" ) ) );
  addOutput( new QgsProcessingOutputNumber( u"RANGE"_s, QObject::tr( "Range" ) ) );
  addOutput( new QgsProcessingOutputNumber( u"MEDIAN"_s, QObject::tr( "Median" ) ) );
  addOutput( new QgsProcessingOutputNumber( u"MINORITY"_s, QObject::tr( "Minority (rarest occurring value)" ) ) );
  addOutput( new QgsProcessingOutputNumber( u"MAJORITY"_s, QObject::tr( "Majority (most frequently occurring value)" ) ) );
  addOutput( new QgsProcessingOutputNumber( u"FIRSTQUARTILE"_s, QObject::tr( "First quartile" ) ) );
  addOutput( new QgsProcessingOutputNumber( u"THIRDQUARTILE"_s, QObject::tr( "Third quartile" ) ) );
  addOutput( new QgsProcessingOutputNumber( u"IQR"_s, QObject::tr( "Interquartile Range (IQR)" ) ) );
}

QVariantMap QgsBasicStatisticsAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr<QgsProcessingFeatureSource> source( parameterAsSource( parameters, u"INPUT_LAYER"_s, context ) );
  if ( !source )
    throw QgsProcessingException( invalidSourceError( parameters, u"INPUT_LAYER"_s ) );

  const QString fieldName = parameterAsString( parameters, u"FIELD_NAME"_s, context );
  const int fieldIndex = source->fields().lookupField( fieldName );
  if ( fieldIndex < 0 )
  {
    throw QgsProcessingException( QObject::tr( "Invalid field for statistics: “%1” does not exist" ).arg( fieldName ) );
  }

  QgsField field = source->fields().at( fieldIndex );

  QString outputHtml = parameterAsFileOutput( parameters, u"OUTPUT_HTML_FILE"_s, context );

  QgsFeatureRequest request;
  request.setFlags( Qgis::FeatureRequestFlag::NoGeometry ).setSubsetOfAttributes( QStringList() << fieldName, source->fields() );
  QgsFeatureIterator features = source->getFeatures( request, Qgis::ProcessingFeatureSourceFlag::SkipGeometryValidityChecks );
  const long long count = source->featureCount();

  QgsFields fields;
  fields.append( QgsField( u"count"_s, QMetaType::Int ) );
  fields.append( QgsField( u"unique"_s, QMetaType::Int ) );
  fields.append( QgsField( u"empty"_s, QMetaType::Int ) );
  fields.append( QgsField( u"filled"_s, QMetaType::Int ) );

  if ( field.isNumeric() )
  {
    fields.append( QgsField( u"min"_s, QMetaType::Double ) );
    fields.append( QgsField( u"max"_s, QMetaType::Double ) );
    fields.append( QgsField( u"range"_s, QMetaType::Double ) );
    fields.append( QgsField( u"sum"_s, QMetaType::Double ) );
    fields.append( QgsField( u"mean"_s, QMetaType::Double ) );
    fields.append( QgsField( u"median"_s, QMetaType::Double ) );
    fields.append( QgsField( u"stddev"_s, QMetaType::Double ) );
    fields.append( QgsField( u"cv"_s, QMetaType::Double ) );
    fields.append( QgsField( u"minority"_s, QMetaType::Double ) );
    fields.append( QgsField( u"majority"_s, QMetaType::Double ) );
    fields.append( QgsField( u"q1"_s, QMetaType::Double ) );
    fields.append( QgsField( u"q3"_s, QMetaType::Double ) );
    fields.append( QgsField( u"iqr"_s, QMetaType::Double ) );
  }
  else if ( field.isDateOrTime() )
  {
    if ( field.type() == QMetaType::Type::QDate )
    {
      fields.append( QgsField( u"min"_s, QMetaType::QDate ) );
      fields.append( QgsField( u"max"_s, QMetaType::QDate ) );
    }
    else if ( field.type() == QMetaType::Type::QTime )
    {
      fields.append( QgsField( u"min"_s, QMetaType::QTime ) );
      fields.append( QgsField( u"max"_s, QMetaType::QTime ) );
    }
    else
    {
      fields.append( QgsField( u"min"_s, QMetaType::QDateTime ) );
      fields.append( QgsField( u"max"_s, QMetaType::QDateTime ) );
    }
    fields.append( QgsField( u"range"_s, QMetaType::Double ) );
  }
  else
  {
    fields.append( QgsField( u"min"_s, QMetaType::QString ) );
    fields.append( QgsField( u"max"_s, QMetaType::QString ) );
    fields.append( QgsField( u"min_length"_s, QMetaType::Double ) );
    fields.append( QgsField( u"max_length"_s, QMetaType::Double ) );
    fields.append( QgsField( u"mean_length"_s, QMetaType::Double ) );
    fields.append( QgsField( u"minority"_s, QMetaType::QString ) );
    fields.append( QgsField( u"majority"_s, QMetaType::QString ) );
  }

  QString destId;
  std::unique_ptr<QgsFeatureSink> sink( parameterAsSink( parameters, u"OUTPUT"_s, context, destId, fields, Qgis::WkbType::NoGeometry, QgsCoordinateReferenceSystem() ) );
  if ( parameters.value( u"OUTPUT"_s ).isValid() && !sink )
    throw QgsProcessingException( invalidSinkError( parameters, u"OUTPUT"_s ) );

  QStringList data;
  data << QObject::tr( "Analyzed field: %1" ).arg( fieldName );

  QVariantMap outputs;

  if ( field.isNumeric() )
  {
    outputs = calculateNumericStatistics( parameters, fieldIndex, features, count, sink.get(), data, feedback );
  }
  else if ( field.isDateOrTime() )
  {
    outputs = calculateDateTimeStatistics( parameters, fieldIndex, field, features, count, sink.get(), data, feedback );
  }
  else
  {
    outputs = calculateStringStatistics( parameters, fieldIndex, features, count, sink.get(), data, feedback );
  }
  if ( sink )
    sink->finalize();

  if ( !outputHtml.isEmpty() )
  {
    QFile file( outputHtml );
    if ( file.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
    {
      QTextStream out( &file );
      out << u"<html><head><meta http-equiv=\"Content-Type\" content=\"text/html;charset=utf-8\"/></head><body>\n"_s;
      for ( const QString &s : data )
      {
        out << u"<p>%1</p>"_s.arg( s );
      }
      out << u"</body></html>"_s;

      outputs.insert( u"OUTPUT_HTML_FILE"_s, outputHtml );
    }
  }

  if ( sink )
  {
    outputs.insert( u"OUTPUT"_s, destId );
  }

  return outputs;
}

QVariantMap QgsBasicStatisticsAlgorithm::calculateNumericStatistics( const QVariantMap &parameters, const int fieldIndex, QgsFeatureIterator features, const long long count, QgsFeatureSink *sink, QStringList &data, QgsProcessingFeedback *feedback )
{
  const double step = count > 0 ? 100.0 / count : 1;
  long long current = 0;

  QgsFeature f;
  QgsStatisticalSummary stat;

  while ( features.nextFeature( f ) )
  {
    if ( feedback->isCanceled() )
    {
      break;
    }

    stat.addVariant( f.attribute( fieldIndex ) );
    feedback->setProgress( current * step );
    current++;
  }
  stat.finalize();

  const double cv = stat.mean() != 0 ? stat.stDev() / stat.mean() : 0;

  QVariantMap outputs;
  outputs.insert( u"COUNT"_s, stat.count() );
  outputs.insert( u"UNIQUE"_s, stat.variety() );
  outputs.insert( u"EMPTY"_s, stat.countMissing() );
  outputs.insert( u"FILLED"_s, count - stat.countMissing() );
  outputs.insert( u"MIN"_s, stat.min() );
  outputs.insert( u"MAX"_s, stat.max() );
  outputs.insert( u"RANGE"_s, stat.range() );
  outputs.insert( u"SUM"_s, stat.sum() );
  outputs.insert( u"MEAN"_s, stat.mean() );
  outputs.insert( u"MEDIAN"_s, stat.median() );
  outputs.insert( u"STD_DEV"_s, stat.stDev() );
  outputs.insert( u"CV"_s, cv );
  outputs.insert( u"MINORITY"_s, stat.minority() );
  outputs.insert( u"MAJORITY"_s, stat.majority() );
  outputs.insert( u"FIRSTQUARTILE"_s, stat.firstQuartile() );
  outputs.insert( u"THIRDQUARTILE"_s, stat.thirdQuartile() );
  outputs.insert( u"IQR"_s, stat.interQuartileRange() );

  data << QObject::tr( "Count: %1" ).arg( stat.count() )
       << QObject::tr( "Unique values: %1" ).arg( stat.variety() )
       << QObject::tr( "NULL (missing) values: %1" ).arg( stat.countMissing() )
       << QObject::tr( "NOT NULL (filled) values: %1" ).arg( count - stat.countMissing() )
       << QObject::tr( "Minimum value: %1" ).arg( stat.min() )
       << QObject::tr( "Maximum value: %1" ).arg( stat.max() )
       << QObject::tr( "Range: %1" ).arg( stat.range() )
       << QObject::tr( "Sum: %1" ).arg( stat.sum(), 0, 'f' )
       << QObject::tr( "Mean value: %1" ).arg( stat.mean(), 0, 'f' )
       << QObject::tr( "Median value: %1" ).arg( stat.median(), 0, 'f' )
       << QObject::tr( "Standard deviation: %1" ).arg( stat.stDev(), 0, 'f', 12 )
       << QObject::tr( "Coefficient of Variation: %1" ).arg( cv, 0, 'f' )
       << QObject::tr( "Minority (rarest occurring value): %1" ).arg( stat.minority() )
       << QObject::tr( "Majority (most frequently occurring value): %1" ).arg( stat.majority() )
       << QObject::tr( "First quartile: %1" ).arg( stat.firstQuartile(), 0, 'f' )
       << QObject::tr( "Third quartile: %1" ).arg( stat.thirdQuartile(), 0, 'f' )
       << QObject::tr( "Interquartile Range (IQR): %1" ).arg( stat.interQuartileRange() );

  if ( sink )
  {
    QgsFeature f;
    f.setAttributes( QgsAttributes() << outputs.value( u"COUNT"_s ) << outputs.value( u"UNIQUE"_s ) << outputs.value( u"EMPTY"_s ) << outputs.value( u"FILLED"_s ) << outputs.value( u"MIN"_s ) << outputs.value( u"MAX"_s ) << outputs.value( u"RANGE"_s ) << outputs.value( u"SUM"_s ) << outputs.value( u"MEAN"_s ) << outputs.value( u"MEDIAN"_s ) << outputs.value( u"STD_DEV"_s ) << outputs.value( u"CV"_s ) << outputs.value( u"MINORITY"_s ) << outputs.value( u"MAJORITY"_s ) << outputs.value( u"FIRSTQUARTILE"_s ) << outputs.value( u"THIRDQUARTILE"_s ) << outputs.value( u"IQR"_s ) );
    if ( !sink->addFeature( f, QgsFeatureSink::FastInsert ) )
    {
      throw QgsProcessingException( writeFeatureError( sink, parameters, QString() ) );
    }
  }

  return outputs;
}

QVariantMap QgsBasicStatisticsAlgorithm::calculateDateTimeStatistics( const QVariantMap &parameters, const int fieldIndex, QgsField field, QgsFeatureIterator features, const long long count, QgsFeatureSink *sink, QStringList &data, QgsProcessingFeedback *feedback )
{
  const double step = count > 0 ? 100.0 / count : 1;
  long long current = 0;

  QgsFeature f;
  QgsDateTimeStatisticalSummary stat;

  while ( features.nextFeature( f ) )
  {
    if ( feedback->isCanceled() )
    {
      break;
    }

    stat.addValue( f.attribute( fieldIndex ) );
    feedback->setProgress( current * step );
    current++;
  }
  stat.finalize();

  QVariantMap outputs;
  outputs.insert( u"COUNT"_s, stat.count() );
  outputs.insert( u"UNIQUE"_s, stat.countDistinct() );
  outputs.insert( u"EMPTY"_s, stat.countMissing() );
  outputs.insert( u"FILLED"_s, stat.count() - stat.countMissing() );
  outputs.insert( u"MIN"_s, stat.statistic( Qgis::DateTimeStatistic::Min ) );
  outputs.insert( u"MAX"_s, stat.statistic( Qgis::DateTimeStatistic::Max ) );
  outputs.insert( u"RANGE"_s, stat.range().seconds() );

  data << QObject::tr( "Count: %1" ).arg( stat.count() )
       << QObject::tr( "Unique values: %1" ).arg( stat.countDistinct() )
       << QObject::tr( "NULL (missing) values: %1" ).arg( stat.countMissing() )
       << QObject::tr( "NOT NULL (filled) values: %1" ).arg( stat.count() - stat.countMissing() )
       << QObject::tr( "Minimum value: %1" ).arg( field.displayString( stat.statistic( Qgis::DateTimeStatistic::Min ) ) )
       << QObject::tr( "Maximum value: %1" ).arg( field.displayString( stat.statistic( Qgis::DateTimeStatistic::Max ) ) )
       << QObject::tr( "Range (seconds): %1" ).arg( stat.range().seconds() );

  if ( sink )
  {
    QgsFeature f;
    f.setAttributes( QgsAttributes() << outputs.value( u"COUNT"_s ) << outputs.value( u"UNIQUE"_s ) << outputs.value( u"EMPTY"_s ) << outputs.value( u"FILLED"_s ) << outputs.value( u"MIN"_s ) << outputs.value( u"MAX"_s ) << outputs.value( u"RANGE"_s ) );
    if ( !sink->addFeature( f, QgsFeatureSink::FastInsert ) )
    {
      throw QgsProcessingException( writeFeatureError( sink, parameters, QString() ) );
    }
  }

  return outputs;
}

QVariantMap QgsBasicStatisticsAlgorithm::calculateStringStatistics( const QVariantMap &parameters, const int fieldIndex, QgsFeatureIterator features, const long long count, QgsFeatureSink *sink, QStringList &data, QgsProcessingFeedback *feedback )
{
  const double step = count > 0 ? 100.0 / count : 1;
  long long current = 0;

  QgsFeature f;
  QgsStringStatisticalSummary stat;

  while ( features.nextFeature( f ) )
  {
    if ( feedback->isCanceled() )
    {
      break;
    }

    stat.addValue( f.attribute( fieldIndex ) );
    feedback->setProgress( current * step );
    current++;
  }
  stat.finalize();

  QVariantMap outputs;
  outputs.insert( u"COUNT"_s, stat.count() );
  outputs.insert( u"UNIQUE"_s, stat.countDistinct() );
  outputs.insert( u"EMPTY"_s, stat.countMissing() );
  outputs.insert( u"FILLED"_s, stat.count() - stat.countMissing() );
  outputs.insert( u"MIN"_s, stat.min() );
  outputs.insert( u"MAX"_s, stat.max() );
  outputs.insert( u"MIN_LENGTH"_s, stat.minLength() );
  outputs.insert( u"MAX_LENGTH"_s, stat.maxLength() );
  outputs.insert( u"MEAN_LENGTH"_s, stat.meanLength() );
  outputs.insert( u"MINORITY"_s, stat.minority() );
  outputs.insert( u"MAJORITY"_s, stat.majority() );

  data << QObject::tr( "Count: %1" ).arg( stat.count() )
       << QObject::tr( "Unique values: %1" ).arg( stat.countDistinct() )
       << QObject::tr( "NULL (missing) values: %1" ).arg( stat.countMissing() )
       << QObject::tr( "NOT NULL (filled) values: %1" ).arg( count - stat.countMissing() )
       << QObject::tr( "Minimum value: %1" ).arg( stat.min() )
       << QObject::tr( "Maximum value: %1" ).arg( stat.max() )
       << QObject::tr( "Minimum length: %1" ).arg( stat.minLength() )
       << QObject::tr( "Maximum length: %1" ).arg( stat.maxLength() )
       << QObject::tr( "Mean length: %1" ).arg( stat.meanLength(), 0, 'f' )
       << QObject::tr( "Minority: %1" ).arg( stat.minority() )
       << QObject::tr( "Majority: %1" ).arg( stat.majority() );

  if ( sink )
  {
    QgsFeature f;
    f.setAttributes( QgsAttributes() << outputs.value( u"COUNT"_s ) << outputs.value( u"UNIQUE"_s ) << outputs.value( u"EMPTY"_s ) << outputs.value( u"FILLED"_s ) << outputs.value( u"MIN"_s ) << outputs.value( u"MAX"_s ) << outputs.value( u"MIN_LENGTH"_s ) << outputs.value( u"MAX_LENGTH"_s ) << outputs.value( u"MEAN_LENGTH"_s ) << outputs.value( u"MINORITY"_s ) << outputs.value( u"MAJORITY"_s ) );
    if ( !sink->addFeature( f, QgsFeatureSink::FastInsert ) )
    {
      throw QgsProcessingException( writeFeatureError( sink, parameters, QString() ) );
    }
  }

  return outputs;
}

///@endcond
