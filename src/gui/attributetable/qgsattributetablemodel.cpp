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
#include "qgsattributeaction.h"
#include "qgsmapcanvas.h"
#include "qgsrendererv2.h"
#include "qgsmaplayerregistry.h"
#include "qgsexpression.h"

#include <QtGui>
#include <QVariant>

#include <limits>

QgsAttributeTableModel::QgsAttributeTableModel( QgsVectorLayerCache *layerCache, QObject *parent )
    : QAbstractTableModel( parent )
    , mLayerCache( layerCache )
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

  connect( layer(), SIGNAL( attributeValueChanged( QgsFeatureId, int, const QVariant& ) ), this, SLOT( attributeValueChanged( QgsFeatureId, int, const QVariant& ) ) );
  connect( layer(), SIGNAL( featureAdded( QgsFeatureId ) ), this, SLOT( featureAdded( QgsFeatureId ) ) );
  connect( layer(), SIGNAL( featureDeleted( QgsFeatureId ) ), this, SLOT( featureDeleted( QgsFeatureId ) ) );
  connect( layer(), SIGNAL( updatedFields() ), this, SLOT( updatedFields() ) );
  connect( mLayerCache, SIGNAL( cachedLayerDeleted() ), this, SLOT( layerDeleted() ) );
}

QgsAttributeTableModel::~QgsAttributeTableModel()
{
  const QMap<QString, QVariant> *item;
  foreach ( item, mValueMaps )
  {
    delete item;
  }

  mValueMaps.clear();
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
  QgsDebugMsgLevel( QString( "deleted fid=%1 => row=%2" ).arg( fid ).arg( idToRow( fid ) ), 3 );

  int row = idToRow( fid );

  beginRemoveRows( QModelIndex(), row, row );
  removeRow( row );
  endRemoveRows();
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

void QgsAttributeTableModel::featureAdded( QgsFeatureId fid, bool newOperation )
{
  QgsDebugMsgLevel( QString( "feature %1 added (%2, rows %3, ids %4)" ).arg( fid ).arg( newOperation ).arg( mRowIdMap.size() ).arg( mIdRowMap.size() ), 4 );

  int n = mRowIdMap.size();
  if ( newOperation )
    beginInsertRows( QModelIndex(), n, n );

  mIdRowMap.insert( fid, n );
  mRowIdMap.insert( n, fid );

  if ( newOperation )
    endInsertRows();

  reload( index( rowCount() - 1, 0 ), index( rowCount() - 1, columnCount() ) );
}

void QgsAttributeTableModel::updatedFields()
{
  QgsDebugMsg( "entered." );
  loadAttributes();
  emit modelChanged();
}

void QgsAttributeTableModel::layerDeleted()
{
  QgsDebugMsg( "entered." );

  beginRemoveRows( QModelIndex(), 0, rowCount() - 1 );
  removeRows( 0, rowCount() );
  endRemoveRows();

  const QMap<QString, QVariant> *item;
  foreach ( item, mValueMaps )
  {
    delete item;
  }

  mValueMaps.clear();
}

void QgsAttributeTableModel::attributeValueChanged( QgsFeatureId fid, int idx, const QVariant &value )
{
  if ( fid == mFeat.id() )
  {
    mFeat.setValid( false );
  }
  setData( index( idToRow( fid ), fieldCol( idx ) ), value, Qt::EditRole );
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
  for ( int idx = 0; idx < fields.count(); ++idx )
  {
    switch ( layer()->editType( idx ) )
    {
      case QgsVectorLayer::Hidden:
        continue;

      case QgsVectorLayer::ValueMap:
        mValueMaps.insert( idx, &layer()->valueMap( idx ) );
        break;

      case QgsVectorLayer::ValueRelation:
      {
        const QgsVectorLayer::ValueRelationData &data =  layer()->valueRelation( idx );

        QgsVectorLayer *layer = qobject_cast<QgsVectorLayer*>( QgsMapLayerRegistry::instance()->mapLayer( data.mLayer ) );
        if ( !layer )
          continue;

        int ki = layer->fieldNameIndex( data.mKey );
        int vi = layer->fieldNameIndex( data.mValue );

        QgsExpression *e = 0;
        if ( !data.mFilterExpression.isEmpty() )
        {
          e = new QgsExpression( data.mFilterExpression );
          if ( e->hasParserError() || !e->prepare( layer->pendingFields() ) )
            continue;
        }

        if ( ki >= 0 && vi >= 0 )
        {
          QSet<int> attributes;
          attributes << ki << vi;

          QgsFeatureRequest::Flag flags = QgsFeatureRequest::NoGeometry;

          if ( e )
          {
            if ( e->needsGeometry() )
              flags = QgsFeatureRequest::NoFlags;

            foreach ( const QString &field, e->referencedColumns() )
            {
              int idx = layer->fieldNameIndex( field );
              if ( idx < 0 )
                continue;
              attributes << idx;
            }
          }

          QMap< QString, QVariant > *map = new QMap< QString, QVariant >();

          QgsFeatureIterator fit = layer->getFeatures( QgsFeatureRequest().setFlags( flags ).setSubsetOfAttributes( attributes.toList() ) );
          QgsFeature f;
          while ( fit.nextFeature( f ) )
          {
            if ( e && !e->evaluate( &f ).toBool() )
              continue;

            map->insert( f.attribute( vi ).toString(), f.attribute( ki ) );
          }

          mValueMaps.insert( idx, map );
        }
      }
      break;

      default:
        break;
    }

    attributes << idx;
  }

  if ( columnCount() < attributes.size() )
  {
    ins = true;
    beginInsertColumns( QModelIndex(), columnCount(), attributes.size() - 1 );
  }
  else if ( attributes.size() < columnCount() )
  {
    rm = true;
    beginRemoveColumns( QModelIndex(), attributes.size(), columnCount() - 1 );
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

  beginRemoveRows( QModelIndex(), 0, rowCount() - 1 );
  removeRows( 0, rowCount() );
  endRemoveRows();

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
      emit( progress( i, cancel ) );
      if ( cancel )
        break;

      t.restart();
    }
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
  bool fldNumeric = ( fldType == QVariant::Int || fldType == QVariant::Double );

  if ( role == Qt::TextAlignmentRole )
  {
    if ( fldNumeric )
      return QVariant( Qt::AlignRight );
    else
      return QVariant( Qt::AlignLeft );
  }

  // if we don't have the row in current cache, load it from layer first
  if ( mFeat.id() != rowId || !mFeat.isValid() )
  {
    if ( !loadFeatureAtId( rowId ) )
      return QVariant( "ERROR" );
  }

  if ( mFeat.id() != rowId )
    return QVariant( "ERROR" );

  const QVariant &val = mFeat.attribute( fieldId );

  if ( val.isNull() )
  {
    // if the value is NULL, show that in table, but don't show "NULL" text in editor
    if ( role == Qt::EditRole )
    {
      return QVariant( fldType );
    }
    else
    {
      QSettings settings;
      return settings.value( "qgis/nullValue", "NULL" );
    }
  }

  if ( role == Qt::DisplayRole )
  {
    if ( mValueMaps.contains( fieldId ) )
    {
      return mValueMaps[ fieldId ]->key( val.toString(), QString( "(%1)" ).arg( val.toString() ) );
    }

    if ( layer()->editType( fieldId ) == QgsVectorLayer::Calendar && val.canConvert( QVariant::Date ) )
    {
      return val.toDate().toString( layer()->dateFormat( fieldId ) );
    }
  }

  if ( role == SortRole )
  {
    return val;
  }

  return field.displayString( val );
}

bool QgsAttributeTableModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  Q_UNUSED( value )

  if ( !index.isValid() || index.column() >= mFieldCount || role != Qt::EditRole || !layer()->isEditable() )
    return false;

  if ( !layer()->isModified() )
    return false;

  emit dataChanged( index, index );

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
       layer()->editType( mAttributes[ index.column()] ) != QgsVectorLayer::Immutable )
    flags |= Qt::ItemIsEditable;

  return flags;
}

void QgsAttributeTableModel::reload( const QModelIndex &index1, const QModelIndex &index2 )
{
  for ( int row = index1.row(); row <= index2.row(); row++ )
  {
    QgsFeatureId fid = rowToId( row );
    mLayerCache->removeCachedFeature( fid );
  }

  mFeat.setFeatureId( std::numeric_limits<int>::min() );
  emit dataChanged( index1, index2 );
}

void QgsAttributeTableModel::resetModel()
{
  reset();
}

void QgsAttributeTableModel::executeAction( int action, const QModelIndex &idx ) const
{
  QgsFeature f = feature( idx );
  layer()->actions()->doAction( action, f, fieldIdx( idx.column() ) );
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
