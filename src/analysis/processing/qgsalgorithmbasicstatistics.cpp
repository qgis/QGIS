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
#include "qgsstatisticalsummary.h"
#include "qgsdatetimestatisticalsummary.h"
#include "qgsstringstatisticalsummary.h"


///@cond PRIVATE

QString QgsBasicStatisticsAlgorithm::name() const
{
  return QStringLiteral( "basicstatisticsforfields" );
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
  return QStringLiteral( "vectoranalysis" );
}

QString QgsBasicStatisticsAlgorithm::shortHelpString() const
{
  return QObject::tr( "Generates basic statistics from the analysis of a values in a field in the attribute table of a vector layer. Numeric, date, time and string fields are supported. The statistics returned will depend on the field type." );
}

QgsBasicStatisticsAlgorithm *QgsBasicStatisticsAlgorithm::createInstance() const
{
  return new QgsBasicStatisticsAlgorithm();
}

void QgsBasicStatisticsAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT_LAYER" ), QObject::tr( "Input layer" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::Vector ) ) );
  addParameter( new QgsProcessingParameterField( QStringLiteral( "FIELD_NAME" ), QObject::tr( "Field to calculate statistics on" ), QVariant(), QStringLiteral( "INPUT_LAYER" ) ) );
  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Statistics" ), Qgis::ProcessingSourceType::Vector, QVariant(), true ) );
  addParameter( new QgsProcessingParameterFileDestination( QStringLiteral( "OUTPUT_HTML_FILE" ), QObject::tr( "Statistics report" ), QObject::tr( "'HTML files (*.html)" ), QVariant(), true ) );

  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "COUNT" ), QObject::tr( "Count" ) ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "UNIQUE" ), QObject::tr( "Number of unique values" ) ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "EMPTY" ), QObject::tr( "Number of empty (null) values" ) ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "FILLED" ), QObject::tr( "Number of non-empty values" ) ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "MIN" ), QObject::tr( "Minimum value" ) ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "MAX" ), QObject::tr( "Maximum value" ) ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "MIN_LENGTH" ), QObject::tr( "Minimum length" ) ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "MAX_LENGTH" ), QObject::tr( "Maximum length" ) ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "MEAN_LENGTH" ), QObject::tr( "Mean length" ) ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "CV" ), QObject::tr( "Coefficient of Variation" ) ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "SUM" ), QObject::tr( "Sum" ) ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "MEAN" ), QObject::tr( "Mean value" ) ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "STD_DEV" ), QObject::tr( "Standard deviation" ) ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "RANGE" ), QObject::tr( "Range" ) ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "MEDIAN" ), QObject::tr( "Median" ) ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "MINORITY" ), QObject::tr( "Minority (rarest occurring value)" ) ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "MAJORITY" ), QObject::tr( "Majority (most frequently occurring value)" ) ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "FIRSTQUARTILE" ), QObject::tr( "First quartile" ) ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "THIRDQUARTILE" ), QObject::tr( "Third quartile" ) ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "IQR" ), QObject::tr( "Interquartile Range (IQR)" ) ) );
}

QVariantMap QgsBasicStatisticsAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr<QgsProcessingFeatureSource> source( parameterAsSource( parameters, QStringLiteral( "INPUT_LAYER" ), context ) );
  if ( !source )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "INPUT_LAYER" ) ) );

  const QString fieldName = parameterAsString( parameters, QStringLiteral( "FIELD_NAME" ), context );
  const int fieldIndex = source->fields().lookupField( fieldName );
  if ( fieldIndex < 0 )
  {
    throw QgsProcessingException( QObject::tr( "Invalid field for statistics: “%1” does not exist" ).arg( fieldName ) );
  }

  QgsField field = source->fields().at( fieldIndex );

  QString outputHtml = parameterAsFileOutput( parameters, QStringLiteral( "OUTPUT_HTML_FILE" ), context );

  QgsFeatureRequest request;
  request.setFlags( Qgis::FeatureRequestFlag::NoGeometry ).setSubsetOfAttributes( QStringList() << fieldName, source->fields() );
  QgsFeatureIterator features = source->getFeatures( request, Qgis::ProcessingFeatureSourceFlag::SkipGeometryValidityChecks );
  const long long count = source->featureCount();

  QgsFields fields;
  fields.append( QgsField( QStringLiteral( "count" ), QMetaType::Int ) );
  fields.append( QgsField( QStringLiteral( "unique" ), QMetaType::Int ) );
  fields.append( QgsField( QStringLiteral( "empty" ), QMetaType::Int ) );
  fields.append( QgsField( QStringLiteral( "filled" ), QMetaType::Int ) );

  if ( field.isNumeric() )
  {
    fields.append( QgsField( QStringLiteral( "min" ), QMetaType::Double ) );
    fields.append( QgsField( QStringLiteral( "max" ), QMetaType::Double ) );
    fields.append( QgsField( QStringLiteral( "range" ), QMetaType::Double ) );
    fields.append( QgsField( QStringLiteral( "sum" ), QMetaType::Double ) );
    fields.append( QgsField( QStringLiteral( "mean" ), QMetaType::Double ) );
    fields.append( QgsField( QStringLiteral( "median" ), QMetaType::Double ) );
    fields.append( QgsField( QStringLiteral( "stddev" ), QMetaType::Double ) );
    fields.append( QgsField( QStringLiteral( "cv" ), QMetaType::Double ) );
    fields.append( QgsField( QStringLiteral( "minority" ), QMetaType::Double ) );
    fields.append( QgsField( QStringLiteral( "majority" ), QMetaType::Double ) );
    fields.append( QgsField( QStringLiteral( "q1" ), QMetaType::Double ) );
    fields.append( QgsField( QStringLiteral( "q3" ), QMetaType::Double ) );
    fields.append( QgsField( QStringLiteral( "iqr" ), QMetaType::Double ) );
  }
  else if ( field.isDateOrTime() )
  {
    if ( field.type() == QMetaType::Type::QDate )
    {
      fields.append( QgsField( QStringLiteral( "min" ), QMetaType::QDate ) );
      fields.append( QgsField( QStringLiteral( "max" ), QMetaType::QDate ) );
    }
    else if ( field.type() == QMetaType::Type::QTime )
    {
      fields.append( QgsField( QStringLiteral( "min" ), QMetaType::QTime ) );
      fields.append( QgsField( QStringLiteral( "max" ), QMetaType::QTime ) );
    }
    else
    {
      fields.append( QgsField( QStringLiteral( "min" ), QMetaType::QDateTime ) );
      fields.append( QgsField( QStringLiteral( "max" ), QMetaType::QDateTime ) );
    }
    fields.append( QgsField( QStringLiteral( "range" ), QMetaType::Double ) );
  }
  else
  {
    fields.append( QgsField( QStringLiteral( "min" ), QMetaType::QString ) );
    fields.append( QgsField( QStringLiteral( "max" ), QMetaType::QString ) );
    fields.append( QgsField( QStringLiteral( "min_length" ), QMetaType::Double ) );
    fields.append( QgsField( QStringLiteral( "max_length" ), QMetaType::Double ) );
    fields.append( QgsField( QStringLiteral( "mean_length" ), QMetaType::Double ) );
    fields.append( QgsField( QStringLiteral( "minority" ), QMetaType::QString ) );
    fields.append( QgsField( QStringLiteral( "majority" ), QMetaType::QString ) );
  }

  QString destId;
  std::unique_ptr<QgsFeatureSink> sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, destId, fields, Qgis::WkbType::NoGeometry, QgsCoordinateReferenceSystem() ) );
  if ( parameters.value( QStringLiteral( "OUTPUT" ) ).isValid() && !sink )
    throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "OUTPUT" ) ) );

  QStringList data;
  data << QObject::tr( "Analyzed field: %1" ).arg( fieldName );

  QVariantMap outputs;

  if ( field.isNumeric() )
  {
    outputs = calculateNumericStatistics( fieldIndex, features, count, sink.get(), data, feedback );
  }
  else if ( field.isDateOrTime() )
  {
    outputs = calculateDateTimeStatistics( fieldIndex, field, features, count, sink.get(), data, feedback );
  }
  else
  {
    outputs = calculateStringStatistics( fieldIndex, features, count, sink.get(), data, feedback );
  }
  if ( sink )
    sink->finalize();

  if ( !outputHtml.isEmpty() )
  {
    QFile file( outputHtml );
    if ( file.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
    {
      QTextStream out( &file );
#if QT_VERSION < QT_VERSION_CHECK( 6, 0, 0 )
      out.setCodec( "UTF-8" );
#endif
      out << QStringLiteral( "<html><head><meta http-equiv=\"Content-Type\" content=\"text/html;charset=utf-8\"/></head><body>\n" );
      for ( const QString &s : data )
      {
        out << QStringLiteral( "<p>%1</p>" ).arg( s );
      }
      out << QStringLiteral( "</body></html>" );

      outputs.insert( QStringLiteral( "OUTPUT_HTML_FILE" ), outputHtml );
    }
  }

  if ( sink )
  {
    outputs.insert( QStringLiteral( "OUTPUT" ), destId );
  }

  return outputs;
}

QVariantMap QgsBasicStatisticsAlgorithm::calculateNumericStatistics( const int fieldIndex, QgsFeatureIterator features, const long long count, QgsFeatureSink *sink, QStringList &data, QgsProcessingFeedback *feedback )
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
  outputs.insert( QStringLiteral( "COUNT" ), stat.count() );
  outputs.insert( QStringLiteral( "UNIQUE" ), stat.variety() );
  outputs.insert( QStringLiteral( "EMPTY" ), stat.countMissing() );
  outputs.insert( QStringLiteral( "FILLED" ), count - stat.countMissing() );
  outputs.insert( QStringLiteral( "MIN" ), stat.min() );
  outputs.insert( QStringLiteral( "MAX" ), stat.max() );
  outputs.insert( QStringLiteral( "RANGE" ), stat.range() );
  outputs.insert( QStringLiteral( "SUM" ), stat.sum() );
  outputs.insert( QStringLiteral( "MEAN" ), stat.mean() );
  outputs.insert( QStringLiteral( "MEDIAN" ), stat.median() );
  outputs.insert( QStringLiteral( "STD_DEV" ), stat.stDev() );
  outputs.insert( QStringLiteral( "CV" ), cv );
  outputs.insert( QStringLiteral( "MINORITY" ), stat.minority() );
  outputs.insert( QStringLiteral( "MAJORITY" ), stat.majority() );
  outputs.insert( QStringLiteral( "FIRSTQUARTILE" ), stat.firstQuartile() );
  outputs.insert( QStringLiteral( "THIRDQUARTILE" ), stat.thirdQuartile() );
  outputs.insert( QStringLiteral( "IQR" ), stat.interQuartileRange() );

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
    f.setAttributes( QgsAttributes() << outputs.value( QStringLiteral( "COUNT" ) ) << outputs.value( QStringLiteral( "UNIQUE" ) ) << outputs.value( QStringLiteral( "EMPTY" ) ) << outputs.value( QStringLiteral( "FILLED" ) ) << outputs.value( QStringLiteral( "MIN" ) ) << outputs.value( QStringLiteral( "MAX" ) ) << outputs.value( QStringLiteral( "RANGE" ) ) << outputs.value( QStringLiteral( "SUM" ) ) << outputs.value( QStringLiteral( "MEAN" ) ) << outputs.value( QStringLiteral( "MEDIAN" ) ) << outputs.value( QStringLiteral( "STD_DEV" ) ) << outputs.value( QStringLiteral( "CV" ) ) << outputs.value( QStringLiteral( "MINORITY" ) ) << outputs.value( QStringLiteral( "MAJORITY" ) ) << outputs.value( QStringLiteral( "FIRSTQUARTILE" ) ) << outputs.value( QStringLiteral( "THIRDQUARTILE" ) ) << outputs.value( QStringLiteral( "IQR" ) ) );
    sink->addFeature( f, QgsFeatureSink::FastInsert );
  }

  return outputs;
}

QVariantMap QgsBasicStatisticsAlgorithm::calculateDateTimeStatistics( const int fieldIndex, QgsField field, QgsFeatureIterator features, const long long count, QgsFeatureSink *sink, QStringList &data, QgsProcessingFeedback *feedback )
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
  outputs.insert( QStringLiteral( "COUNT" ), stat.count() );
  outputs.insert( QStringLiteral( "UNIQUE" ), stat.countDistinct() );
  outputs.insert( QStringLiteral( "EMPTY" ), stat.countMissing() );
  outputs.insert( QStringLiteral( "FILLED" ), stat.count() - stat.countMissing() );
  outputs.insert( QStringLiteral( "MIN" ), stat.statistic( Qgis::DateTimeStatistic::Min ) );
  outputs.insert( QStringLiteral( "MAX" ), stat.statistic( Qgis::DateTimeStatistic::Max ) );
  outputs.insert( QStringLiteral( "RANGE" ), stat.range().seconds() );

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
    f.setAttributes( QgsAttributes() << outputs.value( QStringLiteral( "COUNT" ) ) << outputs.value( QStringLiteral( "UNIQUE" ) ) << outputs.value( QStringLiteral( "EMPTY" ) ) << outputs.value( QStringLiteral( "FILLED" ) ) << outputs.value( QStringLiteral( "MIN" ) ) << outputs.value( QStringLiteral( "MAX" ) ) << outputs.value( QStringLiteral( "RANGE" ) ) );
    sink->addFeature( f, QgsFeatureSink::FastInsert );
  }

  return outputs;
}

QVariantMap QgsBasicStatisticsAlgorithm::calculateStringStatistics( const int fieldIndex, QgsFeatureIterator features, const long long count, QgsFeatureSink *sink, QStringList &data, QgsProcessingFeedback *feedback )
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
  outputs.insert( QStringLiteral( "COUNT" ), stat.count() );
  outputs.insert( QStringLiteral( "UNIQUE" ), stat.countDistinct() );
  outputs.insert( QStringLiteral( "EMPTY" ), stat.countMissing() );
  outputs.insert( QStringLiteral( "FILLED" ), stat.count() - stat.countMissing() );
  outputs.insert( QStringLiteral( "MIN" ), stat.min() );
  outputs.insert( QStringLiteral( "MAX" ), stat.max() );
  outputs.insert( QStringLiteral( "MIN_LENGTH" ), stat.minLength() );
  outputs.insert( QStringLiteral( "MAX_LENGTH" ), stat.maxLength() );
  outputs.insert( QStringLiteral( "MEAN_LENGTH" ), stat.meanLength() );
  outputs.insert( QStringLiteral( "MINORITY" ), stat.minority() );
  outputs.insert( QStringLiteral( "MAJORITY" ), stat.majority() );

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
    f.setAttributes( QgsAttributes() << outputs.value( QStringLiteral( "COUNT" ) ) << outputs.value( QStringLiteral( "UNIQUE" ) ) << outputs.value( QStringLiteral( "EMPTY" ) ) << outputs.value( QStringLiteral( "FILLED" ) ) << outputs.value( QStringLiteral( "MIN" ) ) << outputs.value( QStringLiteral( "MAX" ) ) << outputs.value( QStringLiteral( "MIN_LENGTH" ) ) << outputs.value( QStringLiteral( "MAX_LENGTH" ) ) << outputs.value( QStringLiteral( "MEAN_LENGTH" ) ) << outputs.value( QStringLiteral( "MINORITY" ) ) << outputs.value( QStringLiteral( "MAJORITY" ) ) );
    sink->addFeature( f, QgsFeatureSink::FastInsert );
  }

  return outputs;
}

///@endcond
