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
#include "qgis.h"

#include "qgsrasterdataprovider.h" // for QgsImageFetcher dtor visibility

class QgsLayerTreeLayer;
class QgsLayerTreeModel;
class QgsLegendSettings;
class QgsMapSettings;
class QgsSymbol;
class QgsRenderContext;

/**
 * \ingroup core
 * The QgsLegendRendererItem class is abstract interface for legend items
 * returned from QgsMapLayerLegend implementation.
 *
 * The objects are used in QgsLayerTreeModel. Custom implementations may offer additional interactivity
 * and customized look.
 *
 * \since QGIS 2.6
 */
class CORE_EXPORT QgsLayerTreeModelLegendNode : public QObject
{
    Q_OBJECT
  public:

    enum LegendNodeRoles
    {
      RuleKeyRole = Qt::UserRole,     //!< Rule key of the node (QString)
      ParentRuleKeyRole               //!< Rule key of the parent legend node - for legends with tree hierarchy (QString). Added in 2.8
    };

    //! Return pointer to the parent layer node
    QgsLayerTreeLayer *layerNode() const { return mLayerNode; }

    //! Return pointer to model owning this legend node
    QgsLayerTreeModel *model() const;

    //! Return item flags associated with the item. Default implementation returns Qt::ItemIsEnabled.
    virtual Qt::ItemFlags flags() const;

    //! Return data associated with the item. Must be implemented in derived class.
    virtual QVariant data( int role ) const = 0;

    //! Set some data associated with the item. Default implementation does nothing and returns false.
    virtual bool setData( const QVariant &value, int role );

    virtual bool isEmbeddedInParent() const { return mEmbeddedInParent; }
    virtual void setEmbeddedInParent( bool embedded ) { mEmbeddedInParent = embedded; }

    virtual QString userLabel() const { return mUserLabel; }
    virtual void setUserLabel( const QString &userLabel ) { mUserLabel = userLabel; }

    virtual bool isScaleOK( double scale ) const { Q_UNUSED( scale ); return true; }

    /**
     * Notification from model that information from associated map view has changed.
     *  Default implementation does nothing. */
    virtual void invalidateMapBasedData() {}

    struct ItemContext
    {
      //! Painter
      QPainter *painter = nullptr;
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

    /**
     * Entry point called from QgsLegendRenderer to do the rendering.
     *  Default implementation calls drawSymbol() and drawSymbolText() methods.
     *
     *  If ctx is null, this is just first stage when preparing layout - without actual rendering.
     */
    virtual ItemMetrics draw( const QgsLegendSettings &settings, ItemContext *ctx );

    /**
     * Draws symbol on the left side of the item
     * \param settings Legend layout configuration
     * \param ctx Context for rendering - may be null if only doing layout without actual rendering
     * \param itemHeight Minimal height of the legend item - used for correct positioning when rendering
     * \returns Real size of the symbol (may be bigger than "normal" symbol size from settings)
     */
    virtual QSizeF drawSymbol( const QgsLegendSettings &settings, ItemContext *ctx, double itemHeight ) const;

    /**
     * Draws label on the right side of the item
     * \param settings Legend layout configuration
     * \param ctx Context for rendering - may be null if only doing layout without actual rendering
     * \param symbolSize  Real size of the associated symbol - used for correct positioning when rendering
     * \returns Size of the label (may span multiple lines)
     */
    virtual QSizeF drawSymbolText( const QgsLegendSettings &settings, ItemContext *ctx, QSizeF symbolSize ) const;

  signals:
    //! Emitted on internal data change so the layer tree model can forward the signal to views
    void dataChanged();

  protected:
    //! Construct the node with pointer to its parent layer node
    explicit QgsLayerTreeModelLegendNode( QgsLayerTreeLayer *nodeL, QObject *parent SIP_TRANSFERTHIS = nullptr );

    //! Returns a temporary context or null if legendMapViewData are not valid
    QgsRenderContext *createTemporaryRenderContext() const SIP_FACTORY;

  protected:
    QgsLayerTreeLayer *mLayerNode = nullptr;
    bool mEmbeddedInParent;
    QString mUserLabel;
};

#include "qgslegendsymbolitem.h"

/**
 * \ingroup core
 * Implementation of legend node interface for displaying preview of vector symbols and their labels
 * and allowing interaction with the symbol / renderer.
 *
 * \since QGIS 2.6
 */
class CORE_EXPORT QgsSymbolLegendNode : public QgsLayerTreeModelLegendNode
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsSymbolLegendNode.
     * \param nodeLayer layer node
     * \param item the legend symbol item
     * \param parent attach a parent QObject to the legend node.
     */
    QgsSymbolLegendNode( QgsLayerTreeLayer *nodeLayer, const QgsLegendSymbolItem &item, QObject *parent SIP_TRANSFERTHIS = nullptr );

    virtual Qt::ItemFlags flags() const override;
    virtual QVariant data( int role ) const override;
    virtual bool setData( const QVariant &value, int role ) override;

    QSizeF drawSymbol( const QgsLegendSettings &settings, ItemContext *ctx, double itemHeight ) const override;

    virtual void setEmbeddedInParent( bool embedded ) override;

    void setUserLabel( const QString &userLabel ) override { mUserLabel = userLabel; updateLabel(); }

    virtual bool isScaleOK( double scale ) const override { return mItem.isScaleOK( scale ); }

    virtual void invalidateMapBasedData() override;

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
     * Sets the symbol to be used by the legend node. The symbol change is also propagated
     * to the associated vector layer's renderer.
     * \param symbol new symbol for node. Ownership is transferred.
     * \see symbol()
     * \since QGIS 2.14
     */
    void setSymbol( QgsSymbol *symbol );

  public slots:

    /**
     * Checks all items belonging to the same layer as this node.
     * \since QGIS 2.14
     * \see uncheckAllItems()
     */
    void checkAllItems();

    /**
     * Unchecks all items belonging to the same layer as this node.
     * \since QGIS 2.14
     * \see checkAllItems()
     */
    void uncheckAllItems();

  private:
    void updateLabel();

  private:
    QgsLegendSymbolItem mItem;
    mutable QPixmap mPixmap; // cached symbol preview
    QString mLabel;
    bool mSymbolUsesMapUnits;
    QSize mIconSize;

    // ident the symbol icon to make it look like a tree structure
    static const int INDENT_SIZE = 20;

    /**
     * Sets all items belonging to the same layer as this node to the same check state.
     * \param state check state
     */
    void checkAll( bool state );
};


/**
 * \ingroup core
 * Implementation of legend node interface for displaying arbitrary label with icon.
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

    virtual QVariant data( int role ) const override;

  private:
    QString mLabel;
    QString mId;
    QIcon mIcon;
    QString mKey;
};


/**
 * \ingroup core
 * Implementation of legend node interface for displaying arbitrary raster image
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

    virtual QVariant data( int role ) const override;

    QSizeF drawSymbol( const QgsLegendSettings &settings, ItemContext *ctx, double itemHeight ) const override;

  private:
    QImage mImage;
};

/**
 * \ingroup core
 * Implementation of legend node interface for displaying raster legend entries
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
     */
    QgsRasterSymbolLegendNode( QgsLayerTreeLayer *nodeLayer, const QColor &color, const QString &label, QObject *parent SIP_TRANSFERTHIS = nullptr );

    virtual QVariant data( int role ) const override;

    QSizeF drawSymbol( const QgsLegendSettings &settings, ItemContext *ctx, double itemHeight ) const override;

  private:
    QColor mColor;
    QString mLabel;
};

class QgsImageFetcher;

/**
 * \ingroup core
 * Implementation of legend node interface for displaying WMS legend entries
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

    virtual QVariant data( int role ) const override;

    virtual QSizeF drawSymbol( const QgsLegendSettings &settings, ItemContext *ctx, double itemHeight ) const override;

    virtual void invalidateMapBasedData() override;

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
 * Produces legend node with a marker symbol
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsDataDefinedSizeLegendNode : public QgsLayerTreeModelLegendNode
{
    Q_OBJECT

  public:
    //! Construct the node using QgsDataDefinedSizeLegend as definition of the node's appearance
    QgsDataDefinedSizeLegendNode( QgsLayerTreeLayer *nodeLayer, const QgsDataDefinedSizeLegend &settings, QObject *parent SIP_TRANSFERTHIS = nullptr );
    ~QgsDataDefinedSizeLegendNode();

    virtual QVariant data( int role ) const override;

    ItemMetrics draw( const QgsLegendSettings &settings, ItemContext *ctx ) override;

  private:
    void cacheImage() const;
    QgsDataDefinedSizeLegend *mSettings = nullptr;
    mutable QImage mImage;
};

#endif // QGSLAYERTREEMODELLEGENDNODE_H
