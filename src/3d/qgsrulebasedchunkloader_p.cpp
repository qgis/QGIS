#include "qgsrulebasedchunkloader_p.h"

#include "qgs3dutils.h"
#include "qgschunknode_p.h"
#include "qgspolygon3dsymbol_p.h"
#include "qgseventtracing.h"

#include "qgsvectorlayer.h"
#include "qgsvectorlayerfeatureiterator.h"

#include "qgsrulebased3drenderer.h"

#include <QtConcurrent>


QgsRuleBasedChunkLoader::QgsRuleBasedChunkLoader( const QgsRuleBasedChunkLoaderFactory *factory, QgsChunkNode *node )
  : QgsChunkLoader( node )
  , mFactory( factory )
  , mContext( factory->mMap )
{
  if ( node->level() < mFactory->mLeafLevel )
  {
    QTimer::singleShot( 0, this, &QgsRuleBasedChunkLoader::finished );
    return;
  }

  QgsVectorLayer *layer = mFactory->mLayer;
  const Qgs3DMapSettings &map = mFactory->mMap;

  QgsExpressionContext exprContext( Qgs3DUtils::globalProjectLayerExpressionContext( layer ) );
  exprContext.setFields( layer->fields() );
  mContext.setExpressionContext( exprContext );

  mFactory->mRootRule->createHandlers( layer, mHandlers );

  QSet<QString> attributeNames;
  mFactory->mRootRule->prepare( mContext, attributeNames, mHandlers );

  // build the feature request
  QgsFeatureRequest req;
  req.setDestinationCrs( map.crs(), map.transformContext() );
  req.setSubsetOfAttributes( attributeNames, layer->fields() );

  // only a subset of data to be queried
  QgsRectangle rect = Qgs3DUtils::worldToMapExtent( node->bbox(), mFactory->mLayer->crs(), map.origin(), map.crs() );
  req.setFilterRect( rect );

  //
  // this will be run in a background thread
  //

  QFuture<void> future = QtConcurrent::run( [req, this]
  {
    QgsEventTracing::ScopedEvent e( "3D", "RB chunk load" );

    QgsFeature f;
    QgsFeatureIterator fi = mFactory->mSource->getFeatures( req );
    while ( fi.nextFeature( f ) )
    {
      if ( mCanceled )
        break;
      mContext.expressionContext().setFeature( f );
      mFactory->mRootRule->registerFeature( f, mContext, mHandlers );
    }
  } );

  // emit finished() as soon as the handler is populated with features
  QFutureWatcher<void> *fw = new QFutureWatcher<void>( nullptr );
  fw->setFuture( future );
  connect( fw, &QFutureWatcher<void>::finished, this, &QgsChunkQueueJob::finished );
}

void QgsRuleBasedChunkLoader::cancel()
{
  mCanceled = true;
}

Qt3DCore::QEntity *QgsRuleBasedChunkLoader::createEntity( Qt3DCore::QEntity *parent )
{
  if ( mNode->level() < mFactory->mLeafLevel )
  {
    return new Qt3DCore::QEntity( parent );  // dummy entity
  }

  Qt3DCore::QEntity *entity = new Qt3DCore::QEntity( parent );
  for ( QgsFeature3DHandler *handler : mHandlers.values() )
    handler->finalize( entity, mContext );

  qDeleteAll( mHandlers );
  mHandlers.clear();

  return entity;
}


///////////////


QgsRuleBasedChunkLoaderFactory::QgsRuleBasedChunkLoaderFactory( const Qgs3DMapSettings &map, QgsVectorLayer *vl, QgsRuleBased3DRenderer::Rule *rootRule, int leafLevel )
  : mMap( map )
  , mLayer( vl )
  , mRootRule( rootRule->clone() )
  , mLeafLevel( leafLevel )
  , mSource( new QgsVectorLayerFeatureSource( vl ) )
{
}

QgsChunkLoader *QgsRuleBasedChunkLoaderFactory::createChunkLoader( QgsChunkNode *node ) const
{
  return new QgsRuleBasedChunkLoader( this, node );
}


///////////////

QgsRuleBasedChunkedEntity::QgsRuleBasedChunkedEntity( QgsVectorLayer *vl, QgsRuleBased3DRenderer::Rule *rootRule, const Qgs3DMapSettings &map )
  : QgsChunkedEntity( Qgs3DUtils::mapToWorldExtent( vl->extent(), vl->crs(), map.origin(), map.crs() ),
                      -1,  // rootError  TODO: negative error should mean that the node does not contain anything
                      -1, // tau = max. allowed screen error. TODO: negative tau should mean that we need to go until leaves are reached
                      2, // TODO: figure out from the number of features
                      new QgsRuleBasedChunkLoaderFactory( map, vl, rootRule, 2 ) )
{
}

QgsRuleBasedChunkedEntity::~QgsRuleBasedChunkedEntity()
{
  // cancel / wait for jobs
  cancelActiveJobs();
}
