/***************************************************************************
                        qgsalllayersfeatureslocatorfilters.cpp
                        ----------------------------
   begin                : May 2017
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

#include "qgsalllayersfeatureslocatorfilter.h"
#include "qgssettings.h"
#include "qgsproject.h"
#include "qgsvectorlayer.h"
#include "qgsexpressioncontextutils.h"
#include "qgsfeatureaction.h"
#include "qgsfeedback.h"
#include "qgsiconutils.h"
#include "qgisapp.h"
#include "qgsmapcanvas.h"

#include <QSpinBox>

QgsAllLayersFeaturesLocatorFilter::QgsAllLayersFeaturesLocatorFilter( QObject *parent )
  : QgsLocatorFilter( parent )
{
  setUseWithoutPrefix( false );
}

QgsAllLayersFeaturesLocatorFilter *QgsAllLayersFeaturesLocatorFilter::clone() const
{
  return new QgsAllLayersFeaturesLocatorFilter();
}

QStringList QgsAllLayersFeaturesLocatorFilter::prepare( const QString &string, const QgsLocatorContext &context )
{
  // Normally skip very short search strings, unless when specifically searching using this filter
  if ( string.length() < 3 && !context.usingPrefix )
    return QStringList();

  QgsSettings settings;
  mMaxTotalResults = settings.value( "locator_filters/all_layers_features/limit_global", 15, QgsSettings::App ).toInt();
  mMaxResultsPerLayer = settings.value( "locator_filters/all_layers_features/limit_per_layer", 8, QgsSettings::App ).toInt();

  mPreparedLayers.clear();
  const QMap<QString, QgsMapLayer *> layers = QgsProject::instance()->mapLayers();
  for ( auto it = layers.constBegin(); it != layers.constEnd(); ++it )
  {
    QgsVectorLayer *layer = qobject_cast< QgsVectorLayer *>( it.value() );
    if ( !layer || !layer->dataProvider() || !layer->flags().testFlag( QgsMapLayer::Searchable ) )
      continue;

    QgsExpression expression( layer->displayExpression() );
    QgsExpressionContext context;
    context.appendScopes( QgsExpressionContextUtils::globalProjectLayerScopes( layer ) );
    expression.prepare( &context );

    QgsFeatureRequest req;
    req.setSubsetOfAttributes( qgis::setToList( expression.referencedAttributeIndexes( layer->fields() ) ) );
    if ( !expression.needsGeometry() )
      req.setFlags( Qgis::FeatureRequestFlag::NoGeometry );
    QString enhancedSearch = string;
    enhancedSearch.replace( ' ', '%' );
    req.setFilterExpression( QStringLiteral( "%1 ILIKE '%%2%'" )
                             .arg( layer->displayExpression(), enhancedSearch ) );
    req.setLimit( mMaxResultsPerLayer );

    QgsFeatureRequest exactMatchRequest = req;
    exactMatchRequest.setFilterExpression( QStringLiteral( "%1 ILIKE '%2'" )
                                           .arg( layer->displayExpression(), enhancedSearch ) );
    exactMatchRequest.setLimit( mMaxResultsPerLayer );

    std::shared_ptr<PreparedLayer> preparedLayer( new PreparedLayer() );
    preparedLayer->expression = expression;
    preparedLayer->context = context;
    preparedLayer->layerId = layer->id();
    preparedLayer->layerName = layer->name();
    preparedLayer->featureSource.reset( new QgsVectorLayerFeatureSource( layer ) );
    preparedLayer->request = req;
    preparedLayer->exactMatchRequest = exactMatchRequest;
    preparedLayer->layerIcon = QgsIconUtils::iconForLayer( layer );
    preparedLayer->layerIsSpatial = layer->isSpatial();

    mPreparedLayers.append( preparedLayer );
  }

  return QStringList();
}

void QgsAllLayersFeaturesLocatorFilter::fetchResults( const QString &string, const QgsLocatorContext &, QgsFeedback *feedback )
{
  int foundInCurrentLayer;
  int foundInTotal = 0;
  QgsFeature f;

  // we cannot used const loop since iterator::nextFeature is not const
  for ( auto preparedLayer : std::as_const( mPreparedLayers ) )
  {
    foundInCurrentLayer = 0;

    QgsFeatureIds foundFeatureIds;

    QgsFeatureIterator exactMatchIt = preparedLayer->featureSource->getFeatures( preparedLayer->exactMatchRequest );
    while ( exactMatchIt.nextFeature( f ) )
    {
      if ( feedback->isCanceled() )
        return;

      QgsLocatorResult result;
      result.group = preparedLayer->layerName;

      preparedLayer->context.setFeature( f );

      result.displayString = preparedLayer->expression.evaluate( &( preparedLayer->context ) ).toString();

      result.setUserData( ResultData( f.id(), preparedLayer->layerId, preparedLayer->layerIsSpatial ).toVariant() );
      foundFeatureIds << f.id();
      result.icon = preparedLayer->layerIcon;
      result.score = static_cast< double >( string.length() ) / result.displayString.size();

      result.actions << QgsLocatorResult::ResultAction( OpenForm, tr( "Open form…" ) );
      emit resultFetched( result );

      foundInCurrentLayer++;
      foundInTotal++;
      if ( foundInCurrentLayer >= mMaxResultsPerLayer )
        break;
    }
    if ( foundInCurrentLayer >= mMaxResultsPerLayer )
      continue;
    if ( foundInTotal >= mMaxTotalResults )
      break;

    QgsFeatureIterator it = preparedLayer->featureSource->getFeatures( preparedLayer->request );
    while ( it.nextFeature( f ) )
    {
      if ( feedback->isCanceled() )
        return;

      if ( foundFeatureIds.contains( f.id() ) )
        continue;

      QgsLocatorResult result;
      result.group = preparedLayer->layerName;

      preparedLayer->context.setFeature( f );

      result.displayString = preparedLayer->expression.evaluate( &( preparedLayer->context ) ).toString();

      result.setUserData( ResultData( f.id(), preparedLayer->layerId, preparedLayer->layerIsSpatial ).toVariant() );
      result.icon = preparedLayer->layerIcon;
      result.score = static_cast< double >( string.length() ) / result.displayString.size();

      if ( preparedLayer->layerIsSpatial )
        result.actions << QgsLocatorResult::ResultAction( OpenForm, tr( "Open form…" ) );
      emit resultFetched( result );

      foundInCurrentLayer++;
      foundInTotal++;
      if ( foundInCurrentLayer >= mMaxResultsPerLayer )
        break;
    }
    if ( foundInTotal >= mMaxTotalResults )
      break;
  }
}

void QgsAllLayersFeaturesLocatorFilter::triggerResult( const QgsLocatorResult &result )
{
  triggerResultFromAction( result, NoEntry );
}

void QgsAllLayersFeaturesLocatorFilter::triggerResultFromAction( const QgsLocatorResult &result, const int actionId )
{
  ResultData data = ResultData::fromVariant( result.userData() );
  QgsFeatureId fid = data.id();
  QString layerId = data.layerId();
  bool layerIsSpatial = data.layerIsSpatial();
  QgsVectorLayer *layer = QgsProject::instance()->mapLayer<QgsVectorLayer *>( layerId );
  if ( !layer )
    return;

  if ( actionId == OpenForm || !layerIsSpatial )
  {
    QgsFeature f;
    QgsFeatureRequest request;
    request.setFilterFid( fid );
    bool fetched = layer->getFeatures( request ).nextFeature( f );
    if ( !fetched )
      return;
    QgsFeatureAction action( tr( "Attributes changed" ), f, layer, QUuid(), -1, QgisApp::instance() );
    if ( layer->isEditable() )
    {
      action.editFeature( false );
    }
    else
    {
      action.viewFeatureForm();
    }
  }
  else
  {
    QgisApp::instance()->mapCanvas()->zoomToFeatureIds( layer, QgsFeatureIds() << fid );
    QgisApp::instance()->mapCanvas()->flashFeatureIds( layer, QgsFeatureIds() << fid );
  }
}

void QgsAllLayersFeaturesLocatorFilter::openConfigWidget( QWidget *parent )
{
  QString key = "locator_filters/all_layers_features";
  QgsSettings settings;
  std::unique_ptr<QDialog> dlg( new QDialog( parent ) );
  dlg->restoreGeometry( settings.value( QStringLiteral( "Windows/%1/geometry" ).arg( key ) ).toByteArray() );
  dlg->setWindowTitle( "All layers features locator filter" );
  QFormLayout *formLayout = new QFormLayout;
  QSpinBox *globalLimitSpinBox = new QSpinBox( dlg.get() );
  globalLimitSpinBox->setValue( settings.value( QStringLiteral( "%1/limit_global" ).arg( key ), 15, QgsSettings::App ).toInt() );
  globalLimitSpinBox->setMinimum( 1 );
  globalLimitSpinBox->setMaximum( 200 );
  formLayout->addRow( tr( "&Maximum number of results:" ), globalLimitSpinBox );
  QSpinBox *perLayerLimitSpinBox = new QSpinBox( dlg.get() );
  perLayerLimitSpinBox->setValue( settings.value( QStringLiteral( "%1/limit_per_layer" ).arg( key ), 8, QgsSettings::App ).toInt() );
  perLayerLimitSpinBox->setMinimum( 1 );
  perLayerLimitSpinBox->setMaximum( 200 );
  formLayout->addRow( tr( "&Maximum number of results per layer:" ), perLayerLimitSpinBox );
  QDialogButtonBox *buttonbBox = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel, dlg.get() );
  formLayout->addRow( buttonbBox );
  dlg->setLayout( formLayout );
  connect( buttonbBox, &QDialogButtonBox::accepted, dlg.get(), [&]()
  {
    settings.setValue( QStringLiteral( "%1/limit_global" ).arg( key ), globalLimitSpinBox->value(), QgsSettings::App );
    settings.setValue( QStringLiteral( "%1/limit_per_layer" ).arg( key ), perLayerLimitSpinBox->value(), QgsSettings::App );
    dlg->accept();
  } );
  connect( buttonbBox, &QDialogButtonBox::rejected, dlg.get(), &QDialog::reject );
  dlg->exec();
}
