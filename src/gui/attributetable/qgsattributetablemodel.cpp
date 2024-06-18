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

#include "qgsactionmanager.h"
#include "qgseditorwidgetregistry.h"
#include "qgsexpression.h"
#include "qgsfeatureiterator.h"
#include "qgsconditionalstyle.h"
#include "qgsfields.h"
#include "qgsfieldformatter.h"
#include "qgslogger.h"
#include "qgsmaplayeraction.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"
#include "qgsfieldformatterregistry.h"
#include "qgsgui.h"
#include "qgsexpressionnodeimpl.h"
#include "qgsfieldmodel.h"
#include "qgsexpressioncontextutils.h"
#include "qgsstringutils.h"
#include "qgsvectorlayerutils.h"
#include "qgsvectorlayercache.h"

#include <QVariant>
#include <QUuid>

#include <limits>

QgsAttributeTableModel::QgsAttributeTableModel( QgsVectorLayerCache *layerCache, QObject *parent )
  : QAbstractTableModel( parent )
  , mLayer( layerCache->layer() )
  , mLayerCache( layerCache )
{
  mExpressionContext.appendScopes( QgsExpressionContextUtils::globalProjectLayerScopes( layerCache->layer() ) );

  if ( mLayer->geometryType() == Qgis::GeometryType::Null )
  {
    mFeatureRequest.setFlags( Qgis::FeatureRequestFlag::NoGeometry );
  }

  mFeat.setId( std::numeric_limits<int>::min() );

  if ( !mLayer->isSpatial() )
    mFeatureRequest.setFlags( Qgis::FeatureRequestFlag::NoGeometry );

  loadAttributes();

  connect( mLayer, &QgsVectorLayer::featuresDeleted, this, &QgsAttributeTableModel::featuresDeleted );
  connect( mLayer, &QgsVectorLayer::attributeDeleted, this, &QgsAttributeTableModel::attributeDeleted );
  connect( mLayer, &QgsVectorLayer::updatedFields, this, &QgsAttributeTableModel::updatedFields );

  connect( mLayer, &QgsVectorLayer::editCommandStarted, this, &QgsAttributeTableModel::bulkEditCommandStarted );
  connect( mLayer, &QgsVectorLayer::beforeRollBack, this,  &QgsAttributeTableModel::bulkEditCommandStarted );
  connect( mLayer, &QgsVectorLayer::afterRollBack, this, [ = ]
  {
    mIsCleaningUpAfterRollback = true;
    bulkEditCommandEnded();
    mIsCleaningUpAfterRollback = false;
  } );

  connect( mLayer, &QgsVectorLayer::editCommandEnded, this, &QgsAttributeTableModel::editCommandEnded );
  connect( mLayerCache, &QgsVectorLayerCache::attributeValueChanged, this, &QgsAttributeTableModel::attributeValueChanged );
  connect( mLayerCache, &QgsVectorLayerCache::featureAdded, this, [ = ]( QgsFeatureId id ) { featureAdded( id ); } );
  connect( mLayerCache, &QgsVectorLayerCache::cachedLayerDeleted, this, &QgsAttributeTableModel::layerDeleted );
}

bool QgsAttributeTableModel::loadFeatureAtId( QgsFeatureId fid ) const
{
  QgsDebugMsgLevel( QStringLiteral( "loading feature %1" ).arg( fid ), 3 );

  if ( fid == std::numeric_limits<int>::min() )
  {
    return false;
  }

  return mLayerCache->featureAtId( fid, mFeat );
}

bool QgsAttributeTableModel::loadFeatureAtId( QgsFeatureId fid, int fieldIdx ) const
{
  QgsDebugMsgLevel( QStringLiteral( "loading feature %1 with field %2" ).arg( fid, fieldIdx ), 3 );

  if ( mLayerCache->cacheSubsetOfAttributes().contains( fieldIdx ) )
  {
    return loadFeatureAtId( fid );
  }

  if ( fid == std::numeric_limits<int>::min() )
  {
    return false;
  }
  return mLayerCache->featureAtIdWithAllAttributes( fid, mFeat );
}

int QgsAttributeTableModel::extraColumns() const
{
  return mExtraColumns;
}

void QgsAttributeTableModel::setExtraColumns( int extraColumns )
{
  if ( extraColumns > mExtraColumns )
  {
    beginInsertColumns( QModelIndex(), mFieldCount + mExtraColumns, mFieldCount + extraColumns - 1 );
    mExtraColumns = extraColumns;
    endInsertColumns();
  }
  else if ( extraColumns < mExtraColumns )
  {
    beginRemoveColumns( QModelIndex(), mFieldCount + extraColumns, mFieldCount + mExtraColumns - 1 );
    mExtraColumns = extraColumns;
    endRemoveColumns();
  }
}

void QgsAttributeTableModel::featuresDeleted( const QgsFeatureIds &fids )
{
  QList<int> rows;

  const auto constFids = fids;
  for ( const QgsFeatureId fid : constFids )
  {
    QgsDebugMsgLevel( QStringLiteral( "(%2) fid: %1, size: %3" ).arg( fid ).arg( qgsEnumValueToKey( mFeatureRequest.filterType() ) ).arg( mIdRowMap.size() ), 4 );

    const int row = idToRow( fid );
    if ( row != -1 )
      rows << row;
  }

  std::sort( rows.begin(), rows.end() );

  int lastRow = -1;
  int beginRow = -1;
  int currentRowCount = 0;
  int removedRows = 0;
  bool reset = false;

  const auto constRows = rows;
  for ( const int row : constRows )
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

  if ( !mResettingModel )
  {
    beginRemoveRows( parent, row, row + count - 1 );
  }

#ifdef QGISDEBUG
  if ( 3 <= QgsLogger::debugLevel() )
    QgsDebugMsgLevel( QStringLiteral( "remove %2 rows at %1 (rows %3, ids %4)" ).arg( row ).arg( count ).arg( mRowIdMap.size() ).arg( mIdRowMap.size() ), 3 );
#endif

  if ( mBulkEditCommandRunning && !mResettingModel )
  {
    for ( int i = row; i < row + count; i++ )
    {
      const QgsFeatureId fid { rowToId( row ) };
      mInsertedRowsChanges.removeOne( fid );
    }
  }

  // clean old references
  for ( int i = row; i < row + count; i++ )
  {
    for ( SortCache &cache : mSortCaches )
      cache.sortCache.remove( mRowIdMap[i] );
    mIdRowMap.remove( mRowIdMap[i] );
    mRowIdMap.remove( i );
  }

  // update maps
  const int n = mRowIdMap.size() + count;
  for ( int i = row + count; i < n; i++ )
  {
    const QgsFeatureId id = mRowIdMap[i];
    mIdRowMap[id] -= count;
    mRowIdMap[i - count] = id;
    mRowIdMap.remove( i );
  }

#ifdef QGISDEBUG
  if ( 4 <= QgsLogger::debugLevel() )
  {
    QgsDebugMsgLevel( QStringLiteral( "after removal rows %1, ids %2" ).arg( mRowIdMap.size() ).arg( mIdRowMap.size() ), 4 );
    QgsDebugMsgLevel( QStringLiteral( "id->row" ), 4 );
    for ( QHash<QgsFeatureId, int>::const_iterator it = mIdRowMap.constBegin(); it != mIdRowMap.constEnd(); ++it )
      QgsDebugMsgLevel( QStringLiteral( "%1->%2" ).arg( FID_TO_STRING( it.key() ) ).arg( *it ), 4 );

    QgsDebugMsgLevel( QStringLiteral( "row->id" ), 4 );
    for ( QHash<int, QgsFeatureId>::const_iterator it = mRowIdMap.constBegin(); it != mRowIdMap.constEnd(); ++it )
      QgsDebugMsgLevel( QStringLiteral( "%1->%2" ).arg( it.key() ).arg( FID_TO_STRING( *it ) ), 4 );
  }
#endif

  Q_ASSERT( mRowIdMap.size() == mIdRowMap.size() );

  if ( !mResettingModel )
    endRemoveRows();

  return true;
}

void QgsAttributeTableModel::featureAdded( QgsFeatureId fid )
{
  QgsDebugMsgLevel( QStringLiteral( "(%2) fid: %1" ).arg( fid ).arg( qgsEnumValueToKey( mFeatureRequest.filterType() ) ), 4 );
  bool featOk = true;

  if ( mFeat.id() != fid )
    featOk = loadFeatureAtId( fid );

  if ( featOk && mFeatureRequest.acceptFeature( mFeat ) )
  {
    for ( SortCache &cache : mSortCaches )
    {
      if ( cache.sortFieldIndex >= 0 )
      {
        const WidgetData &widgetData = getWidgetData( cache.sortFieldIndex );
        const QVariant sortValue = widgetData.fieldFormatter->sortValue( mLayer, cache.sortFieldIndex, widgetData.config, widgetData.cache, mFeat.attribute( cache.sortFieldIndex ) );
        cache.sortCache.insert( mFeat.id(), sortValue );
      }
      else if ( cache.sortCacheExpression.isValid() )
      {
        mExpressionContext.setFeature( mFeat );
        cache.sortCache[mFeat.id()] = cache.sortCacheExpression.evaluate( &mExpressionContext );
      }
    }

    // Skip if the fid is already in the map (do not add twice)!
    if ( ! mIdRowMap.contains( fid ) )
    {
      const int n = mRowIdMap.size();
      if ( !mResettingModel )
        beginInsertRows( QModelIndex(), n, n );
      mIdRowMap.insert( fid, n );
      mRowIdMap.insert( n, fid );
      if ( !mResettingModel )
        endInsertRows();
      reload( index( rowCount() - 1, 0 ), index( rowCount() - 1, columnCount() ) );
      if ( mBulkEditCommandRunning && !mResettingModel )
      {
        mInsertedRowsChanges.append( fid );
      }
    }
  }
}

void QgsAttributeTableModel::updatedFields()
{
  loadAttributes();
  emit modelChanged();
}

void QgsAttributeTableModel::editCommandEnded()
{
  // do not do reload(...) due would trigger (dataChanged) row sort
  // giving issue: https://github.com/qgis/QGIS/issues/23892
  bulkEditCommandEnded( );
}

void QgsAttributeTableModel::attributeDeleted( int idx )
{
  int cacheIndex = 0;
  for ( const SortCache &cache : mSortCaches )
  {
    if ( cache.sortCacheAttributes.contains( idx ) )
    {
      prefetchSortData( QString(), cacheIndex );
    }
    cacheIndex++;
  }
}

void QgsAttributeTableModel::layerDeleted()
{
  mLayerCache = nullptr;
  mLayer = nullptr;
  removeRows( 0, rowCount() );

  mAttributes.clear();
  mWidgetDatas.clear();
}

void QgsAttributeTableModel::fieldFormatterRemoved( QgsFieldFormatter *fieldFormatter )
{
  for ( WidgetData &widgetData : mWidgetDatas )
  {
    if ( widgetData.fieldFormatter == fieldFormatter )
      widgetData.fieldFormatter = QgsApplication::fieldFormatterRegistry()->fallbackFieldFormatter();
  }
}

void QgsAttributeTableModel::attributeValueChanged( QgsFeatureId fid, int idx, const QVariant &value )
{
  // Defer all updates if a bulk edit/rollback command is running
  if ( mBulkEditCommandRunning )
  {
    mAttributeValueChanges.insert( QPair<QgsFeatureId, int>( fid, idx ), value );
    return;
  }
  QgsDebugMsgLevel( QStringLiteral( "(%4) fid: %1, idx: %2, value: %3" ).arg( fid ).arg( idx ).arg( value.toString() ).arg( qgsEnumValueToKey( mFeatureRequest.filterType() ) ), 2 );

  for ( SortCache &cache : mSortCaches )
  {
    if ( cache.sortCacheAttributes.contains( idx ) )
    {
      if ( cache.sortFieldIndex == -1 )
      {
        if ( loadFeatureAtId( fid ) )
        {
          mExpressionContext.setFeature( mFeat );
          cache.sortCache[fid] = cache.sortCacheExpression.evaluate( &mExpressionContext );
        }
      }
      else
      {
        const WidgetData &widgetData = getWidgetData( cache.sortFieldIndex );
        const QVariant sortValue = widgetData.fieldFormatter->representValue( mLayer, cache.sortFieldIndex, widgetData.config, widgetData.cache, value );
        cache.sortCache.insert( fid, sortValue );
      }
    }
  }
  // No filter request: skip all possibly heavy checks
  if ( mFeatureRequest.filterType() == Qgis::FeatureRequestFilterType::NoFilter )
  {
    if ( loadFeatureAtId( fid ) )
    {
      const QModelIndex modelIndex = index( idToRow( fid ), fieldCol( idx ) );
      setData( modelIndex, value, Qt::EditRole );
      emit dataChanged( modelIndex, modelIndex, QVector<int>() << Qt::DisplayRole );
    }
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
          const QModelIndex modelIndex = index( idToRow( fid ), fieldCol( idx ) );
          setData( modelIndex, value, Qt::EditRole );
          emit dataChanged( modelIndex, modelIndex, QVector<int>() << Qt::DisplayRole );
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
  if ( !mLayer )
  {
    return;
  }

  const QgsFields fields = mLayer->fields();
  if ( mFields == fields )
    return;

  mFields = fields;

  bool ins = false, rm = false;

  QgsAttributeList attributes;

  mWidgetDatas.clear();

  for ( int idx = 0; idx < fields.count(); ++idx )
  {
    attributes << idx;
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
  mWidgetDatas.resize( mFieldCount );

  for ( SortCache &cache : mSortCaches )
  {
    if ( cache.sortFieldIndex >= mAttributes.count() )
      cache.sortFieldIndex = -1;
  }

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

  mResettingModel = true;
  beginResetModel();

  if ( rowCount() != 0 )
  {
    removeRows( 0, rowCount() );
  }

  // Layer might have been deleted and cache set to nullptr!
  if ( mLayerCache )
  {
    QgsFeatureIterator features = mLayerCache->getFeatures( mFeatureRequest );

    int i = 0;

    QElapsedTimer t;
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
    connect( mLayerCache, &QgsVectorLayerCache::invalidated, this, &QgsAttributeTableModel::loadLayer, Qt::UniqueConnection );
  }

  endResetModel();

  mResettingModel = false;
}


void QgsAttributeTableModel::fieldConditionalStyleChanged( const QString &fieldName )
{
  if ( fieldName.isNull() )
  {
    mRowStylesMap.clear();
    mConstraintStylesMap.clear();
    emit dataChanged( index( 0, 0 ), index( rowCount() - 1, columnCount() - 1 ) );
    return;
  }

  const int fieldIndex = mLayer->fields().lookupField( fieldName );
  if ( fieldIndex == -1 )
    return;

  //whole column has changed
  const int col = fieldCol( fieldIndex );
  emit dataChanged( index( 0, col ), index( rowCount() - 1, col ) );
}

void QgsAttributeTableModel::swapRows( QgsFeatureId a, QgsFeatureId b )
{
  if ( a == b )
    return;

  const int rowA = idToRow( a );
  const int rowB = idToRow( b );

  //emit layoutAboutToBeChanged();

  mRowIdMap.remove( rowA );
  mRowIdMap.remove( rowB );
  mRowIdMap.insert( rowA, b );
  mRowIdMap.insert( rowB, a );

  mIdRowMap.remove( a );
  mIdRowMap.remove( b );
  mIdRowMap.insert( a, rowB );
  mIdRowMap.insert( b, rowA );
  Q_ASSERT( mRowIdMap.size() == mIdRowMap.size() );


  //emit layoutChanged();
}

int QgsAttributeTableModel::idToRow( QgsFeatureId id ) const
{
  if ( !mIdRowMap.contains( id ) )
  {
    QgsDebugError( QStringLiteral( "idToRow: id %1 not in the map" ).arg( id ) );
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

  const int row = idToRow( id );
  const int columns = columnCount();
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
    QgsDebugError( QStringLiteral( "rowToId: row %1 not in the map" ).arg( row ) );
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
  Q_UNUSED( parent )
  return mRowIdMap.size();
}

int QgsAttributeTableModel::columnCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent )
  return std::max( 1, mFieldCount + mExtraColumns );  // if there are zero columns all model indices will be considered invalid
}

QVariant QgsAttributeTableModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  if ( !mLayer )
    return QVariant();

  if ( role == Qt::DisplayRole )
  {
    if ( orientation == Qt::Vertical ) //row
    {
      return QVariant( section );
    }
    else if ( section >= 0 && section < mFieldCount )
    {
      const QString attributeName = mLayer->fields().at( mAttributes.at( section ) ).displayName();
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
      const QgsField field = mLayer->fields().at( mAttributes.at( section ) );
      return QgsFieldModel::fieldToolTipExtended( field, mLayer );
    }
  }
  else
  {
    return QVariant();
  }
}

QVariant QgsAttributeTableModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() || !mLayer ||
       ( role != Qt::TextAlignmentRole
         && role != Qt::DisplayRole
         && role != Qt::ToolTipRole
         && role != Qt::EditRole
         && role != static_cast< int >( CustomRole::FeatureId )
         && role != static_cast< int >( CustomRole::FieldIndex )
         && role != Qt::BackgroundRole
         && role != Qt::ForegroundRole
         && role != Qt::DecorationRole
         && role != Qt::FontRole
         && role < static_cast< int >( CustomRole::Sort )
       )
     )
    return QVariant();

  const QgsFeatureId rowId = rowToId( index.row() );

  if ( role == static_cast< int >( CustomRole::FeatureId ) )
    return rowId;

  if ( index.column() >= mFieldCount )
    return QVariant();

  const int fieldId = mAttributes.at( index.column() );

  if ( role == static_cast< int >( CustomRole::FieldIndex ) )
    return fieldId;

  if ( role >= static_cast< int >( CustomRole::Sort ) )
  {
    const unsigned long cacheIndex = role - static_cast< int >( CustomRole::Sort );
    if ( cacheIndex < mSortCaches.size() )
      return mSortCaches.at( cacheIndex ).sortCache.value( rowId );
    else
      return QVariant();
  }

  const QgsField field = mLayer->fields().at( fieldId );

  if ( role == Qt::TextAlignmentRole )
  {
    const WidgetData &widgetData = getWidgetData( index.column() );
    return static_cast<Qt::Alignment::Int>( widgetData.fieldFormatter->alignmentFlag( mLayer, fieldId, widgetData.config ) | Qt::AlignVCenter );
  }

  if ( mFeat.id() != rowId || !mFeat.isValid() || ! mLayerCache->cacheSubsetOfAttributes().contains( fieldId ) )
  {
    if ( !loadFeatureAtId( rowId, fieldId ) )
      return QVariant( "ERROR" );

    if ( mFeat.id() != rowId )
      return QVariant( "ERROR" );
  }

  QVariant val = mFeat.attribute( fieldId );

  switch ( role )
  {
    case Qt::DisplayRole:
    {
      const WidgetData &widgetData = getWidgetData( index.column() );
      QString s = widgetData.fieldFormatter->representValue( mLayer, fieldId, widgetData.config, widgetData.cache, val );
      // In table view, too long strings kill performance. Just truncate them
      constexpr int MAX_STRING_LENGTH = 10 * 1000;
      if ( static_cast<size_t>( s.size() ) > static_cast<size_t>( MAX_STRING_LENGTH ) )
      {
        s.resize( MAX_STRING_LENGTH );
        s.append( tr( "... truncated ..." ) );
      }
      return s;
    }
    case Qt::ToolTipRole:
    {
      const WidgetData &widgetData = getWidgetData( index.column() );
      QString tooltip = widgetData.fieldFormatter->representValue( mLayer, fieldId, widgetData.config, widgetData.cache, val );
      if ( val.userType() == QMetaType::Type::QString && QgsStringUtils::isUrl( val.toString() ) )
      {
        tooltip = tr( "%1 (Ctrl+click to open)" ).arg( tooltip );
      }
      return tooltip;
    }
    case Qt::EditRole:
      return val;

    case Qt::BackgroundRole:
    case Qt::ForegroundRole:
    case Qt::DecorationRole:
    case Qt::FontRole:
    {
      mExpressionContext.setFeature( mFeat );
      QList<QgsConditionalStyle> styles;
      if ( mRowStylesMap.contains( mFeat.id() ) )
      {
        styles = mRowStylesMap[mFeat.id()];
      }
      else
      {
        styles = QgsConditionalStyle::matchingConditionalStyles( mLayer->conditionalStyles()->rowStyles(), QVariant(),  mExpressionContext );
        mRowStylesMap.insert( mFeat.id(), styles );
      }
      const QgsConditionalStyle rowstyle = QgsConditionalStyle::compressStyles( styles );

      QgsConditionalStyle constraintstyle;
      if ( mShowValidityState && QgsVectorLayerUtils::attributeHasConstraints( mLayer, fieldId ) )
      {
        if ( mConstraintStylesMap.contains( mFeat.id() ) &&
             mConstraintStylesMap[mFeat.id()].contains( fieldId ) )
        {
          constraintstyle = mConstraintStylesMap[mFeat.id()][fieldId];
        }
        else
        {
          QStringList errors;
          if ( !QgsVectorLayerUtils::validateAttribute( mLayer, mFeat, fieldId, errors, QgsFieldConstraints::ConstraintStrengthHard ) )
          {
            constraintstyle = mLayer->conditionalStyles()->constraintFailureStyles( QgsFieldConstraints::ConstraintStrengthHard );
          }
          else
          {
            if ( !QgsVectorLayerUtils::validateAttribute( mLayer, mFeat, fieldId, errors, QgsFieldConstraints::ConstraintStrengthSoft ) )
            {
              constraintstyle = mLayer->conditionalStyles()->constraintFailureStyles( QgsFieldConstraints::ConstraintStrengthSoft );
            }
          }
          mConstraintStylesMap[mFeat.id()].insert( fieldId, constraintstyle );
        }
      }

      styles = mLayer->conditionalStyles()->fieldStyles( field.name() );
      styles = QgsConditionalStyle::matchingConditionalStyles( styles, val,  mExpressionContext );
      styles.insert( 0, rowstyle );
      styles.insert( 0, constraintstyle );
      const QgsConditionalStyle style = QgsConditionalStyle::compressStyles( styles );

      if ( style.isValid() )
      {
        if ( role == Qt::BackgroundRole && style.validBackgroundColor() )
          return style.backgroundColor();
        if ( role == Qt::ForegroundRole )
          return style.textColor();
        if ( role == Qt::DecorationRole )
          return style.icon();
        if ( role == Qt::FontRole )
          return style.font();
      }
      else if ( val.userType() == QMetaType::Type::QString && QgsStringUtils::isUrl( val.toString() ) )
      {
        if ( role == Qt::ForegroundRole )
        {
          return QColor( Qt::blue );
        }
        else if ( role == Qt::FontRole )
        {
          QFont font;
          font.setUnderline( true );
          return font;
        }
      }

      return QVariant();
    }
  }

  return QVariant();
}

bool QgsAttributeTableModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  Q_UNUSED( value )

  if ( !index.isValid() || index.column() >= mFieldCount || role != Qt::EditRole || !mLayer->isEditable() )
    return false;

  mRowStylesMap.remove( mFeat.id() );
  mConstraintStylesMap.remove( mFeat.id() );

  if ( !mLayer->isModified() )
    return false;

  return true;
}

Qt::ItemFlags QgsAttributeTableModel::flags( const QModelIndex &index ) const
{
  if ( !index.isValid() )
    return Qt::ItemIsEnabled;

  if ( index.column() >= mFieldCount || !mLayer )
    return Qt::NoItemFlags;

  Qt::ItemFlags flags = QAbstractTableModel::flags( index );

  const int fieldIndex = mAttributes[index.column()];
  const QgsFeatureId fid = rowToId( index.row() );

  if ( QgsVectorLayerUtils::fieldIsEditable( mLayer, fieldIndex, fid ) )
    flags |= Qt::ItemIsEditable;

  return flags;
}

void QgsAttributeTableModel::bulkEditCommandStarted()
{
  mBulkEditCommandRunning = true;
  mAttributeValueChanges.clear();
}

void QgsAttributeTableModel::bulkEditCommandEnded()
{
  mBulkEditCommandRunning = false;
  // Full model update if the changed rows are more than half the total rows
  // or if their count is > layer cache size

  const long long fullModelUpdateThreshold = std::min<long long >( mLayerCache->cacheSize(), std::ceil( rowCount() * 0.5 ) );
  bool fullModelUpdate = false;

  // try the cheaper check first
  if ( mInsertedRowsChanges.size() > fullModelUpdateThreshold )
  {
    fullModelUpdate = true;
  }
  else
  {
    QSet< QgsFeatureId > changedRows;
    changedRows.reserve( mAttributeValueChanges.size() );
    // we need to count changed features, not the total of changed attributes (which may all apply to one feature)
    for ( auto it = mAttributeValueChanges.constBegin(); it != mAttributeValueChanges.constEnd(); ++it )
    {
      changedRows.insert( it.key().first );
      if ( changedRows.size() > fullModelUpdateThreshold )
      {
        fullModelUpdate = true;
        break;
      }
    }
  }

  QgsDebugMsgLevel( QStringLiteral( "Bulk edit command ended modified rows over (%3), cache size is %1, starting %2 update." )
                    .arg( mLayerCache->cacheSize() )
                    .arg( fullModelUpdate ? QStringLiteral( "full" ) :  QStringLiteral( "incremental" ) )
                    .arg( rowCount() ),
                    3 );

  // Remove added rows on rollback
  if ( mIsCleaningUpAfterRollback )
  {
    for ( const int fid : std::as_const( mInsertedRowsChanges ) )
    {
      const int row( idToRow( fid ) );
      if ( row < 0 )
      {
        continue;
      }
      removeRow( row );
    }
  }

  // Invalidates the whole model
  if ( fullModelUpdate )
  {
    // Invalidates the cache (there is no API for doing this directly)
    emit mLayer->dataChanged();
    emit dataChanged( createIndex( 0, 0 ), createIndex( rowCount() - 1, columnCount() - 1 ) );
  }
  else
  {

    int minRow = rowCount();
    int minCol = columnCount();
    int maxRow = 0;
    int maxCol = 0;
    const auto keys = mAttributeValueChanges.keys();
    for ( const auto &key : keys )
    {
      attributeValueChanged( key.first, key.second, mAttributeValueChanges.value( key ) );
      const int row( idToRow( key.first ) );
      const int col( fieldCol( key.second ) );
      minRow = std::min<int>( row, minRow );
      minCol = std::min<int>( col, minCol );
      maxRow = std::max<int>( row, maxRow );
      maxCol = std::max<int>( col, maxCol );
    }

    emit dataChanged( createIndex( minRow, minCol ), createIndex( maxRow, maxCol ) );
  }
  mAttributeValueChanges.clear();
}

void QgsAttributeTableModel::reload( const QModelIndex &index1, const QModelIndex &index2 )
{
  mFeat.setId( std::numeric_limits<int>::min() );
  emit dataChanged( index1, index2 );
}


void QgsAttributeTableModel::executeAction( QUuid action, const QModelIndex &idx ) const
{
  const QgsFeature f = feature( idx );
  mLayer->actions()->doAction( action, f, fieldIdx( idx.column() ) );
}

void QgsAttributeTableModel::executeMapLayerAction( QgsMapLayerAction *action, const QModelIndex &idx, const QgsMapLayerActionContext &context ) const
{
  const QgsFeature f = feature( idx );
  Q_NOWARN_DEPRECATED_PUSH
  action->triggerForFeature( mLayer, f );
  Q_NOWARN_DEPRECATED_POP
  action->triggerForFeature( mLayer, f, context );
}

QgsFeature QgsAttributeTableModel::feature( const QModelIndex &idx ) const
{
  QgsFeature f( mLayer->fields() );
  f.initAttributes( mAttributes.size() );
  f.setId( rowToId( idx.row() ) );
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
    prefetchSortData( QString() );
  }
  else
  {
    prefetchSortData( QgsExpression::quotedColumnRef( mLayer->fields().at( mAttributes.at( column ) ).name() ) );
  }
}

void QgsAttributeTableModel::prefetchSortData( const QString &expressionString, unsigned long cacheIndex )
{
  if ( cacheIndex >= mSortCaches.size() )
  {
    mSortCaches.resize( cacheIndex + 1 );
  }
  SortCache &cache = mSortCaches[cacheIndex];
  cache.sortCache.clear();
  cache.sortCacheAttributes.clear();
  cache.sortFieldIndex = -1;
  if ( !expressionString.isEmpty() )
    cache.sortCacheExpression = QgsExpression( expressionString );
  else
  {
    // no sorting
    cache.sortCacheExpression = QgsExpression();
    return;
  }

  WidgetData widgetData;

  if ( cache.sortCacheExpression.isField() )
  {
    const QString fieldName = static_cast<const QgsExpressionNodeColumnRef *>( cache.sortCacheExpression.rootNode() )->name();
    cache.sortFieldIndex = mLayer->fields().lookupField( fieldName );
  }

  if ( cache.sortFieldIndex == -1 )
  {
    cache.sortCacheExpression.prepare( &mExpressionContext );

    const QSet<QString> &referencedColumns = cache.sortCacheExpression.referencedColumns();

    for ( const QString &col : referencedColumns )
    {
      cache.sortCacheAttributes.append( mLayer->fields().lookupField( col ) );
    }
  }
  else
  {
    cache.sortCacheAttributes.append( cache.sortFieldIndex );

    widgetData = getWidgetData( cache.sortFieldIndex );
  }

  const QgsFeatureRequest request = QgsFeatureRequest( mFeatureRequest ).setFlags( cache.sortCacheExpression.needsGeometry() ? Qgis::FeatureRequestFlag::NoFlags : Qgis::FeatureRequestFlag::NoGeometry ).setSubsetOfAttributes( cache.sortCacheAttributes );

  QgsFeatureIterator it = mLayerCache->getFeatures( request );

  QgsFeature f;
  while ( it.nextFeature( f ) )
  {
    if ( cache.sortFieldIndex == -1 )
    {
      mExpressionContext.setFeature( f );
      const QVariant cacheValue = cache.sortCacheExpression.evaluate( &mExpressionContext );
      cache.sortCache.insert( f.id(), cacheValue );
    }
    else
    {
      const QVariant sortValue = widgetData.fieldFormatter->sortValue( mLayer, cache.sortFieldIndex, widgetData.config, widgetData.cache, f.attribute( cache.sortFieldIndex ) );
      cache.sortCache.insert( f.id(), sortValue );
    }
  }
}

QString QgsAttributeTableModel::sortCacheExpression( unsigned long cacheIndex ) const
{
  QString expressionString;

  if ( cacheIndex >= mSortCaches.size() )
    return expressionString;

  const QgsExpression &expression = mSortCaches[cacheIndex].sortCacheExpression;

  if ( expression.isValid() )
    expressionString = expression.expression();
  else
    expressionString = QString();

  return expressionString;
}

void QgsAttributeTableModel::setRequest( const QgsFeatureRequest &request )
{
  if ( ! mFeatureRequest.compare( request ) )
  {
    mFeatureRequest = request;
    if ( mLayer && !mLayer->isSpatial() )
      mFeatureRequest.setFlags( mFeatureRequest.flags() | Qgis::FeatureRequestFlag::NoGeometry );
    // Prefetch data for sorting, resetting all caches
    for ( unsigned long i = 0; i < mSortCaches.size(); ++i )
    {
      prefetchSortData( sortCacheExpression( i ), i );
    }
  }
}

const QgsFeatureRequest &QgsAttributeTableModel::request() const
{
  return mFeatureRequest;
}

const QgsAttributeTableModel::WidgetData &QgsAttributeTableModel::getWidgetData( int column ) const
{
  Q_ASSERT( column >= 0 && column < mAttributes.size() );

  WidgetData &widgetData = mWidgetDatas[ column ];
  if ( !widgetData.loaded )
  {
    const int idx = fieldIdx( column );
    const QgsEditorWidgetSetup setup = QgsGui::editorWidgetRegistry()->findBest( mLayer, mFields[ idx ].name() );
    widgetData.fieldFormatter = QgsApplication::fieldFormatterRegistry()->fieldFormatter( setup.type() );
    widgetData.config = setup.config();
    widgetData.cache = widgetData.fieldFormatter->createCache( mLayer, idx, setup.config() );
    widgetData.loaded = true;
  }

  return widgetData;
}
