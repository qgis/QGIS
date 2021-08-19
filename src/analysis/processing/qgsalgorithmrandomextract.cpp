/***************************************************************************
                         qgsalgorithmrandomextract.cpp
                         ---------------------
    begin                : December 2019
    copyright            : (C) 2019 by Alexander Bruy
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

#include "qgsalgorithmrandomextract.h"
#include <random>
#include <functional>

///@cond PRIVATE

QString QgsRandomExtractAlgorithm::name() const
{
  return QStringLiteral( "randomextract" );
}

QString QgsRandomExtractAlgorithm::displayName() const
{
  return QObject::tr( "Random extract" );
}

QStringList QgsRandomExtractAlgorithm::tags() const
{
  return QObject::tr( "extract,filter,random,number,percentage" ).split( ',' );
}

QString QgsRandomExtractAlgorithm::group() const
{
  return QObject::tr( "Vector selection" );
}

QString QgsRandomExtractAlgorithm::groupId() const
{
  return QStringLiteral( "vectorselection" );
}

QString QgsRandomExtractAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm takes a vector layer and generates a new one that contains only a subset "
                      "of the features in the input layer.\n\n"
                      "The subset is defined randomly, using a percentage or count value to define the total number "
                      "of features in the subset." );
}

QgsRandomExtractAlgorithm *QgsRandomExtractAlgorithm::createInstance() const
{
  return new QgsRandomExtractAlgorithm();
}

void QgsRandomExtractAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ),
                QList< int >() << QgsProcessing::TypeVector ) );
  addParameter( new QgsProcessingParameterEnum( QStringLiteral( "METHOD" ), QObject::tr( "Method" ), QStringList() << QObject::tr( "Number of features" ) << QObject::tr( "Percentage of features" ), false, 0 ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "NUMBER" ), QObject::tr( "Number/percentage of features" ),
                QgsProcessingParameterNumber::Integer, 10, false, 0 ) );

  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Extracted (random)" ) ) );
}

QVariantMap QgsRandomExtractAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr< QgsProcessingFeatureSource > source( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !source )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "INPUT" ) ) );

  QString dest;
  std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, dest, source->fields(),
                                          source->wkbType(), source->sourceCrs(), QgsFeatureSink::RegeneratePrimaryKey ) );
  if ( !sink )
    throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "OUTPUT" ) ) );

  const int method = parameterAsEnum( parameters, QStringLiteral( "METHOD" ), context );
  int number = parameterAsInt( parameters, QStringLiteral( "NUMBER" ), context );

  const long count = source->featureCount();

  if ( method == 0 )
  {
    // number of features
    if ( number > count )
      throw QgsProcessingException( QObject::tr( "Selected number is greater than feature count. Choose a lower value and try again." ) );
  }
  else
  {
    // percentage of features
    if ( number > 100 )
      throw QgsProcessingException( QObject::tr( "Percentage can't be greater than 100. Choose a lower value and try again." ) );

    number = static_cast< int >( std::ceil( number * count / 100 ) );
  }

  // initialize random engine
  std::random_device randomDevice;
  const std::mt19937 mersenneTwister( randomDevice() );
  const std::uniform_int_distribution<int> fidsDistribution( 0, count );

  QVector< QgsFeatureId > fids( number );
  std::generate( fids.begin(), fids.end(), bind( fidsDistribution, mersenneTwister ) );

  QHash< QgsFeatureId, int > idsCount;
  for ( const QgsFeatureId id : fids )
  {
    if ( feedback->isCanceled() )
    {
      break;
    }

    idsCount[ id ] += 1;
  }

  const QgsFeatureIds ids = qgis::listToSet( idsCount.keys() );
  QgsFeatureIterator fit = source->getFeatures( QgsFeatureRequest().setFilterFids( ids ), QgsProcessingFeatureSource::FlagSkipGeometryValidityChecks );

  QgsFeature f;
  while ( fit.nextFeature( f ) )
  {
    if ( feedback->isCanceled() )
    {
      break;
    }

    const int count = idsCount.value( f.id() );
    for ( int i = 0; i < count; ++i )
    {
      if ( !sink->addFeature( f, QgsFeatureSink::FastInsert ) )
        throw QgsProcessingException( writeFeatureError( sink.get(), parameters, QStringLiteral( "OUTPUT" ) ) );
    }
  }

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), dest );
  return outputs;
}

///@endcond
