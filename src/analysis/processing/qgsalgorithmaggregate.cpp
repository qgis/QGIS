/***************************************************************************
                         qgsalgorithmaggregate.h
                         ---------------------------------
    begin                : June 2020
    copyright            : (C) 2020 by Nyall Dawson
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

#include "qgsalgorithmaggregate.h"
#include "qgsprocessingparameteraggregate.h"
#include "qgsexpressioncontextutils.h"

///@cond PRIVATE

QString QgsAggregateAlgorithm::name() const
{
  return QStringLiteral( "aggregate" );
}

QString QgsAggregateAlgorithm::displayName() const
{
  return QObject::tr( "Aggregate" );
}

QString QgsAggregateAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm take a vector or table layer and aggregate features based on a group by expression. Features for which group by expression return the same value are grouped together.\n\n"
                      "It is possible to group all source features together using constant value in group by parameter, example: NULL.\n\n"
                      "It is also possible to group features using multiple fields using Array function, example: Array(\"Field1\", \"Field2\").\n\n"
                      "Geometries (if present) are combined into one multipart geometry for each group.\n\n"
                      "Output attributes are computed depending on each given aggregate definition." );
}

QStringList QgsAggregateAlgorithm::tags() const
{
  return QObject::tr( "attributes,sum,mean,collect,dissolve,statistics" ).split( ',' );
}

QString QgsAggregateAlgorithm::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsAggregateAlgorithm::groupId() const
{
  return QStringLiteral( "vectorgeometry" );
}

QgsAggregateAlgorithm *QgsAggregateAlgorithm::createInstance() const
{
  return new QgsAggregateAlgorithm();
}

void QgsAggregateAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ), QList<int>() << QgsProcessing::TypeVector ) );
  addParameter( new QgsProcessingParameterExpression( QStringLiteral( "GROUP_BY" ), QObject::tr( "Group by expression (NULL to group all features)" ), QStringLiteral( "NULL" ), QStringLiteral( "INPUT" ) ) );
  addParameter( new QgsProcessingParameterAggregate( QStringLiteral( "AGGREGATES" ), QObject::tr( "Aggregates" ), QStringLiteral( "INPUT" ) ) );
  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Aggregated" ) ) );
}

bool QgsAggregateAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mSource.reset( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !mSource )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "INPUT" ) ) );

  mGroupBy = parameterAsExpression( parameters, QStringLiteral( "GROUP_BY" ), context );

  mDa.setSourceCrs( mSource->sourceCrs(), context.transformContext() );
  mDa.setEllipsoid( context.ellipsoid() );

  mGroupByExpression = createExpression( mGroupBy, context );
  mGeometryExpression = createExpression( QStringLiteral( "collect($geometry, %1)" ).arg( mGroupBy ), context );

  const QVariantList aggregates = parameters.value( QStringLiteral( "AGGREGATES" ) ).toList();
  int currentAttributeIndex = 0;
  for ( const QVariant &aggregate : aggregates )
  {
    const QVariantMap aggregateDef = aggregate.toMap();

    const QString name = aggregateDef.value( QStringLiteral( "name" ) ).toString();
    if ( name.isEmpty() )
      throw QgsProcessingException( QObject::tr( "Field name cannot be empty" ) );

    const QVariant::Type type = static_cast< QVariant::Type >( aggregateDef.value( QStringLiteral( "type" ) ).toInt() );
    const QString typeName = aggregateDef.value( QStringLiteral( "type_name" ) ).toString();
    const QVariant::Type subType = static_cast< QVariant::Type >( aggregateDef.value( QStringLiteral( "sub_type" ) ).toInt() );

    const int length = aggregateDef.value( QStringLiteral( "length" ), 0 ).toInt();
    const int precision = aggregateDef.value( QStringLiteral( "precision" ), 0 ).toInt();

    mFields.append( QgsField( name, type, typeName, length, precision, QString(), subType ) );


    const QString aggregateType = aggregateDef.value( QStringLiteral( "aggregate" ) ).toString();
    const QString source = aggregateDef.value( QStringLiteral( "input" ) ).toString();
    const QString delimiter = aggregateDef.value( QStringLiteral( "delimiter" ) ).toString();

    QString expression;
    if ( aggregateType == QLatin1String( "first_value" ) )
    {
      expression = source;
    }
    else if ( aggregateType == QLatin1String( "last_value" ) )
    {
      expression = source;
      mAttributesRequireLastFeature << currentAttributeIndex;
    }
    else if ( aggregateType == QLatin1String( "concatenate" ) || aggregateType == QLatin1String( "concatenate_unique" ) )
    {
      expression = QStringLiteral( "%1(%2, %3, %4, %5)" ).arg( aggregateType,
                   source,
                   mGroupBy,
                   QStringLiteral( "TRUE" ),
                   QgsExpression::quotedString( delimiter ) );
    }
    else
    {
      expression = QStringLiteral( "%1(%2, %3)" ).arg( aggregateType, source, mGroupBy );
    }
    mExpressions.append( createExpression( expression, context ) );
    currentAttributeIndex++;
  }

  return true;
}

QVariantMap QgsAggregateAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QgsExpressionContext expressionContext = createExpressionContext( parameters, context, mSource.get() );
  mGroupByExpression.prepare( &expressionContext );

  // Group features in memory layers
  const long long count = mSource->featureCount();
  double progressStep = count > 0 ? 50.0 / count : 1;
  long long current = 0;

  QHash< QVariantList, Group > groups;
  QVector< QVariantList > keys; // We need deterministic order for the tests
  QgsFeature feature;

  std::vector< std::unique_ptr< QgsFeatureSink > > groupSinks;

  QgsFeatureIterator it = mSource->getFeatures( QgsFeatureRequest() );
  while ( it.nextFeature( feature ) )
  {
    expressionContext.setFeature( feature );
    const QVariant groupByValue = mGroupByExpression.evaluate( &expressionContext );
    if ( mGroupByExpression.hasEvalError() )
    {
      throw QgsProcessingException( QObject::tr( "Evaluation error in group by expression \"%1\": %2" ).arg( mGroupByExpression.expression(),
                                    mGroupByExpression.evalErrorString() ) );
    }

    // upgrade group by value to a list, so that we get correct behavior with the QHash
    const QVariantList key = groupByValue.type() == QVariant::List ? groupByValue.toList() : ( QVariantList() << groupByValue );

    const auto groupIt = groups.find( key );
    if ( groupIt == groups.end() )
    {
      QString id = QStringLiteral( "memory:" );
      std::unique_ptr< QgsFeatureSink > sink( QgsProcessingUtils::createFeatureSink( id,
                                              context,
                                              mSource->fields(),
                                              mSource->wkbType(),
                                              mSource->sourceCrs() ) );

      if ( !sink->addFeature( feature, QgsFeatureSink::FastInsert ) )
        throw QgsProcessingException( writeFeatureError( sink.get(), parameters, QString() ) );

      QgsMapLayer *layer = QgsProcessingUtils::mapLayerFromString( id, context );

      Group group;
      group.sink = sink.get();
      //store ownership of sink in groupSinks, so that these get deleted automatically if an exception is raised later..
      groupSinks.emplace_back( std::move( sink ) );
      group.layer = layer;
      group.firstFeature = feature;
      group.lastFeature = feature;
      groups[key] = group;
      keys.append( key );
    }
    else
    {
      if ( !groupIt->sink->addFeature( feature, QgsFeatureSink::FastInsert ) )
        throw QgsProcessingException( writeFeatureError( groupIt->sink, parameters, QString() ) );
      groupIt->lastFeature = feature;
    }

    current++;
    feedback->setProgress( current * progressStep );
    if ( feedback->isCanceled() )
      break;
  }

  // early cleanup
  groupSinks.clear();

  QString destId;
  std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, destId, mFields, QgsWkbTypes::multiType( mSource->wkbType() ), mSource->sourceCrs() ) );
  if ( !sink )
    throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "OUTPUT" ) ) );

  // Calculate aggregates on memory layers
  if ( !keys.empty() )
    progressStep = 50.0 / keys.size();

  current = 0;
  for ( const QVariantList &key : keys )
  {
    const Group &group = groups[ key ];

    QgsExpressionContext exprContext = createExpressionContext( parameters, context );
    exprContext.appendScope( QgsExpressionContextUtils::layerScope( group.layer ) );
    exprContext.setFeature( group.firstFeature );

    QgsGeometry geometry = mGeometryExpression.evaluate( &exprContext ).value< QgsGeometry >();
    if ( mGeometryExpression.hasEvalError() )
    {
      throw QgsProcessingException( QObject::tr( "Evaluation error in geometry expression \"%1\": %2" ).arg( mGeometryExpression.expression(),
                                    mGeometryExpression.evalErrorString() ) );
    }

    if ( !geometry.isNull() && !geometry.isEmpty() )
    {
      geometry = QgsGeometry::unaryUnion( geometry.asGeometryCollection() );
      if ( geometry.isEmpty() )
      {
        QStringList keyString;
        for ( const QVariant &v : key )
          keyString << v.toString();

        throw QgsProcessingException( QObject::tr( "Impossible to combine geometries for %1 = %2" ).arg( mGroupBy, keyString.join( ',' ) ) );
      }
    }

    QgsAttributes attributes;
    attributes.reserve( mExpressions.size() );
    int currentAttributeIndex = 0;
    for ( auto it = mExpressions.begin(); it != mExpressions.end(); ++it )
    {
      exprContext.setFeature( mAttributesRequireLastFeature.contains( currentAttributeIndex ) ? group.lastFeature : group.firstFeature );
      if ( it->isValid() )
      {
        const QVariant value = it->evaluate( &exprContext );
        if ( it->hasEvalError() )
        {
          throw QgsProcessingException( QObject::tr( "Evaluation error in expression \"%1\": %2" ).arg( it->expression(), it->evalErrorString() ) );
        }
        attributes.append( value );
      }
      else
      {
        attributes.append( QVariant() );
      }
      currentAttributeIndex++;
    }

    // Write output feature
    QgsFeature outFeat;
    outFeat.setGeometry( geometry );
    outFeat.setAttributes( attributes );
    if ( !sink->addFeature( outFeat, QgsFeatureSink::FastInsert ) )
      throw QgsProcessingException( writeFeatureError( sink.get(), parameters, QStringLiteral( "OUTPUT" ) ) );

    current++;
    feedback->setProgress( 50 + current * progressStep );
    if ( feedback->isCanceled() )
      break;
  }

  QVariantMap results;
  results.insert( QStringLiteral( "OUTPUT" ), destId );
  return results;
}

bool QgsAggregateAlgorithm::supportInPlaceEdit( const QgsMapLayer *layer ) const
{
  Q_UNUSED( layer )
  return false;
}

QgsExpression QgsAggregateAlgorithm::createExpression( const QString &expressionString, QgsProcessingContext &context ) const
{
  QgsExpression expr( expressionString );
  expr.setGeomCalculator( &mDa );
  expr.setDistanceUnits( context.distanceUnit() );
  expr.setAreaUnits( context.areaUnit() );
  if ( expr.hasParserError() )
  {
    throw QgsProcessingException(
      QObject::tr( "Parser error in expression \"%1\": %2" ).arg( expressionString, expr.parserErrorString() ) );
  }
  return expr;
}

///@endcond
