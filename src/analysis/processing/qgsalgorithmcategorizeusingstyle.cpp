/***************************************************************************
                         qgsalgorithmcategorizeusingstyle.cpp
                         ---------------------
    begin                : August 2018
    copyright            : (C) 2018 by Nyall Dawson
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

#include "qgsalgorithmcategorizeusingstyle.h"
#include "qgsstyle.h"
#include "qgscategorizedsymbolrenderer.h"
#include "qgsvectorlayer.h"
#include "qgsexpressioncontextutils.h"

///@cond PRIVATE

QgsCategorizeUsingStyleAlgorithm::QgsCategorizeUsingStyleAlgorithm() = default;

QgsCategorizeUsingStyleAlgorithm::~QgsCategorizeUsingStyleAlgorithm() = default;

void QgsCategorizeUsingStyleAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterVectorLayer( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ),
                QList< int >() << QgsProcessing::TypeVector ) );
  addParameter( new QgsProcessingParameterExpression( QStringLiteral( "FIELD" ), QObject::tr( "Categorize using expression" ), QVariant(), QStringLiteral( "INPUT" ) ) );

  addParameter( new QgsProcessingParameterFile( QStringLiteral( "STYLE" ), QObject::tr( "Style database (leave blank to use saved symbols)" ), QgsProcessingParameterFile::File, QStringLiteral( "xml" ), QVariant(), true ) );
  addParameter( new QgsProcessingParameterBoolean( QStringLiteral( "CASE_SENSITIVE" ), QObject::tr( "Use case-sensitive match to symbol names" ), false ) );
  addParameter( new QgsProcessingParameterBoolean( QStringLiteral( "TOLERANT" ), QObject::tr( "Ignore non-alphanumeric characters while matching" ), false ) );

  addOutput( new QgsProcessingOutputVectorLayer( QStringLiteral( "OUTPUT" ), QObject::tr( "Categorized layer" ) ) );

  std::unique_ptr< QgsProcessingParameterFeatureSink > failCategories = std::make_unique< QgsProcessingParameterFeatureSink >( QStringLiteral( "NON_MATCHING_CATEGORIES" ),  QObject::tr( "Non-matching categories" ),
      QgsProcessing::TypeVector, QVariant(), true, false );
  // not supported for outputs yet!
  //failCategories->setFlags( failCategories->flags() | QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( failCategories.release() );

  std::unique_ptr< QgsProcessingParameterFeatureSink > failSymbols = std::make_unique< QgsProcessingParameterFeatureSink >( QStringLiteral( "NON_MATCHING_SYMBOLS" ),  QObject::tr( "Non-matching symbol names" ),
      QgsProcessing::TypeVector, QVariant(), true, false );
  //failSymbols->setFlags( failSymbols->flags() | QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( failSymbols.release() );
}

QgsProcessingAlgorithm::Flags QgsCategorizeUsingStyleAlgorithm::flags() const
{
  Flags f = QgsProcessingAlgorithm::flags();
  f |= FlagNotAvailableInStandaloneTool;
  return f;
}

QString QgsCategorizeUsingStyleAlgorithm::name() const
{
  return QStringLiteral( "categorizeusingstyle" );
}

QString QgsCategorizeUsingStyleAlgorithm::displayName() const
{
  return QObject::tr( "Create categorized renderer from styles" );
}

QStringList QgsCategorizeUsingStyleAlgorithm::tags() const
{
  return QObject::tr( "file,database,symbols,names,category,categories" ).split( ',' );
}

QString QgsCategorizeUsingStyleAlgorithm::group() const
{
  return QObject::tr( "Cartography" );
}

QString QgsCategorizeUsingStyleAlgorithm::groupId() const
{
  return QStringLiteral( "cartography" );
}

QString QgsCategorizeUsingStyleAlgorithm::shortHelpString() const
{
  return QObject::tr( "Sets a vector layer's renderer to a categorized renderer using matching symbols from a style database. If no "
                      "style file is specified, symbols from the user's current style library are used instead.\n\n"
                      "The specified expression (or field name) is used to create categories for the renderer. A category will be "
                      "created for each unique value within the layer.\n\n"
                      "Each category is individually matched to the symbols which exist within the specified QGIS XML style database. Whenever "
                      "a matching symbol name is found, the category's symbol will be set to this matched symbol.\n\n"
                      "The matching is case-insensitive by default, but can be made case-sensitive if required.\n\n"
                      "Optionally, non-alphanumeric characters in both the category value and symbol name can be ignored "
                      "while performing the match. This allows for greater tolerance when matching categories to symbols.\n\n"
                      "If desired, tables can also be output containing lists of the categories which could not be matched "
                      "to symbols, and symbols which were not matched to categories."
                    );
}

QString QgsCategorizeUsingStyleAlgorithm::shortDescription() const
{
  return QObject::tr( "Sets a vector layer's renderer to a categorized renderer using symbols from a style database." );
}

QgsCategorizeUsingStyleAlgorithm *QgsCategorizeUsingStyleAlgorithm::createInstance() const
{
  return new QgsCategorizeUsingStyleAlgorithm();
}

class SetCategorizedRendererPostProcessor : public QgsProcessingLayerPostProcessorInterface
{
  public:

    SetCategorizedRendererPostProcessor( std::unique_ptr< QgsCategorizedSymbolRenderer > renderer )
      : mRenderer( std::move( renderer ) )
    {}

    void postProcessLayer( QgsMapLayer *layer, QgsProcessingContext &, QgsProcessingFeedback * ) override
    {
      if ( QgsVectorLayer *vl = qobject_cast< QgsVectorLayer * >( layer ) )
      {

        vl->setRenderer( mRenderer.release() );
        vl->triggerRepaint();
      }
    }

  private:

    std::unique_ptr<QgsCategorizedSymbolRenderer> mRenderer;
};

// Do most of the heavy lifting in a background thread, but save the thread-sensitive stuff for main thread execution!

bool QgsCategorizeUsingStyleAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QgsVectorLayer *layer = parameterAsVectorLayer( parameters, QStringLiteral( "INPUT" ), context );
  if ( !layer )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "INPUT" ) ) );

  mField = parameterAsString( parameters, QStringLiteral( "FIELD" ), context );

  mLayerId = layer->id();
  mLayerName = layer->name();
  mLayerGeometryType = layer->geometryType();
  mLayerFields = layer->fields();

  mExpressionContext << QgsExpressionContextUtils::globalScope()
                     << QgsExpressionContextUtils::projectScope( context.project() )
                     << QgsExpressionContextUtils::layerScope( layer );

  mExpression = QgsExpression( mField );
  mExpression.prepare( &mExpressionContext );

  QgsFeatureRequest req;
  req.setSubsetOfAttributes( mExpression.referencedColumns(), mLayerFields );
  if ( !mExpression.needsGeometry() )
    req.setFlags( QgsFeatureRequest::NoGeometry );

  mIterator = layer->getFeatures( req );

  return true;
}

QVariantMap QgsCategorizeUsingStyleAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  const QString styleFile = parameterAsFile( parameters, QStringLiteral( "STYLE" ), context );
  const bool caseSensitive = parameterAsBoolean( parameters, QStringLiteral( "CASE_SENSITIVE" ), context );
  const bool tolerant = parameterAsBoolean( parameters, QStringLiteral( "TOLERANT" ), context );

  QgsStyle *style = nullptr;
  std::unique_ptr< QgsStyle >importedStyle;
  if ( !styleFile.isEmpty() )
  {
    importedStyle = std::make_unique< QgsStyle >();
    if ( !importedStyle->importXml( styleFile ) )
    {
      throw QgsProcessingException( QObject::tr( "An error occurred while reading style file: %1" ).arg( importedStyle->errorString() ) );
    }
    style = importedStyle.get();
  }
  else
  {
    style = QgsStyle::defaultStyle();
  }

  QgsFields nonMatchingCategoryFields;
  nonMatchingCategoryFields.append( QgsField( QStringLiteral( "category" ), QVariant::String ) );
  QString nonMatchingCategoriesDest;
  std::unique_ptr< QgsFeatureSink > nonMatchingCategoriesSink( parameterAsSink( parameters, QStringLiteral( "NON_MATCHING_CATEGORIES" ), context, nonMatchingCategoriesDest, nonMatchingCategoryFields, QgsWkbTypes::NoGeometry ) );
  if ( !nonMatchingCategoriesSink && parameters.contains( QStringLiteral( "NON_MATCHING_CATEGORIES" ) ) && parameters.value( QStringLiteral( "NON_MATCHING_CATEGORIES" ) ).isValid() )
    throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "NON_MATCHING_CATEGORIES" ) ) );

  QgsFields nonMatchingSymbolFields;
  nonMatchingSymbolFields.append( QgsField( QStringLiteral( "name" ), QVariant::String ) );
  QString nonMatchingSymbolsDest;
  std::unique_ptr< QgsFeatureSink > nonMatchingSymbolsSink( parameterAsSink( parameters, QStringLiteral( "NON_MATCHING_SYMBOLS" ), context, nonMatchingSymbolsDest, nonMatchingSymbolFields, QgsWkbTypes::NoGeometry ) );
  if ( !nonMatchingSymbolsSink && parameters.contains( QStringLiteral( "NON_MATCHING_SYMBOLS" ) ) && parameters.value( QStringLiteral( "NON_MATCHING_SYMBOLS" ) ).isValid() )
    throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "NON_MATCHING_SYMBOLS" ) ) );

  QSet<QVariant> uniqueVals;
  QgsFeature feature;
  while ( mIterator.nextFeature( feature ) )
  {
    mExpressionContext.setFeature( feature );
    QVariant value = mExpression.evaluate( &mExpressionContext );
    if ( uniqueVals.contains( value ) )
      continue;
    uniqueVals << value;
  }

  QVariantList sortedUniqueVals = qgis::setToList( uniqueVals );
  std::sort( sortedUniqueVals.begin(), sortedUniqueVals.end() );

  QgsCategoryList cats;
  cats.reserve( uniqueVals.count() );
  std::unique_ptr< QgsSymbol > defaultSymbol( QgsSymbol::defaultSymbol( mLayerGeometryType ) );
  for ( const QVariant &val : std::as_const( sortedUniqueVals ) )
  {
    cats.append( QgsRendererCategory( val, defaultSymbol->clone(), val.toString() ) );
  }

  mRenderer = std::make_unique< QgsCategorizedSymbolRenderer >( mField, cats );

  const Qgis::SymbolType type = mLayerGeometryType == QgsWkbTypes::PointGeometry ? Qgis::SymbolType::Marker
                                : mLayerGeometryType == QgsWkbTypes::LineGeometry ? Qgis::SymbolType::Line
                                : Qgis::SymbolType::Fill;

  QVariantList unmatchedCategories;
  QStringList unmatchedSymbols;
  const int matched = mRenderer->matchToSymbols( style, type, unmatchedCategories, unmatchedSymbols, caseSensitive, tolerant );

  if ( matched > 0 )
  {
    feedback->pushInfo( QObject::tr( "Matched %n categories to symbols from file.", nullptr, matched ) );
  }
  else
  {
    feedback->reportError( QObject::tr( "No categories could be matched to symbols in file." ) );
  }

  if ( !unmatchedCategories.empty() )
  {
    feedback->pushInfo( QObject::tr( "\n%n categorie(s) could not be matched:", nullptr, unmatchedCategories.count() ) );
    std::sort( unmatchedCategories.begin(), unmatchedCategories.end() );
    for ( const QVariant &cat : std::as_const( unmatchedCategories ) )
    {
      feedback->pushInfo( QStringLiteral( "∙ “%1”" ).arg( cat.toString() ) );
      if ( nonMatchingCategoriesSink )
      {
        QgsFeature f;
        f.setAttributes( QgsAttributes() << cat.toString() );
        if ( !nonMatchingCategoriesSink->addFeature( f, QgsFeatureSink::FastInsert ) )
          throw QgsProcessingException( writeFeatureError( nonMatchingCategoriesSink.get(), parameters, QStringLiteral( "NON_MATCHING_CATEGORIES" ) ) );
      }
    }
  }

  if ( !unmatchedSymbols.empty() )
  {
    feedback->pushInfo( QObject::tr( "\n%n symbol(s) in style were not matched:", nullptr, unmatchedSymbols.count() ) );
    std::sort( unmatchedSymbols.begin(), unmatchedSymbols.end() );
    for ( const QString &name : std::as_const( unmatchedSymbols ) )
    {
      feedback->pushInfo( QStringLiteral( "∙ “%1”" ).arg( name ) );
      if ( nonMatchingSymbolsSink )
      {
        QgsFeature f;
        f.setAttributes( QgsAttributes() << name );
        if ( !nonMatchingSymbolsSink->addFeature( f, QgsFeatureSink::FastInsert ) )
          throw QgsProcessingException( writeFeatureError( nonMatchingSymbolsSink.get(), parameters, QStringLiteral( "NON_MATCHING_SYMBOLS" ) ) );
      }
    }
  }

  context.addLayerToLoadOnCompletion( mLayerId, QgsProcessingContext::LayerDetails( mLayerName, context.project(), mLayerName ) );
  context.layerToLoadOnCompletionDetails( mLayerId ).setPostProcessor( new SetCategorizedRendererPostProcessor( std::move( mRenderer ) ) );

  QVariantMap results;
  results.insert( QStringLiteral( "OUTPUT" ), mLayerId );
  if ( nonMatchingCategoriesSink )
    results.insert( QStringLiteral( "NON_MATCHING_CATEGORIES" ), nonMatchingCategoriesDest );
  if ( nonMatchingSymbolsSink )
    results.insert( QStringLiteral( "NON_MATCHING_SYMBOLS" ), nonMatchingSymbolsDest );
  return results;
}

///@endcond



