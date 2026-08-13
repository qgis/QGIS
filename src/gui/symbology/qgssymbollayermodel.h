/***************************************************************************
    qgssymbollayermodel.h
    ---------------------
    begin                : June 2026
    copyright            : (C) 2026 by Valentin Buira
    email                : valentin dot buira at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSYMBOLLAYERMODEL_H
#define QGSSYMBOLLAYERMODEL_H

#include <deque>

#include "qgis.h"
#include "qgis_gui.h"
#include "qgsvectorlayer.h"

#include <QAbstractItemModel>

#define SIP_NO_FILE

class QgsSymbolLayerModelNode;
class QgsSymbol;
class QScreen;
class QgsSymbolLayer;


/**
 * \brief A node contained within a QgsSymbolLayerModel. May represent a symbol or a layer.
 * \warning Not part of stable API and may change in future QGIS releases.
 * \ingroup gui
 * \since QGIS 4.4
 */
class GUI_EXPORT QgsSymbolLayerModelNode
{
  public:
    /**
     * Constructor for QgsSymbolLayerModelNode for an empty node. e.g a root node.
     */
    QgsSymbolLayerModelNode();
    /**
     * Constructor for QgsSymbolLayerModelNode for a symbol layer node. e.g a child node of a symbol.
     * \note The ownership of the symbol layer is not transferred and must exist for the lifetime of the object.
     */
    QgsSymbolLayerModelNode( QgsSymbolLayer *layer, Qgis::SymbolType symbolType, QgsVectorLayer *vectorLayer, QScreen *screen );
    /**
     * Constructor for QgsSymbolLayerModelNode for a symbol node.
     * \note The ownership of the symbol is not transferred and must exist for the lifetime of the object.
     */
    QgsSymbolLayerModelNode( QgsSymbol *symbol, QgsVectorLayer *vectorLayer, QScreen *screen );
    ~QgsSymbolLayerModelNode();

    /**
     * Sets the \a layer associated with the node, and the \a symbolType of the QgsSymbol it belongs to.
     * \note The ownership of the symbol layer is not transferred and must exist for the lifetime of the object.
     */
    void setLayer( QgsSymbolLayer *layer, Qgis::SymbolType symbolType );

    /**
     * Sets the \a screen associated with the node, which is used to render the preview icons.
     */
    void setScreen( QScreen *screen );

    //! Returns TRUE if the node is a symbol layer. And otherwise, it is a symbol.
    bool isLayer() const { return mIsLayer; }

    //! Returns the item preview icon.
    QIcon icon() const;

    //! Returns the item's data for the given role.
    QVariant data( int role ) const;

    //! Returns the symbol pointer; helpful in determining a layer's parent symbol.
    QgsSymbol *symbol() { return mSymbol; }

    //! Returns the symbol layer pointer.
    QgsSymbolLayer *layer() { return mLayer; }

    /**
     * Adds a child \a node to this node, transferring ownership of the node.
     * to this node.
     */
    QgsSymbolLayerModelNode *addChildNode( std::unique_ptr<QgsSymbolLayerModelNode> node );

    /**
     * Inserts a child \a node to this node at \a index, transferring ownership of the node.
     * to this node.
     */
    QgsSymbolLayerModelNode *insertChildNode( int index, std::unique_ptr<QgsSymbolLayerModelNode> node );

    /**
     * Move a child \a node of the current node, the ownership of the moved node is transferred
     * to the current node.
     */
    void moveChildNode( QgsSymbolLayerModelNode *node, int to );

    /**
     * Remove a child \a node of the current node.
     */
    void removeChildNode( QgsSymbolLayerModelNode *node );

    /**
     * Deletes all child nodes from this node.
     */
    void deleteChildren();

    //! Returns the pointer of the parent node. If parent is NULLPTR, the node is a root node.
    QgsSymbolLayerModelNode *parent() { return mParent; }

    //! Returns TRUE if the node is a root node (i.e. has no parent).
    bool isRootNode() const { return mParent == nullptr; }

    /**
     * Returns the child node at the given \a index.
     */
    QgsSymbolLayerModelNode *childAt( int index ) const;

    /**
     * Returns the index position of the given \a node within the current node's children.
     */
    int indexOf( const QgsSymbolLayerModelNode *node );

    /**
     * Returns the number of rows(children) belonging to the node.
     */
    int rowCount() const;

    /**
     * Returns the row index of the node within its parent.
     */
    int rowIndex() const;

  private:
    /**
     * Sets the \a symbol associated with the node.
     * \note The ownership of the symbol is not transferred and must exist for the lifetime of the object.
     */
    void setSymbol( QgsSymbol *symbol );

    QgsSymbolLayer *mLayer = nullptr;
    QgsSymbol *mSymbol = nullptr;
    QPointer<QgsVectorLayer> mVectorLayer;
    bool mIsLayer = false;
    QSize mSize;
    Qgis::SymbolType mSymbolType = Qgis::SymbolType::Hybrid;
    QPointer<QScreen> mScreen;

    QgsSymbolLayerModelNode *mParent = nullptr;
    std::deque<std::unique_ptr<QgsSymbolLayerModelNode>> mChildren;

    Q_DISABLE_COPY( QgsSymbolLayerModelNode )

    friend class TestQgsSymbolLayerModel;
};

/**
 * \brief A QAbstractItemModel subclass for displaying a QgsSymbol in a tree view in the symbology panel.
 * \warning Not part of stable API and may change in future QGIS releases.
 * \ingroup gui
 * \since QGIS 4.4
 */
class GUI_EXPORT QgsSymbolLayerModel : public QAbstractItemModel
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsSymbolLayerModel, with the specified \a parent object.
     */
    QgsSymbolLayerModel( QgsVectorLayer *vl, QObject *parent SIP_TRANSFERTHIS = nullptr, QScreen *screen = nullptr );

    QVariant data( const QModelIndex &index, int role ) const override;
    int rowCount( const QModelIndex &parent = QModelIndex() ) const override;
    int columnCount( const QModelIndex & = QModelIndex() ) const override;
    QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const override;
    QModelIndex parent( const QModelIndex &child ) const override;

    /**
     * Updates the descendants of the given \a parent node with the given \a symbol.
     */
    void updateNode( QgsSymbol *symbol, QgsSymbolLayerModelNode *parent );

    /**
     * Moves the symbol layer \a node by the \a offset in the parent hierarchy.
     */
    void moveLayerByOffset( QgsSymbolLayerModelNode *node, int offset );

    /**
     * Duplicates the symbol layer \a node in the parent node hierarchy.
     */
    QgsSymbolLayerModelNode *duplicateLayer( QgsSymbolLayerModelNode *node );

    /**
     * Adds the symbol layer node inserted right before the \a idx.
     */
    QgsSymbolLayerModelNode *addLayer( QModelIndex index );

    /**
     * Removes the symbol layer \a node from it's parent node hierarchy.
     */
    void removeLayer( QgsSymbolLayerModelNode *node );

    /**
     * Change the symbol layer associated with \a node by a \a newLayer symbol layer.
     */
    void changeLayer( QgsSymbolLayerModelNode *node, QgsSymbolLayer *newLayer );

    /**
     * Updates the preview icon of the given \a node. And recursively updates the parents of the node.
     */
    void updatePreview( QgsSymbolLayerModelNode *node );

    /**
     * Sets the \a symbol associated with the model.
     * \note The ownership of the symbol is not transferred and must exist for the lifetime of the model.
     */
    void setSymbol( QgsSymbol *symbol );

    /**
     * Set the QScreen of the model itself. And traverses all the nodes in the model
     * to set their QScreen and update their preview icons.
     *
     * Used to determine the icon size on different dpi monitors.
     */
    void setScreen( QScreen *screen );

    /**
     * Returns the model index corresponding to the given \a node.
     * \see index2node()
     */
    QModelIndex node2index( QgsSymbolLayerModelNode *node ) const;

    /**
     * Returns the model node corresponding to the given \a index.
     * \see node2index()
     */
    QgsSymbolLayerModelNode *index2node( const QModelIndex &index ) const;

    //! Returns the root node of the model.
    QgsSymbolLayerModelNode *rootNode() const { return mRootNode.get(); }


  private:
    QModelIndex indexOfParentTreeNode( QgsSymbolLayerModelNode *parentNode ) const;
    void loadSymbol( QgsSymbol *symbol, QgsSymbolLayerModelNode *parent );
    void rebuild();

    std::unique_ptr<QgsSymbolLayerModelNode> mRootNode;

    QgsSymbol *mSymbol = nullptr;

    QPointer<QgsVectorLayer> mVectorLayer;

    QPointer<QScreen> mScreen;

    friend class TestQgsSymbolLayerModel;
};

#endif // QGSSYMBOLLAYERMODEL_H
