
/**
 * The QgsMapLayerLegend class is abstract interface for implementations
 * of legends for one map layer.
 *
 * @note added in 2.6
 */
class QgsMapLayerLegend : QObject
{
%TypeHeaderCode
#include <qgsmaplayerlegend.h>
%End

  public:
    explicit QgsMapLayerLegend( QObject *parent /TransferThis/ = 0 );

    // TODO: type, load/save settings

    /**
     * Return list of legend nodes to be used for a particular layer tree layer node.
     * Ownership is transferred to the caller.
     */
    virtual QList<QgsLayerTreeModelLegendNode*> createLayerTreeModelLegendNodes( QgsLayerTreeLayer* nodeLayer ) = 0 /Factory/;

    // TODO: support for layer tree view delegates

    //! Create new legend implementation for vector layer
    static QgsMapLayerLegend* defaultVectorLegend( QgsVectorLayer* vl ) /Factory/;

    //! Create new legend implementation for raster layer
    static QgsMapLayerLegend* defaultRasterLegend( QgsRasterLayer* rl ) /Factory/;

    //! Create new legend implementation for raster layer
    static QgsMapLayerLegend* defaultPluginLegend( QgsPluginLayer* pl ) /Factory/;

  signals:
    //! Emitted when existing items/nodes got invalid and should be replaced by new ones
    void itemsChanged();
};


/** Default legend implementation for vector layers
 * @note added in 2.6
 */
class QgsDefaultVectorLayerLegend : QgsMapLayerLegend
{
%TypeHeaderCode
#include <qgsmaplayerlegend.h>
%End
  public:
    explicit QgsDefaultVectorLayerLegend( QgsVectorLayer* vl );

    virtual QList<QgsLayerTreeModelLegendNode*> createLayerTreeModelLegendNodes( QgsLayerTreeLayer* nodeLayer ) /Factory/;

};


/** Default legend implementation for raster layers
 * @note added in 2.6
 */
class QgsDefaultRasterLayerLegend : QgsMapLayerLegend
{
%TypeHeaderCode
#include <qgsmaplayerlegend.h>
%End
  public:
    explicit QgsDefaultRasterLayerLegend( QgsRasterLayer* rl );

    virtual QList<QgsLayerTreeModelLegendNode*> createLayerTreeModelLegendNodes( QgsLayerTreeLayer* nodeLayer ) /Factory/;

};


/** Default legend implementation for plugin layers
 * @note added in 2.6
 */
class QgsDefaultPluginLayerLegend : QgsMapLayerLegend
{
%TypeHeaderCode
#include <qgsmaplayerlegend.h>
%End
  public:
    explicit QgsDefaultPluginLayerLegend( QgsPluginLayer* pl );

    virtual QList<QgsLayerTreeModelLegendNode*> createLayerTreeModelLegendNodes( QgsLayerTreeLayer* nodeLayer ) /Factory/;

};

