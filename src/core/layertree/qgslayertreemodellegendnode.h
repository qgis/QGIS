/***************************************************************************
  qgslayertreemodellegendnode.h
  --------------------------------------
  Date                 : August 2014
  Copyright            : (C) 2014 by Martin Dobias
  Email                : wonder dot sk at gmail dot com

  QgsWMSLegendNode     : Sandro Santilli < strk at keybit dot net >

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAYERTREEMODELLEGENDNODE_H
#define QGSLAYERTREEMODELLEGENDNODE_H

#include <QIcon>
#include <QObject>

#include "qgis_core.h"
#include "qgis_sip.h"

#include "qgsexpressioncontext.h"
#include "qgslegendpatchshape.h"
#include "qgspallabeling.h"

class QgsLayerTreeLayer;
class QgsLayerTreeModel;
class QgsLegendSettings;
class QgsMapSettings;
class QgsSymbol;
class QgsRenderContext;

/**
 * \ingroup core
 * \brief The QgsLegendRendererItem class is abstract interface for legend items
 * returned from QgsMapLayerLegend implementation.
 *
 * The objects are used in QgsLayerTreeModel. Custom implementations may offer additional interactivity
 * and customized look.
 *
 * \since QGIS 2.6
 */
class CORE_EXPORT QgsLayerTreeModelLegendNode : public QObject
{
#ifdef SIP_RUN
#include "qgscolorramplegendnode.h"
#endif

#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    if ( qobject_cast<QgsSymbolLegendNode *> ( sipCpp ) )
      sipType = sipType_QgsSymbolLegendNode;
    else if ( qobject_cast<QgsDataDefinedSizeLegendNode *> ( sipCpp ) )
      sipType = sipType_QgsDataDefinedSizeLegendNode;
    else if ( qobject_cast<QgsImageLegendNode *> ( sipCpp ) )
      sipType = sipType_QgsImageLegendNode;
    else if ( qobject_cast<QgsRasterSymbolLegendNode *> ( sipCpp ) )
      sipType = sipType_QgsRasterSymbolLegendNode;
    else if ( qobject_cast<QgsSimpleLegendNode *> ( sipCpp ) )
      sipType = sipType_QgsSimpleLegendNode;
    else if ( qobject_cast<QgsWmsLegendNode *> ( sipCpp ) )
      sipType = sipType_QgsWmsLegendNode;
    else if ( qobject_cast<QgsColorRampLegendNode *> ( sipCpp ) )
      sipType = sipType_QgsColorRampLegendNode;
    else
      sipType = 0;
    SIP_END
#endif

    Q_OBJECT

  public:

    //! Legend node data roles
    enum LegendNodeRoles
    {
      RuleKeyRole = Qt::UserRole, //!< Rule key of the node (QString)
      ParentRuleKeyRole, //!< Rule key of the parent legend node - for legends with tree hierarchy (QString). Added in 2.8
      NodeTypeRole, //!< Type of node. Added in 3.16
    };

    //! Types of legend nodes
    enum NodeTypes
    {
      SimpleLegend, //!< Simple label with icon legend node type
      SymbolLegend, //!< Vector symbol legend node type
      RasterSymbolLegend, //!< Raster symbol legend node type
      ImageLegend, //!< Raster image legend node type
      WmsLegend, //!< WMS legend node type
      DataDefinedSizeLegend, //!< Marker symbol legend node type
      EmbeddedWidget, //!< Embedded widget placeholder node type
      ColorRampLegend, //!< Color ramp legend (since QGIS 3.18)
    };

    //! Returns pointer to the parent layer node
    QgsLayerTreeLayer *layerNode() const { return mLayerNode; }

    //! Returns pointer to model owning this legend node
    QgsLayerTreeModel *model() const;

    //! Returns item flags associated with the item. Default implementation returns Qt::ItemIsEnabled.
    virtual Qt::ItemFlags flags() const;

    //! Returns data associated with the item. Must be implemented in derived class.
    virtual QVariant data( int role ) const = 0;

    //! Sets some data associated with the item. Default implementation does nothing and returns FALSE.
    virtual bool setData( const QVariant &value, int role );

    virtual bool isEmbeddedInParent() const { return mEmbeddedInParent; }
    virtual void setEmbeddedInParent( bool embedded ) { mEmbeddedInParent = embedded; }

    virtual QString userLabel() const { return mUserLabel; }
    virtual void setUserLabel( const QString &userLabel ) { mUserLabel = userLabel; }

    /**
     * Returns the user (overridden) size for the legend node.
     *
     * If either the width or height are non-zero, they will be used when rendering the legend node instead of the default
     * symbol width or height from QgsLegendSettings.
     *
     * \see setUserPatchSize()
     * \since QGIS 3.14
     */
    virtual QSizeF userPatchSize() const;

    /**
     * Sets the user (overridden) \a size for the legend node.
     *
     * If either the width or height are non-zero, they will be used when rendering the legend node instead of the default
     * symbol width or height from QgsLegendSettings.
     *
     * \see userPatchSize()
     * \since QGIS 3.14
     */
    virtual void setUserPatchSize( QSizeF size );

    /**
     * Sets whether a forced column break should occur before the node.
     *
     * \see columnBreak()
     * \since QGIS 3.14
     */
    virtual void setColumnBreak( bool breakBeforeNode ) { mColumnBreakBeforeNode = breakBeforeNode; }

    /**
     * Returns whether a forced column break should occur before the node.
     *
     * \see setColumnBreak()
     * \since QGIS 3.14
     */
    virtual bool columnBreak() const { return mColumnBreakBeforeNode; }

    virtual bool isScaleOK( double scale ) const { Q_UNUSED( scale ) return true; }

    /**
     * Notification from model that information from associated map view has changed.
     *  Default implementation does nothing.
    */
    virtual void invalidateMapBasedData() {}

    struct ItemContext
    {
      Q_NOWARN_DEPRECATED_PUSH     //because of deprecated members
      ItemContext() = default;
      Q_NOWARN_DEPRECATED_POP

      //! Render context, if available
      QgsRenderContext *context = nullptr;
      //! Painter
      QPainter *painter = nullptr;

      /**
       * Top-left corner of the legend item.
       * \deprecated Use top, columnLeft, columnRight instead.
       */
      Q_DECL_DEPRECATED QPointF point;

      /**
       * Offset from the left side where label should start.
       * \deprecated use columnLeft, columnRight instead.
       */
      Q_DECL_DEPRECATED double labelXOffset = 0.0;

      /**
       * Top y-position of legend item.
       * \since QGIS 3.10
       */
      double top = 0.0;

      /**
       * Left side of current legend column. This should be used when determining
       * where to render legend item content, correctly respecting the symbol and text
       * alignment from the legend settings.
       * \since QGIS 3.10
       */
      double columnLeft = 0.0;

      /**
       * Right side of current legend column. This should be used when determining
       * where to render legend item content, correctly respecting the symbol and text
       * alignment from the legend settings.
       * \since QGIS 3.10
       */
      double columnRight = 0.0;

      /**
       * Largest symbol width, considering all other sibling legend components associated with
       * the current component.
       * \since QGIS 3.10
       */
      double maxSiblingSymbolWidth = 0.0;

      /**
       * The patch shape to render for the node.
       *
       * \since QGIS 3.14
       */
      QgsLegendPatchShape patchShape;

      /**
       * Symbol patch size to render for the node.
       *
       * If either the width or height are zero, then the default width/height from QgsLegendSettings::symbolSize() should be used instead.
       *
       * \since QGIS 3.14
       */
      QSizeF patchSize;
    };

    struct ItemMetrics
    {
      QSizeF symbolSize;
      QSizeF labelSize;
    };

    /**
     * Entry point called from QgsLegendRenderer to do the rendering.
     *  Default implementation calls drawSymbol() and drawSymbolText() methods.
     *
     *  If ctx is NULLPTR, this is just first stage when preparing layout - without actual rendering.
     */
    virtual ItemMetrics draw( const QgsLegendSettings &settings, ItemContext *ctx );

    /**
     * Entry point called from QgsLegendRenderer to do the rendering in a
     * JSON object.
     * \param settings Legend layout configuration
     * \param context Rendering context
     * \since QGIS 3.8
     */
    QJsonObject exportToJson( const QgsLegendSettings &settings, const QgsRenderContext &context );

    /**
     * Draws symbol on the left side of the item
     * \param settings Legend layout configuration
     * \param ctx Context for rendering - may be NULLPTR if only doing layout without actual rendering
     * \param itemHeight Minimal height of the legend item - used for correct positioning when rendering
     * \returns Real size of the symbol (may be bigger than "normal" symbol size from settings)
     */
    virtual QSizeF drawSymbol( const QgsLegendSettings &settings, ItemContext *ctx, double itemHeight ) const;

    /**
     * Adds a symbol in base64 string within a JSON object with the key "icon".
     * \param settings Legend layout configuration
     * \param context Rendering context
     * \since QGIS 3.8
     */
    virtual QJsonObject exportSymbolToJson( const QgsLegendSettings &settings, const QgsRenderContext &context ) const;

    /**
     * Draws label on the right side of the item
     * \param settings Legend layout configuration
     * \param ctx Context for rendering - may be NULLPTR if only doing layout without actual rendering
     * \param symbolSize  Real size of the associated symbol - used for correct positioning when rendering
     * \returns Size of the label (may span multiple lines)
     */
    virtual QSizeF drawSymbolText( const QgsLegendSettings &settings, ItemContext *ctx, QSizeF symbolSize ) const;

  public slots:

    /**
     * Checks all checkable items belonging to the same layer as this node.
     * \see uncheckAllItems()
     * \see toggleAllItems()
     * \since QGIS 3.18 (previously was available in QgsSymbolLegendNode subclass only)
     */
    void checkAllItems();

    /**
     * Unchecks all checkable items belonging to the same layer as this node.
     * \see checkAllItems()
     * \see toggleAllItems()
     * \since QGIS 3.18 (previously was available in QgsSymbolLegendNode subclass only)
     */
    void uncheckAllItems();

    /**
     * Toggle all checkable items belonging to the same layer as this node.
     * \see checkAllItems()
     * \see uncheckAllItems()
     * \since QGIS 3.18 (previously was available in QgsSymbolLegendNode subclass only)
     */
    void toggleAllItems();

  signals:
    //! Emitted on internal data change so the layer tree model can forward the signal to views
    void dataChanged();

    /**
     * Emitted when the size of this node changes.
     *
     * \since QGIS 3.16
     */
    void sizeChanged();

  protected:
    //! Construct the node with pointer to its parent layer node
    explicit QgsLayerTreeModelLegendNode( QgsLayerTreeLayer *nodeL, QObject *parent SIP_TRANSFERTHIS = nullptr );

    //! Returns a temporary context or NULLPTR if legendMapViewData are not valid
    QgsRenderContext *createTemporaryRenderContext() const SIP_FACTORY;

  protected:
    QgsLayerTreeLayer *mLayerNode = nullptr;
    bool mEmbeddedInParent;
    QString mUserLabel;
    QgsLegendPatchShape mPatchShape;
    QSizeF mUserSize;
    bool mColumnBreakBeforeNode = false;

  private:

    /**
     * Sets all items belonging to the same layer as this node to the same check state.
     * \param state check state
     */
    void checkAll( bool state );
};
Q_DECLARE_METATYPE( QgsLayerTreeModelLegendNode::NodeTypes )

#include "qgslegendsymbolitem.h"
#include "qgstextformat.h"

/**
 * \ingroup core
 * \brief Implementation of legend node interface for displaying preview of vector symbols and their labels
 * and allowing interaction with the symbol / renderer.
 *
 * \since QGIS 2.6
 */
class CORE_EXPORT QgsSymbolLegendNode : public QgsLayerTreeModelLegendNode
{
    Q_OBJECT

  public:

    static double MINIMUM_SIZE;
    static double MAXIMUM_SIZE;

    /**
     * Constructor for QgsSymbolLegendNode.
     * \param nodeLayer layer node
     * \param item the legend symbol item
     * \param parent attach a parent QObject to the legend node.
     */
    QgsSymbolLegendNode( QgsLayerTreeLayer *nodeLayer, const QgsLegendSymbolItem &item, QObject *parent SIP_TRANSFERTHIS = nullptr );

    Qt::ItemFlags flags() const override;
    QVariant data( int role ) const override;
    bool setData( const QVariant &value, int role ) override;

    QSizeF drawSymbol( const QgsLegendSettings &settings, ItemContext *ctx, double itemHeight ) const override;

    QJsonObject exportSymbolToJson( const QgsLegendSettings &settings, const QgsRenderContext &context ) const override;

    void setEmbeddedInParent( bool embedded ) override;

    void setUserLabel( const QString &userLabel ) override { mUserLabel = userLabel; updateLabel(); }

    bool isScaleOK( double scale ) const override { return mItem.isScaleOK( scale ); }

    void invalidateMapBasedData() override;

    /**
     * Set the icon size
     * \since QGIS 2.10
     */
    void setIconSize( QSize sz ) { mIconSize = sz; }
    //! \since QGIS 2.10
    QSize iconSize() const { return mIconSize; }

    /**
     * Calculates the minimum icon size to prevent cropping. When evaluating
     * the size for multiple icons it is more efficient to create a single
     * render context in advance and use the variant which accepts a QgsRenderContext
     * argument.
     * \since QGIS 2.10
     */
    QSize minimumIconSize() const;

    /**
     * Calculates the minimum icon size to prevent cropping. When evaluating
     * the size for multiple icons it is more efficient to create a single
     * render context in advance and call this method instead of minimumIconSize().
     * \since QGIS 2.18
     */
    QSize minimumIconSize( QgsRenderContext *context ) const;

    /**
     * Returns the symbol used by the legend node.
     * \see setSymbol()
     * \since QGIS 2.14
     */
    const QgsSymbol *symbol() const;

    /**
     * Sets the \a symbol to be used by the legend node. The symbol change is also propagated
     * to the associated vector layer's renderer.
     * \param symbol new symbol for node. Ownership is transferred.
     * \see symbol()
     * \since QGIS 2.14
     */
    void setSymbol( QgsSymbol *symbol SIP_TRANSFER );

    /**
     * Returns label of text to be shown on top of the symbol.
     * \since QGIS 3.2
     */
    QString textOnSymbolLabel() const { return mTextOnSymbolLabel; }

    /**
     * Sets label of text to be shown on top of the symbol.
     * \since QGIS 3.2
     */
    void setTextOnSymbolLabel( const QString &label ) { mTextOnSymbolLabel = label; }

    /**
     * Returns text format of the label to be shown on top of the symbol.
     * \since QGIS 3.2
     */
    QgsTextFormat textOnSymbolTextFormat() const { return mTextOnSymbolTextFormat; }

    /**
     * Sets format of text to be shown on top of the symbol.
     * \since QGIS 3.2
     */
    void setTextOnSymbolTextFormat( const QgsTextFormat &format ) { mTextOnSymbolTextFormat = format; }

    /**
     * Label of the symbol, user defined label will be used, otherwise will default to the label made by QGIS.
     * \since QGIS 3.10
     */
    QString symbolLabel() const;

    /**
     * Returns the symbol patch shape to use when rendering the legend node symbol.
     *
     * \see setPatchShape()
     * \since QGIS 3.14
     */
    QgsLegendPatchShape patchShape() const;

    /**
     * Sets the symbol patch \a shape to use when rendering the legend node symbol.
     *
     * \see patchShape()
     * \since QGIS 3.14
     */
    void setPatchShape( const QgsLegendPatchShape &shape );

    /**
     * Returns the node's custom symbol.
     *
     * If a non-NULLPTR value is returned, then this symbol will be used for rendering
     * the legend node instead of the default symbol().
     *
     * \see setCustomSymbol()
     * \since QGIS 3.14
     */
    QgsSymbol *customSymbol() const;

    /**
     * Sets the node's custom \a symbol.
     *
     * If a non-NULLPTR value is set, then this symbol will be used for rendering
     * the legend node instead of the default symbol().
     *
     * Ownership of \a symbol is transferred.
     *
     * \see customSymbol()
     * \since QGIS 3.14
     */
    void setCustomSymbol( QgsSymbol *symbol SIP_TRANSFER );

    /**
     * Evaluates  and returns the text label of the current node
     * \param context extra QgsExpressionContext to use for evaluating the expression
     * \param label text to evaluate instead of the layer layertree string
     * \since QGIS 3.10
     */
    QString evaluateLabel( const QgsExpressionContext &context = QgsExpressionContext(), const QString &label = QString() );

  private:
    void updateLabel();

  private:
    QgsLegendSymbolItem mItem;
    mutable QPixmap mPixmap; // cached symbol preview
    QString mLabel;
    bool mSymbolUsesMapUnits;

    QSize mIconSize;

    QString mTextOnSymbolLabel;
    QgsTextFormat mTextOnSymbolTextFormat;

    std::unique_ptr< QgsSymbol > mCustomSymbol;

    // ident the symbol icon to make it look like a tree structure
    static const int INDENT_SIZE = 20;

    /**
     * Create an expressionContextScope containing symbol related variables
     * \since QGIS 3.10
     */
    QgsExpressionContextScope *createSymbolScope() const SIP_FACTORY;

};


/**
 * \ingroup core
 * \brief Implementation of legend node interface for displaying arbitrary label with icon.
 *
 * \since QGIS 2.6
 */
class CORE_EXPORT QgsSimpleLegendNode : public QgsLayerTreeModelLegendNode
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsSimpleLegendNode.
     * \param nodeLayer layer node
     * \param label label
     * \param icon icon
     * \param parent attach a parent QObject to the legend node.
     * \param key the rule key
     */
    QgsSimpleLegendNode( QgsLayerTreeLayer *nodeLayer, const QString &label, const QIcon &icon = QIcon(), QObject *parent SIP_TRANSFERTHIS = nullptr, const QString &key = QString() );

    QVariant data( int role ) const override;

  private:
    QString mLabel;
    QString mId;
    QIcon mIcon;
    QString mKey;
};


/**
 * \ingroup core
 * \brief Implementation of legend node interface for displaying arbitrary raster image
 *
 * \since QGIS 2.6
 */
class CORE_EXPORT QgsImageLegendNode : public QgsLayerTreeModelLegendNode
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsImageLegendNode.
     * \param nodeLayer layer node
     * \param img the image
     * \param parent attach a parent QObject to the legend node.
     */
    QgsImageLegendNode( QgsLayerTreeLayer *nodeLayer, const QImage &img, QObject *parent SIP_TRANSFERTHIS = nullptr );

    QVariant data( int role ) const override;

    QSizeF drawSymbol( const QgsLegendSettings &settings, ItemContext *ctx, double itemHeight ) const override;

    QJsonObject exportSymbolToJson( const QgsLegendSettings &settings, const QgsRenderContext &context ) const override;

  private:
    QImage mImage;
};

/**
 * \ingroup core
 * \brief Implementation of legend node interface for displaying raster legend entries
 *
 * \since QGIS 2.6
 */
class CORE_EXPORT QgsRasterSymbolLegendNode : public QgsLayerTreeModelLegendNode
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsRasterSymbolLegendNode.
     * \param nodeLayer layer node
     * \param color color
     * \param label label
     * \param parent attach a parent QObject to the legend node.
     * \param isCheckable set to TRUE to enable the checkbox for the node (since QGIS 3.18)
     * \param ruleKey optional identifier to allow a unique ID to be assigned to the node by a renderer (since QGIS 3.18)
     */
    QgsRasterSymbolLegendNode( QgsLayerTreeLayer *nodeLayer, const QColor &color, const QString &label, QObject *parent SIP_TRANSFERTHIS = nullptr, bool isCheckable = false, const QString &ruleKey = QString() );

    Qt::ItemFlags flags() const override;
    QVariant data( int role ) const override;
    bool setData( const QVariant &value, int role ) override;
    QSizeF drawSymbol( const QgsLegendSettings &settings, ItemContext *ctx, double itemHeight ) const override;
    QJsonObject exportSymbolToJson( const QgsLegendSettings &settings, const QgsRenderContext &context ) const override;

    /**
     * Returns the unique identifier of node for identification of the item within renderer.
     *
     * \since QGIS 3.18
     */
    QString ruleKey() const { return mRuleKey; }

    /**
     * Returns whether the item is user-checkable - whether renderer supports enabling/disabling it.
     *
     * \since QGIS 3.18
     */
    bool isCheckable() const { return mCheckable; }

  private:
    QColor mColor;
    QString mLabel;
    bool mCheckable = false;
    QString mRuleKey;
};

class QgsImageFetcher;

/**
 * \ingroup core
 * \brief Implementation of legend node interface for displaying WMS legend entries
 *
 * \since QGIS 2.8
 */
class CORE_EXPORT QgsWmsLegendNode : public QgsLayerTreeModelLegendNode
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsWmsLegendNode.
     * \param nodeLayer layer node
     * \param parent attach a parent QObject to the legend node.
     */
    QgsWmsLegendNode( QgsLayerTreeLayer *nodeLayer, QObject *parent SIP_TRANSFERTHIS = nullptr );

    ~QgsWmsLegendNode() override;

    QVariant data( int role ) const override;

    QSizeF drawSymbol( const QgsLegendSettings &settings, ItemContext *ctx, double itemHeight ) const override;

    QJsonObject exportSymbolToJson( const QgsLegendSettings &settings, const QgsRenderContext &context ) const override;

    void invalidateMapBasedData() override;

  private slots:

    void getLegendGraphicFinished( const QImage & );
    void getLegendGraphicErrored( const QString & );
    void getLegendGraphicProgress( qint64, qint64 );

  private:

    // Lazily initializes mImage
    QImage getLegendGraphic() const;

    QImage renderMessage( const QString &msg ) const;

    QImage mImage;

    bool mValid;

    mutable std::unique_ptr<QgsImageFetcher> mFetcher;
};


/**
 * \ingroup core
 * \brief Produces legend node with a marker symbol
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsDataDefinedSizeLegendNode : public QgsLayerTreeModelLegendNode
{
    Q_OBJECT

  public:
    //! Construct the node using QgsDataDefinedSizeLegend as definition of the node's appearance
    QgsDataDefinedSizeLegendNode( QgsLayerTreeLayer *nodeLayer, const QgsDataDefinedSizeLegend &settings, QObject *parent SIP_TRANSFERTHIS = nullptr );
    ~QgsDataDefinedSizeLegendNode() override;

    QVariant data( int role ) const override;

    ItemMetrics draw( const QgsLegendSettings &settings, ItemContext *ctx ) override;

  private:
    void cacheImage() const;
    QgsDataDefinedSizeLegend *mSettings = nullptr;
    mutable QImage mImage;
};

/**
 * \ingroup core
 * \brief Produces legend node for a labeling text symbol
 * \since QGIS 3.20
 */
class CORE_EXPORT QgsVectorLabelLegendNode : public QgsLayerTreeModelLegendNode
{
    Q_OBJECT
  public:

    /**
     * \brief QgsVectorLabelLegendNode
     * \param nodeLayer the parent node
     * \param labelSettings setting of the label class
     * \param parent the parent object
     */
    QgsVectorLabelLegendNode( QgsLayerTreeLayer *nodeLayer, const QgsPalLayerSettings &labelSettings, QObject *parent = nullptr );
    ~QgsVectorLabelLegendNode() override;

    /**
     * \brief data Returns data associated with the item
     * \param role the data role
     * \returns variant containing the data for the role
     */
    QVariant data( int role ) const override;

    /**
     * \brief drawSymbol
     * \param settings the legend settings
     * \param ctx context for the item
     * \param itemHeight the height of the item
     * \returns size of the item
     */
    QSizeF drawSymbol( const QgsLegendSettings &settings, ItemContext *ctx, double itemHeight ) const override;

    /**
     * \brief exportSymbolToJson
     * \param settings the legend settings
     * \param context the item context
     * \returns the json object
     */
    QJsonObject exportSymbolToJson( const QgsLegendSettings &settings, const QgsRenderContext &context ) const override;

  private:
    QgsPalLayerSettings mLabelSettings;
    QSizeF drawSymbol( const QgsLegendSettings &settings, const QgsRenderContext &renderContext, double xOffset = 0.0, double yOffset = 0.0 ) const;
    void textWidthHeight( double &width, double &height, QgsRenderContext &ctx, const QgsTextFormat &textFormat, const QStringList &textLines ) const;
};


#endif // QGSLAYERTREEMODELLEGENDNODE_H
