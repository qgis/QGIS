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

Qgis::ProcessingAlgorithmDocumentationFlags QgsRandomExtractAlgorithm::documentationFlags() const
{
  return Qgis::ProcessingAlgorithmDocumentationFlag::RegeneratesPrimaryKey;
}

QgsRandomExtractAlgorithm *QgsRandomExtractAlgorithm::createInstance() const
{
  return new QgsRandomExtractAlgorithm();
}

void QgsRandomExtractAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::Vector ) ) );
  addParameter( new QgsProcessingParameterEnum( QStringLiteral( "METHOD" ), QObject::tr( "Method" ), QStringList() << QObject::tr( "Number of features" ) << QObject::tr( "Percentage of features" ), false, 0 ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "NUMBER" ), QObject::tr( "Number/percentage of features" ), Qgis::ProcessingNumberParameterType::Integer, 10, false, 0 ) );

  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Extracted (random)" ) ) );
}

QVariantMap QgsRandomExtractAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr<QgsProcessingFeatureSource> source( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !source )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "INPUT" ) ) );

  QString dest;
  std::unique_ptr<QgsFeatureSink> sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, dest, source->fields(), source->wkbType(), source->sourceCrs(), QgsFeatureSink::RegeneratePrimaryKey ) );
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

    number = static_cast<int>( std::ceil( number * count / 100 ) );
  }

  // Build a list of all feature ids
  QgsFeatureIterator fit = source->getFeatures( QgsFeatureRequest()
                                                  .setFlags( Qgis::FeatureRequestFlag::NoGeometry )
                                                  .setNoAttributes() );
  std::vector<QgsFeatureId> allFeats;
  allFeats.reserve( count );
  QgsFeature f;
  feedback->pushInfo( QObject::tr( "Building list of all features..." ) );
  while ( fit.nextFeature( f ) )
  {
    if ( feedback->isCanceled() )
      return QVariantMap();
    allFeats.push_back( f.id() );
  }
  feedback->pushInfo( QObject::tr( "Done." ) );

  // initialize random engine
  std::random_device randomDevice;
  std::mt19937 mersenneTwister( randomDevice() );
  std::uniform_int_distribution<size_t> fidsDistribution;

  // If the number of features to select is greater than half the total number of features
  // we will instead randomly select features to *exclude* from the output layer
  size_t actualFeatureCount = allFeats.size();
  size_t shuffledFeatureCount = number;
  bool invertSelection = static_cast<size_t>( number ) > actualFeatureCount / 2;
  if ( invertSelection )
    shuffledFeatureCount = actualFeatureCount - number;

  size_t nb = actualFeatureCount;

  // Shuffle <number> features at the start of the iterator
  feedback->pushInfo( QObject::tr( "Randomly select %1 features" ).arg( number ) );
  auto cursor = allFeats.begin();
  using difference_type = std::vector<QgsFeatureId>::difference_type;
  while ( shuffledFeatureCount-- )
  {
    if ( feedback->isCanceled() )
      return QVariantMap();

    // Update the distribution to match the number of unshuffled features
    fidsDistribution.param( std::uniform_int_distribution<size_t>::param_type( 0, nb - 1 ) );
    // Swap the current feature with a random one
    std::swap( *cursor, *( cursor + static_cast<difference_type>( fidsDistribution( mersenneTwister ) ) ) );
    // Move the cursor to the next feature
    ++cursor;

    // Decrement the number of unshuffled features
    --nb;
  }

  // Insert the selected features into a QgsFeatureIds set
  QgsFeatureIds selected;
  if ( invertSelection )
    for ( auto it = cursor; it != allFeats.end(); ++it )
      selected.insert( *it );
  else
    for ( auto it = allFeats.begin(); it != cursor; ++it )
      selected.insert( *it );

  feedback->pushInfo( QObject::tr( "Adding selected features" ) );
  fit = source->getFeatures( QgsFeatureRequest().setFilterFids( selected ), Qgis::ProcessingFeatureSourceFlag::SkipGeometryValidityChecks );
  while ( fit.nextFeature( f ) )
  {
    if ( feedback->isCanceled() )
      return QVariantMap();

    if ( !sink->addFeature( f, QgsFeatureSink::FastInsert ) )
      throw QgsProcessingException( writeFeatureError( sink.get(), parameters, QStringLiteral( "OUTPUT" ) ) );
  }

  sink->finalize();

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), dest );
  return outputs;
}

///@endcond
