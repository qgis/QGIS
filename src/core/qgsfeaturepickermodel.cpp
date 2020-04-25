/***************************************************************************
  qgsfeaturepickermodel.cpp - QgsFeaturePickerModel
 ---------------------
 begin                : 03.04.2020
 copyright            : (C) 2020 by Denis Rouzaud
 email                : denis@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsfeaturepickermodel.h"
#include "qgsfeatureexpressionvaluesgatherer.h"

#include "qgsvectorlayer.h"
#include "qgsconditionalstyle.h"
#include "qgsapplication.h"
#include "qgssettings.h"


QgsFeaturePickerModel::QgsFeaturePickerModel( QObject *parent )
  : QAbstractItemModel( parent )
{
  mReloadTimer.setInterval( 100 );
  mReloadTimer.setSingleShot( true );
  connect( &mReloadTimer, &QTimer::timeout, this, &QgsFeaturePickerModel::scheduledReload );
  setCurrentFeatureUnguarded( FID_NULL );
}

QgsFeaturePickerModel::~QgsFeaturePickerModel()
{
  if ( mGatherer )
    connect( mGatherer, &QgsFeatureExpressionValuesGatherer::finished, mGatherer, &QgsFeatureExpressionValuesGatherer::deleteLater );
}

QgsVectorLayer *QgsFeaturePickerModel::sourceLayer() const
{
  return mSourceLayer;
}

void QgsFeaturePickerModel::setSourceLayer( QgsVectorLayer *sourceLayer )
{
  if ( mSourceLayer == sourceLayer )
    return;

  mSourceLayer = sourceLayer;
  mExpressionContext = sourceLayer->createExpressionContext();
  reload();
  emit sourceLayerChanged();

  setDisplayExpression( sourceLayer->displayExpression() );
}

QString QgsFeaturePickerModel::displayExpression() const
{
  return mDisplayExpression.expression();
}

void QgsFeaturePickerModel::setDisplayExpression( const QString &displayExpression )
{
  if ( mDisplayExpression.expression() == displayExpression )
    return;

  mDisplayExpression = QgsExpression( displayExpression );
  reload();
  emit displayExpressionChanged();
}

QString QgsFeaturePickerModel::filterValue() const
{
  return mFilterValue;
}

void QgsFeaturePickerModel::setFilterValue( const QString &filterValue )
{
  if ( mFilterValue == filterValue )
    return;

  mFilterValue = filterValue;
  reload();
  emit filterValueChanged();
}

QString QgsFeaturePickerModel::filterExpression() const
{
  return mFilterExpression;
}

void QgsFeaturePickerModel::setFilterExpression( const QString &filterExpression )
{
  if ( mFilterExpression == filterExpression )
    return;

  mFilterExpression = filterExpression;
  reload();
  emit filterExpressionChanged();
}

bool QgsFeaturePickerModel::isLoading() const
{
  return mGatherer;
}

QModelIndex QgsFeaturePickerModel::index( int row, int column, const QModelIndex &parent ) const
{
  Q_UNUSED( parent )
  return createIndex( row, column, nullptr );
}

QModelIndex QgsFeaturePickerModel::parent( const QModelIndex &child ) const
{
  Q_UNUSED( child )
  return QModelIndex();
}

int QgsFeaturePickerModel::rowCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent )

  return mEntries.size();
}

int QgsFeaturePickerModel::columnCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent )
  return 1;
}

QVariant QgsFeaturePickerModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() )
    return QVariant();

  switch ( role )
  {
    case Qt::DisplayRole:
    case Qt::EditRole:
    case ValueRole:
      return mEntries.value( index.row() ).value;

    case FeatureIdRole:
      return mEntries.value( index.row() ).feature.id();

    case FeatureRole:
      return mEntries.value( index.row() ).feature;

    case Qt::BackgroundColorRole:
    case Qt::TextColorRole:
    case Qt::DecorationRole:
    case Qt::FontRole:
    {
      bool isNull = !mEntries.value( index.row() ).feature.isValid();
      if ( isNull )
      {
        // Representation for NULL value
        if ( role == Qt::TextColorRole )
        {
          return QBrush( QColor( Qt::gray ) );
        }
        if ( role == Qt::FontRole )
        {
          QFont font = QFont();
          if ( index.row() == mCurrentIndex )
            font.setBold( true );
          else
            font.setItalic( true );
          return font;
        }
      }
      else
      {
        // Respect conditional style
        const QgsConditionalStyle style = featureStyle( mEntries.value( index.row() ).feature );

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
      }
      break;
    }
  }

  return QVariant();
}

void QgsFeaturePickerModel::updateCompleter()
{
  emit beginUpdate();

  QgsFeatureExpressionValuesGatherer *gatherer = qobject_cast<QgsFeatureExpressionValuesGatherer *>( sender() );
  if ( gatherer->wasCanceled() )
  {
    delete gatherer;
    return;
  }

  QVector<QgsFeatureExpressionValuesGatherer::Entry> entries = mGatherer->entries();

  if ( mCurrentIndex == -1 )
  {
    setCurrentFeatureUnguarded( FID_NULL );
  }

  // Only reloading the current entry?
  bool reloadCurrentFeatureOnly = mGatherer->data().toBool();
  if ( reloadCurrentFeatureOnly )
  {
    if ( !entries.isEmpty() )
    {
      mEntries.replace( mCurrentIndex, entries.at( 0 ) );
      emit dataChanged( index( mCurrentIndex, 0, QModelIndex() ), index( mCurrentIndex, 0, QModelIndex() ) );
      mShouldReloadCurrentFeature = 0;
      setExtraValueDoesNotExist( false );
    }
    else
    {
      setExtraValueDoesNotExist( true );
    }

    mShouldReloadCurrentFeature = 0;

    if ( mFilterValue.isEmpty() )
      reload();
  }
  else
  {
    // We got strings for a filter selection
    std::sort( entries.begin(), entries.end(), []( const QgsFeatureExpressionValuesGatherer::Entry & a, const QgsFeatureExpressionValuesGatherer::Entry & b ) { return a.value.localeAwareCompare( b.value ) < 0; } );

    if ( mAllowNull )
    {
      entries.prepend( QgsFeatureExpressionValuesGatherer::nullEntry() );
    }

    const int newEntriesSize = entries.size();

    // fixed entry is either NULL or extra value
    const int nbFixedEntry = ( mExtraValueDoesNotExist ? 1 : 0 ) + ( mAllowNull ? 1 : 0 );

    // Find the index of the current entry in the new list
    int currentEntryInNewList = -1;
    if ( mCurrentIndex != -1 && mCurrentIndex < mEntries.count() )
    {
      for ( int i = 0; i < newEntriesSize; ++i )
      {
        if ( entries.at( i ).feature.id() == mEntries.at( mCurrentIndex ).feature.id() )
        {
          mEntries.replace( mCurrentIndex, entries.at( i ) );
          currentEntryInNewList = i;
          break;
        }
      }
    }

    int firstRow = 0;

    // Move current entry to the first position if this is a fixed entry or because
    // the entry exists in the new list
    if ( mCurrentIndex > -1 && ( mCurrentIndex < nbFixedEntry || currentEntryInNewList != -1 ) )
    {
      if ( mCurrentIndex != 0 )
      {
        beginMoveRows( QModelIndex(), mCurrentIndex, mCurrentIndex, QModelIndex(), 0 );
        mEntries.move( mCurrentIndex, 0 );
        endMoveRows();
      }
      firstRow = 1;
    }

    // Remove all entries (except for extra entry if existent)
    beginRemoveRows( QModelIndex(), firstRow, mEntries.size() - firstRow );
    mEntries.remove( firstRow, mEntries.size() - firstRow );

    // if we remove all rows before endRemoveRows, setExtraIdentifierValuesUnguarded will be called
    // and a null value will be added to mEntries, so we block setExtraIdentifierValuesUnguarded call

    endRemoveRows();


    if ( currentEntryInNewList == -1 )
    {
      beginInsertRows( QModelIndex(), firstRow, entries.size() + 1 );
      mEntries += entries;
      endInsertRows();

      // if all entries have been cleaned (firstRow == 0)
      // and there is a value in entries, prefer this value over NULL
      // else chose the first one (the previous one)
      setCurrentIndex( firstRow == 0 && mAllowNull && !entries.isEmpty() ? 1 : 0, firstRow == 0 );
    }
    else
    {
      if ( currentEntryInNewList != 0 )
      {
        beginInsertRows( QModelIndex(), 0, currentEntryInNewList - 1 );
        mEntries = entries.mid( 0, currentEntryInNewList ) + mEntries;
        endInsertRows();
      }
      else
      {
        mEntries.replace( 0, entries.at( 0 ) );
      }

      // don't notify for a change if it's a fixed entry
      if ( currentEntryInNewList >= nbFixedEntry )
      {
        emit dataChanged( index( currentEntryInNewList, 0, QModelIndex() ), index( currentEntryInNewList, 0, QModelIndex() ) );
      }

      beginInsertRows( QModelIndex(), currentEntryInNewList + 1, newEntriesSize - currentEntryInNewList - 1 );
      mEntries += entries.mid( currentEntryInNewList + 1 );
      endInsertRows();
      setCurrentIndex( currentEntryInNewList );
    }

    emit filterJobCompleted();
  }
  emit endUpdate();


  // scheduleReload and updateCompleter lives in the same thread so if the gatherer hasn't been stopped
  // (checked before), mGatherer still references the current gatherer
  Q_ASSERT( gatherer == mGatherer );
  delete mGatherer;
  mGatherer = nullptr;
  emit isLoadingChanged();
}

void QgsFeaturePickerModel::scheduledReload()
{
  if ( !mSourceLayer )
    return;

  bool wasLoading = false;

  if ( mGatherer )
  {
    mGatherer->stop();
    wasLoading = true;
  }

  QgsFeatureRequest request;

  if ( mShouldReloadCurrentFeature != 0 )
  {
    request.setFilterFid( mShouldReloadCurrentFeature );
  }
  else
  {
    QString filterClause;
    if ( mFilterValue.isEmpty() && !mFilterExpression.isEmpty() )
      filterClause = mFilterExpression;
    else if ( mFilterExpression.isEmpty() && !mFilterValue.isEmpty() )
      filterClause = QStringLiteral( "(%1) ILIKE '%%2%'" ).arg( mDisplayExpression, mFilterValue );
    else if ( !mFilterExpression.isEmpty() && !mFilterValue.isEmpty() )
      filterClause = QStringLiteral( "(%1) AND ((%2) ILIKE '%%3%')" ).arg( mFilterExpression, mDisplayExpression, mFilterValue );

    if ( !filterClause.isEmpty() )
      request.setFilterExpression( filterClause );
  }

  request.setLimit( QgsSettings().value( QStringLiteral( "maxEntriesRelationWidget" ), 100, QgsSettings::Gui ).toInt() );

  mGatherer = new QgsFeatureExpressionValuesGatherer( mSourceLayer, mDisplayExpression, request );
  mGatherer->setData( mShouldReloadCurrentFeature );
  connect( mGatherer, &QgsFeatureExpressionValuesGatherer::finished, this, &QgsFeaturePickerModel::updateCompleter );

  mGatherer->start();
  if ( !wasLoading )
    emit isLoadingChanged();
}

void QgsFeaturePickerModel::setCurrentIndex( int index, bool force )
{
  if ( mCurrentIndex == index && !force )
    return;

  mCurrentIndex = index;
  emit currentIndexChanged( index );
  emit currentFeatureChanged( currentFeature() );
}

void QgsFeaturePickerModel::setCurrentFeature( const QgsFeatureId &featureId )
{
  if ( featureId == 0 || featureId == currentFeature().id() )
    return;

  if ( mIsSettingCurrentFeature )
    return;

  mIsSettingCurrentFeature = true;

  setCurrentFeatureUnguarded( featureId );

  mIsSettingCurrentFeature = false;
}

void QgsFeaturePickerModel::setCurrentFeatureUnguarded( const QgsFeatureId &featureId )
{
  const QVector<QgsFeatureExpressionValuesGatherer::Entry> entries = mEntries;

  int index = 0;
  for ( const QgsFeatureExpressionValuesGatherer::Entry &entry : entries )
  {
    if ( entry.feature.id() == featureId )
    {
      setCurrentIndex( index );
      break;
    }
    index++;
  }

  // Value not found in current entries
  if ( mCurrentIndex != index && ( mAllowNull || featureId != 0 ) )
  {
    beginInsertRows( QModelIndex(), 0, 0 );
    if ( featureId != 0 )
    {
      mShouldReloadCurrentFeature = featureId;
      mReloadTimer.start();
    }
    else
    {
      mEntries.prepend( QgsFeatureExpressionValuesGatherer::nullEntry() );
    }
    endInsertRows();
    setCurrentIndex( 0, true );
  }
}

void QgsFeaturePickerModel::setExtraValueDoesNotExist( bool extraValueDoesNotExist )
{
  if ( mExtraValueDoesNotExist == extraValueDoesNotExist )
    return;

  mExtraValueDoesNotExist = extraValueDoesNotExist;
  emit extraValueDoesNotExistChanged();
}

QgsConditionalStyle QgsFeaturePickerModel::featureStyle( const QgsFeature &feature ) const
{
  if ( !mSourceLayer )
    return QgsConditionalStyle();

  QgsVectorLayer *layer = mSourceLayer;
  QgsFeatureId fid = feature.id();
  mExpressionContext.setFeature( feature );

  auto styles = QgsConditionalStyle::matchingConditionalStyles( layer->conditionalStyles()->rowStyles(), QVariant(),  mExpressionContext );

  if ( mDisplayExpression.referencedColumns().count() == 1 )
  {
    // Style specific for this field
    QString fieldName = *mDisplayExpression.referencedColumns().constBegin();
    const auto allStyles = layer->conditionalStyles()->fieldStyles( fieldName );
    const auto matchingFieldStyles = QgsConditionalStyle::matchingConditionalStyles( allStyles, feature.attribute( fieldName ),  mExpressionContext );

    styles += matchingFieldStyles;
  }

  QgsConditionalStyle style;
  style = QgsConditionalStyle::compressStyles( styles );
  mEntryStylesMap.insert( fid, style );

  return style;
}

bool QgsFeaturePickerModel::allowNull() const
{
  return mAllowNull;
}

void QgsFeaturePickerModel::setAllowNull( bool allowNull )
{
  if ( mAllowNull == allowNull )
    return;

  mAllowNull = allowNull;
  emit allowNullChanged();

  reload();
}

QgsFeature QgsFeaturePickerModel::currentFeature() const
{
  if ( mCurrentIndex >= 0 && mCurrentIndex < mEntries.count() )
    return mEntries.at( mCurrentIndex ).feature;
  else
    return QgsFeature();
}

void QgsFeaturePickerModel::reload()
{
  mReloadTimer.start();
}
