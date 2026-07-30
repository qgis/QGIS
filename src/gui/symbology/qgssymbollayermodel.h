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
 * \brief Abstract base class for nodes contained within a QgsSymbolLayerModel. May represent a symbol or a layer.
 * \warning Not part of stable API and may change in future QGIS releases.
 * \ingroup gui
 * \since QGIS 4.4
 */
class GUI_EXPORT QgsSymbolLayerModelNode : public QObject
{
    Q_OBJECT
  public:
    /**
     * Constructor for QgsSymbolLayerModelNode for an empty node. e.g a root node.
     */
    QgsSymbolLayerModelNode();
    /**
     * Constructor for QgsSymbolLayerModelNode for a symbol layer node. e.g a child node of a symbol.
     */
    QgsSymbolLayerModelNode( QgsSymbolLayer *layer, Qgis::SymbolType symbolType, QgsVectorLayer *vectorLayer, QScreen *screen );
    /**
     * Constructor for QgsSymbolLayerModelNode for a symbol node
     */
    QgsSymbolLayerModelNode( QgsSymbol *symbol, QgsVectorLayer *vectorLayer, QScreen *screen );
    ~QgsSymbolLayerModelNode() override;


    //! Returns whether the node is a symbol layer. And otherwise, it is a symbol.
    bool isLayer() const { return mIsLayer; }

    //! Returns the item preview icon.
    QIcon icon() const;

    //! Returns the item's data for the given role
    QVariant data( int role ) const;

    //! returns the symbol pointer; helpful in determining a layer's parent symbol
    QgsSymbol *symbol() { return mSymbol; }

    //! returns the symbol layer pointer
    QgsSymbolLayer *layer() { return mLayer; }

    /**
     * Adds a child \a node to this node, transferring ownership of the node
     * to this node.
     */
    void addChildNode( QgsSymbolLayerModelNode *node );

    /**
     * Deletes all child nodes from this node.
     */
    void deleteChildren();

    //! Returns whether the node should be shown as expanded or collapsed in GUI
    bool expanded() const;
    //! Sets whether the node should be shown as expanded or collapsed in GUI
    void setExpanded( bool expanded );

    //! Gets pointer to the parent. If parent is NULLPTR, the node is a root node
    QgsSymbolLayerModelNode *parent() { return mParent; }

    //! Returns whether the node is a root node (i.e. has no parent)
    bool isRootNode() const { return mParent == nullptr; }

    /**
     * Returns a list of children belonging to the node.
     */
    QList<QgsSymbolLayerModelNode *> children() { return mChildren; }

    /**
     * Returns a list of children belonging to the node.
     */
    QList<QgsSymbolLayerModelNode *> children() const { return mChildren; }


    /**
     * Returns the number of rows(children) belonging to the node.
     */
    int rowCount() const;

    /**
     * Returns the row index of the node within its parent.
     */
    int rowIndex() const;

  private:
    void setLayer( QgsSymbolLayer *layer, Qgis::SymbolType symbolType );
    void setSymbol( QgsSymbol *symbol );

    QgsSymbolLayer *mLayer = nullptr;
    QgsSymbol *mSymbol = nullptr;
    QPointer<QgsVectorLayer> mVectorLayer;
    bool mIsLayer = false;
    QSize mSize;
    Qgis::SymbolType mSymbolType = Qgis::SymbolType::Hybrid;
    QPointer<QScreen> mScreen;

    QgsSymbolLayerModelNode *mParent = nullptr;
    QList<QgsSymbolLayerModelNode *> mChildren;

    //! whether the node should be shown in GUI as expanded
    bool mExpanded = true;

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
     * Updates the preview icon of the given \a node. And recursively updates the parents of the node.
     */
    void updatePreview( QgsSymbolLayerModelNode *node );

    /**
     * Sets the \a symbol associated with the model.
     */
    void setSymbol( QgsSymbol *symbol );

    /**
     * Set the QScreen of the model and rebuild the model
     *
     * used to determine the icon size on different dpi monitor
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

    //! Returns the root node of the model
    QgsSymbolLayerModelNode *rootNode() const { return mRootNode.get(); }


  private:
    QModelIndex indexOfParentTreeNode( QgsSymbolLayerModelNode *parentNode ) const;
    void loadSymbol( QgsSymbol *symbol, QgsSymbolLayerModelNode *parent, bool update = false );
    void rebuild();

    std::unique_ptr<QgsSymbolLayerModelNode> mRootNode;

    QgsSymbol *mSymbol = nullptr;

    QPointer<QgsVectorLayer> mVectorLayer;

    QPointer<QScreen> mScreen;

    friend class TestQgsSymbolLayerModel;
};

#endif // QGSSYMBOLLAYERMODEL_H
