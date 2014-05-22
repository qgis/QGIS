/***************************************************************************
  qgslayertreemodel.h
  --------------------------------------
  Date                 : May 2014
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

#ifndef QGSLAYERTREEMODEL_H
#define QGSLAYERTREEMODEL_H

#include <QAbstractItemModel>
#include <QIcon>

class QgsLayerTreeNode;
class QgsLayerTreeGroup;
class QgsLayerTreeLayer;
class QgsMapLayer;

/** internal class, not in public API */
class QgsLayerTreeModelSymbologyNode : public QObject
{
    Q_OBJECT
  public:
    QgsLayerTreeModelSymbologyNode( QgsLayerTreeLayer* parent, const QString& name, const QIcon& icon = QIcon() )
        : mParent( parent ), mName( name ), mIcon( icon ) {}

    QgsLayerTreeLayer* parent() const { return mParent; }
    QString name() const { return mName; }
    QIcon icon() const { return mIcon; }

    // TODO: ref to renderer

  protected:
    QgsLayerTreeLayer* mParent;
    QString mName;
    QIcon mIcon;
};



class GUI_EXPORT QgsLayerTreeModel : public QAbstractItemModel
{
    Q_OBJECT
  public:
    explicit QgsLayerTreeModel( QgsLayerTreeGroup* rootNode, QObject *parent = 0 );
    ~QgsLayerTreeModel();

    int rowCount( const QModelIndex &parent = QModelIndex() ) const;
    int columnCount( const QModelIndex &parent = QModelIndex() ) const;
    QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const;
    QModelIndex parent( const QModelIndex &child ) const;
    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const;
    Qt::ItemFlags flags( const QModelIndex &index ) const;
    bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole );
    Qt::DropActions supportedDropActions() const;
    QStringList mimeTypes() const;
    QMimeData* mimeData( const QModelIndexList& indexes ) const;
    bool dropMimeData( const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent );

    bool removeRows( int row, int count, const QModelIndex& parent = QModelIndex() );

    enum Flag
    {
      // display flags
      ShowSymbology             = 0x0001,  //!< Add symbology items for layer nodes

      // behavioral flags
      AllowNodeReorder          = 0x1000,  //!< Allow reordering with drag'n'drop
      AllowNodeRename           = 0x2000,  //!< Allow renaming of groups and layers
      AllowNodeChangeVisibility = 0x4000,  //!< Allow user to set node visibility with a check box
    };
    Q_DECLARE_FLAGS( Flags, Flag )

    void setFlags( Flags f ) { mFlags = f; }
    void setFlag( Flag f, bool on = true ) { if ( on ) mFlags |= f; else mFlags &= ~f; }
    Flags flags() const { return mFlags; }
    bool testFlag( Flag f ) const { return mFlags.testFlag( f ); }

    // conversion functions used by views

    QgsLayerTreeNode* index2node( const QModelIndex& index ) const;
    static QgsLayerTreeModelSymbologyNode* index2symnode( const QModelIndex& index );
    QModelIndex node2index( QgsLayerTreeNode* node ) const;

    QList<QgsLayerTreeNode*> indexes2nodes( const QModelIndexList& list, bool skipInternal = false ) const;

    QgsLayerTreeGroup* rootGroup() { return mRootNode; }

    void refreshLayerSymbology( QgsLayerTreeLayer* nodeLayer );

    QgsLayerTreeNode* currentNode() const { return mCurrentNode; }
    void setCurrentNode( QgsLayerTreeNode* currentNode );

  signals:

  protected slots:
    void nodeWillAddChildren( QgsLayerTreeNode* node, int indexFrom, int indexTo );
    void nodeAddedChildren( QgsLayerTreeNode* node, int indexFrom, int indexTo );
    void nodeWillRemoveChildren( QgsLayerTreeNode* node, int indexFrom, int indexTo );
    void nodeRemovedChildren();

    void nodeVisibilityChanged( QgsLayerTreeNode* node );

    void nodeLayerLoaded();
    void layerRendererChanged();

    void layerNeedsUpdate();

  protected:
    void removeSymbologyFromLayer( QgsLayerTreeLayer* nodeLayer );
    void addSymbologyToLayer( QgsLayerTreeLayer* nodeL );
    void addSymbologyToVectorLayer( QgsLayerTreeLayer* nodeL );
    void addSymbologyToRasterLayer( QgsLayerTreeLayer* nodeL );
    void addSymbologyToPluginLayer( QgsLayerTreeLayer* nodeL );

    void connectToLayer( QgsLayerTreeLayer* nodeLayer );
    void disconnectFromLayer( QgsLayerTreeLayer* nodeLayer );

  protected:
    QgsLayerTreeGroup* mRootNode; // not owned!
    Flags mFlags;

    QMap<QgsLayerTreeLayer*, QList<QgsLayerTreeModelSymbologyNode*> > mSymbologyNodes;

    QgsLayerTreeNode* mCurrentNode;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsLayerTreeModel::Flags )

#endif // QGSLAYERTREEMODEL_H
