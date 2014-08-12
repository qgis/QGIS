/***************************************************************************
  qgslayertreemodellegendnode.h
  --------------------------------------
  Date                 : August 2014
  Copyright            : (C) 2014 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
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

class QgsLayerTreeLayer;
class QgsLegendSettings;
class QgsSymbolV2;


/**
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

    /** Return pointer to the parent layer node */
    QgsLayerTreeLayer* parent() const { return mParent; }

    /** Return item flags associated with the item. Default implementation returns Qt::ItemIsEnabled. */
    virtual Qt::ItemFlags flags() const;

    /** Return data associated with the item. Must be implemented in derived class. */
    virtual QVariant data( int role ) const = 0;

    /** Set some data associated with the item. Default implementation does nothing and returns false. */
    virtual bool setData( const QVariant& value, int role );

    virtual bool isEmbeddedInParent() const { return mEmbeddedInParent; }
    virtual void setEmbeddedInParent( bool embedded ) { mEmbeddedInParent = embedded; }

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
     * @param itemHeight Minimal height of the legend item - used for correct positioning when rendering
     * @return Real size of the symbol (may be bigger than "normal" symbol size from settings)
     */
    virtual QSizeF drawSymbol( const QgsLegendSettings& settings, ItemContext* ctx, double itemHeight ) const;

    /**
     * Draws label on the right side of the item
     * @param symbolSize  Real size of the associated symbol - used for correct positioning when rendering
     * @return Size of the label (may span multiple lines)
     */
    virtual QSizeF drawSymbolText( const QgsLegendSettings& settings, ItemContext* ctx, const QSizeF& symbolSize ) const;

  protected:
    /** Construct the node with pointer to its parent layer node */
    explicit QgsLayerTreeModelLegendNode( QgsLayerTreeLayer* nodeL );

  protected:
    QgsLayerTreeLayer* mParent;
    bool mEmbeddedInParent;
};

#include "qgslegendsymbolitemv2.h"

/**
 * Implementation of legend node interface for displaying preview of vector symbols and their labels
 * and allowing interaction with the symbol / renderer.
 *
 * @note added in 2.6
 */
class CORE_EXPORT QgsSymbolV2LegendNode : public QgsLayerTreeModelLegendNode
{
  public:
    QgsSymbolV2LegendNode( QgsLayerTreeLayer* nodeLayer, const QgsLegendSymbolItemV2& item );
    ~QgsSymbolV2LegendNode();

    virtual Qt::ItemFlags flags() const;
    virtual QVariant data( int role ) const;
    virtual bool setData( const QVariant& value, int role );

    /** Draws a symbol at the current y position and returns the new x position. Returns real symbol height, because for points,
     it is possible that it differs from mSymbolHeight */
    QSizeF drawSymbol( const QgsLegendSettings& settings, ItemContext* ctx, double itemHeight ) const;

    virtual void setEmbeddedInParent( bool embedded );

  private:
    void updateLabel();

  private:
    QgsLegendSymbolItemV2 mItem;
    mutable QIcon mIcon; // cached symbol preview
    QString mLabel;
};


/**
 * Implementation of legend node interface for displaying arbitrary label with icon.
 *
 * @note added in 2.6
 */
class CORE_EXPORT QgsSimpleLegendNode : public QgsLayerTreeModelLegendNode
{
  public:
    QgsSimpleLegendNode( QgsLayerTreeLayer* nodeLayer, const QString& label, const QIcon& icon = QIcon() );

    virtual QVariant data( int role ) const;

  private:
    QString mLabel;
    QIcon mIcon;
};


/**
 * Implementation of legend node interface for displaying arbitrary raster image
 *
 * @note added in 2.6
 */
class CORE_EXPORT QgsImageLegendNode : public QgsLayerTreeModelLegendNode
{
  public:
    QgsImageLegendNode( QgsLayerTreeLayer* nodeLayer, const QImage& img );

    virtual QVariant data( int role ) const;

    QSizeF drawSymbol( const QgsLegendSettings& settings, ItemContext* ctx, double itemHeight ) const;

  private:
    QImage mImage;
};

/**
 * Implementation of legend node interface for displaying raster legend entries
 *
 * @note added in 2.6
 */
class CORE_EXPORT QgsRasterSymbolLegendNode : public QgsLayerTreeModelLegendNode
{
  public:
    QgsRasterSymbolLegendNode( QgsLayerTreeLayer* nodeLayer, const QColor& color, const QString& label );

    virtual QVariant data( int role ) const;

    QSizeF drawSymbol( const QgsLegendSettings& settings, ItemContext* ctx, double itemHeight ) const;

  private:
    QColor mColor;
    QString mLabel;
};

#endif // QGSLAYERTREEMODELLEGENDNODE_H
