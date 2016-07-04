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

#include "qgsrasterdataprovider.h" // for QgsImageFetcher dtor visibility

class QgsLayerTreeLayer;
class QgsLayerTreeModel;
class QgsLegendSettings;
class QgsMapSettings;
class QgsSymbolV2;
class QgsRenderContext;

/** \ingroup core
 * The QgsLegendRendererItem class is abstract interface for legend items
 * returned from QgsMapLayerLegend implementation.
 *
 * The objects are used in QgsLayerTreeModel. Custom implementations may offer additional interactivity
 * and customized look.
 *
 * @note added in 2.6
 */
class CORE_EXPORT QgsLayerTreeModelLegendNode : public QObject
{
    Q_OBJECT
  public:
    ~QgsLayerTreeModelLegendNode();

    enum LegendNodeRoles
    {
      RuleKeyRole = Qt::UserRole,     //!< rule key of the node (QString)
      SymbolV2LegacyRuleKeyRole,      //!< for QgsSymbolV2LegendNode only - legacy rule key (void ptr, to be cast to QgsSymbolV2 ptr)
      ParentRuleKeyRole               //!< rule key of the parent legend node - for legends with tree hierarchy (QString). Added in 2.8
    };

    /** Return pointer to the parent layer node */
    QgsLayerTreeLayer* layerNode() const { return mLayerNode; }

    /** Return pointer to model owning this legend node */
    QgsLayerTreeModel* model() const;

    /** Return item flags associated with the item. Default implementation returns Qt::ItemIsEnabled. */
    virtual Qt::ItemFlags flags() const;

    /** Return data associated with the item. Must be implemented in derived class. */
    virtual QVariant data( int role ) const = 0;

    /** Set some data associated with the item. Default implementation does nothing and returns false. */
    virtual bool setData( const QVariant& value, int role );

    virtual bool isEmbeddedInParent() const { return mEmbeddedInParent; }
    virtual void setEmbeddedInParent( bool embedded ) { mEmbeddedInParent = embedded; }

    virtual QString userLabel() const { return mUserLabel; }
    virtual void setUserLabel( const QString& userLabel ) { mUserLabel = userLabel; }

    virtual bool isScaleOK( double scale ) const { Q_UNUSED( scale ); return true; }

    /** Notification from model that information from associated map view has changed.
     *  Default implementation does nothing. */
    virtual void invalidateMapBasedData() {}

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
    virtual QSizeF drawSymbolText( const QgsLegendSettings& settings, ItemContext* ctx, QSizeF symbolSize ) const;

  signals:
    //! Emitted on internal data change so the layer tree model can forward the signal to views
    void dataChanged();

  protected:
    /** Construct the node with pointer to its parent layer node */
    explicit QgsLayerTreeModelLegendNode( QgsLayerTreeLayer* nodeL, QObject* parent = nullptr );

  protected:
    QgsLayerTreeLayer* mLayerNode;
    bool mEmbeddedInParent;
    QString mUserLabel;
};

#include "qgslegendsymbolitemv2.h"

/** \ingroup core
 * Implementation of legend node interface for displaying preview of vector symbols and their labels
 * and allowing interaction with the symbol / renderer.
 *
 * @note added in 2.6
 */
class CORE_EXPORT QgsSymbolV2LegendNode : public QgsLayerTreeModelLegendNode
{
    Q_OBJECT

  public:
    QgsSymbolV2LegendNode( QgsLayerTreeLayer* nodeLayer, const QgsLegendSymbolItemV2& item, QObject* parent = nullptr );
    ~QgsSymbolV2LegendNode();

    virtual Qt::ItemFlags flags() const override;
    virtual QVariant data( int role ) const override;
    virtual bool setData( const QVariant& value, int role ) override;

    QSizeF drawSymbol( const QgsLegendSettings& settings, ItemContext* ctx, double itemHeight ) const override;

    virtual void setEmbeddedInParent( bool embedded ) override;

    void setUserLabel( const QString& userLabel ) override { mUserLabel = userLabel; updateLabel(); }

    virtual bool isScaleOK( double scale ) const override { return mItem.isScaleOK( scale ); }

    virtual void invalidateMapBasedData() override;

    //! Set the icon size
    //! @note added in 2.10
    void setIconSize( QSize sz ) { mIconSize = sz; }
    //! @note added in 2.10
    QSize iconSize() const { return mIconSize; }

    //! Get the minimum icon size to prevent cropping
    //! @note added in 2.10
    QSize minimumIconSize() const;

    /** Returns the symbol used by the legend node.
     * @see setSymbol()
     * @note added in QGIS 2.14
     */
    const QgsSymbolV2* symbol() const;

    /** Sets the symbol to be used by the legend node. The symbol change is also propagated
     * to the associated vector layer's renderer.
     * @param symbol new symbol for node. Ownership is transferred.
     * @see symbol()
     * @note added in QGIS 2.14
     */
    void setSymbol( QgsSymbolV2* symbol );

  public slots:

    /** Checks all items belonging to the same layer as this node.
     * @note added in QGIS 2.14
     * @see uncheckAllItems()
     */
    void checkAllItems();

    /** Unchecks all items belonging to the same layer as this node.
     * @note added in QGIS 2.14
     * @see checkAllItems()
     */
    void uncheckAllItems();

  private:
    void updateLabel();

  private:
    QgsLegendSymbolItemV2 mItem;
    mutable QPixmap mPixmap; // cached symbol preview
    QString mLabel;
    bool mSymbolUsesMapUnits;
    QSize mIconSize;

    // ident the symbol icon to make it look like a tree structure
    static const int indentSize = 20;

    // return a temporary context or null if legendMapViewData are not valid
    QgsRenderContext * createTemporaryRenderContext() const;

    /** Sets all items belonging to the same layer as this node to the same check state.
     * @param state check state
     */
    void checkAll( bool state );
};


/** \ingroup core
 * Implementation of legend node interface for displaying arbitrary label with icon.
 *
 * @note added in 2.6
 */
class CORE_EXPORT QgsSimpleLegendNode : public QgsLayerTreeModelLegendNode
{
    Q_OBJECT

  public:
    QgsSimpleLegendNode( QgsLayerTreeLayer* nodeLayer, const QString& label, const QIcon& icon = QIcon(), QObject* parent = nullptr, const QString& key = QString() );

    virtual QVariant data( int role ) const override;

  private:
    QString mLabel;
    QString mId;
    QIcon mIcon;
    QString mKey;
};


/** \ingroup core
 * Implementation of legend node interface for displaying arbitrary raster image
 *
 * @note added in 2.6
 */
class CORE_EXPORT QgsImageLegendNode : public QgsLayerTreeModelLegendNode
{
    Q_OBJECT

  public:
    QgsImageLegendNode( QgsLayerTreeLayer* nodeLayer, const QImage& img, QObject* parent = nullptr );

    virtual QVariant data( int role ) const override;

    QSizeF drawSymbol( const QgsLegendSettings& settings, ItemContext* ctx, double itemHeight ) const override;

  private:
    QImage mImage;
};

/** \ingroup core
 * Implementation of legend node interface for displaying raster legend entries
 *
 * @note added in 2.6
 */
class CORE_EXPORT QgsRasterSymbolLegendNode : public QgsLayerTreeModelLegendNode
{
    Q_OBJECT

  public:
    QgsRasterSymbolLegendNode( QgsLayerTreeLayer* nodeLayer, const QColor& color, const QString& label, QObject* parent = nullptr );

    virtual QVariant data( int role ) const override;

    QSizeF drawSymbol( const QgsLegendSettings& settings, ItemContext* ctx, double itemHeight ) const override;

  private:
    QColor mColor;
    QString mLabel;
};

class QgsImageFetcher;

/** \ingroup core
 * Implementation of legend node interface for displaying WMS legend entries
 *
 * @note added in 2.8
 */
class CORE_EXPORT QgsWMSLegendNode : public QgsLayerTreeModelLegendNode
{
    Q_OBJECT

  public:
    QgsWMSLegendNode( QgsLayerTreeLayer* nodeLayer, QObject* parent = nullptr );

    virtual QVariant data( int role ) const override;

    virtual QSizeF drawSymbol( const QgsLegendSettings& settings, ItemContext* ctx, double itemHeight ) const override;

    virtual void invalidateMapBasedData() override;

  private slots:

    void getLegendGraphicFinished( const QImage& );
    void getLegendGraphicErrored( const QString& );
    void getLegendGraphicProgress( qint64, qint64 );

  private:

    // Lazily initializes mImage
    const QImage& getLegendGraphic() const;

    QImage renderMessage( const QString& msg ) const;

    QImage mImage;

    bool mValid;

    mutable QScopedPointer<QgsImageFetcher> mFetcher;
};

#endif // QGSLAYERTREEMODELLEGENDNODE_H
