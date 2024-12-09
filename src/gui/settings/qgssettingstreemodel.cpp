/***************************************************************************
  qgssettingstreemodel.cpp
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

#include <QGuiApplication>
#include <QFont>

#include "qgssettingstreemodel.h"
#include "moc_qgssettingstreemodel.cpp"
#include "qgssettingsentry.h"
#include "qgssettingstreenode.h"
#include "qgssettingseditorwidgetwrapper.h"
#include "qgssettingseditorwidgetregistry.h"
#include "qgsgui.h"
#include "qgslogger.h"

///@cond PRIVATE


QgsSettingsTreeModelNodeData *QgsSettingsTreeModelNodeData::createRootNodeData( const QgsSettingsTreeNode *rootNode, QObject *parent = nullptr )
{
  QgsSettingsTreeModelNodeData *nodeData = new QgsSettingsTreeModelNodeData( parent );
  nodeData->mType = Type::RootNode;
  nodeData->mName = rootNode->key();
  nodeData->mTreeNode = rootNode;
  nodeData->fillChildren();
  return nodeData;
}

void QgsSettingsTreeModelNodeData::applyChanges()
{
  switch ( type() )
  {
    case Type::NamedListTreeNode:
    case Type::RootNode:
    case Type::TreeNode:
    case Type::NamedListItem:
    {
      QList<QgsSettingsTreeModelNodeData *>::iterator it = mChildren.begin();
      for ( ; it != mChildren.end(); ++it )
        ( *it )->applyChanges();
      break;
    }
    case Type::Setting:
    {
      if ( isEdited() )
      {
        setting()->setVariantValue( mValue, mNamedParentNodes );
      }
      break;
    }
  }
}

bool QgsSettingsTreeModelNodeData::setValue( const QVariant &value )
{
  Q_ASSERT( mType == Type::Setting );
  if ( !value.isValid() && mValue.isValid() )
  {
    mExists = false;
    mIsEdited = true;
    mValue = QVariant();
  }
  else if ( !mValue.isValid() || mValue != value )
  {
    mValue = value;
    mIsEdited = ( value != mOriginalValue );
  }
  // TODO: check the value of setting is fulfilling the settings' contsraints ?
  return true;
}


void QgsSettingsTreeModelNodeData::addChildForTreeNode( const QgsSettingsTreeNode *node )
{
  QgsSettingsTreeModelNodeData *nodeData = new QgsSettingsTreeModelNodeData( this );
  nodeData->mParent = this;
  nodeData->mNamedParentNodes = mNamedParentNodes;
  nodeData->mName = node->key();
  nodeData->mTreeNode = node;
  if ( node->type() == Qgis::SettingsTreeNodeType::NamedList )
  {
    nodeData->mType = Type::NamedListTreeNode;
    const QgsSettingsTreeNamedListNode *nln = dynamic_cast<const QgsSettingsTreeNamedListNode *>( node );
    const QStringList items = nln->items( mNamedParentNodes );
    for ( const QString &item : items )
    {
      nodeData->addChildForNamedListItemNode( item, nln );
    }
  }
  else
  {
    nodeData->mType = Type::TreeNode;
    nodeData->fillChildren();
  }
  mChildren.append( nodeData );
}

void QgsSettingsTreeModelNodeData::addChildForNamedListItemNode( const QString &item, const QgsSettingsTreeNamedListNode *namedListNode )
{
  QgsSettingsTreeModelNodeData *nodeData = new QgsSettingsTreeModelNodeData( this );
  nodeData->mType = Type::NamedListItem;
  nodeData->mParent = this;
  nodeData->mNamedParentNodes = mNamedParentNodes;
  nodeData->mNamedParentNodes.append( item );
  nodeData->mName = item;
  nodeData->mTreeNode = namedListNode;
  nodeData->fillChildren();
  mChildren.append( nodeData );
}

void QgsSettingsTreeModelNodeData::addChildForSetting( const QgsSettingsEntryBase *setting )
{
  QgsSettingsTreeModelNodeData *nodeData = new QgsSettingsTreeModelNodeData( this );
  nodeData->mType = Type::Setting;
  nodeData->mParent = this;
  nodeData->mNamedParentNodes = mNamedParentNodes;
  nodeData->mSetting = setting;
  nodeData->mName = setting->name();
  nodeData->mValue = setting->valueAsVariant( mNamedParentNodes );
  nodeData->mOriginalValue = nodeData->mValue;
  nodeData->mExists = setting->exists( mNamedParentNodes );

  switch ( mNamedParentNodes.count() )
  {
    case 1:
      QgsDebugMsgLevel( QString( "getting %1 with %2" ).arg( setting->definitionKey(), mNamedParentNodes.at( 0 ) ), 3 );
      break;
    case 2:
      QgsDebugMsgLevel( QString( "getting %1 with %2 and %3" ).arg( setting->definitionKey(), mNamedParentNodes.at( 0 ), mNamedParentNodes.at( 1 ) ), 3 );
      break;
    case 0:
      QgsDebugMsgLevel( QString( "getting %1" ).arg( setting->definitionKey() ), 3 );
      break;
    default:
      Q_ASSERT( false );
      QgsDebugError( QString( "Not handling that many named parent nodes for %1" ).arg( setting->definitionKey() ) );
      break;
  }

  mChildren.append( nodeData );
}

void QgsSettingsTreeModelNodeData::fillChildren()
{
  const QList<QgsSettingsTreeNode *> childrenNodes = mTreeNode->childrenNodes();
  for ( const QgsSettingsTreeNode *childNode : childrenNodes )
  {
    addChildForTreeNode( childNode );
  }
  const QList<const QgsSettingsEntryBase *> childrenSettings = mTreeNode->childrenSettings();
  for ( const QgsSettingsEntryBase *setting : childrenSettings )
  {
    addChildForSetting( setting );
  }
}

///@endcond


QgsSettingsTreeModel::QgsSettingsTreeModel( QgsSettingsTreeNode *rootNode, QObject *parent )
  : QAbstractItemModel( parent )
{
  mRootNode = QgsSettingsTreeModelNodeData::createRootNodeData( rootNode, this );

  QPalette pal = qApp->palette();
  mEditedColorBack = pal.color( QPalette::Active, QPalette::Dark );
  mEditedColorFore = pal.color( QPalette::Active, QPalette::BrightText );
  mNotSetColor = pal.color( QPalette::Disabled, QPalette::WindowText );
}

QgsSettingsTreeModel::~QgsSettingsTreeModel()
{
  //delete mRootNode;
}

void QgsSettingsTreeModel::applyChanges()
{
  beginResetModel();
  mRootNode->applyChanges();
  endResetModel();
}

QgsSettingsTreeModelNodeData *QgsSettingsTreeModel::index2node( const QModelIndex &index ) const
{
  if ( !index.isValid() )
    return mRootNode;

  QObject *obj = reinterpret_cast<QObject *>( index.internalPointer() );
  return qobject_cast<QgsSettingsTreeModelNodeData *>( obj );
}

QModelIndex QgsSettingsTreeModel::node2index( QgsSettingsTreeModelNodeData *node ) const
{
  if ( !node || !node->parent() )
    return QModelIndex(); // this is the only root item -> invalid index

  QModelIndex parentIndex = node2index( node->parent() );

  int row = node->parent()->children().indexOf( node );
  Q_ASSERT( row >= 0 );
  return index( row, static_cast<int>( Column::Name ), parentIndex );
}


QModelIndex QgsSettingsTreeModel::index( int row, int column, const QModelIndex &parent ) const
{
  if ( column < 0 || column >= columnCount( parent ) || row < 0 || row >= rowCount( parent ) )
    return QModelIndex();

  QgsSettingsTreeModelNodeData *n = index2node( parent );
  if ( !n )
    return QModelIndex(); // have no children


  return createIndex( row, column, static_cast<QObject *>( n->children().at( row ) ) );
}

QModelIndex QgsSettingsTreeModel::parent( const QModelIndex &child ) const
{
  if ( !child.isValid() )
    return QModelIndex();

  if ( QgsSettingsTreeModelNodeData *n = index2node( child ) )
  {
    return indexOfParentSettingsTreeNode( n->parent() ); // must not be null
  }
  else
  {
    Q_ASSERT( false ); // no other node types!
    return QModelIndex();
  }
}

QModelIndex QgsSettingsTreeModel::indexOfParentSettingsTreeNode( QgsSettingsTreeModelNodeData *parentNode ) const
{
  Q_ASSERT( parentNode );

  const QgsSettingsTreeModelNodeData *grandParentNode = parentNode->parent();
  if ( !grandParentNode )
    return QModelIndex(); // root node -> invalid index

  int row = grandParentNode->children().indexOf( parentNode );
  Q_ASSERT( row >= 0 );

  return createIndex( row, 0, static_cast<QObject *>( parentNode ) );
}

int QgsSettingsTreeModel::rowCount( const QModelIndex &parent ) const
{
  QgsSettingsTreeModelNodeData *n = index2node( parent );
  if ( !n )
    return 0;

  return n->children().count();
}

int QgsSettingsTreeModel::columnCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent )
  return 3;
}

QVariant QgsSettingsTreeModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() || index.column() > columnCount( index ) )
    return QVariant();

  QgsSettingsTreeModelNodeData *node = index2node( index );

  if ( role == Qt::ForegroundRole && node->type() == QgsSettingsTreeModelNodeData::Type::Setting )
  {
    if ( !node->exists() )
    {
      // settings not yet set
      return mNotSetColor;
    }

    if ( node->isEdited() && ( node->setting()->settingsType() != Qgis::SettingsType::Color || index.column() != static_cast<int>( Column::Value ) ) )
    {
      return mEditedColorFore;
    }
  }

  if ( role == Qt::BackgroundRole && node->type() == QgsSettingsTreeModelNodeData::Type::Setting )
  {
    // background for edited settings (except colors)
    if ( node->isEdited() && ( node->setting()->settingsType() != Qgis::SettingsType::Color || index.column() != static_cast<int>( Column::Value ) ) )
    {
      return mEditedColorBack;
    }
  }

  switch ( static_cast<Column>( index.column() ) )
  {
    case Column::Name:
    {
      if ( role == Qt::DisplayRole || role == Qt::EditRole )
      {
        return node->name();
      }
      break;
    }

    case Column::Value:
    {
      if ( role == Qt::CheckStateRole )
      {
        if ( node->type() == QgsSettingsTreeModelNodeData::Type::Setting && node->setting()->settingsType() == Qgis::SettingsType::Bool )
        {
          // special handling of bool setting to show combobox
          return node->value().toBool() ? Qt::Checked : Qt::Unchecked;
        }
      }
      if ( role == Qt::DisplayRole || role == Qt::EditRole )
      {
        if ( node->type() == QgsSettingsTreeModelNodeData::Type::Setting && node->setting()->settingsType() == Qgis::SettingsType::Bool )
        {
          // special handling of bool setting to show combobox
          return QString();
        }
        else
        {
          return node->value();
        }
      }
      else if ( role == Qt::BackgroundRole )
      {
        if ( node->type() == QgsSettingsTreeModelNodeData::Type::Setting )
        {
          switch ( node->setting()->settingsType() )
          {
            case Qgis::SettingsType::Custom:
            case Qgis::SettingsType::Variant:
            case Qgis::SettingsType::String:
            case Qgis::SettingsType::StringList:
            case Qgis::SettingsType::VariantMap:
            case Qgis::SettingsType::Bool:
            case Qgis::SettingsType::Integer:
            case Qgis::SettingsType::Double:
            case Qgis::SettingsType::EnumFlag:
              break;

            case Qgis::SettingsType::Color:
              return node->value();
          }
        }
      }
      break;
    }

    case Column::Description:
    {
      if ( node->type() == QgsSettingsTreeModelNodeData::Type::Setting )
      {
        if ( role == Qt::DisplayRole || role == Qt::EditRole )
        {
          return node->setting()->description();
        }
      }
      break;
    }

    default:
      break;
  }
  return QVariant();
}


QVariant QgsSettingsTreeModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  if ( orientation == Qt::Orientation::Horizontal && role == Qt::DisplayRole )
  {
    switch ( static_cast<Column>( section ) )
    {
      case Column::Name:
        return tr( "Name" );
      case Column::Value:
        return tr( "Value" );
      case Column::Description:
        return tr( "Description" );
    };
  }

  return QVariant();
}

Qt::ItemFlags QgsSettingsTreeModel::flags( const QModelIndex &index ) const
{
  if ( index.column() == static_cast<int>( Column::Value ) )
  {
    QgsSettingsTreeModelNodeData *nodeData = index2node( index );
    if ( nodeData->type() == QgsSettingsTreeModelNodeData::Type::Setting )
    {
      if ( nodeData->setting()->settingsType() == Qgis::SettingsType::Bool )
      {
        return Qt::ItemIsUserCheckable | Qt::ItemIsEnabled;
      }
      else
      {
        return Qt::ItemIsEnabled | Qt::ItemIsEditable;
      }
    }
  }
  else
  {
    return Qt::ItemIsEnabled;
  }
  return Qt::NoItemFlags;
}

bool QgsSettingsTreeModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  if ( index.column() == static_cast<int>( Column::Value ) )
  {
    if ( role == Qt::EditRole || role == Qt::CheckStateRole )
    {
      QgsSettingsTreeModelNodeData *nodeData = index2node( index );
      if ( nodeData->type() == QgsSettingsTreeModelNodeData::Type::Setting )
      {
        if ( role == Qt::CheckStateRole )
        {
          Q_ASSERT( nodeData->setting()->settingsType() == Qgis::SettingsType::Bool );
          nodeData->setValue( value == Qt::Checked ? true : false );
        }
        else
        {
          nodeData->setValue( value );
        }
        emit dataChanged( index.siblingAtColumn( 0 ), index.siblingAtColumn( columnCount( index.parent() ) - 1 ) );
        return true;
      }
    }
  }
  return false;
}


///@cond PRIVATE


QgsSettingsTreeItemDelegate::QgsSettingsTreeItemDelegate( QgsSettingsTreeModel *model, QObject *parent )
  : QItemDelegate( parent )
  , mModel( model )
{
}

QWidget *QgsSettingsTreeItemDelegate::createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  Q_UNUSED( option )
  if ( static_cast<QgsSettingsTreeModel::Column>( index.column() ) == QgsSettingsTreeModel::Column::Value )
  {
    QModelIndex sourceIndex = index;
    const QgsSettingsTreeProxyModel *proxyModel = qobject_cast<const QgsSettingsTreeProxyModel *>( index.model() );
    if ( proxyModel )
    {
      sourceIndex = proxyModel->mapToSource( index );
    }
    QgsSettingsTreeModelNodeData *nodeData = mModel->index2node( sourceIndex );
    if ( nodeData->type() == QgsSettingsTreeModelNodeData::Type::Setting )
    {
      return QgsGui::settingsEditorWidgetRegistry()->createEditor( nodeData->setting(), nodeData->namedParentNodes(), parent );
    }
  }
  return nullptr;
}

void QgsSettingsTreeItemDelegate::setEditorData( QWidget *editor, const QModelIndex &index ) const
{
  QgsSettingsEditorWidgetWrapper *eww = QgsSettingsEditorWidgetWrapper::fromWidget( editor );
  if ( eww )
  {
    const QVariant value = index.model()->data( index, Qt::DisplayRole );
    eww->setWidgetFromVariant( value );
  }
}

void QgsSettingsTreeItemDelegate::setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const
{
  QgsSettingsEditorWidgetWrapper *eww = QgsSettingsEditorWidgetWrapper::fromWidget( editor );
  if ( eww )
    model->setData( index, eww->variantValueFromWidget(), Qt::EditRole );
}

///@endcond


QgsSettingsTreeProxyModel::QgsSettingsTreeProxyModel( QgsSettingsTreeNode *rootNode, QObject *parent )
  : QSortFilterProxyModel( parent )
{
  mSourceModel = new QgsSettingsTreeModel( rootNode, parent );
  QSortFilterProxyModel::setSourceModel( mSourceModel );
}

void QgsSettingsTreeProxyModel::setFilterText( const QString &filterText )
{
  if ( filterText == mFilterText )
    return;

  mFilterText = filterText;
  invalidateFilter();
}

bool QgsSettingsTreeProxyModel::filterAcceptsRow( int source_row, const QModelIndex &source_parent ) const
{
  QgsSettingsTreeModelNodeData *node = mSourceModel->index2node( mSourceModel->index( source_row, static_cast<int>( QgsSettingsTreeModel::Column::Name ), source_parent ) );
  return nodeShown( node );
}

bool QgsSettingsTreeProxyModel::nodeShown( QgsSettingsTreeModelNodeData *node ) const
{
  if ( !node )
    return false;
  if ( node->type() == QgsSettingsTreeModelNodeData::Type::Setting )
  {
    if ( node->name().contains( mFilterText, Qt::CaseInsensitive ) )
      return true;

    // also returns settings for which the parent nodes have a match
    QModelIndex index = mSourceModel->node2index( node ).parent();
    while ( ( index.isValid() ) )
    {
      QgsSettingsTreeModelNodeData *parentNode = mSourceModel->index2node( mSourceModel->index( index.row(), static_cast<int>( QgsSettingsTreeModel::Column::Name ), index.parent() ) );
      if ( parentNode->name().contains( mFilterText, Qt::CaseInsensitive ) )
        return true;

      index = index.parent();
    }
    return false;
  }
  else
  {
    // show all children if name of node matches
    if ( node->name().contains( mFilterText, Qt::CaseInsensitive ) )
      return true;

    const auto constChildren = node->children();
    for ( QgsSettingsTreeModelNodeData *child : constChildren )
    {
      if ( nodeShown( child ) )
      {
        return true;
      }
    }
    return false;
  }
}
