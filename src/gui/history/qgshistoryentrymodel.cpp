/***************************************************************************
                            qgshistoryentrymodel.cpp
                            --------------------------
    begin                : April 2023
    copyright            : (C) 2023 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgshistoryentrymodel.h"
#include "moc_qgshistoryentrymodel.cpp"
#include "qgshistoryentrynode.h"
#include "qgshistoryproviderregistry.h"
#include "qgsgui.h"
#include "qgshistoryentry.h"
#include "qgshistoryprovider.h"
#include "qgsapplication.h"

#include <QIcon>

///@cond PRIVATE
class QgsHistoryEntryRootNode : public QgsHistoryEntryGroup
{
  public:
    QVariant data( int = Qt::DisplayRole ) const override;

    void addEntryNode( const QgsHistoryEntry &entry, QgsHistoryEntryNode *node, QgsHistoryEntryModel *model );

    /**
     * Returns the date group and a sort key corresponding to the group for a
     * \a timestamp value.
     */
    static QString dateGroup( const QDateTime &timestamp, QString &sortKey );

    QgsHistoryEntryDateGroupNode *dateNode( const QDateTime &timestamp, QgsHistoryEntryModel *model );

  private:
    QMap<QString, QgsHistoryEntryDateGroupNode *> mDateGroupNodes;
};

class QgsHistoryEntryDateGroupNode : public QgsHistoryEntryGroup
{
  public:
    QgsHistoryEntryDateGroupNode( const QString &title, const QString &key )
      : mTitle( title )
      , mKey( key )
    {
    }

    QVariant data( int role = Qt::DisplayRole ) const override
    {
      switch ( role )
      {
        case Qt::DisplayRole:
        case Qt::ToolTipRole:
          return mTitle;

        case Qt::DecorationRole:
          return QgsApplication::getThemeIcon( QStringLiteral( "mIconFolder.svg" ) );

        default:
          break;
      }

      return QVariant();
    }

    QString mTitle;
    QString mKey;
};
///@endcond

QgsHistoryEntryModel::QgsHistoryEntryModel( const QString &providerId, Qgis::HistoryProviderBackends backends, QgsHistoryProviderRegistry *registry, const QgsHistoryWidgetContext &context, QObject *parent )
  : QAbstractItemModel( parent )
  , mContext( context )
  , mRegistry( registry ? registry : QgsGui::historyProviderRegistry() )
  , mProviderId( providerId )
  , mBackends( backends )
{
  mRootNode = std::make_unique<QgsHistoryEntryRootNode>();

  // populate with existing entries
  const QList<QgsHistoryEntry> entries = mRegistry->queryEntries( QDateTime(), QDateTime(), mProviderId, mBackends );
  for ( const QgsHistoryEntry &entry : entries )
  {
    QgsAbstractHistoryProvider *provider = mRegistry->providerById( entry.providerId );
    if ( !provider )
      continue;

    if ( QgsHistoryEntryNode *node = provider->createNodeForEntry( entry, mContext ) )
    {
      mIdToNodeHash.insert( entry.id, node );
      mRootNode->addEntryNode( entry, node, nullptr );
    }
  }

  connect( mRegistry, &QgsHistoryProviderRegistry::entryAdded, this, &QgsHistoryEntryModel::entryAdded );
  connect( mRegistry, &QgsHistoryProviderRegistry::entryUpdated, this, &QgsHistoryEntryModel::entryUpdated );
  connect( mRegistry, &QgsHistoryProviderRegistry::historyCleared, this, &QgsHistoryEntryModel::historyCleared );
}

QgsHistoryEntryModel::~QgsHistoryEntryModel()
{
}

int QgsHistoryEntryModel::rowCount( const QModelIndex &parent ) const
{
  QgsHistoryEntryNode *n = index2node( parent );
  if ( !n )
    return 0;

  return n->childCount();
}

int QgsHistoryEntryModel::columnCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent )
  return 1;
}

QModelIndex QgsHistoryEntryModel::index( int row, int column, const QModelIndex &parent ) const
{
  if ( column < 0 || column >= columnCount( parent ) || row < 0 || row >= rowCount( parent ) )
    return QModelIndex();

  QgsHistoryEntryGroup *n = dynamic_cast<QgsHistoryEntryGroup *>( index2node( parent ) );
  if ( !n )
    return QModelIndex(); // have no children

  return createIndex( row, column, n->childAt( row ) );
}

QModelIndex QgsHistoryEntryModel::parent( const QModelIndex &child ) const
{
  if ( !child.isValid() )
    return QModelIndex();

  if ( QgsHistoryEntryNode *n = index2node( child ) )
  {
    return indexOfParentNode( n->parent() ); // must not be null
  }
  else
  {
    Q_ASSERT( false );
    return QModelIndex();
  }
}

QVariant QgsHistoryEntryModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() || index.column() > 1 )
    return QVariant();

  QgsHistoryEntryNode *node = index2node( index );
  if ( !node )
    return QVariant();

  return node->data( role );
}

Qt::ItemFlags QgsHistoryEntryModel::flags( const QModelIndex &index ) const
{
  if ( !index.isValid() )
  {
    Qt::ItemFlags rootFlags = Qt::ItemFlags();
    return rootFlags;
  }

  Qt::ItemFlags f = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
  return f;
}

QgsHistoryEntryNode *QgsHistoryEntryModel::index2node( const QModelIndex &index ) const
{
  if ( !index.isValid() )
    return mRootNode.get();

  return reinterpret_cast<QgsHistoryEntryNode *>( index.internalPointer() );
}

void QgsHistoryEntryModel::entryAdded( long long id, const QgsHistoryEntry &entry, Qgis::HistoryProviderBackend backend )
{
  // ignore entries we don't care about
  if ( !( mBackends & backend ) )
    return;
  if ( !mProviderId.isEmpty() && entry.providerId != mProviderId )
    return;

  QgsAbstractHistoryProvider *provider = mRegistry->providerById( entry.providerId );
  if ( !provider )
    return;

  if ( QgsHistoryEntryNode *node = provider->createNodeForEntry( entry, mContext ) )
  {
    mIdToNodeHash.insert( id, node );
    mRootNode->addEntryNode( entry, node, this );
  }
}

void QgsHistoryEntryModel::entryUpdated( long long id, const QVariantMap &entry, Qgis::HistoryProviderBackend backend )
{
  // ignore entries we don't care about
  if ( !( mBackends & backend ) )
    return;

  // an update is a remove + reinsert operation
  if ( QgsHistoryEntryNode *node = mIdToNodeHash.value( id ) )
  {
    bool ok = false;
    QgsHistoryEntry historyEntry = mRegistry->entry( id, ok, backend );
    historyEntry.entry = entry;
    const QString providerId = historyEntry.providerId;
    QgsAbstractHistoryProvider *provider = mRegistry->providerById( providerId );
    if ( !provider )
      return;

    const QModelIndex nodeIndex = node2index( node );
    const int existingChildRows = node->childCount();
    provider->updateNodeForEntry( node, historyEntry, mContext );
    const int newChildRows = node->childCount();

    if ( newChildRows < existingChildRows )
    {
      beginRemoveRows( nodeIndex, newChildRows, existingChildRows - 1 );
      endRemoveRows();
    }
    else if ( existingChildRows < newChildRows )
    {
      beginInsertRows( nodeIndex, existingChildRows, newChildRows - 1 );
      endInsertRows();
    }

    const QModelIndex topLeft = index( 0, 0, nodeIndex );
    const QModelIndex bottomRight = index( newChildRows - 1, columnCount() - 1, nodeIndex );
    emit dataChanged( topLeft, bottomRight );
    emit dataChanged( nodeIndex, nodeIndex );
  }
}

void QgsHistoryEntryModel::historyCleared( Qgis::HistoryProviderBackend backend, const QString &providerId )
{
  // ignore entries we don't care about
  if ( !( mBackends & backend ) )
    return;

  if ( !mProviderId.isEmpty() && !providerId.isEmpty() && providerId != mProviderId )
    return;

  beginResetModel();
  mRootNode->clear();
  mIdToNodeHash.clear();
  endResetModel();
}

QModelIndex QgsHistoryEntryModel::node2index( QgsHistoryEntryNode *node ) const
{
  if ( !node || !node->parent() )
    return QModelIndex(); // this is the only root item -> invalid index

  QModelIndex parentIndex = node2index( node->parent() );

  int row = node->parent()->indexOf( node );
  Q_ASSERT( row >= 0 );
  return index( row, 0, parentIndex );
}

QModelIndex QgsHistoryEntryModel::indexOfParentNode( QgsHistoryEntryNode *parentNode ) const
{
  Q_ASSERT( parentNode );

  QgsHistoryEntryGroup *grandParentNode = parentNode->parent();
  if ( !grandParentNode )
    return QModelIndex(); // root node -> invalid index

  int row = grandParentNode->indexOf( parentNode );
  Q_ASSERT( row >= 0 );

  return createIndex( row, 0, parentNode );
}

//
// QgsHistoryEntryRootNode
//
///@cond PRIVATE
QVariant QgsHistoryEntryRootNode::data( int ) const
{
  return QVariant();
}

void QgsHistoryEntryRootNode::addEntryNode( const QgsHistoryEntry &entry, QgsHistoryEntryNode *node, QgsHistoryEntryModel *model )
{
  QgsHistoryEntryDateGroupNode *targetDateNode = dateNode( entry.timestamp, model );

  if ( model )
  {
    const QModelIndex dateNodeIndex = model->node2index( targetDateNode );
    model->beginInsertRows( dateNodeIndex, 0, 0 );
  }
  targetDateNode->insertChild( 0, node );
  if ( model )
  {
    model->endInsertRows();
  }
}

QString QgsHistoryEntryRootNode::dateGroup( const QDateTime &timestamp, QString &sortKey )
{
  QString groupString;
  if ( timestamp.date() == QDateTime::currentDateTime().date() )
  {
    groupString = QObject::tr( "Today" );
    sortKey = QStringLiteral( "0" );
  }
  else
  {
    const qint64 intervalDays = timestamp.date().daysTo( QDateTime::currentDateTime().date() );
    if ( intervalDays == 1 )
    {
      groupString = QObject::tr( "Yesterday" );
      sortKey = QStringLiteral( "1" );
    }
    else if ( intervalDays < 8 )
    {
      groupString = QObject::tr( "Last 7 days" );
      sortKey = QStringLiteral( "2" );
    }
    else
    {
      // a bit of trickiness here, we need dates ordered descending
      sortKey = QStringLiteral( "3: %1 %2" ).arg( QDate::currentDate().year() - timestamp.date().year(), 5, 10, QLatin1Char( '0' ) ).arg( 12 - timestamp.date().month(), 2, 10, QLatin1Char( '0' ) );
      groupString = timestamp.toString( QStringLiteral( "MMMM yyyy" ) );
    }
  }
  return groupString;
}

QgsHistoryEntryDateGroupNode *QgsHistoryEntryRootNode::dateNode( const QDateTime &timestamp, QgsHistoryEntryModel *model )
{
  QString dateGroupKey;
  const QString dateTitle = dateGroup( timestamp, dateGroupKey );

  QgsHistoryEntryDateGroupNode *node = mDateGroupNodes.value( dateGroupKey );
  if ( !node )
  {
    node = new QgsHistoryEntryDateGroupNode( dateTitle, dateGroupKey );
    mDateGroupNodes[dateGroupKey] = node;

    int targetIndex = 0;
    bool isInsert = false;
    for ( const auto &child : mChildren )
    {
      if ( QgsHistoryEntryDateGroupNode *candidateNode = dynamic_cast<QgsHistoryEntryDateGroupNode *>( child.get() ) )
      {
        if ( candidateNode->mKey > dateGroupKey )
        {
          isInsert = true;
          break;
        }
      }
      targetIndex++;
    }

    if ( isInsert )
    {
      if ( model )
      {
        model->beginInsertRows( QModelIndex(), targetIndex, targetIndex );
      }
      insertChild( targetIndex, node );
      if ( model )
      {
        model->endInsertRows();
      }
    }
    else
    {
      if ( model )
      {
        model->beginInsertRows( QModelIndex(), childCount(), childCount() );
      }
      addChild( node );
      if ( model )
      {
        model->endInsertRows();
      }
    }
  }

  return node;
}
///@endcond PRIVATE
