/***************************************************************************
                         qgsalgorithmjoinbylocationsummary.cpp
                         ---------------------
    begin                : September 2023
    copyright            : (C) 2023 by Nyall Dawson
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

#include "qgsalgorithmjoinbylocationsummary.h"

#include "qgsalgorithmjoinbylocation.h"
#include "qgsapplication.h"
#include "qgsdatetimestatisticalsummary.h"
#include "qgsfeature.h"
#include "qgsfeaturesource.h"
#include "qgsgeometryengine.h"
#include "qgsprocessing.h"
#include "qgsstringstatisticalsummary.h"
#include "qgsvectorlayer.h"

///@cond PRIVATE


void QgsJoinByLocationSummaryAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( u"INPUT"_s, QObject::tr( "Join to features in" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorAnyGeometry ) ) );

  auto predicateParam = std::make_unique<QgsProcessingParameterEnum>( u"PREDICATE"_s, QObject::tr( "Where the features" ), QgsJoinByLocationAlgorithm::translatedPredicates(), true, 0 );
  QVariantMap predicateMetadata;
  QVariantMap widgetMetadata;
  widgetMetadata.insert( u"useCheckBoxes"_s, true );
  widgetMetadata.insert( u"columns"_s, 2 );
  predicateMetadata.insert( u"widget_wrapper"_s, widgetMetadata );
  predicateParam->setMetadata( predicateMetadata );
  addParameter( predicateParam.release() );

  addParameter( new QgsProcessingParameterFeatureSource( u"JOIN"_s, QObject::tr( "By comparing to" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorAnyGeometry ) ) );

  addParameter( new QgsProcessingParameterField( u"JOIN_FIELDS"_s, QObject::tr( "Fields to summarise (leave empty to use all fields)" ), QVariant(), u"JOIN"_s, Qgis::ProcessingFieldParameterDataType::Any, true, true ) );

  mAllSummaries << QObject::tr( "count" )
                << QObject::tr( "unique" )
                << QObject::tr( "min" )
                << QObject::tr( "max" )
                << QObject::tr( "range" )
                << QObject::tr( "sum" )
                << QObject::tr( "mean" )
                << QObject::tr( "median" )
                << QObject::tr( "stddev" )
                << QObject::tr( "minority" )
                << QObject::tr( "majority" )
                << QObject::tr( "q1" )
                << QObject::tr( "q3" )
                << QObject::tr( "iqr" )
                << QObject::tr( "empty" )
                << QObject::tr( "filled" )
                << QObject::tr( "min_length" )
                << QObject::tr( "max_length" )
                << QObject::tr( "mean_length" );

  auto summaryParam = std::make_unique<QgsProcessingParameterEnum>( u"SUMMARIES"_s, QObject::tr( "Summaries to calculate (leave empty to use all available)" ), mAllSummaries, true, QVariant(), true );
  addParameter( summaryParam.release() );

  addParameter( new QgsProcessingParameterBoolean( u"DISCARD_NONMATCHING"_s, QObject::tr( "Discard records which could not be joined" ), false ) );
  addParameter( new QgsProcessingParameterFeatureSink( u"OUTPUT"_s, QObject::tr( "Joined layer" ) ) );
}

QString QgsJoinByLocationSummaryAlgorithm::name() const
{
  return u"joinbylocationsummary"_s;
}

QString QgsJoinByLocationSummaryAlgorithm::displayName() const
{
  return QObject::tr( "Join attributes by location (summary)" );
}

QStringList QgsJoinByLocationSummaryAlgorithm::tags() const
{
  return QObject::tr( "summary,aggregate,join,intersects,intersecting,touching,within,contains,overlaps,relation,spatial,"
                      "stats,statistics,sum,maximum,minimum,mean,average,standard,deviation,"
                      "count,distinct,unique,variance,median,quartile,range,majority,minority,histogram,distinct" )
    .split( ',' );
}

QString QgsJoinByLocationSummaryAlgorithm::group() const
{
  return QObject::tr( "Vector general" );
}

QString QgsJoinByLocationSummaryAlgorithm::groupId() const
{
  return u"vectorgeneral"_s;
}

QString QgsJoinByLocationSummaryAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm takes an input vector layer and creates a new vector layer that is an extended version of the input one, with additional attributes in its attribute table.\n\n"
                      "The additional attributes and their values are taken from a second vector layer. A spatial criteria is applied to select the values from the second layer that are added to each feature from the first layer in the resulting one.\n\n"
                      "The algorithm calculates a statistical summary for the values from matching features in the second layer( e.g. maximum value, mean value, etc )." );
}

QString QgsJoinByLocationSummaryAlgorithm::shortDescription() const
{
  return QObject::tr( "Calculates summaries of attributes from one vector layer to another by location." );
}

QIcon QgsJoinByLocationSummaryAlgorithm::icon() const
{
  return QgsApplication::getThemeIcon( u"/algorithms/mAlgorithmBasicStatistics.svg"_s );
}

QString QgsJoinByLocationSummaryAlgorithm::svgIconPath() const
{
  return QgsApplication::iconPath( u"/algorithms/mAlgorithmBasicStatistics.svg"_s );
}

QgsJoinByLocationSummaryAlgorithm *QgsJoinByLocationSummaryAlgorithm::createInstance() const
{
  return new QgsJoinByLocationSummaryAlgorithm();
}

QVariantMap QgsJoinByLocationSummaryAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr<QgsProcessingFeatureSource> baseSource( parameterAsSource( parameters, u"INPUT"_s, context ) );
  if ( !baseSource )
    throw QgsProcessingException( invalidSourceError( parameters, u"INPUT"_s ) );

  std::unique_ptr<QgsProcessingFeatureSource> joinSource( parameterAsSource( parameters, u"JOIN"_s, context ) );
  if ( !joinSource )
    throw QgsProcessingException( invalidSourceError( parameters, u"JOIN"_s ) );

  if ( joinSource->hasSpatialIndex() == Qgis::SpatialIndexPresence::NotPresent )
    feedback->reportError( QObject::tr( "No spatial index exists for join layer, performance will be severely degraded" ) );

  QStringList joinedFieldNames = parameterAsStrings( parameters, u"JOIN_FIELDS"_s, context );

  bool discardNonMatching = parameterAsBoolean( parameters, u"DISCARD_NONMATCHING"_s, context );

  QList<int> summaries = parameterAsEnums( parameters, u"SUMMARIES"_s, context );
  if ( summaries.empty() )
  {
    for ( int i = 0; i < mAllSummaries.size(); ++i )
      summaries << i;
  }

  QgsFields sourceFields = baseSource->fields();
  QgsFields fieldsToJoin;
  QList<int> joinFieldIndices;
  if ( joinedFieldNames.empty() )
  {
    // no fields selected, use all
    for ( const QgsField &sourceField : joinSource->fields() )
    {
      joinedFieldNames.append( sourceField.name() );
    }
  }

  // Adds a field to the output, keeping the same data type as the original
  auto addFieldKeepType = [&fieldsToJoin]( const QgsField &original, const QString &statistic ) {
    QgsField field = QgsField( original );
    field.setName( field.name() + '_' + statistic );
    fieldsToJoin.append( field );
  };

  // Adds a field to the output, with a specified type
  auto addFieldWithType = [&fieldsToJoin]( const QgsField &original, const QString &statistic, QMetaType::Type type ) {
    QgsField field = QgsField( original );
    field.setName( field.name() + '_' + statistic );
    field.setType( type );
    if ( type == QMetaType::Type::Double )
    {
      field.setLength( 20 );
      field.setPrecision( 6 );
    }
    fieldsToJoin.append( field );
  };

  enum class FieldType
  {
    Numeric,
    DateTime,
    String
  };
  QList<FieldType> fieldTypes;

  struct FieldStatistic
  {
      FieldStatistic( int enumIndex, const QString &name, QMetaType::Type type )
        : enumIndex( enumIndex )
        , name( name )
        , type( type )
      {}

      int enumIndex = 0;
      QString name;
      QMetaType::Type type;
  };
  static const QVector<FieldStatistic> sNumericStats {
    FieldStatistic( 0, u"count"_s, QMetaType::Type::LongLong ),
    FieldStatistic( 1, u"unique"_s, QMetaType::Type::LongLong ),
    FieldStatistic( 2, u"min"_s, QMetaType::Type::Double ),
    FieldStatistic( 3, u"max"_s, QMetaType::Type::Double ),
    FieldStatistic( 4, u"range"_s, QMetaType::Type::Double ),
    FieldStatistic( 5, u"sum"_s, QMetaType::Type::Double ),
    FieldStatistic( 6, u"mean"_s, QMetaType::Type::Double ),
    FieldStatistic( 7, u"median"_s, QMetaType::Type::Double ),
    FieldStatistic( 8, u"stddev"_s, QMetaType::Type::Double ),
    FieldStatistic( 9, u"minority"_s, QMetaType::Type::Double ),
    FieldStatistic( 10, u"majority"_s, QMetaType::Type::Double ),
    FieldStatistic( 11, u"q1"_s, QMetaType::Type::Double ),
    FieldStatistic( 12, u"q3"_s, QMetaType::Type::Double ),
    FieldStatistic( 13, u"iqr"_s, QMetaType::Type::Double ),
  };
  static const QVector<FieldStatistic> sDateTimeStats {
    FieldStatistic( 0, u"count"_s, QMetaType::Type::LongLong ),
    FieldStatistic( 1, u"unique"_s, QMetaType::Type::LongLong ),
    FieldStatistic( 14, u"empty"_s, QMetaType::Type::LongLong ),
    FieldStatistic( 15, u"filled"_s, QMetaType::Type::LongLong ),
    FieldStatistic( 2, u"min"_s, QMetaType::Type::UnknownType ),
    FieldStatistic( 3, u"max"_s, QMetaType::Type::UnknownType ),
  };
  static const QVector<FieldStatistic> sStringStats {
    FieldStatistic( 0, u"count"_s, QMetaType::Type::LongLong ),
    FieldStatistic( 1, u"unique"_s, QMetaType::Type::LongLong ),
    FieldStatistic( 14, u"empty"_s, QMetaType::Type::LongLong ),
    FieldStatistic( 15, u"filled"_s, QMetaType::Type::LongLong ),
    FieldStatistic( 2, u"min"_s, QMetaType::Type::UnknownType ),
    FieldStatistic( 3, u"max"_s, QMetaType::Type::UnknownType ),
    FieldStatistic( 16, u"min_length"_s, QMetaType::Type::Int ),
    FieldStatistic( 17, u"max_length"_s, QMetaType::Type::Int ),
    FieldStatistic( 18, u"mean_length"_s, QMetaType::Type::Double ),
  };

  for ( const QString &field : std::as_const( joinedFieldNames ) )
  {
    const int fieldIndex = joinSource->fields().lookupField( field );
    if ( fieldIndex >= 0 )
    {
      joinFieldIndices.append( fieldIndex );

      const QgsField joinField = joinSource->fields().at( fieldIndex );
      QVector<FieldStatistic> statisticList;
      if ( joinField.isNumeric() )
      {
        fieldTypes.append( FieldType::Numeric );
        statisticList = sNumericStats;
      }
      else if ( joinField.type() == QMetaType::Type::QDate
                || joinField.type() == QMetaType::Type::QTime
                || joinField.type() == QMetaType::Type::QDateTime )
      {
        fieldTypes.append( FieldType::DateTime );
        statisticList = sDateTimeStats;
      }
      else
      {
        fieldTypes.append( FieldType::String );
        statisticList = sStringStats;
      }

      for ( const FieldStatistic &statistic : std::as_const( statisticList ) )
      {
        if ( summaries.contains( statistic.enumIndex ) )
        {
          if ( statistic.type != QMetaType::Type::UnknownType )
            addFieldWithType( joinField, statistic.name, statistic.type );
          else
            addFieldKeepType( joinField, statistic.name );
        }
      }
    }
  }

  const QgsFields outputFields = QgsProcessingUtils::combineFields( sourceFields, fieldsToJoin );

  QString destId;
  std::unique_ptr<QgsFeatureSink> sink( parameterAsSink( parameters, u"OUTPUT"_s, context, destId, outputFields, baseSource->wkbType(), baseSource->sourceCrs() ) );

  if ( !sink )
    throw QgsProcessingException( invalidSinkError( parameters, u"OUTPUT"_s ) );


  QList<int> predicates = parameterAsEnums( parameters, u"PREDICATE"_s, context );
  QgsJoinByLocationAlgorithm::sortPredicates( predicates );

  QgsFeatureIterator sourceIter = baseSource->getFeatures();
  QgsFeature f;
  const double step = baseSource->featureCount() > 0 ? 100.0 / baseSource->featureCount() : 1;
  long long i = 0;
  while ( sourceIter.nextFeature( f ) )
  {
    if ( feedback->isCanceled() )
      break;

    if ( !f.hasGeometry() )
    {
      if ( !discardNonMatching )
      {
        // ensure consistent count of attributes - otherwise non matching
        // features will have incorrect attribute length
        // and provider may reject them
        f.resizeAttributes( outputFields.size() );
        if ( !sink->addFeature( f, QgsFeatureSink::FastInsert ) )
          throw QgsProcessingException( writeFeatureError( sink.get(), parameters, u"OUTPUT"_s ) );
      }
      continue;
    }

    std::unique_ptr<QgsGeometryEngine> engine;
    QVector<QVector<QVariant>> values;

    QgsFeatureRequest request;
    request.setFilterRect( f.geometry().boundingBox() );
    request.setSubsetOfAttributes( joinFieldIndices );
    request.setDestinationCrs( baseSource->sourceCrs(), context.transformContext() );

    QgsFeatureIterator joinIter = joinSource->getFeatures( request );
    QgsFeature testJoinFeature;
    while ( joinIter.nextFeature( testJoinFeature ) )
    {
      if ( feedback->isCanceled() )
        break;

      if ( !engine )
      {
        engine.reset( QgsGeometry::createGeometryEngine( f.geometry().constGet() ) );
        engine->prepareGeometry();
      }

      if ( QgsJoinByLocationAlgorithm::featureFilter( testJoinFeature, engine.get(), true, predicates ) )
      {
        QgsAttributes joinAttributes;
        joinAttributes.reserve( joinFieldIndices.size() );
        for ( int joinIndex : std::as_const( joinFieldIndices ) )
        {
          joinAttributes.append( testJoinFeature.attribute( joinIndex ) );
        }
        values.append( joinAttributes );
      }
    }

    i++;
    feedback->setProgress( i * step );

    if ( feedback->isCanceled() )
      break;

    if ( values.empty() )
    {
      if ( discardNonMatching )
      {
        continue;
      }
      else
      {
        // ensure consistent count of attributes - otherwise non matching
        // features will have incorrect attribute length
        // and provider may reject them
        f.resizeAttributes( outputFields.size() );
        if ( !sink->addFeature( f, QgsFeatureSink::FastInsert ) )
          throw QgsProcessingException( writeFeatureError( sink.get(), parameters, u"OUTPUT"_s ) );
      }
    }
    else
    {
      // calculate statistics
      QgsAttributes outputAttributes = f.attributes();
      outputAttributes.reserve( outputFields.size() );
      for ( int fieldIndex = 0; fieldIndex < joinFieldIndices.size(); ++fieldIndex )
      {
        const FieldType &fieldType = fieldTypes.at( fieldIndex );
        switch ( fieldType )
        {
          case FieldType::Numeric:
          {
            QgsStatisticalSummary stat;
            for ( const QVector<QVariant> &value : std::as_const( values ) )
            {
              stat.addVariant( value.at( fieldIndex ) );
            }
            stat.finalize();
            for ( const FieldStatistic &statistic : sNumericStats )
            {
              if ( summaries.contains( statistic.enumIndex ) )
              {
                QVariant val;
                switch ( statistic.enumIndex )
                {
                  case 0:
                    val = stat.count();
                    break;
                  case 1:
                    val = stat.variety();
                    break;
                  case 2:
                    val = stat.min();
                    break;
                  case 3:
                    val = stat.max();
                    break;
                  case 4:
                    val = stat.range();
                    break;
                  case 5:
                    val = stat.sum();
                    break;
                  case 6:
                    val = stat.mean();
                    break;
                  case 7:
                    val = stat.median();
                    break;
                  case 8:
                    val = stat.stDev();
                    break;
                  case 9:
                    val = stat.minority();
                    break;
                  case 10:
                    val = stat.majority();
                    break;
                  case 11:
                    val = stat.firstQuartile();
                    break;
                  case 12:
                    val = stat.thirdQuartile();
                    break;
                  case 13:
                    val = stat.interQuartileRange();
                    break;
                }
                if ( val.isValid() && std::isnan( val.toDouble() ) )
                  val = QVariant();
                outputAttributes.append( val );
              }
            }
            break;
          }

          case FieldType::DateTime:
          {
            QgsDateTimeStatisticalSummary stat;
            QVariantList inputValues;
            inputValues.reserve( values.size() );
            for ( const QVector<QVariant> &value : std::as_const( values ) )
            {
              inputValues << value.at( fieldIndex );
            }
            stat.calculate( inputValues );
            for ( const FieldStatistic &statistic : sDateTimeStats )
            {
              if ( summaries.contains( statistic.enumIndex ) )
              {
                QVariant val;
                switch ( statistic.enumIndex )
                {
                  case 0:
                    val = stat.count();
                    break;
                  case 1:
                    val = stat.countDistinct();
                    break;
                  case 2:
                    val = stat.min();
                    break;
                  case 3:
                    val = stat.max();
                    break;
                  case 14:
                    val = stat.countMissing();
                    break;
                  case 15:
                    val = stat.count() - stat.countMissing();
                    break;
                }
                outputAttributes.append( val );
              }
            }
            break;
          }

          case FieldType::String:
          {
            QgsStringStatisticalSummary stat;
            QVariantList inputValues;
            inputValues.reserve( values.size() );
            for ( const QVector<QVariant> &value : std::as_const( values ) )
            {
              if ( value.at( fieldIndex ).isNull() )
                stat.addString( QString() );
              else
                stat.addString( value.at( fieldIndex ).toString() );
            }
            stat.finalize();
            for ( const FieldStatistic &statistic : sStringStats )
            {
              if ( summaries.contains( statistic.enumIndex ) )
              {
                QVariant val;
                switch ( statistic.enumIndex )
                {
                  case 0:
                    val = stat.count();
                    break;
                  case 1:
                    val = stat.countDistinct();
                    break;
                  case 2:
                    val = stat.min();
                    break;
                  case 3:
                    val = stat.max();
                    break;
                  case 14:
                    val = stat.countMissing();
                    break;
                  case 15:
                    val = stat.count() - stat.countMissing();
                    break;
                  case 16:
                    val = stat.minLength();
                    break;
                  case 17:
                    val = stat.maxLength();
                    break;
                  case 18:
                    val = stat.meanLength();
                    break;
                }
                outputAttributes.append( val );
              }
            }
            break;
          }
        }
      }

      f.setAttributes( outputAttributes );
      if ( !sink->addFeature( f, QgsFeatureSink::FastInsert ) )
        throw QgsProcessingException( writeFeatureError( sink.get(), parameters, u"OUTPUT"_s ) );
    }
  }

  sink->finalize();
  sink.reset();

  QVariantMap results;
  results.insert( u"OUTPUT"_s, destId );
  return results;
}

///@endcond
