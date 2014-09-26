#include "qgsmaphittest.h"

#include "qgsmaplayerregistry.h"
#include "qgsrendercontext.h"
#include "qgsrendererv2.h"
#include "qgsvectorlayer.h"


QgsMapHitTest::QgsMapHitTest( const QgsMapSettings& settings )
    : mSettings( settings )
{
}


void QgsMapHitTest::run()
{
  // TODO: do we need this temp image?
  QImage tmpImage( mSettings.outputSize(), mSettings.outputImageFormat() );
  tmpImage.setDotsPerMeterX( mSettings.outputDpi() * 25.4 );
  tmpImage.setDotsPerMeterY( mSettings.outputDpi() * 25.4 );
  QPainter painter( &tmpImage );

  QgsRenderContext context = QgsRenderContext::fromMapSettings( mSettings );
  context.setPainter( &painter ); // we are not going to draw anything, but we still need a working painter

  foreach ( QString layerID, mSettings.layers() )
  {
    QgsVectorLayer* vl = qobject_cast<QgsVectorLayer*>( QgsMapLayerRegistry::instance()->mapLayer( layerID ) );
    if ( !vl || !vl->rendererV2() )
      continue;

    if ( vl->hasScaleBasedVisibility() && ( mSettings.scale() < vl->minimumScale() || mSettings.scale() > vl->maximumScale() ) )
    {
      mHitTest[vl] = SymbolV2Set(); // no symbols -> will not be shown
      continue;
    }

    if ( mSettings.hasCrsTransformEnabled() )
    {
      context.setCoordinateTransform( mSettings.layerTransfrom( vl ) );
      context.setExtent( mSettings.outputExtentToLayerExtent( vl, mSettings.visibleExtent() ) );
    }

    SymbolV2Set& usedSymbols = mHitTest[vl];
    runHitTestLayer( vl, usedSymbols, context );
  }

  painter.end();
}


void QgsMapHitTest::runHitTestLayer( QgsVectorLayer* vl, SymbolV2Set& usedSymbols, QgsRenderContext& context )
{
  QgsFeatureRendererV2* r = vl->rendererV2();
  bool moreSymbolsPerFeature = r->capabilities() & QgsFeatureRendererV2::MoreSymbolsPerFeature;
  r->startRender( context, vl->pendingFields() );
  QgsFeature f;
  QgsFeatureRequest request( context.extent() );
  request.setFlags( QgsFeatureRequest::ExactIntersect );
  QgsFeatureIterator fi = vl->getFeatures( request );
  while ( fi.nextFeature( f ) )
  {
    if ( moreSymbolsPerFeature )
    {
      foreach ( QgsSymbolV2* s, r->originalSymbolsForFeature( f ) )
        usedSymbols.insert( s );
    }
    else
      usedSymbols.insert( r->originalSymbolForFeature( f ) );
  }
  r->stopRender( context );
}
