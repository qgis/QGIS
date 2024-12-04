/***************************************************************************
  qgssettingstreemodel.h
  --------------------------------------
  Date                 : January 2023
  Copyright            : (C) 2023 by Denis Rouzaud
  Email                : denis@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSETTINGSTREEMODEL_H
#define QGSSETTINGSTREEMODEL_H


#include "qgis_sip.h"
#include "qgis_gui.h"

#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
#include <QItemDelegate>

class QgsSettingsEntryBase;
class QgsSettingsTreeNode;
class QgsSettingsTreeModel;
class QgsSettingsTreeNamedListNode;

#ifndef SIP_RUN

///@cond PRIVATE

/**
 * \ingroup gui
 * \class QgsSettingsTreeNodeData
 * \brief QgsSettingsTree holds data of the tree model for the settings tree.
 *
 * \note Not available in Python bindings
 *
 * \since QGIS 3.32
 */
class GUI_EXPORT QgsSettingsTreeModelNodeData : public QObject
{
    Q_OBJECT
  public:
    //! Type of tree element
    enum class Type
    {
      RootNode,
      TreeNode,
      NamedListTreeNode,
      NamedListItem,
      Setting
    };

    //! Constructor for the tree node data
    static QgsSettingsTreeModelNodeData *createRootNodeData( const QgsSettingsTreeNode *rootNode, QObject *parent );

    //! Apply changes to the settings
    void applyChanges();

    //! Returns if the node is the root node
    bool isRoot() const { return !mParent; }

    //! Returns the dynamic key parts of the named list parent tree nodes
    QStringList namedParentNodes() const { return mNamedParentNodes; }

    //! Returns the children nodes of the node (setting or tree node)
    QList<QgsSettingsTreeModelNodeData *> children() const { return mChildren; }

    //! Returns the parent of the node
    QgsSettingsTreeModelNodeData *parent() const { return mParent; }

    //! Returns the type of the node (setting or tree node)
    Type type() const { return mType; }

    //! Returns the name of the node (setting or tree node)
    QString name() const { return mName; }

    //! Returns the value of the node (setting or tree node)
    QVariant value() const { return mValue; }

    //! Returns the value of the node (setting or tree node)
    QVariant originalValue() const { return mOriginalValue; }

    //! Sets the \a value of the setting node
    bool setValue( const QVariant &value );

    //! Returns if the setting exists (value is set)
    bool exists() const { return mExists; }

    //! Returns if the setting is edited
    bool isEdited() const { return mIsEdited; }

    /**
     * Returns a pointer to the setting of the node or NULLPTR if the
     * setting does not exist.
     */
    const QgsSettingsEntryBase *setting() const { return mSetting; }

  private:
    //! Private constructor, use createRootNodeData() instead
    QgsSettingsTreeModelNodeData( QObject *parent )
      : QObject( parent ) {}
    void addChildForTreeNode( const QgsSettingsTreeNode *node );
    void addChildForNamedListItemNode( const QString &item, const QgsSettingsTreeNamedListNode *namedListNode );
    void addChildForSetting( const QgsSettingsEntryBase *setting );
    void fillChildren();

    Type mType = Type::TreeNode;
    QString mName;
    QVariant mValue;
    QVariant mOriginalValue;
    QStringList mNamedParentNodes;
    bool mExists = false;
    bool mIsEdited = false;

    QList<QgsSettingsTreeModelNodeData *> mChildren;
    QgsSettingsTreeModelNodeData *mParent = nullptr;

    const QgsSettingsTreeNode *mTreeNode = nullptr;
    const QgsSettingsEntryBase *mSetting = nullptr;
};


/**
 * \ingroup gui
 * \class QgsSettingsTreeItemDelegate
 * \brief QgsSettingsTreeItemDelegate allows editing the settings in the settings tree
 *
 * \note Not available in Python bindings
 *
 * \since QGIS 3.32
 */
class GUI_EXPORT QgsSettingsTreeItemDelegate : public QItemDelegate
{
    Q_OBJECT

  public:
    //! Constructor
    explicit QgsSettingsTreeItemDelegate( QgsSettingsTreeModel *model, QObject *parent = nullptr );

    QWidget *createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;
    void setEditorData( QWidget *editor, const QModelIndex &index ) const override;
    void setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const override;

  private:
    QgsSettingsTreeModel *mModel = nullptr;
};

///@endcond

#endif


/**
 * \ingroup gui
 * \class QgsSettingsTreeModel
 * \brief QgsSettingsTreeModel is a tree model for the settings tree.
 *
 * \since QGIS 3.32
 */
class GUI_EXPORT QgsSettingsTreeModel : public QAbstractItemModel
{
    Q_OBJECT
  public:
    //! Columns
    enum class Column
    {
      Name,        //!< Name
      Value,       //!< Value
      Description, //!< Description
    };

    //! Constructor
    QgsSettingsTreeModel( QgsSettingsTreeNode *rootNode = nullptr, QObject *parent = nullptr );

    ~QgsSettingsTreeModel();

    //! Apply pending changes in the model to the corresponding settings
    void applyChanges();

    /**
     * Returns settings tree node for given \a index or the root node if the index is invalid.
     */
    QgsSettingsTreeModelNodeData *index2node( const QModelIndex &index ) const SIP_SKIP;

    //! Returns the index from the settings tree node
    QModelIndex node2index( QgsSettingsTreeModelNodeData *node ) const SIP_SKIP;


    QModelIndex index( int row, int column, const QModelIndex &parent ) const override;
    QModelIndex parent( const QModelIndex &child ) const override;
    int rowCount( const QModelIndex &parent ) const override;
    int columnCount( const QModelIndex &parent ) const override;
    QVariant data( const QModelIndex &index, int role ) const override;
    QVariant headerData( int section, Qt::Orientation orientation, int role ) const override;
    Qt::ItemFlags flags( const QModelIndex &index ) const override;
    bool setData( const QModelIndex &index, const QVariant &value, int role ) override;

  private:
    QModelIndex indexOfParentSettingsTreeNode( QgsSettingsTreeModelNodeData *parentNode ) const;

    QgsSettingsTreeModelNodeData *mRootNode = nullptr;

    QColor mEditedColorBack;
    QColor mEditedColorFore;
    QColor mNotSetColor;
};

/**
 * \ingroup gui
 * \class QgsSettingsTreeProxyModel
 * \brief QgsSettingsTreeProxyModel allows filtering the settings tree
 *
 * \since QGIS 3.32
 */
class GUI_EXPORT QgsSettingsTreeProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
  public:
    //! Constructor
    QgsSettingsTreeProxyModel( QgsSettingsTreeNode *rootNode = nullptr, QObject *parent = nullptr );

    //! Apply pending changes in the model to the corresponding settings
    void applyChanges() { mSourceModel->applyChanges(); }

  public slots:
    //! Sets the filter text
    void setFilterText( const QString &filterText = QString() );


  protected:
    bool filterAcceptsRow( int source_row, const QModelIndex &source_parent ) const override;

  private:
    QgsSettingsTreeModel *mSourceModel = nullptr;

    bool nodeShown( QgsSettingsTreeModelNodeData *node ) const;
    QString mFilterText;
};

#endif // QGSSETTINGSTREEMODEL_H
