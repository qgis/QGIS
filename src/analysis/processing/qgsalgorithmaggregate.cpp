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

#include "qgsexpressioncontextutils.h"
#include "qgsprocessingparameteraggregate.h"

///@cond PRIVATE

QString QgsAggregateAlgorithm::name() const
{
  return u"aggregate"_s;
}

QString QgsAggregateAlgorithm::displayName() const
{
  return QObject::tr( "Aggregate" );
}

QString QgsAggregateAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm takes a vector or table layer and aggregates features based on a group by expression. Features for which group by expression return the same value are grouped together.\n\n"
                      "It is possible to group all source features together using constant value in group by parameter, example: NULL.\n\n"
                      "It is also possible to group features using multiple fields using Array function, example: Array(\"Field1\", \"Field2\").\n\n"
                      "Geometries (if present) are combined into one multipart geometry for each group.\n\n"
                      "Output attributes are computed depending on each given aggregate definition." );
}

QString QgsAggregateAlgorithm::shortDescription() const
{
  return QObject::tr( "Aggregates features based on a group by expression, combining geometries (if present) into one multipart geometry for each group." );
}

Qgis::ProcessingAlgorithmDocumentationFlags QgsAggregateAlgorithm::documentationFlags() const
{
  return Qgis::ProcessingAlgorithmDocumentationFlag::RespectsEllipsoid;
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
  return u"vectorgeometry"_s;
}

QgsAggregateAlgorithm *QgsAggregateAlgorithm::createInstance() const
{
  return new QgsAggregateAlgorithm();
}

void QgsAggregateAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( u"INPUT"_s, QObject::tr( "Input layer" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::Vector ) ) );
  addParameter( new QgsProcessingParameterExpression( u"GROUP_BY"_s, QObject::tr( "Group by expression (NULL to group all features)" ), u"NULL"_s, u"INPUT"_s ) );
  addParameter( new QgsProcessingParameterAggregate( u"AGGREGATES"_s, QObject::tr( "Aggregates" ), u"INPUT"_s ) );
  addParameter( new QgsProcessingParameterFeatureSink( u"OUTPUT"_s, QObject::tr( "Aggregated" ) ) );
}

bool QgsAggregateAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mSource.reset( parameterAsSource( parameters, u"INPUT"_s, context ) );
  if ( !mSource )
    throw QgsProcessingException( invalidSourceError( parameters, u"INPUT"_s ) );

  mGroupBy = parameterAsExpression( parameters, u"GROUP_BY"_s, context );

  mDa.setSourceCrs( mSource->sourceCrs(), context.transformContext() );
  mDa.setEllipsoid( context.ellipsoid() );

  mGroupByExpression = createExpression( mGroupBy, context );
  mGeometryExpression = createExpression( u"collect($geometry, %1)"_s.arg( mGroupBy ), context );

  const QVariantList aggregates = parameters.value( u"AGGREGATES"_s ).toList();
  int currentAttributeIndex = 0;
  for ( const QVariant &aggregate : aggregates )
  {
    const QVariantMap aggregateDef = aggregate.toMap();

    const QString name = aggregateDef.value( u"name"_s ).toString();
    if ( name.isEmpty() )
      throw QgsProcessingException( QObject::tr( "Field name cannot be empty" ) );

    const QMetaType::Type type = static_cast<QMetaType::Type>( aggregateDef.value( u"type"_s ).toInt() );
    const QString typeName = aggregateDef.value( u"type_name"_s ).toString();
    const QMetaType::Type subType = static_cast<QMetaType::Type>( aggregateDef.value( u"sub_type"_s ).toInt() );

    const int length = aggregateDef.value( u"length"_s, 0 ).toInt();
    const int precision = aggregateDef.value( u"precision"_s, 0 ).toInt();

    mFields.append( QgsField( name, type, typeName, length, precision, QString(), subType ) );


    const QString aggregateType = aggregateDef.value( u"aggregate"_s ).toString();
    const QString source = aggregateDef.value( u"input"_s ).toString();
    const QString delimiter = aggregateDef.value( u"delimiter"_s ).toString();

    QString expression;
    if ( aggregateType == "first_value"_L1 )
    {
      expression = source;
    }
    else if ( aggregateType == "last_value"_L1 )
    {
      expression = source;
      mAttributesRequireLastFeature << currentAttributeIndex;
    }
    else if ( aggregateType == "concatenate"_L1 || aggregateType == "concatenate_unique"_L1 )
    {
      expression = u"%1(%2, %3, %4, %5)"_s.arg( aggregateType, source, mGroupBy, u"TRUE"_s, QgsExpression::quotedString( delimiter ) );
    }
    else
    {
      expression = u"%1(%2, %3)"_s.arg( aggregateType, source, mGroupBy );
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

  QHash<QVariantList, Group> groups;
  QVector<QVariantList> keys; // We need deterministic order for the tests
  QgsFeature feature;

  std::vector<std::unique_ptr<QgsFeatureSink>> groupSinks;

  QgsFeatureIterator it = mSource->getFeatures( QgsFeatureRequest() );
  while ( it.nextFeature( feature ) )
  {
    expressionContext.setFeature( feature );
    const QVariant groupByValue = mGroupByExpression.evaluate( &expressionContext );
    if ( mGroupByExpression.hasEvalError() )
    {
      throw QgsProcessingException( QObject::tr( "Evaluation error in group by expression \"%1\": %2" ).arg( mGroupByExpression.expression(), mGroupByExpression.evalErrorString() ) );
    }

    // upgrade group by value to a list, so that we get correct behavior with the QHash
    const QVariantList key = groupByValue.userType() == QMetaType::Type::QVariantList ? groupByValue.toList() : ( QVariantList() << groupByValue );

    const auto groupIt = groups.find( key );
    if ( groupIt == groups.end() )
    {
      QString id = u"memory:"_s;
      std::unique_ptr<QgsFeatureSink> sink( QgsProcessingUtils::createFeatureSink( id, context, mSource->fields(), mSource->wkbType(), mSource->sourceCrs() ) );

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
      groups[key] = std::move( group );
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
  std::unique_ptr<QgsFeatureSink> sink( parameterAsSink( parameters, u"OUTPUT"_s, context, destId, mFields, QgsWkbTypes::multiType( mSource->wkbType() ), mSource->sourceCrs() ) );
  if ( !sink )
    throw QgsProcessingException( invalidSinkError( parameters, u"OUTPUT"_s ) );

  // Calculate aggregates on memory layers
  if ( !keys.empty() )
    progressStep = 50.0 / keys.size();

  current = 0;
  for ( const QVariantList &key : keys )
  {
    const Group &group = groups[key];

    QgsExpressionContext exprContext = createExpressionContext( parameters, context );
    exprContext.appendScope( QgsExpressionContextUtils::layerScope( group.layer ) );
    exprContext.setFeature( group.firstFeature );

    QgsGeometry geometry = mGeometryExpression.evaluate( &exprContext ).value<QgsGeometry>();
    if ( mGeometryExpression.hasEvalError() )
    {
      throw QgsProcessingException( QObject::tr( "Evaluation error in geometry expression \"%1\": %2" ).arg( mGeometryExpression.expression(), mGeometryExpression.evalErrorString() ) );
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
      throw QgsProcessingException( writeFeatureError( sink.get(), parameters, u"OUTPUT"_s ) );

    current++;
    feedback->setProgress( 50 + current * progressStep );
    if ( feedback->isCanceled() )
      break;
  }

  sink->finalize();

  QVariantMap results;
  results.insert( u"OUTPUT"_s, destId );
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
      QObject::tr( "Parser error in expression \"%1\": %2" ).arg( expressionString, expr.parserErrorString() )
    );
  }
  return expr;
}

///@endcond
