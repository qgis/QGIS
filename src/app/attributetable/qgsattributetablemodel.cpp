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
#include "qgisapp.h"
#include "qgsattributeaction.h"
#include "qgsmapcanvas.h"

#include <QtGui>
#include <QVariant>
#include <limits>

////////////////////////////
// QgsAttributeTableModel //
////////////////////////////

QgsAttributeTableModel::QgsAttributeTableModel( QgsVectorLayer *theLayer, QObject *parent )
    : QAbstractTableModel( parent )
{
  mFeat.setFeatureId( std::numeric_limits<int>::min() );
  mLayer = theLayer;
  loadAttributes();

  connect( mLayer, SIGNAL( layerModified( bool ) ), this, SLOT( layerModified( bool ) ) );
  //connect(mLayer, SIGNAL(attributeValueChanged(int, int, const QVariant&)), this, SLOT( attributeValueChanged(int, int, const QVariant&)));
  //connect(mLayer, SIGNAL(featureDeleted(int)), this, SLOT( featureDeleted(int)));
  //connect(mLayer, SIGNAL(featureAdded(int)), this, SLOT( featureAdded(int)));

  loadLayer();
}

bool QgsAttributeTableModel::featureAtId( int fid ) const
{
  return mLayer->featureAtId( fid, mFeat, false, true );
}

#if 0
void QgsAttributeTableModel::featureDeleted( int fid )
{
  QgsDebugMsg( "entered." );

#ifdef QGISDEBUG
  int idx = mIdRowMap[fid];
  QgsDebugMsg( idx );
  QgsDebugMsg( fid );
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
#endif

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

  loadAttributes();
  loadLayer();
  emit modelChanged();
  emit headerDataChanged( Qt::Horizontal, 0, columnCount() - 1 );
}

void QgsAttributeTableModel::loadAttributes()
{
  if ( !mLayer )
  {
    return;
  }

  bool ins = false, rm = false;

  QgsAttributeList attributes;
  for ( QgsFieldMap::const_iterator it = mLayer->pendingFields().constBegin(); it != mLayer->pendingFields().end(); it++ )
  {
    switch ( mLayer->editType( it.key() ) )
    {
      case QgsVectorLayer::Hidden:
        continue;

      case QgsVectorLayer::ValueMap:
        mValueMaps.insert( it.key(), &mLayer->valueMap( it.key() ) );
        break;

      default:
        break;
    }

    attributes << it.key();
  }

  if ( mFieldCount < attributes.size() )
  {
    ins = true;
    beginInsertColumns( QModelIndex(), mFieldCount, attributes.size() - 1 );
  }
  else if ( attributes.size() < mFieldCount )
  {
    rm = true;
    beginRemoveColumns( QModelIndex(), attributes.size(), mFieldCount - 1 );
  }

  mFieldCount = attributes.size();
  mAttributes = attributes;
  mValueMaps.clear();

  if ( ins )
  {
    endInsertColumns();
  }
  else if ( rm )
  {
    endRemoveColumns();
  }
}

void QgsAttributeTableModel::loadLayer()
{
  QgsDebugMsg( "entered." );

  QgsFeature f;
  bool ins = false, rm = false;

  int previousSize = mRowIdMap.size();

  mRowIdMap.clear();
  mIdRowMap.clear();

  QSettings settings;
  int behaviour = settings.value( "/qgis/attributeTableBehaviour", 0 ).toInt();

  if ( behaviour == 1 )
  {
    const QgsFeatureList &features = mLayer->selectedFeatures();

    for ( int i = 0; i < features.size(); ++i )
    {
      mRowIdMap.insert( i, features[i].id() );
      mIdRowMap.insert( features[i].id(), i );
    }
  }
  else
  {
    QgsRectangle rect;
    if ( behaviour == 2 )
    {
      // current canvas only
      rect = QgisApp::instance()->mapCanvas()->extent();
    }

    mLayer->select( mAttributes, rect, false );

    for ( int i = 0; mLayer->nextFeature( f ); ++i )
    {
      mRowIdMap.insert( i, f.id() );
      mIdRowMap.insert( f.id(), i );
    }
  }

  if ( previousSize < mRowIdMap.size() )
  {
    QgsDebugMsg( "ins" );
    ins = true;
    beginInsertRows( QModelIndex(), previousSize, mRowIdMap.size() - 1 );
  }
  else if ( previousSize > mRowIdMap.size() )
  {
    QgsDebugMsg( "rm" );
    rm = true;
    beginRemoveRows( QModelIndex(), mRowIdMap.size(), previousSize - 1 );
  }

  // not needed when we have featureAdded signal
  mFieldCount = mAttributes.size();

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
    return std::numeric_limits<int>::min();
  }

  return mRowIdMap[id];
}

int QgsAttributeTableModel::fieldIdx( int col ) const
{
  return mAttributes[ col ];
}

int QgsAttributeTableModel::rowCount( const QModelIndex &parent ) const
{
  return mRowIdMap.size();
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
  else
  {
    return QVariant();
  }
}

void QgsAttributeTableModel::sort( int column, Qt::SortOrder order )
{
  QgsAttributeMap row;
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
    mSortList.append( QgsAttributeTableIdColumnPair( f.id(), row[ mAttributes[column] ] ) );
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
    mRowIdMap.insert( i, it->id() );
    mIdRowMap.insert( it->id(), i );
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

  int fieldId = mAttributes[ index.column()];

  QVariant::Type fldType = mLayer->pendingFields()[ fieldId ].type();
  bool fldNumeric = ( fldType == QVariant::Int || fldType == QVariant::Double );

  if ( role == Qt::TextAlignmentRole )
  {
    if ( fldNumeric )
      return QVariant( Qt::AlignRight );
    else
      return QVariant( Qt::AlignLeft );
  }

  // if we don't have the row in current cache, load it from layer first
  int rowId = rowToId( index.row() );
  if ( mFeat.id() != rowId )
  {
    if ( !featureAtId( rowId ) )
      return QVariant( "ERROR" );
  }

  if ( mFeat.id() != rowId )
    return QVariant( "ERROR" );

  const QVariant &val = mFeat.attributeMap()[ fieldId ];

  if ( val.isNull() )
  {
    // if the value is NULL, show that in table, but don't show "NULL" text in editor
    if ( role == Qt::EditRole )
    {
      return QVariant( fldType );
    }
    else
    {
      return QVariant( "NULL" );
    }
  }

  if ( role == Qt::DisplayRole && mValueMaps.contains( index.column() ) )
  {
    return mValueMaps[ index.column()]->key( val.toString(), QString( "(%1)" ).arg( val.toString() ) );
  }

  return val.toString();
}

bool QgsAttributeTableModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  if ( !index.isValid() || role != Qt::EditRole || !mLayer->isEditable() )
    return false;

  int rowId = rowToId( index.row() );
  if ( mFeat.id() == rowId || featureAtId( rowId ) )
  {
    int fieldId = mAttributes[ index.column()];

    disconnect( mLayer, SIGNAL( layerModified( bool ) ), this, SLOT( layerModified( bool ) ) );

    mLayer->beginEditCommand( tr( "Attribute changed" ) );
    mLayer->changeAttributeValue( rowId, fieldId, value, true );
    mLayer->endEditCommand();

    mFeat.changeAttribute( fieldId, value );

    connect( mLayer, SIGNAL( layerModified( bool ) ), this, SLOT( layerModified( bool ) ) );
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

  if ( mLayer->isEditable() &&
       mLayer->editType( mAttributes[ index.column()] ) != QgsVectorLayer::Immutable )
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

void QgsAttributeTableModel::executeAction( int action, const QModelIndex &idx ) const
{
  QList< QPair<QString, QString> > attributes;

  for ( int i = 0; i < mAttributes.size(); i++ )
  {
    attributes << QPair<QString, QString>(
      mLayer->pendingFields()[ mAttributes[i] ].name(),
      data( index( idx.row(), i ), Qt::EditRole ).toString()
    );
  }

  mLayer->actions()->doAction( action, attributes, fieldIdx( idx.column() ) );
}
