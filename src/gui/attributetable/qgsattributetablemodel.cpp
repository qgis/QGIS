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

#include "qgsactionmanager.h"
#include "qgseditorwidgetregistry.h"
#include "qgsexpression.h"
#include "qgsconditionalstyle.h"
#include "qgsfield.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayeractionregistry.h"
#include "qgsmaplayerregistry.h"
#include "qgsrendererv2.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"
#include "qgssymbollayerv2utils.h"

#include <QVariant>

#include <limits>

QgsAttributeTableModel::QgsAttributeTableModel( QgsVectorLayerCache *layerCache, QObject *parent )
    : QAbstractTableModel( parent )
    , mLayerCache( layerCache )
    , mFieldCount( 0 )
    , mSortCacheExpression( "" )
    , mSortFieldIndex( -1 )
    , mExtraColumns( 0 )
{
  mExpressionContext << QgsExpressionContextUtils::globalScope()
  << QgsExpressionContextUtils::projectScope()
  << QgsExpressionContextUtils::layerScope( layerCache->layer() );

  if ( layerCache->layer()->geometryType() == QGis::NoGeometry )
  {
    mFeatureRequest.setFlags( QgsFeatureRequest::NoGeometry );
  }

  mFeat.setFeatureId( std::numeric_limits<int>::min() );

  if ( !layer()->hasGeometryType() )
    mFeatureRequest.setFlags( QgsFeatureRequest::NoGeometry );

  loadAttributes();

  connect( mLayerCache, SIGNAL( attributeValueChanged( QgsFeatureId, int, const QVariant& ) ), this, SLOT( attributeValueChanged( QgsFeatureId, int, const QVariant& ) ) );
  connect( layer(), SIGNAL( featuresDeleted( QgsFeatureIds ) ), this, SLOT( featuresDeleted( QgsFeatureIds ) ) );
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

int QgsAttributeTableModel::extraColumns() const
{
  return mExtraColumns;
}

void QgsAttributeTableModel::setExtraColumns( int extraColumns )
{
  mExtraColumns = extraColumns;
  loadAttributes();
}

void QgsAttributeTableModel::featuresDeleted( const QgsFeatureIds& fids )
{
  QList<int> rows;

  Q_FOREACH ( QgsFeatureId fid, fids )
  {
    QgsDebugMsgLevel( QString( "(%2) fid: %1, size: %3" ).arg( fid ).arg( mFeatureRequest.filterType() ).arg( mIdRowMap.size() ), 4 );

    int row = idToRow( fid );
    if ( row != -1 )
      rows << row;
  }

  qSort( rows );

  int lastRow = -1;
  int beginRow = -1;
  int currentRowCount = 0;
  int removedRows = 0;
  bool reset = false;

  Q_FOREACH ( int row, rows )
  {
#if 0
    qDebug() << "Row: " << row << ", begin " << beginRow << ", last " << lastRow << ", current " << currentRowCount << ", removed " << removedRows;
#endif
    if ( lastRow == -1 )
    {
      beginRow = row;
    }

    if ( row != lastRow + 1 && lastRow != -1 )
    {
      if ( rows.count() > 100 && currentRowCount < 10 )
      {
        reset = true;
        break;
      }
      removeRows( beginRow - removedRows, currentRowCount );

      beginRow = row;
      removedRows += currentRowCount;
      currentRowCount = 0;
    }

    currentRowCount++;

    lastRow = row;
  }

  if ( !reset )
    removeRows( beginRow - removedRows, currentRowCount );
  else
    resetModel();
}

bool QgsAttributeTableModel::removeRows( int row, int count, const QModelIndex &parent )
{
  if ( row < 0 || count < 1 )
    return false;

  beginRemoveRows( parent, row, row + count - 1 );
#ifdef QGISDEBUG
  if ( 3 <= QgsLogger::debugLevel() )
    QgsDebugMsgLevel( QString( "remove %2 rows at %1 (rows %3, ids %4)" ).arg( row ).arg( count ).arg( mRowIdMap.size() ).arg( mIdRowMap.size() ), 3 );
#endif

  // clean old references
  for ( int i = row; i < row + count; i++ )
  {
    mSortCache.remove( mRowIdMap[i] );
    mIdRowMap.remove( mRowIdMap[i] );
    mRowIdMap.remove( i );
  }

  // update maps
  int n = mRowIdMap.size() + count;
  for ( int i = row + count; i < n; i++ )
  {
    QgsFeatureId id = mRowIdMap[i];
    mIdRowMap[id] -= count;
    mRowIdMap[i-count] = id;
    mRowIdMap.remove( i );
  }

#ifdef QGISDEBUG
  if ( 4 <= QgsLogger::debugLevel() )
  {
    QgsDebugMsgLevel( QString( "after removal rows %1, ids %2" ).arg( mRowIdMap.size() ).arg( mIdRowMap.size() ), 4 );
    QgsDebugMsgLevel( "id->row", 4 );
    for ( QHash<QgsFeatureId, int>::const_iterator it = mIdRowMap.constBegin(); it != mIdRowMap.constEnd(); ++it )
      QgsDebugMsgLevel( QString( "%1->%2" ).arg( FID_TO_STRING( it.key() ) ).arg( *it ), 4 );

    QgsDebugMsgLevel( "row->id", 4 );
    for ( QHash<int, QgsFeatureId>::const_iterator it = mRowIdMap.constBegin(); it != mRowIdMap.constEnd(); ++it )
      QgsDebugMsgLevel( QString( "%1->%2" ).arg( it.key() ).arg( FID_TO_STRING( *it ) ), 4 );
  }
#endif

  Q_ASSERT( mRowIdMap.size() == mIdRowMap.size() );

  endRemoveRows();

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
    if ( mSortFieldIndex == -1 )
    {
      mExpressionContext.setFeature( mFeat );
      mSortCache[mFeat.id()] = mSortCacheExpression.evaluate( &mExpressionContext );
    }
    else
    {
      QgsEditorWidgetFactory* widgetFactory = mWidgetFactories.at( mSortFieldIndex );
      const QVariant& widgetCache = mAttributeWidgetCaches.at( mSortFieldIndex );
      const QgsEditorWidgetConfig& widgetConfig = mWidgetConfigs.at( mSortFieldIndex );
      QVariant sortValue = widgetFactory->representValue( layer(), mSortFieldIndex, widgetConfig, widgetCache, mFeat.attribute( mSortFieldIndex ) );
      mSortCache.insert( mFeat.id(), sortValue );
    }

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
  if ( mSortCacheAttributes.contains( idx ) )
    prefetchSortData( "" );
}

void QgsAttributeTableModel::layerDeleted()
{
  removeRows( 0, rowCount() );

  mAttributeWidgetCaches.clear();
  mAttributes.clear();
  mWidgetFactories.clear();
  mWidgetConfigs.clear();
}

void QgsAttributeTableModel::attributeValueChanged( QgsFeatureId fid, int idx, const QVariant &value )
{
  QgsDebugMsgLevel( QString( "(%4) fid: %1, idx: %2, value: %3" ).arg( fid ).arg( idx ).arg( value.toString() ).arg( mFeatureRequest.filterType() ), 3 );

  if ( mSortCacheAttributes.contains( idx ) )
  {
    if ( mSortFieldIndex == -1 )
    {
      loadFeatureAtId( fid );
      mExpressionContext.setFeature( mFeat );
      mSortCache[fid] = mSortCacheExpression.evaluate( &mExpressionContext );
    }
    else
    {
      QgsEditorWidgetFactory* widgetFactory = mWidgetFactories.at( mSortFieldIndex );
      const QVariant& widgetCache = mAttributeWidgetCaches.at( mSortFieldIndex );
      const QgsEditorWidgetConfig& widgetConfig = mWidgetConfigs.at( mSortFieldIndex );
      QVariant sortValue = widgetFactory->representValue( layer(), mSortFieldIndex, widgetConfig, widgetCache, value );
      mSortCache.insert( fid, sortValue );
    }
  }
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
          featuresDeleted( QgsFeatureIds() << fid );
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
  const QgsFields& fields = layer()->fields();

  mWidgetFactories.clear();
  mAttributeWidgetCaches.clear();
  mWidgetConfigs.clear();

  for ( int idx = 0; idx < fields.count(); ++idx )
  {
    const QString widgetType = layer()->editFormConfig()->widgetType( idx );
    QgsEditorWidgetFactory* widgetFactory = QgsEditorWidgetRegistry::instance()->factory( widgetType );
    if ( widgetFactory )
    {
      mWidgetFactories.append( widgetFactory );
      mWidgetConfigs.append( layer()->editFormConfig()->widgetConfig( idx ) );
      mAttributeWidgetCaches.append( widgetFactory->createCache( layer(), idx, mWidgetConfigs.last() ) );

      attributes << idx;
    }
  }

  if ( mFieldCount + mExtraColumns < attributes.size() + mExtraColumns )
  {
    ins = true;
    beginInsertColumns( QModelIndex(), mFieldCount + mExtraColumns, attributes.size() - 1 );
  }
  else if ( attributes.size() + mExtraColumns < mFieldCount + mExtraColumns )
  {
    rm = true;
    beginRemoveColumns( QModelIndex(), attributes.size(), mFieldCount + mExtraColumns - 1 );
  }

  mFieldCount = attributes.size();
  mAttributes = attributes;

  if ( mSortFieldIndex >= mAttributes.count() )
    mSortFieldIndex = -1;

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
  // make sure attributes are properly updated before caching the data
  // (emit of progress() signal may enter event loop and thus attribute
  // table view may be updated with inconsistent model which may assume
  // wrong number of attributes)
  loadAttributes();

  beginResetModel();

  if ( rowCount() != 0 )
  {
    removeRows( 0, rowCount() );
  }

  QgsFeatureIterator features = mLayerCache->getFeatures( mFeatureRequest );

  int i = 0;

  QTime t;
  t.start();

  while ( features.nextFeature( mFeat ) )
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
    featureAdded( mFeat.id() );
  }

  emit finished();

  connect( mLayerCache, SIGNAL( invalidated() ), this, SLOT( loadLayer() ), Qt::UniqueConnection );

  endResetModel();
}

void QgsAttributeTableModel::fieldConditionalStyleChanged( const QString &fieldName )
{
  if ( fieldName.isNull() )
  {
    mRowStylesMap.clear();
    emit dataChanged( index( 0, 0 ), index( rowCount() - 1, columnCount() - 1 ) );
    return;
  }

  int fieldIndex = mLayerCache->layer()->fieldNameIndex( fieldName );
  if ( fieldIndex == -1 )
    return;

  //whole column has changed
  int col = fieldCol( fieldIndex );
  emit dataChanged( index( 0, col ), index( rowCount() - 1, col ) );
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
  int columns = columnCount();
  indexes.reserve( columns );
  for ( int column = 0; column < columns; ++column )
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
  return mAttributes[col];
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
  return qMax( 1, mFieldCount + mExtraColumns );  // if there are zero columns all model indices will be considered invalid
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
      QString attributeName = layer()->attributeAlias( mAttributes.at( section ) );
      if ( attributeName.isEmpty() )
      {
        QgsField field = layer()->fields().at( mAttributes.at( section ) );
        attributeName = field.name();
      }
      return QVariant( attributeName );
    }
    else
    {
      return tr( "extra column" );
    }
  }
  else if ( role == Qt::ToolTipRole )
  {
    if ( orientation == Qt::Vertical )
    {
      // TODO show DisplayExpression
      return tr( "Feature ID: %1" ).arg( rowToId( section ) );
    }
    else
    {
      QgsField field = layer()->fields().at( mAttributes.at( section ) );
      return field.name();
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
         && role != Qt::BackgroundColorRole
         && role != Qt::TextColorRole
         && role != Qt::DecorationRole
         && role != Qt::FontRole
       )
     )
    return QVariant();

  QgsFeatureId rowId = rowToId( index.row() );

  if ( role == FeatureIdRole )
    return rowId;

  if ( index.column() >= mFieldCount )
    return QVariant();

  int fieldId = mAttributes.at( index.column() );

  if ( role == FieldIndexRole )
    return fieldId;

  if ( role == SortRole )
  {
    return mSortCache[rowId];
  }

  QgsField field = layer()->fields().at( fieldId );

  if ( role == Qt::TextAlignmentRole )
  {
    return mWidgetFactories.at( index.column() )->alignmentFlag( layer(), fieldId, mWidgetConfigs.at( index.column() ) );
  }

  if ( mFeat.id() != rowId || !mFeat.isValid() )
  {
    if ( !loadFeatureAtId( rowId ) )
      return QVariant( "ERROR" );

    if ( mFeat.id() != rowId )
      return QVariant( "ERROR" );
  }

  QVariant val = mFeat.attribute( fieldId );

  switch ( role )
  {
    case Qt::DisplayRole:
      return mWidgetFactories.at( index.column() )->representValue( layer(), fieldId, mWidgetConfigs.at( index.column() ),
             mAttributeWidgetCaches.at( index.column() ), val );

    case Qt::EditRole:
      return val;

    case Qt::BackgroundColorRole:
    case Qt::TextColorRole:
    case Qt::DecorationRole:
    case Qt::FontRole:
    {
      mExpressionContext.setFeature( mFeat );
      QList<QgsConditionalStyle> styles;
      if ( mRowStylesMap.contains( index.row() ) )
      {
        styles = mRowStylesMap[index.row()];
      }
      else
      {
        styles = QgsConditionalStyle::matchingConditionalStyles( layer()->conditionalStyles()->rowStyles(), QVariant(),  mExpressionContext );
        mRowStylesMap.insert( index.row(), styles );

      }

      QgsConditionalStyle rowstyle = QgsConditionalStyle::compressStyles( styles );
      styles = layer()->conditionalStyles()->fieldStyles( field.name() );
      styles = QgsConditionalStyle::matchingConditionalStyles( styles , val,  mExpressionContext );
      styles.insert( 0, rowstyle );
      QgsConditionalStyle style = QgsConditionalStyle::compressStyles( styles );

      if ( style.isValid() )
      {
        if ( role == Qt::BackgroundColorRole && style.validBackgroundColor() )
          return style.backgroundColor();
        if ( role == Qt::TextColorRole && style.validTextColor() )
          return style.textColor();
        if ( role == Qt::DecorationRole )
          return style.icon();
        if ( role == Qt::FontRole )
          return style.font();
      }

      return QVariant();
    }
  }

  return QVariant();
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
       !layer()->editFormConfig()->readOnly( mAttributes[index.column()] ) &&
       (( layer()->dataProvider() && layer()->dataProvider()->capabilities() & QgsVectorDataProvider::ChangeAttributeValues ) ||
        FID_IS_NEW( rowToId( index.row() ) ) ) )
    flags |= Qt::ItemIsEditable;

  return flags;
}

void QgsAttributeTableModel::reload( const QModelIndex &index1, const QModelIndex &index2 )
{
  mFeat.setFeatureId( std::numeric_limits<int>::min() );
  emit dataChanged( index1, index2 );
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
  if ( column == -1 || column >= mAttributes.count() )
  {
    prefetchSortData( "" );
  }
  else
  {
    prefetchSortData( QgsExpression::quotedColumnRef( mLayerCache->layer()->fields().at( mAttributes.at( column ) ).name() ) );
  }
}

void QgsAttributeTableModel::prefetchSortData( const QString& expressionString )
{
  mSortCache.clear();
  mSortCacheAttributes.clear();
  mSortFieldIndex = -1;
  mSortCacheExpression = QgsExpression( expressionString );

  QgsEditorWidgetFactory* widgetFactory = nullptr;
  QVariant widgetCache;
  QgsEditorWidgetConfig widgetConfig;

  if ( mSortCacheExpression.isField() )
  {
    QString fieldName = dynamic_cast<const QgsExpression::NodeColumnRef*>( mSortCacheExpression.rootNode() )->name();
    mSortFieldIndex = mLayerCache->layer()->fieldNameIndex( fieldName );
  }

  if ( mSortFieldIndex == -1 )
  {
    mSortCacheExpression.prepare( &mExpressionContext );

    Q_FOREACH ( const QString& col, mSortCacheExpression.referencedColumns() )
    {
      mSortCacheAttributes.append( mLayerCache->layer()->fieldNameIndex( col ) );
    }
  }
  else
  {
    mSortCacheAttributes.append( mSortFieldIndex );

    widgetFactory = mWidgetFactories.at( mSortFieldIndex );
    widgetCache = mAttributeWidgetCaches.at( mSortFieldIndex );
    widgetConfig = mWidgetConfigs.at( mSortFieldIndex );
  }

  QgsFeatureRequest request = QgsFeatureRequest( mFeatureRequest )
                              .setFlags( QgsFeatureRequest::NoGeometry )
                              .setSubsetOfAttributes( mSortCacheAttributes );
  QgsFeatureIterator it = mLayerCache->getFeatures( request );

  QgsFeature f;
  while ( it.nextFeature( f ) )
  {
    if ( mSortFieldIndex == -1 )
    {
      mExpressionContext.setFeature( f );
      mSortCache.insert( f.id(), mSortCacheExpression.evaluate( &mExpressionContext ) );
    }
    else
    {
      QVariant sortValue = widgetFactory->representValue( layer(), mSortFieldIndex, widgetConfig, widgetCache, f.attribute( mSortFieldIndex ) );
      mSortCache.insert( f.id(), sortValue );
    }
  }
}

QString QgsAttributeTableModel::sortCacheExpression() const
{
  if ( mSortCacheExpression.rootNode() )
    return mSortCacheExpression.expression();
  else
    return QString();
}

void QgsAttributeTableModel::setRequest( const QgsFeatureRequest& request )
{
  mFeatureRequest = request;
  if ( layer() && !layer()->hasGeometryType() )
    mFeatureRequest.setFlags( mFeatureRequest.flags() | QgsFeatureRequest::NoGeometry );
}

const QgsFeatureRequest& QgsAttributeTableModel::request() const
{
  return mFeatureRequest;
}
