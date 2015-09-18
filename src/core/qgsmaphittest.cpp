#include "qgsmaphittest.h"

#include "qgsmaplayerregistry.h"
#include "qgsrendercontext.h"
#include "qgsrendererv2.h"
#include "qgspointdisplacementrenderer.h"
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

  Q_FOREACH ( const QString& layerID, mSettings.layers() )
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
      context.setCoordinateTransform( mSettings.layerTransform( vl ) );
      context.setExtent( mSettings.outputExtentToLayerExtent( vl, mSettings.visibleExtent() ) );
    }

    context.expressionContext() << QgsExpressionContextUtils::layerScope( vl );
    SymbolV2Set& usedSymbols = mHitTest[vl];
    runHitTestLayer( vl, usedSymbols, context );
  }

  painter.end();
}


void QgsMapHitTest::runHitTestLayer( QgsVectorLayer* vl, SymbolV2Set& usedSymbols, QgsRenderContext& context )
{
  QgsFeatureRendererV2* r = vl->rendererV2();
  bool moreSymbolsPerFeature = r->capabilities() & QgsFeatureRendererV2::MoreSymbolsPerFeature;
  r->startRender( context, vl->fields() );
  QgsFeature f;
  QgsFeatureRequest request( context.extent() );
  request.setFlags( QgsFeatureRequest::ExactIntersect );
  QgsFeatureIterator fi = vl->getFeatures( request );
  while ( fi.nextFeature( f ) )
  {
    context.expressionContext().setFeature( f );
    if ( moreSymbolsPerFeature )
    {
      Q_FOREACH ( QgsSymbolV2* s, r->originalSymbolsForFeature( f, context ) )
        usedSymbols.insert( s );
    }
    else
      usedSymbols.insert( r->originalSymbolForFeature( f, context ) );
  }
  r->stopRender( context );
}
