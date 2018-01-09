/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layertree/qgslayertreemodellegendnode.h                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/







class QgsLayerTreeModelLegendNode : QObject
{
%Docstring
The QgsLegendRendererItem class is abstract interface for legend items
returned from QgsMapLayerLegend implementation.

The objects are used in QgsLayerTreeModel. Custom implementations may offer additional interactivity
and customized look.

.. versionadded:: 2.6
%End

%TypeHeaderCode
#include "qgslayertreemodellegendnode.h"
%End
  public:

    enum LegendNodeRoles
    {
      RuleKeyRole,
      ParentRuleKeyRole
    };

    QgsLayerTreeLayer *layerNode() const;
%Docstring
Return pointer to the parent layer node
%End

    QgsLayerTreeModel *model() const;
%Docstring
Return pointer to model owning this legend node
%End

    virtual Qt::ItemFlags flags() const;
%Docstring
Return item flags associated with the item. Default implementation returns Qt.ItemIsEnabled.
%End

    virtual QVariant data( int role ) const = 0;
%Docstring
Return data associated with the item. Must be implemented in derived class.
%End

    virtual bool setData( const QVariant &value, int role );
%Docstring
Set some data associated with the item. Default implementation does nothing and returns false.
%End

    virtual bool isEmbeddedInParent() const;
    virtual void setEmbeddedInParent( bool embedded );

    virtual QString userLabel() const;
    virtual void setUserLabel( const QString &userLabel );

    virtual bool isScaleOK( double scale ) const;

    virtual void invalidateMapBasedData();
%Docstring
Notification from model that information from associated map view has changed.
Default implementation does nothing. *
%End

    struct ItemContext
    {
      QPainter *painter;
      QPointF point;
      double labelXOffset;
    };

    struct ItemMetrics
    {
      QSizeF symbolSize;
      QSizeF labelSize;
    };

    virtual ItemMetrics draw( const QgsLegendSettings &settings, ItemContext *ctx );
%Docstring
Entry point called from QgsLegendRenderer to do the rendering.
Default implementation calls drawSymbol() and drawSymbolText() methods.

If ctx is null, this is just first stage when preparing layout - without actual rendering.
%End

    virtual QSizeF drawSymbol( const QgsLegendSettings &settings, ItemContext *ctx, double itemHeight ) const;
%Docstring
Draws symbol on the left side of the item

:param settings: Legend layout configuration
:param ctx: Context for rendering - may be null if only doing layout without actual rendering
:param itemHeight: Minimal height of the legend item - used for correct positioning when rendering

:return: Real size of the symbol (may be bigger than "normal" symbol size from settings)
%End

    virtual QSizeF drawSymbolText( const QgsLegendSettings &settings, ItemContext *ctx, QSizeF symbolSize ) const;
%Docstring
Draws label on the right side of the item

:param settings: Legend layout configuration
:param ctx: Context for rendering - may be null if only doing layout without actual rendering
:param symbolSize:  Real size of the associated symbol - used for correct positioning when rendering

:return: Size of the label (may span multiple lines)
%End

  signals:
    void dataChanged();
%Docstring
Emitted on internal data change so the layer tree model can forward the signal to views
%End

  protected:
    explicit QgsLayerTreeModelLegendNode( QgsLayerTreeLayer *nodeL, QObject *parent /TransferThis/ = 0 );
%Docstring
Construct the node with pointer to its parent layer node
%End

    QgsRenderContext *createTemporaryRenderContext() const /Factory/;
%Docstring
Returns a temporary context or null if legendMapViewData are not valid
%End

  protected:
};


class QgsSymbolLegendNode : QgsLayerTreeModelLegendNode
{
%Docstring
Implementation of legend node interface for displaying preview of vector symbols and their labels
and allowing interaction with the symbol / renderer.

.. versionadded:: 2.6
%End

%TypeHeaderCode
#include "qgslayertreemodellegendnode.h"
%End
  public:

    QgsSymbolLegendNode( QgsLayerTreeLayer *nodeLayer, const QgsLegendSymbolItem &item, QObject *parent /TransferThis/ = 0 );
%Docstring
Constructor for QgsSymbolLegendNode.

:param nodeLayer: layer node
:param item: the legend symbol item
:param parent: attach a parent QObject to the legend node.
%End

    virtual Qt::ItemFlags flags() const;

    virtual QVariant data( int role ) const;

    virtual bool setData( const QVariant &value, int role );


    virtual QSizeF drawSymbol( const QgsLegendSettings &settings, ItemContext *ctx, double itemHeight ) const;


    virtual void setEmbeddedInParent( bool embedded );


    virtual void setUserLabel( const QString &userLabel );

    virtual bool isScaleOK( double scale ) const;

    virtual void invalidateMapBasedData();


    void setIconSize( QSize sz );
%Docstring
Set the icon size

.. versionadded:: 2.10
%End
    QSize iconSize() const;
%Docstring

.. versionadded:: 2.10
%End

    QSize minimumIconSize() const;
%Docstring
Calculates the minimum icon size to prevent cropping. When evaluating
the size for multiple icons it is more efficient to create a single
render context in advance and use the variant which accepts a :py:class:`QgsRenderContext`
argument.

.. versionadded:: 2.10
%End

    QSize minimumIconSize( QgsRenderContext *context ) const;
%Docstring
Calculates the minimum icon size to prevent cropping. When evaluating
the size for multiple icons it is more efficient to create a single
render context in advance and call this method instead of minimumIconSize().

.. versionadded:: 2.18
%End

    const QgsSymbol *symbol() const;
%Docstring
Returns the symbol used by the legend node.

.. seealso:: :py:func:`setSymbol`

.. versionadded:: 2.14
%End

    void setSymbol( QgsSymbol *symbol );
%Docstring
Sets the symbol to be used by the legend node. The symbol change is also propagated
to the associated vector layer's renderer.

:param symbol: new symbol for node. Ownership is transferred.

.. seealso:: :py:func:`symbol`

.. versionadded:: 2.14
%End

  public slots:

    void checkAllItems();
%Docstring
Checks all items belonging to the same layer as this node.

.. versionadded:: 2.14

.. seealso:: :py:func:`uncheckAllItems`
%End

    void uncheckAllItems();
%Docstring
Unchecks all items belonging to the same layer as this node.

.. versionadded:: 2.14

.. seealso:: :py:func:`checkAllItems`
%End

};


class QgsSimpleLegendNode : QgsLayerTreeModelLegendNode
{
%Docstring
Implementation of legend node interface for displaying arbitrary label with icon.

.. versionadded:: 2.6
%End

%TypeHeaderCode
#include "qgslayertreemodellegendnode.h"
%End
  public:

    QgsSimpleLegendNode( QgsLayerTreeLayer *nodeLayer, const QString &label, const QIcon &icon = QIcon(), QObject *parent /TransferThis/ = 0, const QString &key = QString() );
%Docstring
Constructor for QgsSimpleLegendNode.

:param nodeLayer: layer node
:param label: label
:param icon: icon
:param parent: attach a parent QObject to the legend node.
:param key: the rule key
%End

    virtual QVariant data( int role ) const;


};


class QgsImageLegendNode : QgsLayerTreeModelLegendNode
{
%Docstring
Implementation of legend node interface for displaying arbitrary raster image

.. versionadded:: 2.6
%End

%TypeHeaderCode
#include "qgslayertreemodellegendnode.h"
%End
  public:

    QgsImageLegendNode( QgsLayerTreeLayer *nodeLayer, const QImage &img, QObject *parent /TransferThis/ = 0 );
%Docstring
Constructor for QgsImageLegendNode.

:param nodeLayer: layer node
:param img: the image
:param parent: attach a parent QObject to the legend node.
%End

    virtual QVariant data( int role ) const;


    virtual QSizeF drawSymbol( const QgsLegendSettings &settings, ItemContext *ctx, double itemHeight ) const;


};

class QgsRasterSymbolLegendNode : QgsLayerTreeModelLegendNode
{
%Docstring
Implementation of legend node interface for displaying raster legend entries

.. versionadded:: 2.6
%End

%TypeHeaderCode
#include "qgslayertreemodellegendnode.h"
%End
  public:

    QgsRasterSymbolLegendNode( QgsLayerTreeLayer *nodeLayer, const QColor &color, const QString &label, QObject *parent /TransferThis/ = 0 );
%Docstring
Constructor for QgsRasterSymbolLegendNode.

:param nodeLayer: layer node
:param color: color
:param label: label
:param parent: attach a parent QObject to the legend node.
%End

    virtual QVariant data( int role ) const;


    virtual QSizeF drawSymbol( const QgsLegendSettings &settings, ItemContext *ctx, double itemHeight ) const;


};


class QgsWmsLegendNode : QgsLayerTreeModelLegendNode
{
%Docstring
Implementation of legend node interface for displaying WMS legend entries

.. versionadded:: 2.8
%End

%TypeHeaderCode
#include "qgslayertreemodellegendnode.h"
%End
  public:

    QgsWmsLegendNode( QgsLayerTreeLayer *nodeLayer, QObject *parent /TransferThis/ = 0 );
%Docstring
Constructor for QgsWmsLegendNode.

:param nodeLayer: layer node
:param parent: attach a parent QObject to the legend node.
%End

    virtual QVariant data( int role ) const;


    virtual QSizeF drawSymbol( const QgsLegendSettings &settings, ItemContext *ctx, double itemHeight ) const;


    virtual void invalidateMapBasedData();


};


class QgsDataDefinedSizeLegendNode : QgsLayerTreeModelLegendNode
{
%Docstring
Produces legend node with a marker symbol

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslayertreemodellegendnode.h"
%End
  public:
    QgsDataDefinedSizeLegendNode( QgsLayerTreeLayer *nodeLayer, const QgsDataDefinedSizeLegend &settings, QObject *parent /TransferThis/ = 0 );
%Docstring
Construct the node using QgsDataDefinedSizeLegend as definition of the node's appearance
%End
    ~QgsDataDefinedSizeLegendNode();

    virtual QVariant data( int role ) const;


    virtual ItemMetrics draw( const QgsLegendSettings &settings, ItemContext *ctx );


};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layertree/qgslayertreemodellegendnode.h                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
