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
  return QStringLiteral( "extractbyattribute" );
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
  return QStringLiteral( "vectorselection" );
}

void QgsExtractByAttributeAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ),
                QList< int >() << static_cast< int >( Qgis::ProcessingSourceType::Vector ) ) );
  addParameter( new QgsProcessingParameterField( QStringLiteral( "FIELD" ), QObject::tr( "Selection attribute" ), QVariant(), QStringLiteral( "INPUT" ) ) );
  addParameter( new QgsProcessingParameterEnum( QStringLiteral( "OPERATOR" ), QObject::tr( "Operator" ), QStringList()
                << QObject::tr( "=" )
                << QObject::tr( "≠" )
                << QObject::tr( ">" )
                << QObject::tr( "≥" )
                << QObject::tr( "<" )
                << QObject::tr( "≤" )
                << QObject::tr( "begins with" )
                << QObject::tr( "contains" )
                << QObject::tr( "is null" )
                << QObject::tr( "is not null" )
                << QObject::tr( "does not contain" ), false, 0 ) );
  addParameter( new QgsProcessingParameterString( QStringLiteral( "VALUE" ), QObject::tr( "Value" ), QVariant(), false, true ) );

  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Extracted (attribute)" ) ) );
  QgsProcessingParameterFeatureSink *failOutput = new QgsProcessingParameterFeatureSink( QStringLiteral( "FAIL_OUTPUT" ),  QObject::tr( "Extracted (non-matching)" ),
      Qgis::ProcessingSourceType::VectorAnyGeometry, QVariant(), true );
  failOutput->setCreateByDefault( false );
  addParameter( failOutput );
}

QString QgsExtractByAttributeAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm creates a new vector layer that only contains matching features from an input layer. "
                      "The criteria for adding features to the resulting layer is defined based on the values "
                      "of an attribute from the input layer." );
}

QgsExtractByAttributeAlgorithm *QgsExtractByAttributeAlgorithm::createInstance() const
{
  return new QgsExtractByAttributeAlgorithm();
}

QVariantMap QgsExtractByAttributeAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr< QgsProcessingFeatureSource > source( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !source )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "INPUT" ) ) );

  const QString fieldName = parameterAsString( parameters, QStringLiteral( "FIELD" ), context );
  const Operation op = static_cast< Operation >( parameterAsEnum( parameters, QStringLiteral( "OPERATOR" ), context ) );
  const QString value = parameterAsString( parameters, QStringLiteral( "VALUE" ), context );

  QString matchingSinkId;
  std::unique_ptr< QgsFeatureSink > matchingSink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, matchingSinkId, source->fields(),
      source->wkbType(), source->sourceCrs() ) );
  if ( !matchingSink )
    throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "OUTPUT" ) ) );

  QString nonMatchingSinkId;
  std::unique_ptr< QgsFeatureSink > nonMatchingSink( parameterAsSink( parameters, QStringLiteral( "FAIL_OUTPUT" ), context, nonMatchingSinkId, source->fields(),
      source->wkbType(), source->sourceCrs() ) );

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
      expr = QStringLiteral( "%1 = %3" ).arg( fieldRef, quotedVal );
      break;
    case  NotEquals:
      expr = QStringLiteral( "%1 != %3" ).arg( fieldRef, quotedVal );
      break;
    case GreaterThan:
      expr = QStringLiteral( "%1 > %3" ).arg( fieldRef, quotedVal );
      break;
    case GreaterThanEqualTo:
      expr = QStringLiteral( "%1 >= %3" ).arg( fieldRef, quotedVal );
      break;
    case LessThan:
      expr = QStringLiteral( "%1 < %3" ).arg( fieldRef, quotedVal );
      break;
    case LessThanEqualTo:
      expr = QStringLiteral( "%1 <= %3" ).arg( fieldRef, quotedVal );
      break;
    case BeginsWith:
      expr = QStringLiteral( "%1 LIKE '%2%'" ).arg( fieldRef, value );
      break;
    case Contains:
      expr = QStringLiteral( "%1 LIKE '%%2%'" ).arg( fieldRef, value );
      break;
    case IsNull:
      expr = QStringLiteral( "%1 IS NULL" ).arg( fieldRef );
      break;
    case IsNotNull:
      expr = QStringLiteral( "%1 IS NOT NULL" ).arg( fieldRef );
      break;
    case DoesNotContain:
      expr = QStringLiteral( "%1 NOT LIKE '%%2%'" ).arg( fieldRef, value );
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
        throw QgsProcessingException( writeFeatureError( matchingSink.get(), parameters, QStringLiteral( "OUTPUT" ) ) );

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
          throw QgsProcessingException( writeFeatureError( matchingSink.get(), parameters, QStringLiteral( "OUTPUT" ) ) );
      }
      else
      {
        if ( !nonMatchingSink->addFeature( f, QgsFeatureSink::FastInsert ) )
          throw QgsProcessingException( writeFeatureError( nonMatchingSink.get(), parameters, QStringLiteral( "FAIL_OUTPUT" ) ) );
      }

      feedback->setProgress( current * step );
      current++;
    }
  }


  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), matchingSinkId );
  if ( nonMatchingSink )
    outputs.insert( QStringLiteral( "FAIL_OUTPUT" ), nonMatchingSinkId );
  return outputs;
}

///@endcond


