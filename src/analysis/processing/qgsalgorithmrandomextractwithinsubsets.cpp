/***************************************************************************
                         qgsalgorithmrandomextractwithinsubsets.h
                         ---------------------
    begin                : January 2026
    copyright            : (C) 2026 by Alexander Bruy
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

#include "qgsalgorithmrandomextractwithinsubsets.h"

#include <random>

#include "qgsvectorlayer.h"

///@cond PRIVATE

void QgsRandomExtractWithinSubsetsAlgorithmBase::sampleFeatureIds( QgsFeatureSource *source, const double value, const QString &fieldName, QgsProcessingFeedback *feedback )
{
  const int fieldIndex = source->fields().lookupField( fieldName );
  if ( fieldIndex < 0 )
  {
    throw QgsProcessingException( QObject::tr( "Missing field '%1' in input layer" ).arg( fieldName ) );
  }

  QHash<QVariant, QgsFeatureIds> idsHash;

  // Group IDs by attribute
  QgsFeatureRequest request;
  request.setFlags( Qgis::FeatureRequestFlag::NoGeometry );
  request.setSubsetOfAttributes( { fieldIndex } );
  QgsFeatureIterator it = source->getFeatures( request );
  double step = source->featureCount() > 0 ? 50.0 / static_cast<double>( source->featureCount() ) : 1;
  long long i = 0;

  QgsFeature f;
  feedback->pushInfo( QObject::tr( "Building features subsets…" ) );
  while ( it.nextFeature( f ) )
  {
    if ( feedback->isCanceled() )
    {
      return;
    }
    idsHash[f.attribute( fieldIndex )].insert( f.id() );

    i++;
    feedback->setProgress( static_cast<double>( i ) * step );
  }
  feedback->pushInfo( QObject::tr( "Done." ) );

  // initialize random engine
  std::random_device randomDevice;
  std::mt19937 mersenneTwister( randomDevice() );
  std::uniform_int_distribution<std::size_t> fidsDistribution;

  feedback->pushInfo( QObject::tr( "Randomly selecting features within subsets…" ) );
  i = 0;
  step = !idsHash.isEmpty() ? 50.0 / static_cast<double>( idsHash.size() ) : 1;

  for ( auto hashIt = idsHash.constBegin(); hashIt != idsHash.constEnd(); ++hashIt )
  {
    if ( feedback->isCanceled() )
    {
      break;
    }

    const QgsFeatureIds &subsetIds = hashIt.value();
    const long long total = subsetIds.size();
    const long long count = mMethod == 0 ? static_cast<long long>( value ) : static_cast<long long>( std::ceil( static_cast<double>( total ) * value / 100 ) );

    if ( count >= total )
    {
      feedback->reportError( QObject::tr( "Subset '%1' is smaller than requested number of features." ).arg( hashIt.key().toString() ) );
      mSelectedFeatureIds.unite( subsetIds );
    }
    else
    {
      std::vector<QgsFeatureId> allSubsetIds( subsetIds.begin(), subsetIds.end() );
      bool invertSelection = count > total / 2;
      long long shuffledFeatureCount = invertSelection ? total - count : count;
      std::size_t nb = allSubsetIds.size();

      using difference_type = std::vector<QgsFeatureId>::difference_type;
      auto cursor = allSubsetIds.begin();
      for ( long long j = 0; j < shuffledFeatureCount; ++j )
      {
        if ( feedback->isCanceled() )
        {
          return;
        }

        fidsDistribution.param( std::uniform_int_distribution<std::size_t>::param_type( 0, nb - 1 ) );
        std::swap( *cursor, *( cursor + static_cast<difference_type>( fidsDistribution( mersenneTwister ) ) ) );
        ++cursor;
        --nb;
      }

      if ( invertSelection )
      {
        for ( auto selectIt = cursor; selectIt != allSubsetIds.end(); ++selectIt )
        {
          mSelectedFeatureIds.insert( *selectIt );
        }
      }
      else
      {
        for ( auto selectIt = allSubsetIds.begin(); selectIt != cursor; ++selectIt )
        {
          mSelectedFeatureIds.insert( *selectIt );
        }
      }
    }

    i++;
    feedback->setProgress( 50.0 + ( static_cast<double>( i ) * step ) );
  }
  feedback->pushInfo( QObject::tr( "Done." ) );
}

QString QgsRandomExtractWithinSubsetsAlgorithmBase::group() const
{
  return QObject::tr( "Vector selection" );
}

QString QgsRandomExtractWithinSubsetsAlgorithmBase::groupId() const
{
  return u"vectorselection"_s;
}

// Random extract within subsets algorithm

QString QgsRandomExtractWithinSubsetsAlgorithm::name() const
{
  return u"randomextractwithinsubsets"_s;
}

QString QgsRandomExtractWithinSubsetsAlgorithm::displayName() const
{
  return QObject::tr( "Random extract within subsets" );
}

QStringList QgsRandomExtractWithinSubsetsAlgorithm::tags() const
{
  return QObject::tr( "extract,filter,random,number,percentage,subset" ).split( ',' );
}

QString QgsRandomExtractWithinSubsetsAlgorithm::shortDescription() const
{
  return QObject::tr( "Generates a new vector layer that contains only a subset of the features in the input layer." );
}

QString QgsRandomExtractWithinSubsetsAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm takes a vector layer and generates a new one that "
                      "contains only a subset of the features in the input layer.\n\n"
                      "The subset is defined randomly, using a percentage or count value "
                      "to define the total number of features in the subset.\n\n"
                      "The percentage/count value is not applied to the whole layer, but "
                      "instead to each category. Categories are defined according to a "
                      "given attribute, which is also specified as an input parameter "
                      "for the algorithm." );
}

Qgis::ProcessingAlgorithmDocumentationFlags QgsRandomExtractWithinSubsetsAlgorithm::documentationFlags() const
{
  return Qgis::ProcessingAlgorithmDocumentationFlag::RegeneratesPrimaryKey;
}

QgsRandomExtractWithinSubsetsAlgorithm *QgsRandomExtractWithinSubsetsAlgorithm::createInstance() const
{
  return new QgsRandomExtractWithinSubsetsAlgorithm();
}

void QgsRandomExtractWithinSubsetsAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( u"INPUT"_s, QObject::tr( "Input layer" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::Vector ) ) );
  addParameter( new QgsProcessingParameterField( u"FIELD"_s, QObject::tr( "ID field" ), QVariant(), u"INPUT"_s ) );
  addParameter( new QgsProcessingParameterEnum( u"METHOD"_s, QObject::tr( "Method" ), QStringList() << QObject::tr( "Number of features" ) << QObject::tr( "Percentage of features" ), false, 0 ) );
  addParameter( new QgsProcessingParameterNumber( u"NUMBER"_s, QObject::tr( "Number/percentage of features" ), Qgis::ProcessingNumberParameterType::Double, 10, false, 0 ) );

  addParameter( new QgsProcessingParameterFeatureSink( u"OUTPUT"_s, QObject::tr( "Extracted (random stratified)" ) ) );
}

QVariantMap QgsRandomExtractWithinSubsetsAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr<QgsProcessingFeatureSource> source( parameterAsSource( parameters, u"INPUT"_s, context ) );
  if ( !source )
    throw QgsProcessingException( invalidSourceError( parameters, u"INPUT"_s ) );

  const QString fieldName = parameterAsString( parameters, u"FIELD"_s, context );
  mMethod = parameterAsEnum( parameters, u"METHOD"_s, context );
  const double number = parameterAsDouble( parameters, u"NUMBER"_s, context );

  QString dest;
  std::unique_ptr<QgsFeatureSink> sink( parameterAsSink( parameters, u"OUTPUT"_s, context, dest, source->fields(), source->wkbType(), source->sourceCrs(), QgsFeatureSink::RegeneratePrimaryKey ) );
  if ( !sink )
    throw QgsProcessingException( invalidSinkError( parameters, u"OUTPUT"_s ) );

  const long long count = source->featureCount() > 0 ? source->featureCount() : 0;

  if ( mMethod == 0 )
  {
    // number of features
    if ( number > static_cast<double>( count ) )
      throw QgsProcessingException( QObject::tr( "Selected number is greater than feature count. Choose a lower value and try again." ) );
  }
  else
  {
    // percentage of features
    if ( number > 100 )
      throw QgsProcessingException( QObject::tr( "Percentage can't be greater than 100. Choose a lower value and try again." ) );
  }

  sampleFeatureIds( source.get(), number, fieldName, feedback );

  feedback->pushInfo( QObject::tr( "Adding selected features" ) );
  QgsFeature f;
  QgsFeatureIterator fit = source->getFeatures( QgsFeatureRequest().setFilterFids( mSelectedFeatureIds ), Qgis::ProcessingFeatureSourceFlag::SkipGeometryValidityChecks );
  double step = mSelectedFeatureIds.size() > 0 ? 100.0 / static_cast<double>( mSelectedFeatureIds.size() ) : 1;
  long long i = 0;
  while ( fit.nextFeature( f ) )
  {
    if ( feedback->isCanceled() )
      return QVariantMap();

    i++;
    feedback->setProgress( static_cast<double>( i ) * step );

    if ( !sink->addFeature( f, QgsFeatureSink::FastInsert ) )
      throw QgsProcessingException( writeFeatureError( sink.get(), parameters, u"OUTPUT"_s ) );
  }

  sink->finalize();

  QVariantMap outputs;
  outputs.insert( u"OUTPUT"_s, dest );
  return outputs;
}

// Random selection algorithm

QString QgsRandomSelectionWithinSubsetsAlgorithm::name() const
{
  return u"randomselectionwithinsubsets"_s;
}

QString QgsRandomSelectionWithinSubsetsAlgorithm::displayName() const
{
  return QObject::tr( "Random selection within subsets" );
}

QStringList QgsRandomSelectionWithinSubsetsAlgorithm::tags() const
{
  return QObject::tr( "select,random,number,percentage,subset" ).split( ',' );
}

QString QgsRandomSelectionWithinSubsetsAlgorithm::shortDescription() const
{
  return QObject::tr( "Randomly selects features from a subset of a vector layer." );
}

QString QgsRandomSelectionWithinSubsetsAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm takes a vector layer and selects a subset of its features. "
                      "No new layer is generated by this algorithm.\n\n"
                      "The subset is defined randomly, using a percentage or count value to define "
                      "the total number of features in the subset.\n\n"
                      "The percentage/count value is not applied to the whole layer, but instead to each category. "
                      "Categories are defined according to a given attribute, which is also specified "
                      "as an input parameter for the algorithm." );
}

QgsRandomSelectionWithinSubsetsAlgorithm *QgsRandomSelectionWithinSubsetsAlgorithm::createInstance() const
{
  return new QgsRandomSelectionWithinSubsetsAlgorithm();
}

void QgsRandomSelectionWithinSubsetsAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterVectorLayer( u"INPUT"_s, QObject::tr( "Input layer" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::Vector ) ) );
  addParameter( new QgsProcessingParameterField( u"FIELD"_s, QObject::tr( "ID field" ), QVariant(), u"INPUT"_s ) );
  addParameter( new QgsProcessingParameterEnum( u"METHOD"_s, QObject::tr( "Method" ), QStringList() << QObject::tr( "Number of features" ) << QObject::tr( "Percentage of features" ), false, 0 ) );
  addParameter( new QgsProcessingParameterNumber( u"NUMBER"_s, QObject::tr( "Number/percentage of features" ), Qgis::ProcessingNumberParameterType::Double, 10, false, 0 ) );

  addOutput( new QgsProcessingOutputVectorLayer( u"OUTPUT"_s, QObject::tr( "Selected (random)" ) ) );
}

QVariantMap QgsRandomSelectionWithinSubsetsAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  mInput = parameters.value( u"INPUT"_s );
  mTargetLayer = parameterAsVectorLayer( parameters, u"INPUT"_s, context );

  if ( !mTargetLayer )
    throw QgsProcessingException( QObject::tr( "Could not load source layer for INPUT." ) );

  const QString fieldName = parameterAsString( parameters, u"FIELD"_s, context );
  mMethod = parameterAsEnum( parameters, u"METHOD"_s, context );
  const double number = parameterAsDouble( parameters, u"NUMBER"_s, context );
  const long long count = mTargetLayer->featureCount() > 0 ? mTargetLayer->featureCount() : 0;

  if ( mMethod == 0 )
  {
    // number of features
    if ( number > static_cast<double>( count ) )
      throw QgsProcessingException( QObject::tr( "Selected number is greater than feature count. Choose a lower value and try again." ) );
  }
  else
  {
    // percentage of features
    if ( number > 100 )
      throw QgsProcessingException( QObject::tr( "Percentage can't be greater than 100. Choose a lower value and try again." ) );
  }

  sampleFeatureIds( mTargetLayer, number, fieldName, feedback );

  return QVariantMap();
}

QVariantMap QgsRandomSelectionWithinSubsetsAlgorithm::postProcessAlgorithm( QgsProcessingContext &, QgsProcessingFeedback * )
{
  mTargetLayer->selectByIds( mSelectedFeatureIds );

  QVariantMap outputs;
  outputs.insert( u"OUTPUT"_s, mInput );
  return outputs;
}

///@endcond
