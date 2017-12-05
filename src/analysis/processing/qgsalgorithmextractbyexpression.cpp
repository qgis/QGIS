/***************************************************************************
                         qgsalgorithmextractbyexpression.cpp
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

#include "qgsalgorithmextractbyexpression.h"

///@cond PRIVATE

QString QgsExtractByExpressionAlgorithm::name() const
{
  return QStringLiteral( "extractbyexpression" );
}

QString QgsExtractByExpressionAlgorithm::displayName() const
{
  return QObject::tr( "Extract by expression" );
}

QStringList QgsExtractByExpressionAlgorithm::tags() const
{
  return QObject::tr( "extract,filter,expression,field" ).split( ',' );
}

QString QgsExtractByExpressionAlgorithm::group() const
{
  return QObject::tr( "Vector selection" );
}

QString QgsExtractByExpressionAlgorithm::groupId() const
{
  return QStringLiteral( "vectorselection" );
}

QgsProcessingAlgorithm::Flags QgsExtractByExpressionAlgorithm::flags() const
{
  return QgsProcessingAlgorithm::flags() | QgsProcessingAlgorithm::FlagCanRunInBackground;
}

void QgsExtractByExpressionAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterExpression( QStringLiteral( "EXPRESSION" ), QObject::tr( "Expression" ), QVariant(), QStringLiteral( "INPUT" ) ) );

  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Matching features" ) ) );
  QgsProcessingParameterFeatureSink *failOutput = new QgsProcessingParameterFeatureSink( QStringLiteral( "FAIL_OUTPUT" ),  QObject::tr( "Non-matching" ),
      QgsProcessing::TypeVectorAnyGeometry, QVariant(), true );
  failOutput->setCreateByDefault( false );
  addParameter( failOutput );
}

QString QgsExtractByExpressionAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm creates a new vector layer that only contains matching features from an input layer. "
                      "The criteria for adding features to the resulting layer is based on a QGIS expression.\n\n"
                      "For more information about expressions see the <a href =\"{qgisdocs}/user_manual/working_with_vector/expression.html\">user manual</a>" );
}

QgsExtractByExpressionAlgorithm *QgsExtractByExpressionAlgorithm::createInstance() const
{
  return new QgsExtractByExpressionAlgorithm();
}

QVariantMap QgsExtractByExpressionAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr< QgsFeatureSource > source( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !source )
    return QVariantMap();

  QString expressionString = parameterAsExpression( parameters, QStringLiteral( "EXPRESSION" ), context );

  QString matchingSinkId;
  std::unique_ptr< QgsFeatureSink > matchingSink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, matchingSinkId, source->fields(),
      source->wkbType(), source->sourceCrs() ) );
  if ( !matchingSink )
    return QVariantMap();

  QString nonMatchingSinkId;
  std::unique_ptr< QgsFeatureSink > nonMatchingSink( parameterAsSink( parameters, QStringLiteral( "FAIL_OUTPUT" ), context, nonMatchingSinkId, source->fields(),
      source->wkbType(), source->sourceCrs() ) );

  QgsExpression expression( expressionString );
  if ( expression.hasParserError() )
  {
    throw QgsProcessingException( expression.parserErrorString() );
  }

  QgsExpressionContext expressionContext = createExpressionContext( parameters, context, dynamic_cast< QgsProcessingFeatureSource * >( source.get() ) );

  long count = source->featureCount();

  double step = count > 0 ? 100.0 / count : 1;
  int current = 0;

  if ( !nonMatchingSink )
  {
    // not saving failing features - so only fetch good features
    QgsFeatureRequest req;
    req.setFilterExpression( expressionString );
    req.setExpressionContext( expressionContext );

    QgsFeatureIterator it = source->getFeatures( req );
    QgsFeature f;
    while ( it.nextFeature( f ) )
    {
      if ( feedback->isCanceled() )
      {
        break;
      }

      matchingSink->addFeature( f, QgsFeatureSink::FastInsert );

      feedback->setProgress( current * step );
      current++;
    }
  }
  else
  {
    // saving non-matching features, so we need EVERYTHING
    expressionContext.setFields( source->fields() );
    expression.prepare( &expressionContext );

    QgsFeatureIterator it = source->getFeatures();
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
        matchingSink->addFeature( f, QgsFeatureSink::FastInsert );
      }
      else
      {
        nonMatchingSink->addFeature( f, QgsFeatureSink::FastInsert );
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

