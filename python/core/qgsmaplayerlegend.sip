
/**
 * The QgsLegendRendererItem class is abstract interface for legend items
 * returned from QgsMapLayerLegend implementation.
 *
 * The objects are used in QgsLayerTreeModel. Custom implementations may offer additional interactivity
 * and customized look.
 *
 * @note added in 2.6
 */
class QgsLayerTreeModelLegendNode : QObject
{
%TypeHeaderCode
#include <qgsmaplayerlegend.h>
%End

  public:

    /** Return pointer to the parent layer node */
    QgsLayerTreeLayer* parent() const;

    /** Return item flags associated with the item. Default implementation returns Qt::ItemIsEnabled. */
    virtual Qt::ItemFlags flags() const;

    /** Return data associated with the item. Must be implemented in derived class. */
    virtual QVariant data( int role ) const = 0;

    /** Set some data associated with the item. Default implementation does nothing and returns false. */
    virtual bool setData( const QVariant& value, int role );

  protected:
    /** Construct the node with pointer to its parent layer node */
    explicit QgsLayerTreeModelLegendNode( QgsLayerTreeLayer* nodeL );
};


/**
 * Implementation of legend node interface for displaying preview of vector symbols and their labels
 * and allowing interaction with the symbol / renderer.
 *
 * @note added in 2.6
 */
class QgsSymbolV2LegendNode : QgsLayerTreeModelLegendNode
{
%TypeHeaderCode
#include <qgsmaplayerlegend.h>
%End
  public:
    QgsSymbolV2LegendNode( QgsLayerTreeLayer* nodeLayer, QgsSymbolV2* symbol, const QString& label, int rendererRef = -1 );

    virtual Qt::ItemFlags flags() const;
    virtual QVariant data( int role ) const;
    virtual bool setData( const QVariant& value, int role );
};


/**
 * Implementation of legend node interface for displaying arbitrary label with icon.
 *
 * @note added in 2.6
 */
class QgsSimpleLegendNode : QgsLayerTreeModelLegendNode
{
%TypeHeaderCode
#include <qgsmaplayerlegend.h>
%End
  public:
    QgsSimpleLegendNode( QgsLayerTreeLayer* nodeLayer, const QString& label, const QIcon& icon = QIcon() );

    virtual QVariant data( int role ) const;
};


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

    // TODO: support for legend renderer


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

