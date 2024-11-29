/****************************************************************************
**
** Copyright (C) 2009 Stephen Kelly <steveire@gmail.com>
** Contact: https://www.qt.io/licensing/
**
** This file is part of the test suite of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/
#include "dynamictreemodel.h"
#include <QtCore/QHash>
#include <QtCore/QList>
#include <QtCore/QTimer>
#include <QtCore/QDebug>
DynamicTreeModel::DynamicTreeModel( QObject *parent )
  : QAbstractItemModel( parent ), nextId( 1 )
{
}
QModelIndex DynamicTreeModel::index( int row, int column, const QModelIndex &parent ) const
{
  //   if (column != 0)
  //     return QModelIndex();
  if ( column < 0 || row < 0 )
    return QModelIndex();
  QList<QList<qint64>> childIdColumns = m_childItems.value( parent.internalId() );
  const qint64 grandParent = findParentId( parent.internalId() );
  if ( grandParent >= 0 )
  {
    QList<QList<qint64>> parentTable = m_childItems.value( grandParent );
    if ( parent.column() >= parentTable.size() )
      qFatal( "%s: parent.column() must be less than parentTable.size()", Q_FUNC_INFO );
    QList<qint64> parentSiblings = parentTable.at( parent.column() );
    if ( parent.row() >= parentSiblings.size() )
      qFatal( "%s: parent.row() must be less than parentSiblings.size()", Q_FUNC_INFO );
  }
  if ( childIdColumns.size() == 0 )
    return QModelIndex();
  if ( column >= childIdColumns.size() )
    return QModelIndex();
  QList<qint64> rowIds = childIdColumns.at( column );
  if ( row >= rowIds.size() )
    return QModelIndex();
  qint64 id = rowIds.at( row );
  return createIndex( row, column, reinterpret_cast<void *>( id ) );
}
qint64 DynamicTreeModel::findParentId( qint64 searchId ) const
{
  if ( searchId <= 0 )
    return -1;
  for ( auto i = m_childItems.cbegin(), end = m_childItems.cend(); i != end; ++i )
  {
    for ( const auto &list : i.value() )
    {
      if ( list.contains( searchId ) )
        return i.key();
    }
  }
  return -1;
}
QModelIndex DynamicTreeModel::parent( const QModelIndex &index ) const
{
  if ( !index.isValid() )
    return QModelIndex();
  qint64 searchId = index.internalId();
  qint64 parentId = findParentId( searchId );
  // Will never happen for valid index, but what the hey...
  if ( parentId <= 0 )
    return QModelIndex();
  qint64 grandParentId = findParentId( parentId );
  if ( grandParentId < 0 )
    grandParentId = 0;
  int column = 0;
  QList<qint64> childList = m_childItems.value( grandParentId ).at( column );
  int row = childList.indexOf( parentId );
  return createIndex( row, column, reinterpret_cast<void *>( parentId ) );
}
int DynamicTreeModel::rowCount( const QModelIndex &index ) const
{
  QList<QList<qint64>> cols = m_childItems.value( index.internalId() );
  if ( cols.size() == 0 )
    return 0;
  if ( index.column() > 0 )
    return 0;
  return cols.at( 0 ).size();
}
int DynamicTreeModel::columnCount( const QModelIndex &index ) const
{
  //   Q_UNUSED(index);
  return m_childItems.value( index.internalId() ).size();
}
QVariant DynamicTreeModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() )
    return QVariant();
  if ( Qt::DisplayRole == role )
    return m_items.value( index.internalId() );
  return QVariant();
}
void DynamicTreeModel::clear()
{
  beginResetModel();
  m_items.clear();
  m_childItems.clear();
  nextId = 1;
  endResetModel();
}
ModelChangeCommand::ModelChangeCommand( DynamicTreeModel *model, QObject *parent )
  : QObject( parent ), m_model( model ), m_numCols( 1 ), m_startRow( -1 ), m_endRow( -1 )
{
}
QModelIndex ModelChangeCommand::findIndex( const QList<int> &rows ) const
{
  const int col = 0;
  QModelIndex parent = QModelIndex();
  for ( int row : rows )
  {
    parent = m_model->index( row, col, parent );
    if ( !parent.isValid() )
      qFatal( "%s: parent must be valid", Q_FUNC_INFO );
  }
  return parent;
}
ModelInsertCommand::ModelInsertCommand( DynamicTreeModel *model, QObject *parent )
  : ModelChangeCommand( model, parent )
{
}
void ModelInsertCommand::doCommand()
{
  QModelIndex parent = findIndex( m_rowNumbers );
  m_model->beginInsertRows( parent, m_startRow, m_endRow );
  qint64 parentId = parent.internalId();
  for ( int row = m_startRow; row <= m_endRow; row++ )
  {
    for ( int col = 0; col < m_numCols; col++ )
    {
      if ( m_model->m_childItems[parentId].size() <= col )
        m_model->m_childItems[parentId].append( QList<qint64>() );
      //       QString name = QUuid::createUuid().toString();
      qint64 id = m_model->newId();
      QString name = QString::number( id );
      m_model->m_items.insert( id, name );
      m_model->m_childItems[parentId][col].insert( row, id );
    }
  }
  m_model->endInsertRows();
}
ModelMoveCommand::ModelMoveCommand( DynamicTreeModel *model, QObject *parent )
  : ModelChangeCommand( model, parent )
{
}
bool ModelMoveCommand::emitPreSignal( const QModelIndex &srcParent, int srcStart, int srcEnd, const QModelIndex &destParent, int destRow )
{
  return m_model->beginMoveRows( srcParent, srcStart, srcEnd, destParent, destRow );
}
void ModelMoveCommand::doCommand()
{
  QModelIndex srcParent = findIndex( m_rowNumbers );
  QModelIndex destParent = findIndex( m_destRowNumbers );
  if ( !emitPreSignal( srcParent, m_startRow, m_endRow, destParent, m_destRow ) )
    return;
  for ( int column = 0; column < m_numCols; ++column )
  {
    const QList<qint64> l = m_model->m_childItems.value( srcParent.internalId() )[column].mid(
      m_startRow, m_endRow - m_startRow + 1
    );
    for ( int i = m_startRow; i <= m_endRow; i++ )
      m_model->m_childItems[srcParent.internalId()][column].removeAt( m_startRow );
    int d;
    if ( m_destRow < m_startRow )
    {
      d = m_destRow;
    }
    else
    {
      if ( srcParent == destParent )
        d = m_destRow - ( m_endRow - m_startRow + 1 );
      else
        d = m_destRow - ( m_endRow - m_startRow ) + 1;
    }
    for ( qint64 id : l )
      m_model->m_childItems[destParent.internalId()][column].insert( d++, id );
  }
  emitPostSignal();
}
void ModelMoveCommand::emitPostSignal()
{
  m_model->endMoveRows();
}
ModelResetCommand::ModelResetCommand( DynamicTreeModel *model, QObject *parent )
  : ModelMoveCommand( model, parent )
{
}
bool ModelResetCommand::emitPreSignal( const QModelIndex &srcParent, int srcStart, int srcEnd, const QModelIndex &destParent, int destRow )
{
  Q_UNUSED( srcParent );
  Q_UNUSED( srcStart );
  Q_UNUSED( srcEnd );
  Q_UNUSED( destParent );
  Q_UNUSED( destRow );
  return true;
}
void ModelResetCommand::emitPostSignal()
{
  m_model->beginResetModel();
  m_model->endResetModel();
}
ModelResetCommandFixed::ModelResetCommandFixed( DynamicTreeModel *model, QObject *parent )
  : ModelMoveCommand( model, parent )
{
}
bool ModelResetCommandFixed::emitPreSignal( const QModelIndex &srcParent, int srcStart, int srcEnd, const QModelIndex &destParent, int destRow )
{
  Q_UNUSED( srcParent );
  Q_UNUSED( srcStart );
  Q_UNUSED( srcEnd );
  Q_UNUSED( destParent );
  Q_UNUSED( destRow );
  m_model->beginResetModel();
  return true;
}
void ModelResetCommandFixed::emitPostSignal()
{
  m_model->endResetModel();
}
ModelChangeChildrenLayoutsCommand::ModelChangeChildrenLayoutsCommand( DynamicTreeModel *model, QObject *parent )
  : ModelChangeCommand( model, parent )
{
}
void ModelChangeChildrenLayoutsCommand::doCommand()
{
  const QPersistentModelIndex parent1 = findIndex( m_rowNumbers );
  const QPersistentModelIndex parent2 = findIndex( m_secondRowNumbers );
  QList<QPersistentModelIndex> parents;
  parents << parent1;
  parents << parent2;
  emit m_model->layoutAboutToBeChanged( parents );
  int rowSize1 = -1;
  int rowSize2 = -1;
  for ( int column = 0; column < m_numCols; ++column )
  {
    {
      QList<qint64> &l = m_model->m_childItems[parent1.internalId()][column];
      rowSize1 = l.size();
      l.prepend( l.takeLast() );
    }
    {
      QList<qint64> &l = m_model->m_childItems[parent2.internalId()][column];
      rowSize2 = l.size();
      l.append( l.takeFirst() );
    }
  }
  // If we're changing one of the parent indexes, we need to ensure that we do that before
  // changing any children of that parent. The reason is that we're keeping parent1 and parent2
  // around as QPersistentModelIndex instances, and we query idx.parent() in the loop.
  QModelIndexList persistent = m_model->persistentIndexList();
  for ( const QPersistentModelIndex &parent : parents )
  {
    int idx = persistent.indexOf( parent );
    if ( idx != -1 )
      persistent.move( idx, 0 );
  }
  for ( const QModelIndex &idx : persistent )
  {
    if ( idx.parent() == parent1 )
    {
      if ( idx.row() == rowSize1 - 1 )
      {
        m_model->changePersistentIndex( idx, m_model->createIndex( 0, idx.column(), idx.internalPointer() ) );
      }
      else
      {
        m_model->changePersistentIndex( idx, m_model->createIndex( idx.row() + 1, idx.column(), idx.internalPointer() ) );
      }
    }
    else if ( idx.parent() == parent2 )
    {
      if ( idx.row() == 0 )
      {
        m_model->changePersistentIndex( idx, m_model->createIndex( rowSize2 - 1, idx.column(), idx.internalPointer() ) );
      }
      else
      {
        m_model->changePersistentIndex( idx, m_model->createIndex( idx.row() - 1, idx.column(), idx.internalPointer() ) );
      }
    }
  }
  emit m_model->layoutChanged( parents );
}
