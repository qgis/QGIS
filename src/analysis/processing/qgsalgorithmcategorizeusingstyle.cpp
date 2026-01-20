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

#include "qgscategorizedsymbolrenderer.h"
#include "qgsexpressioncontextutils.h"
#include "qgsstyle.h"
#include "qgsvectorlayer.h"

///@cond PRIVATE

QgsCategorizeUsingStyleAlgorithm::QgsCategorizeUsingStyleAlgorithm() = default;

QgsCategorizeUsingStyleAlgorithm::~QgsCategorizeUsingStyleAlgorithm() = default;

void QgsCategorizeUsingStyleAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterVectorLayer( u"INPUT"_s, QObject::tr( "Input layer" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::Vector ) ) );
  addParameter( new QgsProcessingParameterExpression( u"FIELD"_s, QObject::tr( "Categorize using expression" ), QVariant(), u"INPUT"_s ) );

  addParameter( new QgsProcessingParameterFile( u"STYLE"_s, QObject::tr( "Style database (leave blank to use saved symbols)" ), Qgis::ProcessingFileParameterBehavior::File, u"xml"_s, QVariant(), true ) );
  addParameter( new QgsProcessingParameterBoolean( u"CASE_SENSITIVE"_s, QObject::tr( "Use case-sensitive match to symbol names" ), false ) );
  addParameter( new QgsProcessingParameterBoolean( u"TOLERANT"_s, QObject::tr( "Ignore non-alphanumeric characters while matching" ), false ) );

  addOutput( new QgsProcessingOutputVectorLayer( u"OUTPUT"_s, QObject::tr( "Categorized layer" ) ) );

  auto failCategories = std::make_unique<QgsProcessingParameterFeatureSink>( u"NON_MATCHING_CATEGORIES"_s, QObject::tr( "Non-matching categories" ), Qgis::ProcessingSourceType::Vector, QVariant(), true, false );
  // not supported for outputs yet!
  //failCategories->setFlags( failCategories->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( failCategories.release() );

  auto failSymbols = std::make_unique<QgsProcessingParameterFeatureSink>( u"NON_MATCHING_SYMBOLS"_s, QObject::tr( "Non-matching symbol names" ), Qgis::ProcessingSourceType::Vector, QVariant(), true, false );
  //failSymbols->setFlags( failSymbols->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( failSymbols.release() );
}

Qgis::ProcessingAlgorithmFlags QgsCategorizeUsingStyleAlgorithm::flags() const
{
  Qgis::ProcessingAlgorithmFlags f = QgsProcessingAlgorithm::flags();
  f |= Qgis::ProcessingAlgorithmFlag::NotAvailableInStandaloneTool;
  return f;
}

QString QgsCategorizeUsingStyleAlgorithm::name() const
{
  return u"categorizeusingstyle"_s;
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
  return u"cartography"_s;
}

QString QgsCategorizeUsingStyleAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm sets a vector layer's renderer to a categorized renderer using matching symbols from a style database. If no "
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
    SetCategorizedRendererPostProcessor( std::unique_ptr<QgsCategorizedSymbolRenderer> renderer )
      : mRenderer( std::move( renderer ) )
    {}

    void postProcessLayer( QgsMapLayer *layer, QgsProcessingContext &, QgsProcessingFeedback * ) override
    {
      if ( QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( layer ) )
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
  QgsVectorLayer *layer = parameterAsVectorLayer( parameters, u"INPUT"_s, context );
  if ( !layer )
    throw QgsProcessingException( invalidSourceError( parameters, u"INPUT"_s ) );

  mField = parameterAsString( parameters, u"FIELD"_s, context );

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
    req.setFlags( Qgis::FeatureRequestFlag::NoGeometry );

  mIterator = layer->getFeatures( req );

  return true;
}

QVariantMap QgsCategorizeUsingStyleAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  const QString styleFile = parameterAsFile( parameters, u"STYLE"_s, context );
  const bool caseSensitive = parameterAsBoolean( parameters, u"CASE_SENSITIVE"_s, context );
  const bool tolerant = parameterAsBoolean( parameters, u"TOLERANT"_s, context );

  QgsStyle *style = nullptr;
  std::unique_ptr<QgsStyle> importedStyle;
  if ( !styleFile.isEmpty() )
  {
    importedStyle = std::make_unique<QgsStyle>();
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
  nonMatchingCategoryFields.append( QgsField( u"category"_s, QMetaType::Type::QString ) );
  QString nonMatchingCategoriesDest;
  std::unique_ptr<QgsFeatureSink> nonMatchingCategoriesSink( parameterAsSink( parameters, u"NON_MATCHING_CATEGORIES"_s, context, nonMatchingCategoriesDest, nonMatchingCategoryFields, Qgis::WkbType::NoGeometry ) );
  if ( !nonMatchingCategoriesSink && parameters.contains( u"NON_MATCHING_CATEGORIES"_s ) && parameters.value( u"NON_MATCHING_CATEGORIES"_s ).isValid() )
    throw QgsProcessingException( invalidSinkError( parameters, u"NON_MATCHING_CATEGORIES"_s ) );

  QgsFields nonMatchingSymbolFields;
  nonMatchingSymbolFields.append( QgsField( u"name"_s, QMetaType::Type::QString ) );
  QString nonMatchingSymbolsDest;
  std::unique_ptr<QgsFeatureSink> nonMatchingSymbolsSink( parameterAsSink( parameters, u"NON_MATCHING_SYMBOLS"_s, context, nonMatchingSymbolsDest, nonMatchingSymbolFields, Qgis::WkbType::NoGeometry ) );
  if ( !nonMatchingSymbolsSink && parameters.contains( u"NON_MATCHING_SYMBOLS"_s ) && parameters.value( u"NON_MATCHING_SYMBOLS"_s ).isValid() )
    throw QgsProcessingException( invalidSinkError( parameters, u"NON_MATCHING_SYMBOLS"_s ) );

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
  std::unique_ptr<QgsSymbol> defaultSymbol( QgsSymbol::defaultSymbol( mLayerGeometryType ) );
  for ( const QVariant &val : std::as_const( sortedUniqueVals ) )
  {
    cats.append( QgsRendererCategory( val, defaultSymbol->clone(), val.toString() ) );
  }

  mRenderer = std::make_unique<QgsCategorizedSymbolRenderer>( mField, cats );

  const Qgis::SymbolType type = mLayerGeometryType == Qgis::GeometryType::Point  ? Qgis::SymbolType::Marker
                                : mLayerGeometryType == Qgis::GeometryType::Line ? Qgis::SymbolType::Line
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
    feedback->pushInfo( QObject::tr( "\n%n categories could not be matched:", nullptr, unmatchedCategories.count() ) );
    std::sort( unmatchedCategories.begin(), unmatchedCategories.end() );
    for ( const QVariant &cat : std::as_const( unmatchedCategories ) )
    {
      feedback->pushInfo( u"∙ “%1”"_s.arg( cat.toString() ) );
      if ( nonMatchingCategoriesSink )
      {
        QgsFeature f;
        f.setAttributes( QgsAttributes() << cat.toString() );
        if ( !nonMatchingCategoriesSink->addFeature( f, QgsFeatureSink::FastInsert ) )
          throw QgsProcessingException( writeFeatureError( nonMatchingCategoriesSink.get(), parameters, u"NON_MATCHING_CATEGORIES"_s ) );
      }
    }
  }

  if ( !unmatchedSymbols.empty() )
  {
    feedback->pushInfo( QObject::tr( "\n%n symbol(s) in style were not matched:", nullptr, unmatchedSymbols.count() ) );
    std::sort( unmatchedSymbols.begin(), unmatchedSymbols.end() );
    for ( const QString &name : std::as_const( unmatchedSymbols ) )
    {
      feedback->pushInfo( u"∙ “%1”"_s.arg( name ) );
      if ( nonMatchingSymbolsSink )
      {
        QgsFeature f;
        f.setAttributes( QgsAttributes() << name );
        if ( !nonMatchingSymbolsSink->addFeature( f, QgsFeatureSink::FastInsert ) )
          throw QgsProcessingException( writeFeatureError( nonMatchingSymbolsSink.get(), parameters, u"NON_MATCHING_SYMBOLS"_s ) );
      }
    }
  }

  context.addLayerToLoadOnCompletion( mLayerId, QgsProcessingContext::LayerDetails( mLayerName, context.project(), mLayerName ) );
  context.layerToLoadOnCompletionDetails( mLayerId ).setPostProcessor( new SetCategorizedRendererPostProcessor( std::move( mRenderer ) ) );

  if ( nonMatchingCategoriesSink )
    nonMatchingCategoriesSink->finalize();
  if ( nonMatchingSymbolsSink )
    nonMatchingSymbolsSink->finalize();

  QVariantMap results;
  results.insert( u"OUTPUT"_s, mLayerId );
  if ( nonMatchingCategoriesSink )
    results.insert( u"NON_MATCHING_CATEGORIES"_s, nonMatchingCategoriesDest );
  if ( nonMatchingSymbolsSink )
    results.insert( u"NON_MATCHING_SYMBOLS"_s, nonMatchingSymbolsDest );
  return results;
}

///@endcond
