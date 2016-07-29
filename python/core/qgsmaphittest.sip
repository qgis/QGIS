/**
 * Class that runs a hit test with given map settings. Based on the hit test it returns which symbols
 * will be visible on the map - this is useful for content based legend.
 *
 * @note added in 2.6
 */
class QgsMapHitTest
{
%TypeHeaderCode
#include <qgsmaphittest.h>
%End

  public:
    //! Maps an expression string to a layer id
    typedef QMap<QString, QString> LayerFilterExpression;

    //! @param settings Map settings used to evaluate symbols
    //! @param polygon Polygon geometry to refine the hit test
    //! @param layerFilterExpression Expression string for each layer id to evaluate in order to refine the symbol selection
    QgsMapHitTest( const QgsMapSettings& settings, const QgsGeometry& polygon = QgsGeometry(), const QMap<QString, QString>& layerFilterExpression = QMap<QString, QString>() );

    //! Constructor version used with only expressions to filter symbols (no extent or polygon intersection)
    QgsMapHitTest( const QgsMapSettings& settings, const QMap<QString, QString>& layerFilterExpression );

    //! Runs the map hit test
    void run();

    /** Tests whether a symbol is visible for a specified layer.
     * @param symbol symbol to find
     * @param layer vector layer
     * @note added in QGIS 2.12
     * @see legendKeyVisible()
     */
    bool symbolVisible( QgsSymbolV2* symbol, QgsVectorLayer* layer ) const;

    /** Tests whether a given legend key is visible for a specified layer.
     * @param ruleKey legend rule key
     * @param layer vector layer
     * @note added in QGIS 2.14
     * @see symbolVisible()
     */
    bool legendKeyVisible( const QString& ruleKey, QgsVectorLayer* layer ) const;

  protected:

    //! @note not available in Python bindings
    //typedef QSet<QString> SymbolV2Set;

    //! @note not available in Python bindings
    //typedef QMap<QgsVectorLayer*, SymbolV2Set> HitTest;

    /** Runs test for visible symbols within a layer
     * @param vl vector layer
     * @param usedSymbols set for storage of visible symbols
     * @param usedSymbolsRuleKey set of storage of visible legend rule keys
     * @param context render context
     * @note added in QGIS 2.12
     * @note not available in Python bindings
     */
    //void runHitTestLayer( QgsVectorLayer* vl, SymbolV2Set& usedSymbols, SymbolV2Set& usedSymbolsRuleKey, QgsRenderContext& context );


};
