/***************************************************************************
    qgsvectorlayerfeaturecounter.cpp
    ---------------------
    begin                : May 2017
    copyright            : (C) 2017 by Matthias Kuhn
    email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvectorlayerfeaturecounter.h"
#include "qgsvectorlayer.h"
#include "qgsfeatureid.h"
#include "qgsfeedback.h"
#include "qgsrendercontext.h"

QgsVectorLayerFeatureCounter::QgsVectorLayerFeatureCounter( QgsVectorLayer *layer, const QgsExpressionContext &context, bool storeSymbolFids )
  : QgsTask( tr( "Counting features in %1" ).arg( layer->name() ), QgsTask::CanCancel | QgsTask::CancelWithoutPrompt )
  , mSource( new QgsVectorLayerFeatureSource( layer ) )
  , mRenderer( layer->renderer()->clone() )
  , mExpressionContext( context )
  , mWithFids( storeSymbolFids )
  , mFeatureCount( layer->featureCount() )
{
  if ( !mExpressionContext.scopeCount() )
  {
    mExpressionContext = layer->createExpressionContext();
  }
}

QgsVectorLayerFeatureCounter::~QgsVectorLayerFeatureCounter() = default;

bool QgsVectorLayerFeatureCounter::run()
{
  mSymbolFeatureCountMap.clear();
  mSymbolFeatureIdMap.clear();
  const QgsLegendSymbolList symbolList = mRenderer->legendSymbolItems();
  QgsLegendSymbolList::const_iterator symbolIt = symbolList.constBegin();

  for ( ; symbolIt != symbolList.constEnd(); ++symbolIt )
  {
    mSymbolFeatureCountMap.insert( symbolIt->ruleKey(), 0 );
    if ( mWithFids )
      mSymbolFeatureIdMap.insert( symbolIt->ruleKey(), QgsFeatureIds() );
  }

  // If there are no features to be counted, we can spare us the trouble
  if ( mFeatureCount > 0 )
  {
    mFeedback = std::make_unique< QgsFeedback >();

    int featuresCounted = 0;

    // Renderer (rule based) may depend on context scale, with scale is ignored if 0
    QgsRenderContext renderContext;
    renderContext.setRendererScale( 0 );
    renderContext.setExpressionContext( mExpressionContext );

    QgsFeatureRequest request;
    if ( !mRenderer->filterNeedsGeometry() )
      request.setFlags( QgsFeatureRequest::NoGeometry );
    request.setSubsetOfAttributes( mRenderer->usedAttributes( renderContext ), mSource->fields() );

    request.setFeedback( mFeedback.get() );
    mExpressionContext.setFeedback( mFeedback.get() );

    QgsFeatureIterator fit = mSource->getFeatures( request );

    mRenderer->startRender( renderContext, mSource->fields() );

    double progress = 0;
    QgsFeature f;
    while ( fit.nextFeature( f ) )
    {
      renderContext.expressionContext().setFeature( f );

      const QSet<QString> featureKeyList = mRenderer->legendKeysForFeature( f, renderContext );
      for ( const QString &key : featureKeyList )
      {
        mSymbolFeatureCountMap[key] += 1;
        if ( mWithFids )
          mSymbolFeatureIdMap[key].insert( f.id() );
      }
      ++featuresCounted;

      const double p = ( static_cast< double >( featuresCounted ) / mFeatureCount ) * 100;
      if ( p - progress > 1 )
      {
        progress = p;
        setProgress( progress );
      }

      if ( isCanceled() )
      {
        mRenderer->stopRender( renderContext );
        mExpressionContext.setFeedback( nullptr );
        mFeedback.reset();
        return false;
      }
    }
    mRenderer->stopRender( renderContext );
    mExpressionContext.setFeedback( nullptr );
    mFeedback.reset();
  }
  setProgress( 100 );
  emit symbolsCounted();
  return true;
}

void QgsVectorLayerFeatureCounter::cancel()
{
  if ( mFeedback )
    mFeedback->cancel();
  QgsTask::cancel();
}

QHash<QString, long long> QgsVectorLayerFeatureCounter::symbolFeatureCountMap() const
{
  return mSymbolFeatureCountMap;
}

long long QgsVectorLayerFeatureCounter::featureCount( const QString &legendKey ) const
{
  return mSymbolFeatureCountMap.value( legendKey, -1 );
}

QHash<QString, QgsFeatureIds> QgsVectorLayerFeatureCounter::symbolFeatureIdMap() const
{
  return mSymbolFeatureIdMap;
}

QgsFeatureIds QgsVectorLayerFeatureCounter::featureIds( const QString &symbolkey ) const
{
  return mSymbolFeatureIdMap.value( symbolkey, QgsFeatureIds() );
}
