/***************************************************************************
                         qgsalgorithmextractbyattribute.cpp
                         ----------------------------------
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

#include "qgsalgorithmextractbyattribute.h"

///@cond PRIVATE

QString QgsExtractByAttributeAlgorithm::name() const
{
  return u"extractbyattribute"_s;
}

QString QgsExtractByAttributeAlgorithm::displayName() const
{
  return QObject::tr( "Extract by attribute" );
}

QStringList QgsExtractByAttributeAlgorithm::tags() const
{
  return QObject::tr( "extract,filter,attribute,value,contains,null,field" ).split( ',' );
}

QString QgsExtractByAttributeAlgorithm::group() const
{
  return QObject::tr( "Vector selection" );
}

QString QgsExtractByAttributeAlgorithm::groupId() const
{
  return u"vectorselection"_s;
}

void QgsExtractByAttributeAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( u"INPUT"_s, QObject::tr( "Input layer" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::Vector ) ) );
  addParameter( new QgsProcessingParameterField( u"FIELD"_s, QObject::tr( "Selection attribute" ), QVariant(), u"INPUT"_s ) );
  addParameter( new QgsProcessingParameterEnum( u"OPERATOR"_s, QObject::tr( "Operator" ), QStringList() << QObject::tr( "=" ) << QObject::tr( "≠" ) << QObject::tr( ">" ) << QObject::tr( "≥" ) << QObject::tr( "<" ) << QObject::tr( "≤" ) << QObject::tr( "begins with" ) << QObject::tr( "contains" ) << QObject::tr( "is null" ) << QObject::tr( "is not null" ) << QObject::tr( "does not contain" ), false, 0 ) );
  addParameter( new QgsProcessingParameterString( u"VALUE"_s, QObject::tr( "Value" ), QVariant(), false, true ) );

  addParameter( new QgsProcessingParameterFeatureSink( u"OUTPUT"_s, QObject::tr( "Extracted (attribute)" ) ) );
  QgsProcessingParameterFeatureSink *failOutput = new QgsProcessingParameterFeatureSink( u"FAIL_OUTPUT"_s, QObject::tr( "Extracted (non-matching)" ), Qgis::ProcessingSourceType::VectorAnyGeometry, QVariant(), true );
  failOutput->setCreateByDefault( false );
  addParameter( failOutput );
}

QString QgsExtractByAttributeAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm creates a new vector layer that only contains matching features from an input layer. "
                      "The criteria for adding features to the resulting layer is defined based on the values "
                      "of an attribute from the input layer." );
}

QString QgsExtractByAttributeAlgorithm::shortDescription() const
{
  return QObject::tr( "Creates a vector layer that only contains features matching an attribute value from an input layer." );
}

QgsExtractByAttributeAlgorithm *QgsExtractByAttributeAlgorithm::createInstance() const
{
  return new QgsExtractByAttributeAlgorithm();
}

QVariantMap QgsExtractByAttributeAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr<QgsProcessingFeatureSource> source( parameterAsSource( parameters, u"INPUT"_s, context ) );
  if ( !source )
    throw QgsProcessingException( invalidSourceError( parameters, u"INPUT"_s ) );

  const QString fieldName = parameterAsString( parameters, u"FIELD"_s, context );
  const Operation op = static_cast<Operation>( parameterAsEnum( parameters, u"OPERATOR"_s, context ) );
  const QString value = parameterAsString( parameters, u"VALUE"_s, context );

  QString matchingSinkId;
  std::unique_ptr<QgsFeatureSink> matchingSink( parameterAsSink( parameters, u"OUTPUT"_s, context, matchingSinkId, source->fields(), source->wkbType(), source->sourceCrs() ) );
  if ( !matchingSink )
    throw QgsProcessingException( invalidSinkError( parameters, u"OUTPUT"_s ) );

  QString nonMatchingSinkId;
  std::unique_ptr<QgsFeatureSink> nonMatchingSink( parameterAsSink( parameters, u"FAIL_OUTPUT"_s, context, nonMatchingSinkId, source->fields(), source->wkbType(), source->sourceCrs() ) );

  const int idx = source->fields().lookupField( fieldName );
  if ( idx < 0 )
    throw QgsProcessingException( QObject::tr( "Field '%1' was not found in INPUT source" ).arg( fieldName ) );

  const QMetaType::Type fieldType = source->fields().at( idx ).type();

  if ( fieldType != QMetaType::Type::QString && ( op == BeginsWith || op == Contains || op == DoesNotContain ) )
  {
    QString method;
    switch ( op )
    {
      case BeginsWith:
        method = QObject::tr( "begins with" );
        break;
      case Contains:
        method = QObject::tr( "contains" );
        break;
      case DoesNotContain:
        method = QObject::tr( "does not contain" );
        break;

      default:
        break;
    }

    throw QgsProcessingException( QObject::tr( "Operator '%1' can be used only with string fields." ).arg( method ) );
  }

  const QString fieldRef = QgsExpression::quotedColumnRef( fieldName );
  const QString quotedVal = QgsExpression::quotedValue( value );
  QString expr;
  switch ( op )
  {
    case Equals:
      expr = u"%1 = %3"_s.arg( fieldRef, quotedVal );
      break;
    case NotEquals:
      expr = u"%1 != %3"_s.arg( fieldRef, quotedVal );
      break;
    case GreaterThan:
      expr = u"%1 > %3"_s.arg( fieldRef, quotedVal );
      break;
    case GreaterThanEqualTo:
      expr = u"%1 >= %3"_s.arg( fieldRef, quotedVal );
      break;
    case LessThan:
      expr = u"%1 < %3"_s.arg( fieldRef, quotedVal );
      break;
    case LessThanEqualTo:
      expr = u"%1 <= %3"_s.arg( fieldRef, quotedVal );
      break;
    case BeginsWith:
      expr = u"%1 LIKE '%2%'"_s.arg( fieldRef, value );
      break;
    case Contains:
      expr = u"%1 LIKE '%%2%'"_s.arg( fieldRef, value );
      break;
    case IsNull:
      expr = u"%1 IS NULL"_s.arg( fieldRef );
      break;
    case IsNotNull:
      expr = u"%1 IS NOT NULL"_s.arg( fieldRef );
      break;
    case DoesNotContain:
      expr = u"%1 NOT LIKE '%%2%'"_s.arg( fieldRef, value );
      break;
  }

  QgsExpression expression( expr );
  if ( expression.hasParserError() )
  {
    throw QgsProcessingException( expression.parserErrorString() );
  }

  QgsExpressionContext expressionContext = createExpressionContext( parameters, context, source.get() );

  const long count = source->featureCount();

  const double step = count > 0 ? 100.0 / count : 1;
  int current = 0;

  if ( !nonMatchingSink )
  {
    // not saving failing features - so only fetch good features
    QgsFeatureRequest req;
    req.setFilterExpression( expr );
    req.setExpressionContext( expressionContext );

    QgsFeatureIterator it = source->getFeatures( req, Qgis::ProcessingFeatureSourceFlag::SkipGeometryValidityChecks );
    QgsFeature f;
    while ( it.nextFeature( f ) )
    {
      if ( feedback->isCanceled() )
      {
        break;
      }

      if ( !matchingSink->addFeature( f, QgsFeatureSink::FastInsert ) )
        throw QgsProcessingException( writeFeatureError( matchingSink.get(), parameters, u"OUTPUT"_s ) );

      feedback->setProgress( current * step );
      current++;
    }
  }
  else
  {
    // saving non-matching features, so we need EVERYTHING
    expressionContext.setFields( source->fields() );
    expression.prepare( &expressionContext );

    QgsFeatureIterator it = source->getFeatures( QgsFeatureRequest(), Qgis::ProcessingFeatureSourceFlag::SkipGeometryValidityChecks );
    QgsFeature f;
    while ( it.nextFeature( f ) )
    {
      if ( feedback->isCanceled() )
      {
        break;
      }

      expressionContext.setFeature( f );
      if ( expression.evaluate( &expressionContext ).toBool() )
      {
        if ( !matchingSink->addFeature( f, QgsFeatureSink::FastInsert ) )
          throw QgsProcessingException( writeFeatureError( matchingSink.get(), parameters, u"OUTPUT"_s ) );
      }
      else
      {
        if ( !nonMatchingSink->addFeature( f, QgsFeatureSink::FastInsert ) )
          throw QgsProcessingException( writeFeatureError( nonMatchingSink.get(), parameters, u"FAIL_OUTPUT"_s ) );
      }

      feedback->setProgress( current * step );
      current++;
    }
  }

  if ( matchingSink )
    matchingSink->finalize();
  if ( nonMatchingSink )
    nonMatchingSink->finalize();

  QVariantMap outputs;
  outputs.insert( u"OUTPUT"_s, matchingSinkId );
  if ( nonMatchingSink )
    outputs.insert( u"FAIL_OUTPUT"_s, nonMatchingSinkId );
  return outputs;
}

///@endcond
