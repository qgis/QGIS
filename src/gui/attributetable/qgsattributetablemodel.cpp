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

#include "qgsapplication.h"
#include "qgsattributetablemodel.h"
#include "qgsattributetablefiltermodel.h"

#include "qgsattributeaction.h"
#include "qgseditorwidgetregistry.h"
#include "qgsexpression.h"
#include "qgsfield.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayeractionregistry.h"
#include "qgsmaplayerregistry.h"
#include "qgsrendererv2.h"
#include "qgsvectorlayer.h"

#include <QVariant>

#include <limits>

QgsAttributeTableModel::QgsAttributeTableModel( QgsVectorLayerCache *layerCache, QObject *parent )
    : QAbstractTableModel( parent )
    , mLayerCache( layerCache )
    , mFieldCount( 0 )
    , mCachedField( -1 )
{
  QgsDebugMsg( "entered." );

  if ( layerCache->layer()->geometryType() == QGis::NoGeometry )
  {
    mFeatureRequest.setFlags( QgsFeatureRequest::NoGeometry );
  }

  mFeat.setFeatureId( std::numeric_limits<int>::min() );

  if ( !layer()->hasGeometryType() )
    mFeatureRequest.setFlags( QgsFeatureRequest::NoGeometry );

  loadAttributes();

  connect( mLayerCache, SIGNAL( attributeValueChanged( QgsFeatureId, int, const QVariant& ) ), this, SLOT( attributeValueChanged( QgsFeatureId, int, const QVariant& ) ) );
  connect( layer(), SIGNAL( featureDeleted( QgsFeatureId ) ), this, SLOT( featureDeleted( QgsFeatureId ) ) );
  connect( layer(), SIGNAL( attributeDeleted( int ) ), this, SLOT( attributeDeleted( int ) ) );
  connect( layer(), SIGNAL( updatedFields() ), this, SLOT( updatedFields() ) );
  connect( layer(), SIGNAL( editCommandEnded() ), this, SLOT( editCommandEnded() ) );
  connect( mLayerCache, SIGNAL( featureAdded( QgsFeatureId ) ), this, SLOT( featureAdded( QgsFeatureId ) ) );
  connect( mLayerCache, SIGNAL( cachedLayerDeleted() ), this, SLOT( layerDeleted() ) );
}

bool QgsAttributeTableModel::loadFeatureAtId( QgsFeatureId fid ) const
{
  QgsDebugMsgLevel( QString( "loading feature %1" ).arg( fid ), 3 );

  if ( fid == std::numeric_limits<int>::min() )
  {
    return false;
  }

  return mLayerCache->featureAtId( fid, mFeat );
}

void QgsAttributeTableModel::featureDeleted( QgsFeatureId fid )
{
  QgsDebugMsgLevel( QString( "(%2) fid: %1" ).arg( fid ).arg( mFeatureRequest.filterType() ), 4 );
  mFieldCache.remove( fid );

  int row = idToRow( fid );

  if ( row != -1 )
  {
    beginRemoveRows( QModelIndex(), row, row );
    removeRow( row );
    endRemoveRows();
  }
}

bool QgsAttributeTableModel::removeRows( int row, int count, const QModelIndex &parent )
{
  Q_UNUSED( parent );
  QgsDebugMsgLevel( QString( "remove %2 rows at %1 (rows %3, ids %4)" ).arg( row ).arg( count ).arg( mRowIdMap.size() ).arg( mIdRowMap.size() ), 3 );

  // clean old references
  for ( int i = row; i < row + count; i++ )
  {
    mIdRowMap.remove( mRowIdMap[ i ] );
    mRowIdMap.remove( i );
  }

  // update maps
  int n = mRowIdMap.size() + count;
  for ( int i = row + count; i < n; i++ )
  {
    QgsFeatureId id = mRowIdMap[i];
    mIdRowMap[ id ] -= count;
    mRowIdMap[ i-count ] = id;
    mRowIdMap.remove( i );
  }

#ifdef QGISDEBUG
  QgsDebugMsgLevel( QString( "after removal rows %1, ids %2" ).arg( mRowIdMap.size() ).arg( mIdRowMap.size() ), 4 );
  QgsDebugMsgLevel( "id->row", 4 );
  for ( QHash<QgsFeatureId, int>::iterator it = mIdRowMap.begin(); it != mIdRowMap.end(); ++it )
    QgsDebugMsgLevel( QString( "%1->%2" ).arg( FID_TO_STRING( it.key() ) ).arg( *it ), 4 );

  QHash<QgsFeatureId, int>::iterator idit;

  QgsDebugMsgLevel( "row->id", 4 );
  for ( QHash<int, QgsFeatureId>::iterator it = mRowIdMap.begin(); it != mRowIdMap.end(); ++it )
    QgsDebugMsgLevel( QString( "%1->%2" ).arg( it.key() ).arg( FID_TO_STRING( *it ) ), 4 );
#endif

  Q_ASSERT( mRowIdMap.size() == mIdRowMap.size() );

  return true;
}

void QgsAttributeTableModel::featureAdded( QgsFeatureId fid )
{
  QgsDebugMsgLevel( QString( "(%2) fid: %1" ).arg( fid ).arg( mFeatureRequest.filterType() ), 4 );
  bool featOk = true;

  if ( mFeat.id() != fid )
    featOk = loadFeatureAtId( fid );

  if ( featOk && mFeatureRequest.acceptFeature( mFeat ) )
  {
    mFieldCache[ fid ] = mFeat.attribute( mCachedField );

    int n = mRowIdMap.size();
    beginInsertRows( QModelIndex(), n, n );

    mIdRowMap.insert( fid, n );
    mRowIdMap.insert( n, fid );

    endInsertRows();

    reload( index( rowCount() - 1, 0 ), index( rowCount() - 1, columnCount() ) );
  }
}

void QgsAttributeTableModel::updatedFields()
{
  QgsDebugMsg( "entered." );
  loadAttributes();
  emit modelChanged();
}

void QgsAttributeTableModel::editCommandEnded()
{
  reload( createIndex( mChangedCellBounds.top(), mChangedCellBounds.left() ),
          createIndex( mChangedCellBounds.bottom(), mChangedCellBounds.right() ) );

  mChangedCellBounds = QRect();
}

void QgsAttributeTableModel::attributeDeleted( int idx )
{
  if ( idx == mCachedField )
  {
    prefetchColumnData( -1 );
  }
}

void QgsAttributeTableModel::layerDeleted()
{
  QgsDebugMsg( "entered." );

  beginRemoveRows( QModelIndex(), 0, rowCount() - 1 );
  removeRows( 0, rowCount() );
  endRemoveRows();

  mAttributeWidgetCaches.clear();
  mAttributes.clear();
  mWidgetFactories.clear();
  mWidgetConfigs.clear();
}

void QgsAttributeTableModel::attributeValueChanged( QgsFeatureId fid, int idx, const QVariant &value )
{
  QgsDebugMsgLevel( QString( "(%4) fid: %1, idx: %2, value: %3" ).arg( fid ).arg( idx ).arg( value.toString() ).arg( mFeatureRequest.filterType() ), 3 );

  if ( idx == mCachedField )
    mFieldCache[ fid ] = value;

  // No filter request: skip all possibly heavy checks
  if ( mFeatureRequest.filterType() == QgsFeatureRequest::FilterNone )
  {
    setData( index( idToRow( fid ), fieldCol( idx ) ), value, Qt::EditRole );
  }
  else
  {
    if ( loadFeatureAtId( fid ) )
    {
      if ( mFeatureRequest.acceptFeature( mFeat ) )
      {
        if ( !mIdRowMap.contains( fid ) )
        {
          // Feature changed in such a way, it will be shown now
          featureAdded( fid );
        }
        else
        {
          // Update representation
          setData( index( idToRow( fid ), fieldCol( idx ) ), value, Qt::EditRole );
        }
      }
      else
      {
        if ( mIdRowMap.contains( fid ) )
        {
          // Feature changed such, that it is no longer shown
          featureDeleted( fid );
        }
        // else: we don't care
      }
    }
  }
}

void QgsAttributeTableModel::loadAttributes()
{
  if ( !layer() )
  {
    return;
  }

  bool ins = false, rm = false;

  QgsAttributeList attributes;
  const QgsFields& fields = layer()->pendingFields();

  mWidgetFactories.clear();
  mAttributeWidgetCaches.clear();
  mWidgetConfigs.clear();

  for ( int idx = 0; idx < fields.count(); ++idx )
  {
    const QString widgetType = layer()->editorWidgetV2( idx );
    QgsEditorWidgetFactory* widgetFactory = QgsEditorWidgetRegistry::instance()->factory( widgetType );
    if ( widgetFactory && widgetType != "Hidden" )
    {
      mWidgetFactories.append( widgetFactory );
      mWidgetConfigs.append( layer()->editorWidgetV2Config( idx ) );
      mAttributeWidgetCaches.append( widgetFactory->createCache( layer(), idx, mWidgetConfigs.last() ) );

      attributes << idx;
    }
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

  if ( rowCount() != 0 )
  {
    beginRemoveRows( QModelIndex(), 0, rowCount() - 1 );
    removeRows( 0, rowCount() );
    endRemoveRows();
  }

  QgsFeatureIterator features = mLayerCache->getFeatures( mFeatureRequest );

  int i = 0;

  QTime t;
  t.start();

  QgsFeature feat;
  while ( features.nextFeature( feat ) )
  {
    ++i;

    if ( t.elapsed() > 1000 )
    {
      bool cancel = false;
      emit progress( i, cancel );
      if ( cancel )
        break;

      t.restart();
    }
    mFeat = feat;
    featureAdded( feat.id() );
  }

  emit finished();

  mFieldCount = mAttributes.size();
}

void QgsAttributeTableModel::swapRows( QgsFeatureId a, QgsFeatureId b )
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

int QgsAttributeTableModel::idToRow( QgsFeatureId id ) const
{
  if ( !mIdRowMap.contains( id ) )
  {
    QgsDebugMsg( QString( "idToRow: id %1 not in the map" ).arg( id ) );
    return -1;
  }

  return mIdRowMap[id];
}

QModelIndex QgsAttributeTableModel::idToIndex( QgsFeatureId id ) const
{
  return index( idToRow( id ), 0 );
}

QModelIndexList QgsAttributeTableModel::idToIndexList( QgsFeatureId id ) const
{
  QModelIndexList indexes;

  int row = idToRow( id );
  for ( int column = 0; column < columnCount(); ++column )
  {
    indexes.append( index( row, column ) );
  }

  return indexes;
}

QgsFeatureId QgsAttributeTableModel::rowToId( const int row ) const
{
  if ( !mRowIdMap.contains( row ) )
  {
    QgsDebugMsg( QString( "rowToId: row %1 not in the map" ).arg( row ) );
    // return negative infinite (to avoid collision with newly added features)
    return std::numeric_limits<int>::min();
  }

  return mRowIdMap[row];
}

int QgsAttributeTableModel::fieldIdx( int col ) const
{
  return mAttributes[ col ];
}

int QgsAttributeTableModel::fieldCol( int idx ) const
{
  return mAttributes.indexOf( idx );
}

int QgsAttributeTableModel::rowCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent );
  return mRowIdMap.size();
}

int QgsAttributeTableModel::columnCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent );
  return qMax( 1, mFieldCount );  // if there are zero columns all model indices will be considered invalid
}

QVariant QgsAttributeTableModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  if ( !layer() )
    return QVariant();

  if ( role == Qt::DisplayRole )
  {
    if ( orientation == Qt::Vertical ) //row
    {
      return QVariant( section );
    }
    else if ( section >= 0 && section < mFieldCount )
    {
      QString attributeName = layer()->attributeAlias( mAttributes[section] );
      if ( attributeName.isEmpty() )
      {
        QgsField field = layer()->pendingFields()[ mAttributes[section] ];
        attributeName = field.name();
      }
      return QVariant( attributeName );
    }
    else
    {
      return tr( "feature id" );
    }
  }
  else
  {
    return QVariant();
  }
}

QVariant QgsAttributeTableModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() ||
       ( role != Qt::TextAlignmentRole
         && role != Qt::DisplayRole
         && role != Qt::EditRole
         && role != SortRole
         && role != FeatureIdRole
         && role != FieldIndexRole
       )
     )
    return QVariant();

  QgsFeatureId rowId = rowToId( index.row() );

  if ( role == FeatureIdRole )
    return rowId;

  if ( index.column() >= mFieldCount )
    return role == Qt::DisplayRole ? rowId : QVariant();

  int fieldId = mAttributes[ index.column()];

  if ( role == FieldIndexRole )
    return fieldId;

  const QgsField& field = layer()->pendingFields()[ fieldId ];

  QVariant::Type fldType = field.type();
  bool fldNumeric = ( fldType == QVariant::Int || fldType == QVariant::Double || fldType == QVariant::LongLong );

  if ( role == Qt::TextAlignmentRole )
  {
    if ( fldNumeric )
      return QVariant( Qt::AlignRight );
    else
      return QVariant( Qt::AlignLeft );
  }

  QVariant val;

  // if we don't have the row in current cache, load it from layer first
  if ( mCachedField == fieldId )
  {
    val = mFieldCache[ rowId ];
  }
  else
  {
    if ( mFeat.id() != rowId || !mFeat.isValid() )
    {
      if ( !loadFeatureAtId( rowId ) )
        return QVariant( "ERROR" );

      if ( mFeat.id() != rowId )
        return QVariant( "ERROR" );
    }

    val = mFeat.attribute( fieldId );
  }

  if ( role == Qt::DisplayRole )
  {
    return mWidgetFactories[ index.column()]->representValue( layer(), fieldId, mWidgetConfigs[ index.column()], mAttributeWidgetCaches[ index.column()], val );
  }

  return val;
}

bool QgsAttributeTableModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  Q_UNUSED( value )

  if ( !index.isValid() || index.column() >= mFieldCount || role != Qt::EditRole || !layer()->isEditable() )
    return false;

  if ( !layer()->isModified() )
    return false;

  if ( mChangedCellBounds.isNull() )
  {
    mChangedCellBounds = QRect( index.column(), index.row(), 1, 1 );
  }
  else
  {
    if ( index.column() < mChangedCellBounds.left() )
    {
      mChangedCellBounds.setLeft( index.column() );
    }
    if ( index.row() < mChangedCellBounds.top() )
    {
      mChangedCellBounds.setTop( index.row() );
    }
    if ( index.column() > mChangedCellBounds.right() )
    {
      mChangedCellBounds.setRight( index.column() );
    }
    if ( index.row() > mChangedCellBounds.bottom() )
    {
      mChangedCellBounds.setBottom( index.row() );
    }
  }

  return true;
}

Qt::ItemFlags QgsAttributeTableModel::flags( const QModelIndex &index ) const
{
  if ( !index.isValid() )
    return Qt::ItemIsEnabled;

  if ( index.column() >= mFieldCount )
    return Qt::NoItemFlags;

  Qt::ItemFlags flags = QAbstractItemModel::flags( index );

  if ( layer()->isEditable() &&
       layer()->fieldEditable( mAttributes[ index.column()] ) )
    flags |= Qt::ItemIsEditable;

  return flags;
}

void QgsAttributeTableModel::reload( const QModelIndex &index1, const QModelIndex &index2 )
{
  mFeat.setFeatureId( std::numeric_limits<int>::min() );
  emit dataChanged( index1, index2 );
}

void QgsAttributeTableModel::resetModel()
{
  beginResetModel();
  endResetModel();
}

void QgsAttributeTableModel::executeAction( int action, const QModelIndex &idx ) const
{
  QgsFeature f = feature( idx );
  layer()->actions()->doAction( action, f, fieldIdx( idx.column() ) );
}

void QgsAttributeTableModel::executeMapLayerAction( QgsMapLayerAction* action, const QModelIndex &idx ) const
{
  QgsFeature f = feature( idx );
  action->triggerForFeature( layer(), &f );
}

QgsFeature QgsAttributeTableModel::feature( const QModelIndex &idx ) const
{
  QgsFeature f;
  f.initAttributes( mAttributes.size() );
  f.setFeatureId( rowToId( idx.row() ) );
  for ( int i = 0; i < mAttributes.size(); i++ )
  {
    f.setAttribute( mAttributes[i], data( index( idx.row(), i ), Qt::EditRole ) );
  }

  return f;
}

void QgsAttributeTableModel::prefetchColumnData( int column )
{
  mFieldCache.clear();

  if ( column == -1 )
  {
    mCachedField = -1;
  }
  else
  {
    if ( column >= mAttributes.count() )
      return;
    int fieldId = mAttributes[ column ];
    const QgsFields& fields = layer()->pendingFields();
    QStringList fldNames;
    fldNames << fields[ fieldId ].name();

    QgsFeatureRequest r( mFeatureRequest );
    QgsFeatureIterator it = mLayerCache->getFeatures( r.setFlags( QgsFeatureRequest::NoGeometry ).setSubsetOfAttributes( fldNames, fields ) );

    QgsFeature f;
    while ( it.nextFeature( f ) )
    {
      mFieldCache.insert( f.id(), f.attribute( fieldId ) );
    }

    mCachedField = fieldId;
  }
}

void QgsAttributeTableModel::setRequest( const QgsFeatureRequest& request )
{
  mFeatureRequest = request;
  if ( layer() && !layer()->hasGeometryType() )
    mFeatureRequest.setFlags( mFeatureRequest.flags() | QgsFeatureRequest::NoGeometry );
}

const QgsFeatureRequest &QgsAttributeTableModel::request() const
{
  return mFeatureRequest;
}
