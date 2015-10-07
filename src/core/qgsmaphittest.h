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

    /** Tests whether a symbol is visible for a specified layer.
     * @param symbol symbol to find
     * @param layer vector layer
     * @note added in QGIS 2.12
     */
    bool symbolVisible( QgsSymbolV2* symbol, QgsVectorLayer* layer ) const;

  protected:

    typedef QSet<QString> SymbolV2Set;
    typedef QMap<QgsVectorLayer*, SymbolV2Set> HitTest;

    /** Runs test for visible symbols within a layer
     * @param vl vector layer
     * @param usedSymbols set for storage of visible symbols
     * @param context render context
     * @note added in QGIS 2.12
     */
    void runHitTestLayer( QgsVectorLayer* vl, SymbolV2Set& usedSymbols, QgsRenderContext& context );

    QgsMapSettings mSettings;
    HitTest mHitTest;

};

#endif // QGSMAPHITTEST_H
