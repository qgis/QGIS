/***************************************************************************
    qgsrubberband_impl.cpp
     --------------------------------------
    Date                 : July 2026
    Copyright            : (C) 2026 by Nyall Dawson
    Email                : nyall.dawson@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrubberband_impl.h"

#include "qgsexpressioncontextutils.h"
#include "qgsmapcanvas.h"
#include "qgstextrenderer.h"
#include "qgsvectorlayer.h"

QgsVectorLayerLabelRubberBandPreview::QgsVectorLayerLabelRubberBandPreview( QgsRubberBand *rubberBand, const QList< QgsFeatureId > &fids, QgsVectorLayer *layer )
  : QgsRubberBandPreviewItem( rubberBand )
  , mLayer( layer )
{
  QgsFeature f;
  QgsFeatureIterator it = mLayer->getFeatures( qgis::listToSet( fids ) );
  mFeatures.resize( fids.size() );
  int index = 0;
  while ( it.nextFeature( f ) )
  {
    mFeatures[index] = f;
    index++;
  }
}

void QgsVectorLayerLabelRubberBandPreview::render( QgsRenderContext &context )
{
  QgsRubberBand *rubberBand = QgsRubberBandPreviewItem::rubberBand();
  if ( !rubberBand || !mLayer || !mLayer->labelsEnabled() || !mLayer->labeling() || mFeatures.empty() )
    return;

  QgsMapCanvas *canvas = rubberBand->canvas();
  if ( !canvas )
    return;

  // fetch current rubber band geometry
  QgsGeometry previewGeom = rubberBand->asGeometry();
  if ( previewGeom.isEmpty() )
    return;

  previewGeom.transform( canvas->mapSettings().layerTransform( mLayer ), Qgis::TransformDirection::Reverse );

  QgsExpressionContextScope *featureScope = new QgsExpressionContextScope();

  QgsExpressionContextScopePopper scopePopper( context.expressionContext(), featureScope );
  context.setFlag( Qgis::RenderContextFlag::RecordProfile, false );

  // set up a super minimal, fast labeling engine. Why? Well...
  // we can't just use QgsTextRenderer here, as that won't fully reflect the actual appearance
  // of the labels actually rendered for the features. Eg it won't respect settings like text repeat distances,
  // anchor points, overrun, curved placement modes, etc.
  // So to get a useful preview we need to use the actual labeling engine to render the label.
  // But we also don't need a lot of the weight of pal labeling, eg we only need to render the
  // single least-cost candidate for each feature, we won't consider obstacles or overlaps, we don't
  // need the label search tree, etc.
  // Disabling all this gives us a very lean label engine, which is quite cheap to use and gives
  // perfect 1:1 previews of the actual label to be rendered.
  QgsMapSettings mapSettings = canvas->mapSettings();
  mapSettings.setFlag( Qgis::MapSettingsFlag::RecordProfile, false );
  QgsLabelingEngineSettings labelSettings = mapSettings.labelingEngineSettings();
  labelSettings.setFlag( Qgis::LabelingFlag::SingleCandidateOnly, true );
  labelSettings.setFlag( Qgis::LabelingFlag::IgnoreObstacles, true );
  labelSettings.setFlag( Qgis::LabelingFlag::DisableSearchTree, true );
  labelSettings.setFlag( Qgis::LabelingFlag::IgnoreOverlaps, true );
  mapSettings.setLabelingEngineSettings( labelSettings );
  QgsDefaultLabelingEngine engine( mapSettings );

  QgsPalLayerSettings settings = mLayer->labeling()->settings();
  auto provider = new QgsVectorLayerLabelProvider( mLayer, QString(), false, &settings );
  engine.addProvider( provider );

  QSet<QString> attributeNames;
  if ( !provider->prepare( context, attributeNames ) )
  {
    return;
  }

  const QVector< QgsGeometry> previewGeomParts = previewGeom.asGeometryCollection();
  int index = 0;
  for ( const QgsFeature &feature : std::as_const( mFeatures ) )
  {
    if ( index >= previewGeomParts.size() )
      break;

    // each individual part of the rubber band's geometry corresponds to an individual feature,
    // so we re-associate them here:
    QgsFeature tempFeature = feature;
    tempFeature.setGeometry( previewGeomParts.at( index ) );

    featureScope->setFeature( tempFeature );
    featureScope->setFields( tempFeature.fields() );
    provider->registerFeature( tempFeature, context );
    index++;
  }

  QgsScopedQPainterState painterState( context.painter() );
  context.painter()->translate( -rubberBand->pos() );

  // running the engine calculates the placement and does the actual rendering:
  engine.run( context );
}
