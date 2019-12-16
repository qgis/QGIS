#ifndef QGSRULEBASEDCHUNKLOADER_H
#define QGSRULEBASEDCHUNKLOADER_H

#include "qgschunkloader_p.h"
#include "qgsfeature3dhandler_p.h"

class Qgs3DMapSettings;
class QgsVectorLayer;
class QgsVectorLayerFeatureSource;
class QgsAbstract3DSymbol;
class QgsFeature3DHandler;

#include "qgsrulebased3drenderer.h"


class QgsRuleBasedChunkLoaderFactory : public QgsChunkLoaderFactory
{
  public:
    QgsRuleBasedChunkLoaderFactory( const Qgs3DMapSettings &map, QgsVectorLayer *vl, QgsRuleBased3DRenderer::Rule *rootRule, int leafLevel );

    //! Creates loader for the given chunk node. Ownership of the returned is passed to the caller.
    virtual QgsChunkLoader *createChunkLoader( QgsChunkNode *node ) const;

    const Qgs3DMapSettings &mMap;
    QgsVectorLayer *mLayer;
    std::unique_ptr<QgsRuleBased3DRenderer::Rule> mRootRule;
    int mLeafLevel;
    std::unique_ptr<QgsVectorLayerFeatureSource> mSource;
};




class QgsRuleBasedChunkLoader : public QgsChunkLoader
{
  public:
    QgsRuleBasedChunkLoader( const QgsRuleBasedChunkLoaderFactory *factory, QgsChunkNode *node );

    virtual void cancel();
    virtual Qt3DCore::QEntity *createEntity( Qt3DCore::QEntity *parent );

  private:
    const QgsRuleBasedChunkLoaderFactory *mFactory;
    QgsRuleBased3DRenderer::RuleToHandlerMap mHandlers;
    Qgs3DRenderContext mContext;
    bool mCanceled = false;
};



#include "qgschunkedentity_p.h"

class QgsRuleBasedChunkedEntity : public QgsChunkedEntity
{
    Q_OBJECT
  public:

    // TODO: should use a clone of root rule?

    //! Constructs the entity. The argument maxLevel determines how deep the tree of tiles will be
    explicit QgsRuleBasedChunkedEntity( QgsVectorLayer *vl, QgsRuleBased3DRenderer::Rule *rootRule, const Qgs3DMapSettings &map );

    ~QgsRuleBasedChunkedEntity();
};



#endif // QGSRULEBASEDCHUNKLOADER_H
