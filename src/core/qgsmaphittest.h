#ifndef QGSMAPHITTEST_H
#define QGSMAPHITTEST_H

#include "qgsmapsettings.h"

#include <QSet>

class QgsRenderContext;
class QgsSymbolV2;
class QgsVectorLayer;

/**
 * Class that runs a hit test with given map settings. Based on the hit test it returns which symbols
 * will be visible on the map - this is useful for content based legend.
 *
 * @note added in 2.6
 */
class CORE_EXPORT QgsMapHitTest
{
  public:
    QgsMapHitTest( const QgsMapSettings& settings );

    void run();

    QSet<QgsSymbolV2*> symbolsForLayer( QgsVectorLayer* layer ) const { return mHitTest[layer]; }

  protected:

    typedef QSet<QgsSymbolV2*> SymbolV2Set;
    typedef QMap<QgsVectorLayer*, SymbolV2Set> HitTest;

    void runHitTestLayer( QgsVectorLayer* vl, SymbolV2Set& usedSymbols, QgsRenderContext& context );

    QgsMapSettings mSettings;
    HitTest mHitTest;

};

#endif // QGSMAPHITTEST_H
