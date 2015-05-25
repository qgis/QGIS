
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
    ~QgsLayerTreeModelLegendNode();

    enum LegendNodeRoles
    {
      RuleKeyRole,                    //!< rule key of the node (QString)
      SymbolV2LegacyRuleKeyRole,      //!< for QgsSymbolV2LegendNode only - legacy rule key (void ptr, to be cast to QgsSymbolV2 ptr)
      ParentRuleKeyRole               //!< rule key of the parent legend node - for legends with tree hierarchy (QString). Added in 2.8
    };

    /** Return pointer to the parent layer node */
    QgsLayerTreeLayer* layerNode() const;

    /** Return pointer to model owning this legend node */
    QgsLayerTreeModel* model() const;

    /** Return item flags associated with the item. Default implementation returns Qt::ItemIsEnabled. */
    virtual Qt::ItemFlags flags() const;

    /** Return data associated with the item. Must be implemented in derived class. */
    virtual QVariant data( int role ) const = 0;

    /** Set some data associated with the item. Default implementation does nothing and returns false. */
    virtual bool setData( const QVariant& value, int role );

    virtual bool isEmbeddedInParent() const;
    virtual void setEmbeddedInParent( bool embedded );

    virtual QString userLabel() const;
    virtual void setUserLabel( const QString& userLabel );

    virtual bool isScaleOK( double scale ) const;

    /** Notification from model that information from associated map view has changed.
     *  Default implementation does nothing. */
    virtual void invalidateMapBasedData();

    struct ItemContext
    {
      //! Painter
      QPainter* painter;
      //! Top-left corner of the legend item
      QPointF point;
      //! offset from the left side where label should start
      double labelXOffset;
    };

    struct ItemMetrics
    {
      QSizeF symbolSize;
      QSizeF labelSize;
    };

    /** Entry point called from QgsLegendRenderer to do the rendering.
     *  Default implementation calls drawSymbol() and drawSymbolText() methods.
     *
     *  If ctx is null, this is just first stage when preparing layout - without actual rendering.
     */
    virtual ItemMetrics draw( const QgsLegendSettings& settings, ItemContext* ctx );

    /**
     * Draws symbol on the left side of the item
     * @param settings Legend layout configuration
     * @param ctx Context for rendering - may be null if only doing layout without actual rendering
     * @param itemHeight Minimal height of the legend item - used for correct positioning when rendering
     * @return Real size of the symbol (may be bigger than "normal" symbol size from settings)
     */
    virtual QSizeF drawSymbol( const QgsLegendSettings& settings, ItemContext* ctx, double itemHeight ) const;

    /**
     * Draws label on the right side of the item
     * @param settings Legend layout configuration
     * @param ctx Context for rendering - may be null if only doing layout without actual rendering
     * @param symbolSize  Real size of the associated symbol - used for correct positioning when rendering
     * @return Size of the label (may span multiple lines)
     */
    virtual QSizeF drawSymbolText( const QgsLegendSettings& settings, ItemContext* ctx, const QSizeF& symbolSize ) const;

  signals:
    //! Emitted on internal data change so the layer tree model can forward the signal to views
    void dataChanged();

  protected:
    /** Construct the node with pointer to its parent layer node */
    explicit QgsLayerTreeModelLegendNode( QgsLayerTreeLayer* nodeL, QObject* parent /TransferThis/ = 0 );

  private:
    QgsLayerTreeModelLegendNode(const QgsLayerTreeModelLegendNode &);
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
#include <qgslayertreemodellegendnode.h>
%End
  public:
    QgsSymbolV2LegendNode( QgsLayerTreeLayer* nodeLayer, const QgsLegendSymbolItemV2& item, QObject* parent /TransferThis/ = 0 );
    ~QgsSymbolV2LegendNode();

    virtual Qt::ItemFlags flags() const;
    virtual QVariant data( int role ) const;
    virtual bool setData( const QVariant& value, int role );

    QSizeF drawSymbol( const QgsLegendSettings& settings, ItemContext* ctx, double itemHeight ) const;

    virtual void setEmbeddedInParent( bool embedded );

    void setUserLabel( const QString& userLabel );

    virtual bool isScaleOK( double scale ) const;

    virtual void invalidateMapBasedData();

    //! Set the icon size
    //! @note added in 2.10
    void setIconSize( const QSize& sz );
    //! @note added in 2.10
    QSize iconSize() const;

    //! Get the minimum icon size to prevent cropping
    //! @note added in 2.10
    QSize minimumIconSize() const;
};


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
    QgsSimpleLegendNode( QgsLayerTreeLayer* nodeLayer, const QString& label, const QIcon& icon = QIcon(), QObject* parent /TransferThis/ = 0, const QString& key = QString() );

    virtual QVariant data( int role ) const;
};


/**
 * Implementation of legend node interface for displaying arbitrary raster image
 *
 * @note added in 2.6
 */
class QgsImageLegendNode : QgsLayerTreeModelLegendNode
{
%TypeHeaderCode
#include <qgslayertreemodellegendnode.h>
%End
  public:
    QgsImageLegendNode( QgsLayerTreeLayer* nodeLayer, const QImage& img, QObject* parent /TransferThis/ = 0 );

    virtual QVariant data( int role ) const;

    QSizeF drawSymbol( const QgsLegendSettings& settings, ItemContext* ctx, double itemHeight ) const;

};

/**
 * Implementation of legend node interface for displaying raster legend entries
 *
 * @note added in 2.6
 */
class QgsRasterSymbolLegendNode : QgsLayerTreeModelLegendNode
{
%TypeHeaderCode
#include <qgslayertreemodellegendnode.h>
%End
  public:
    QgsRasterSymbolLegendNode( QgsLayerTreeLayer* nodeLayer, const QColor& color, const QString& label, QObject* parent /TransferThis/ = 0 );

    virtual QVariant data( int role ) const;

    QSizeF drawSymbol( const QgsLegendSettings& settings, ItemContext* ctx, double itemHeight ) const;
};
