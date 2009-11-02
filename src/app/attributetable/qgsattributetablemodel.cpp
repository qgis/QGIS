/***************************************************************************
     QgsAttributeTableModel.cpp
     --------------------------------------
    Date                 : Feb 2009
    Copyright            : (C) 2009 Vita Cizek
    Email                : weetya (at) gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsattributetablemodel.h"
#include "qgsattributetablefiltermodel.h"

#include "qgsfield.h"
#include "qgsvectorlayer.h"
#include "qgslogger.h"

#include <QtGui>
#include <QVariant>

////////////////////////////
// QgsAttributeTableModel //
////////////////////////////

QgsAttributeTableModel::QgsAttributeTableModel( QgsVectorLayer *theLayer, QObject *parent )
    : QAbstractTableModel( parent )
{
  mLastRowId = -1;
  mLastRow = NULL;
  mLayer = theLayer;
  mFeatureCount = mLayer->pendingFeatureCount();
  mFieldCount = mLayer->pendingFields().size();
  mAttributes = mLayer->pendingAllAttributesList();

  connect( mLayer, SIGNAL( layerModified( bool ) ), this, SLOT( layerModified( bool ) ) );
  //connect(mLayer, SIGNAL(attributeAdded(int)), this, SLOT( attributeAdded(int)));
  //connect(mLayer, SIGNAL(attributeDeleted(int)), this, SLOT( attributeDeleted(int)));
  //connect(mLayer, SIGNAL(attributeValueChanged(int, int, const QVariant&)), this, SLOT( attributeValueChanged(int, int, const QVariant&)));
  //connect(mLayer, SIGNAL(featureDeleted(int)), this, SLOT( featureDeleted(int)));
  //connect(mLayer, SIGNAL(featureAdded(int)), this, SLOT( featureAdded(int)));

  loadLayer();
}

void QgsAttributeTableModel::featureDeleted( int fid )
{
  QgsDebugMsg( "entered." );

#ifdef QGISDEBUG
  int idx = mIdRowMap[fid];
  QgsDebugMsg( idx );
  QgsDebugMsg( fid );
#endif

#if 0
  --mFeatureCount;
  mIdRowMap.remove( fid );
  mRowIdMap.remove( idx );

  // fill the hole in the view
  if ( idx != mFeatureCount )
  {
    QgsDebugMsg( "jo" );
    //mRowIdMap[idx] = mRowIdMap[mFeatureCount];
    //mIdRowMap[mRowIdMap[idx]] = idx;
    int movedId = mRowIdMap[mFeatureCount];
    mRowIdMap.remove( mFeatureCount );
    mRowIdMap.insert( idx, movedId );
    mIdRowMap[movedId] = idx;
    //mIdRowMap.remove(mRowIdMap[idx]);
    //mIdRowMap.insert(mRowIdMap[idx], idx);
  }

  QgsDebugMsg( QString( "map sizes:%1, %2" ).arg( mRowIdMap.size() ).arg( mIdRowMap.size() ) );
  emit layoutChanged();
  //reload(index(0,0), index(rowCount(), columnCount()));
#endif

  QgsDebugMsg( "id->row" );
  QHash<int, int>::iterator it;
  for ( it = mIdRowMap.begin(); it != mIdRowMap.end(); ++it )
    QgsDebugMsg( QString( "%1->%2" ).arg( it.key() ).arg( *it ) );

  QgsDebugMsg( "row->id" );

  for ( it = mRowIdMap.begin(); it != mRowIdMap.end(); ++it )
    QgsDebugMsg( QString( "%1->%2" ).arg( it.key() ).arg( *it ) );

}

void QgsAttributeTableModel::featureAdded( int fid )
{
  QgsDebugMsg( "BM feature added" );
  ++mFeatureCount;
  mIdRowMap.insert( fid, mFeatureCount - 1 );
  mRowIdMap.insert( mFeatureCount - 1, fid );
  QgsDebugMsg( QString( "map sizes:%1, %2" ).arg( mRowIdMap.size() ).arg( mIdRowMap.size() ) );
  reload( index( 0, 0 ), index( rowCount(), columnCount() ) );
}

void QgsAttributeTableModel::attributeAdded( int idx )
{
  QgsDebugMsg( "BM attribute added" );
  loadLayer();
  QgsDebugMsg( QString( "map sizes:%1, %2" ).arg( mRowIdMap.size() ).arg( mIdRowMap.size() ) );
  reload( index( 0, 0 ), index( rowCount(), columnCount() ) );
  emit modelChanged();
}

void QgsAttributeTableModel::attributeDeleted( int idx )
{
  QgsDebugMsg( "BM attribute deleted" );
  loadLayer();
  QgsDebugMsg( QString( "map sizes:%1, %2" ).arg( mRowIdMap.size() ).arg( mIdRowMap.size() ) );
  reload( index( 0, 0 ), index( rowCount(), columnCount() ) );
  emit modelChanged();
}

void QgsAttributeTableModel::layerDeleted()
{
  QgsDebugMsg( "entered." );
  mIdRowMap.clear();
  mRowIdMap.clear();
  QgsDebugMsg( QString( "map sizes:%1, %2" ).arg( mRowIdMap.size() ).arg( mIdRowMap.size() ) );
  reload( index( 0, 0 ), index( rowCount(), columnCount() ) );
}

//TODO: check whether caching in data()/setData() doesn't cache old value
void QgsAttributeTableModel::attributeValueChanged( int fid, int idx, const QVariant &value )
{
  QgsDebugMsg( "entered." );
  reload( index( 0, 0 ), index( rowCount(), columnCount() ) );
}

void QgsAttributeTableModel::layerModified( bool onlyGeometry )
{
  if ( onlyGeometry )
    return;

  loadLayer();
  emit modelChanged();
  emit headerDataChanged ( Qt::Horizontal, 0, columnCount() - 1);
}

void QgsAttributeTableModel::loadLayer()
{
  QgsDebugMsg( "entered." );

  QgsFeature f;
  bool ins = false, rm = false;

  mRowIdMap.clear();
  mIdRowMap.clear();

  int pendingFeatureCount = mLayer->pendingFeatureCount();
  if ( mFeatureCount < pendingFeatureCount)
  {
    QgsDebugMsg( "ins" );
    ins = true;
    beginInsertRows( QModelIndex(), mFeatureCount, pendingFeatureCount - 1 );
// QgsDebugMsg(QString("%1, %2").arg(mFeatureCount).arg(mLayer->pendingFeatureCount() - 1));
  }
  else if ( mFeatureCount > pendingFeatureCount )
  {
    QgsDebugMsg( "rm" );
    rm = true;
    beginRemoveRows( QModelIndex(), pendingFeatureCount, mFeatureCount - 1 );
// QgsDebugMsg(QString("%1, %2").arg(mFeatureCount).arg(mLayer->pendingFeatureCount() -1));
  }

  mLayer->select( QgsAttributeList(), QgsRectangle(), false );

  // preallocate data before inserting
  mRowIdMap.reserve(pendingFeatureCount + 50);
  mIdRowMap.reserve(pendingFeatureCount + 50);

  for ( int i = 0; mLayer->nextFeature( f ); ++i )
  {
    mRowIdMap.insert( i, f.id() );
    mIdRowMap.insert( f.id(), i );
  }

  // not needed when we have featureAdded signal
  mFeatureCount = pendingFeatureCount;
  mFieldCount = mLayer->pendingFields().size();

  if ( ins )
  {
    endInsertRows();
    QgsDebugMsg( "end ins" );
  }
  else if ( rm )
  {
    endRemoveRows();
    QgsDebugMsg( "end rm" );
  }

#if 0
  QgsDebugMsg( "id->row" );
  QHash<int, int>::iterator it;
  for ( it = mIdRowMap.begin(); it != mIdRowMap.end(); ++it )
    QgsDebugMsg( QString( "%1->%2" ).arg( it.key() ).arg( *it ) );

  QgsDebugMsg( "row->id" );

  for ( it = mRowIdMap.begin(); it != mRowIdMap.end(); ++it )
    QgsDebugMsg( QString( "%1->%2" ).arg( it.key() ).arg( *it ) );
#endif
}

void QgsAttributeTableModel::swapRows( int a, int b )
{
  if ( a == b )
    return;

  int rowA = idToRow( a );
  int rowB = idToRow( b );

  //emit layoutAboutToBeChanged();

  mRowIdMap.remove( rowA );
  mRowIdMap.remove( rowB );
  mRowIdMap.insert( rowA, b );
  mRowIdMap.insert( rowB, a );

  mIdRowMap.remove( a );
  mIdRowMap.remove( b );
  mIdRowMap.insert( a, rowB );
  mIdRowMap.insert( b, rowA );

  //emit layoutChanged();
}

int QgsAttributeTableModel::idToRow( const int id ) const
{
  if ( !mIdRowMap.contains( id ) )
  {
    QgsDebugMsg( QString( "idToRow: id %1 not in the map" ).arg( id ) );
    return -1;
  }

  return mIdRowMap[id];
}

int QgsAttributeTableModel::rowToId( const int id ) const
{
  if ( !mRowIdMap.contains( id ) )
  {
    QgsDebugMsg( QString( "rowToId: row %1 not in the map" ).arg( id ) );
    // return negative infinite (to avoid collision with newly added features)
    return -999999;
  }

  return mRowIdMap[id];
}

int QgsAttributeTableModel::rowCount( const QModelIndex &parent ) const
{
  return mFeatureCount;
}

int QgsAttributeTableModel::columnCount( const QModelIndex &parent ) const
{
  return mFieldCount;
}

QVariant QgsAttributeTableModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  if ( role == Qt::DisplayRole )
  {
    if ( orientation == Qt::Vertical ) //row
    {
      return QVariant( section );
    }
    else
    {
      QString attributeName = mLayer->attributeAlias( mAttributes[section] );
      if ( attributeName.isEmpty() )
      {
        QgsField field = mLayer->pendingFields()[ mAttributes[section] ];
        attributeName = field.name();
      }
      return QVariant( attributeName );
    }
  }
  else return QVariant();
}

void QgsAttributeTableModel::sort( int column, Qt::SortOrder order )
{
  QgsAttributeMap row;
  QgsAttributeTableIdColumnPair pair;
  QgsAttributeList attrs;
  QgsFeature f;

  attrs.append( mAttributes[column] );

  emit layoutAboutToBeChanged();
// QgsDebugMsg("SORTing");

  mSortList.clear();
  mLayer->select( attrs, QgsRectangle(), false );
  while ( mLayer->nextFeature( f ) )
  {
    row = f.attributeMap();

    pair.id = f.id();
    pair.columnItem = row[ mAttributes[column] ];

    mSortList.append( pair );
  }

  if ( order == Qt::AscendingOrder )
    qStableSort( mSortList.begin(), mSortList.end() );
  else
    qStableSort( mSortList.begin(), mSortList.end(), qGreater<QgsAttributeTableIdColumnPair>() );

  // recalculate id<->row maps
  mRowIdMap.clear();
  mIdRowMap.clear();

  int i = 0;
  QList<QgsAttributeTableIdColumnPair>::Iterator it;
  for ( it = mSortList.begin(); it != mSortList.end(); ++it, ++i )
  {
    mRowIdMap.insert( i, it->id );
    mIdRowMap.insert( it->id, i );
  }

  // restore selection
  emit layoutChanged();
  //reset();
  emit modelChanged();
}

QVariant QgsAttributeTableModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() || ( role != Qt::TextAlignmentRole && role != Qt::DisplayRole && role != Qt::EditRole ) )
    return QVariant();

  QVariant::Type fldType = mLayer->pendingFields()[ mAttributes[index.column()] ].type();
  bool fldNumeric = ( fldType == QVariant::Int || fldType == QVariant::Double );

  if ( role == Qt::TextAlignmentRole )
  {
    if ( fldNumeric )
      return QVariant( Qt::AlignRight );
    else
      return QVariant( Qt::AlignLeft );
  }

  // if we don't have the row in current cache, load it from layer first
  if ( mLastRowId != rowToId( index.row() ) )
  {
    bool res = mLayer->featureAtId( rowToId( index.row() ), mFeat, false, true );

    if ( !res )
      return QVariant( "ERROR" );

    mLastRowId = rowToId( index.row() );
    mLastRow = ( QgsAttributeMap * ) & mFeat.attributeMap();
  }

  if ( !mLastRow )
    return QVariant( "ERROR" );

  QVariant& val = ( *mLastRow )[ mAttributes[index.column()] ];

  if ( val.isNull() )
  {
    // if the value is NULL, show that in table, but don't show "NULL" text in editor
    if ( role == Qt::EditRole )
      return QVariant();
    else
      return QVariant( "NULL" );
  }

  // force also numeric data for EditRole to be strings
  // otherwise it creates spinboxes instead of line edits
  // (probably not what we do want)
  if ( fldNumeric && role == Qt::EditRole )
    return val.toString();

  // convert to QString from some other representation
  // this prevents displaying greater numbers in exponential format
  return val.toString();
}

bool QgsAttributeTableModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  if ( !index.isValid() || role != Qt::EditRole )
    return false;

  if ( !mLayer->isEditable() )
    return false;

  bool res = mLayer->featureAtId( rowToId( index.row() ), mFeat, false, true );

  if ( res )
  {
    mLastRowId = rowToId( index.row() );
    mLastRow = ( QgsAttributeMap * )( &( mFeat.attributeMap() ) );
    mLayer->beginEditCommand( tr( "Attribute changed" ) );
    mLayer->changeAttributeValue( rowToId( index.row() ), mAttributes[ index.column()], value, true );
    mLayer->endEditCommand();
  }

  if ( !mLayer->isModified() )
    return false;

  emit dataChanged( index, index );
  return true;
}

Qt::ItemFlags QgsAttributeTableModel::flags( const QModelIndex &index ) const
{
  if ( !index.isValid() )
    return Qt::ItemIsEnabled;

  Qt::ItemFlags flags = QAbstractItemModel::flags( index );

  if ( mLayer->isEditable() )
    flags |= Qt::ItemIsEditable;

  return flags;
}

void QgsAttributeTableModel::reload( const QModelIndex &index1, const QModelIndex &index2 )
{
  emit dataChanged( index1, index2 );
}

void QgsAttributeTableModel::resetModel()
{
  reset();
}

void QgsAttributeTableModel::changeLayout()
{
  emit layoutChanged();
}

void QgsAttributeTableModel::incomingChangeLayout()
{
  emit layoutAboutToBeChanged();
}

