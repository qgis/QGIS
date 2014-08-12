
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
#include <qgslayertreemodellegendnode.h>
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
/*
class QgsSymbolV2LegendNode : QgsLayerTreeModelLegendNode
{
%TypeHeaderCode
#include <qgslayertreemodellegendnode.h>
%End
  public:
    QgsSymbolV2LegendNode( QgsLayerTreeLayer* nodeLayer, const QgsLegendSymbolItemV2& item );

    virtual Qt::ItemFlags flags() const;
    virtual QVariant data( int role ) const;
    virtual bool setData( const QVariant& value, int role );
};*/


/**
 * Implementation of legend node interface for displaying arbitrary label with icon.
 *
 * @note added in 2.6
 */
class QgsSimpleLegendNode : QgsLayerTreeModelLegendNode
{
%TypeHeaderCode
#include <qgslayertreemodellegendnode.h>
%End
  public:
    QgsSimpleLegendNode( QgsLayerTreeLayer* nodeLayer, const QString& label, const QIcon& icon = QIcon() );

    virtual QVariant data( int role ) const;
};

