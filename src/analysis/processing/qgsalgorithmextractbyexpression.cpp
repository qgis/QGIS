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
  return u"extractbyexpression"_s;
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
  return u"vectorselection"_s;
}

void QgsExtractByExpressionAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( u"INPUT"_s, QObject::tr( "Input layer" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::Vector ) ) );
  addParameter( new QgsProcessingParameterExpression( u"EXPRESSION"_s, QObject::tr( "Expression" ), QVariant(), u"INPUT"_s ) );

  addParameter( new QgsProcessingParameterFeatureSink( u"OUTPUT"_s, QObject::tr( "Matching features" ) ) );
  QgsProcessingParameterFeatureSink *failOutput = new QgsProcessingParameterFeatureSink( u"FAIL_OUTPUT"_s, QObject::tr( "Non-matching" ), Qgis::ProcessingSourceType::VectorAnyGeometry, QVariant(), true );
  failOutput->setCreateByDefault( false );
  addParameter( failOutput );
}

QString QgsExtractByExpressionAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm creates a new vector layer that only contains matching features from an input layer. "
                      "The criteria for adding features to the resulting layer is based on a QGIS expression.\n\n"
                      "For help with QGIS expression functions, see the inbuilt help for specific functions "
                      "which is available in the expression builder." );
}

QString QgsExtractByExpressionAlgorithm::shortDescription() const
{
  return QObject::tr( "Creates a vector layer that only contains features matching a QGIS expression from an input layer." );
}

QgsExtractByExpressionAlgorithm *QgsExtractByExpressionAlgorithm::createInstance() const
{
  return new QgsExtractByExpressionAlgorithm();
}

QVariantMap QgsExtractByExpressionAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr<QgsProcessingFeatureSource> source( parameterAsSource( parameters, u"INPUT"_s, context ) );
  if ( !source )
    throw QgsProcessingException( invalidSourceError( parameters, u"INPUT"_s ) );

  const QString expressionString = parameterAsExpression( parameters, u"EXPRESSION"_s, context );

  QString matchingSinkId;
  std::unique_ptr<QgsFeatureSink> matchingSink( parameterAsSink( parameters, u"OUTPUT"_s, context, matchingSinkId, source->fields(), source->wkbType(), source->sourceCrs() ) );
  if ( !matchingSink )
    throw QgsProcessingException( invalidSinkError( parameters, u"OUTPUT"_s ) );

  QString nonMatchingSinkId;
  std::unique_ptr<QgsFeatureSink> nonMatchingSink( parameterAsSink( parameters, u"FAIL_OUTPUT"_s, context, nonMatchingSinkId, source->fields(), source->wkbType(), source->sourceCrs() ) );

  QgsExpression expression( expressionString );
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
    req.setFilterExpression( expressionString );
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
