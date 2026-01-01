/***************************************************************************
                         qgsalgorithmfilter.cpp
                         ---------------------
    begin                : April 2018
    copyright            : (C) 2018 by Matthias Kuhn
    email                : matthias@opengis.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsalgorithmfilter.h"

#include "qgsapplication.h"

///@cond PRIVATE

QString QgsFilterAlgorithm::name() const
{
  return u"filter"_s;
}

QString QgsFilterAlgorithm::displayName() const
{
  return QObject::tr( "Feature filter" );
}

QStringList QgsFilterAlgorithm::tags() const
{
  return QObject::tr( "filter,proxy,redirect,route" ).split( ',' );
}

QString QgsFilterAlgorithm::group() const
{
  return QObject::tr( "Modeler tools" );
}

QString QgsFilterAlgorithm::groupId() const
{
  return u"modelertools"_s;
}

Qgis::ProcessingAlgorithmFlags QgsFilterAlgorithm::flags() const
{
  return Qgis::ProcessingAlgorithmFlag::HideFromToolbox;
}

QString QgsFilterAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm filters features from the input layer and redirects them to one or more outputs." );
}

QString QgsFilterAlgorithm::shortDescription() const
{
  return QObject::tr( "Filters features from the input layer and redirects them to one or more outputs." );
}

QgsFilterAlgorithm *QgsFilterAlgorithm::createInstance() const
{
  return new QgsFilterAlgorithm();
}

QgsFilterAlgorithm::~QgsFilterAlgorithm()
{
  qDeleteAll( mOutputs );
}

void QgsFilterAlgorithm::initAlgorithm( const QVariantMap &configuration )
{
  addParameter( new QgsProcessingParameterFeatureSource( u"INPUT"_s, QObject::tr( "Input layer" ) ) );

  const QVariantList outputs = configuration.value( u"outputs"_s ).toList();
  for ( const QVariant &output : outputs )
  {
    const QVariantMap outputDef = output.toMap();
    const QString name = u"OUTPUT_%1"_s.arg( outputDef.value( u"name"_s ).toString() );
    QgsProcessingParameterFeatureSink *outputParam = new QgsProcessingParameterFeatureSink( name, outputDef.value( u"name"_s ).toString() );
    Qgis::ProcessingParameterFlags flags;
    flags |= Qgis::ProcessingParameterFlag::Hidden;
    if ( outputDef.value( u"isModelOutput"_s ).toBool() )
      flags |= Qgis::ProcessingParameterFlag::IsModelOutput;
    outputParam->setFlags( flags );
    addParameter( outputParam );
    mOutputs.append( new Output( name, outputDef.value( u"expression"_s ).toString() ) );
  }
}


QVariantMap QgsFilterAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr<QgsProcessingFeatureSource> source( parameterAsSource( parameters, u"INPUT"_s, context ) );
  if ( !source )
    throw QgsProcessingException( invalidSourceError( parameters, u"INPUT"_s ) );

  QgsExpressionContext expressionContext = createExpressionContext( parameters, context, source.get() );
  for ( Output *output : std::as_const( mOutputs ) )
  {
    output->sink.reset( parameterAsSink( parameters, output->name, context, output->destinationIdentifier, source->fields(), source->wkbType(), source->sourceCrs() ) );
    if ( !output->sink )
      throw QgsProcessingException( invalidSinkError( parameters, output->name ) );
    output->expression.prepare( &expressionContext );
  }

  long count = source->featureCount();

  QgsFeature f;
  QgsFeatureIterator it = source->getFeatures( QgsFeatureRequest(), Qgis::ProcessingFeatureSourceFlag::SkipGeometryValidityChecks );

  double step = count > 0 ? 100.0 / count : 1;
  int current = 0;

  while ( it.nextFeature( f ) )
  {
    if ( feedback->isCanceled() )
    {
      break;
    }

    expressionContext.setFeature( f );

    for ( Output *output : std::as_const( mOutputs ) )
    {
      if ( output->expression.evaluate( &expressionContext ).toBool() )
      {
        if ( !output->sink->addFeature( f, QgsFeatureSink::FastInsert ) )
          throw QgsProcessingException( writeFeatureError( output->sink.get(), parameters, output->name ) );
      }
    }

    feedback->setProgress( current * step );
    current++;
  }

  QVariantMap outputs;
  for ( const Output *output : std::as_const( mOutputs ) )
  {
    outputs.insert( output->name, output->destinationIdentifier );
  }
  qDeleteAll( mOutputs );
  mOutputs.clear();
  return outputs;
}


///@endcond PRIVATE
