/***************************************************************************
  qgsfeaturepickermodelbase.cpp - QgsFeaturePickerModelBase
 ---------------------
 begin                : 10.3.2017
 copyright            : (C) 2017 by Matthias Kuhn
 email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsfeaturepickermodelbase.h"
#include "qgsfeatureexpressionvaluesgatherer.h"

#include "qgsvectorlayer.h"
#include "qgsconditionalstyle.h"
#include "qgsapplication.h"
#include "qgssettings.h"
#include "qgsexpressioncontextutils.h"

QgsFeaturePickerModelBase::QgsFeaturePickerModelBase( QObject *parent )
  : QAbstractItemModel( parent )
{
  mReloadTimer.setInterval( 100 );
  mReloadTimer.setSingleShot( true );
  connect( &mReloadTimer, &QTimer::timeout, this, &QgsFeaturePickerModelBase::scheduledReload );

  // The fact that the feature changed is a combination of the 2 signals:
  // If the extra value is set to a feature currently not fetched, it will go through an intermediate step while the extra value does not exist (as it call reloadFeature)
  connect( this, &QgsFeaturePickerModelBase::extraIdentifierValueChanged, this, &QgsFeaturePickerModelBase::currentFeatureChanged );
  connect( this, &QgsFeaturePickerModelBase::extraValueDoesNotExistChanged, this, &QgsFeaturePickerModelBase::currentFeatureChanged );
}


QgsFeaturePickerModelBase::~QgsFeaturePickerModelBase()
{
  if ( mGatherer )
    connect( mGatherer, &QgsFeatureExpressionValuesGatherer::finished, mGatherer, &QgsFeatureExpressionValuesGatherer::deleteLater );
}


QgsVectorLayer *QgsFeaturePickerModelBase::sourceLayer() const
{
  return mSourceLayer;
}


void QgsFeaturePickerModelBase::setSourceLayer( QgsVectorLayer *sourceLayer )
{
  if ( mSourceLayer == sourceLayer )
    return;

  mSourceLayer = sourceLayer;
  if ( mSourceLayer )
    mExpressionContext = mSourceLayer->createExpressionContext();

  reload();
  emit sourceLayerChanged();

  if ( mSourceLayer )
    setDisplayExpression( mSourceLayer->displayExpression() );
}


QString QgsFeaturePickerModelBase::displayExpression() const
{
  return mDisplayExpression.expression();
}


void QgsFeaturePickerModelBase::setDisplayExpression( const QString &displayExpression )
{
  if ( mDisplayExpression.expression() == displayExpression )
    return;

  mDisplayExpression = QgsExpression( displayExpression );
  reload();
  emit displayExpressionChanged();
}


QString QgsFeaturePickerModelBase::filterValue() const
{
  return mFilterValue;
}


void QgsFeaturePickerModelBase::setFilterValue( const QString &filterValue )
{
  if ( mFilterValue == filterValue )
    return;

  mFilterValue = filterValue;
  reload();
  emit filterValueChanged();
}


QString QgsFeaturePickerModelBase::filterExpression() const
{
  return mFilterExpression;
}


void QgsFeaturePickerModelBase::setFilterExpression( const QString &filterExpression )
{
  if ( mFilterExpression == filterExpression )
    return;

  mFilterExpression = filterExpression;
  reload();
  emit filterExpressionChanged();
}


bool QgsFeaturePickerModelBase::isLoading() const
{
  return mGatherer;
}

QVariant QgsFeaturePickerModelBase::extraIdentifierValue() const
{
  return mExtraIdentifierValue;
}


QModelIndex QgsFeaturePickerModelBase::index( int row, int column, const QModelIndex &parent ) const
{
  Q_UNUSED( parent )
  return createIndex( row, column, nullptr );
}


QModelIndex QgsFeaturePickerModelBase::parent( const QModelIndex &child ) const
{
  Q_UNUSED( child )
  return QModelIndex();
}


int QgsFeaturePickerModelBase::rowCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent )
  return mEntries.size();
}



QVariant QgsFeaturePickerModelBase::data( const QModelIndex &index, int role ) const
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
      return mEntries.value( index.row() ).featureId;

    case FeatureRole:
      return mEntries.value( index.row() ).feature;

    case IdentifierValueRole:
    {
      const QVariantList values = mEntries.value( index.row() ).identifierFields;
      return values.value( 0 );
    }

    case IdentifierValuesRole:
      return mEntries.value( index.row() ).identifierFields;

    case Qt::BackgroundRole:
    case Qt::ForegroundRole:
    case Qt::DecorationRole:
    case Qt::FontRole:
    {
      const bool isNull = identifierIsNull( entryIdentifier( mEntries.value( index.row() ) ) );
      if ( isNull )
      {
        // Representation for NULL value
        if ( role == Qt::ForegroundRole )
        {
          return QBrush( QColor( Qt::gray ) );
        }
        if ( role == Qt::FontRole )
        {
          QFont font = QFont();
          if ( index.row() == mExtraValueIndex )
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
          if ( role == Qt::BackgroundRole && style.validBackgroundColor() )
            return style.backgroundColor();
          if ( role == Qt::ForegroundRole && style.validTextColor() )
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


void QgsFeaturePickerModelBase::updateCompleter()
{
  emit beginUpdate();

  QgsFeatureExpressionValuesGatherer *gatherer = qobject_cast<QgsFeatureExpressionValuesGatherer *>( sender() );
  if ( gatherer->wasCanceled() )
  {
    delete gatherer;
    return;
  }

  QVector<QgsFeatureExpressionValuesGatherer::Entry> entries = mGatherer->entries();

  if ( mExtraValueIndex == -1 )
  {
    setExtraIdentifierValueUnguarded( nullIdentifier() );
  }

  // Only reloading the current entry?
  const bool reloadCurrentFeatureOnly = mGatherer->data().toBool();
  if ( reloadCurrentFeatureOnly )
  {
    if ( !entries.isEmpty() )
    {
      mEntries.replace( mExtraValueIndex, entries.at( 0 ) );
      emit dataChanged( index( mExtraValueIndex, 0, QModelIndex() ), index( mExtraValueIndex, 0, QModelIndex() ) );
      mShouldReloadCurrentFeature = false;
      setExtraValueDoesNotExist( false );
    }
    else
    {
      setExtraValueDoesNotExist( true );
    }

    mKeepCurrentEntry = true;
    mShouldReloadCurrentFeature = false;

    if ( mFilterValue.isEmpty() )
      reload();
  }
  else
  {
    // We got strings for a filter selection
    std::sort( entries.begin(), entries.end(), []( const QgsFeatureExpressionValuesGatherer::Entry & a, const QgsFeatureExpressionValuesGatherer::Entry & b ) { return a.value.localeAwareCompare( b.value ) < 0; } );

    if ( mAllowNull && mSourceLayer )
    {
      entries.prepend( QgsFeatureExpressionValuesGatherer::nullEntry( mSourceLayer ) );
    }

    const int newEntriesSize = entries.size();

    // fixed entry is either NULL or extra value
    const int nbFixedEntry = ( mKeepCurrentEntry ? 1 : 0 ) + ( mAllowNull ? 1 : 0 );

    // Find the index of the current entry in the new list
    int currentEntryInNewList = -1;
    if ( mExtraValueIndex != -1 && mExtraValueIndex < mEntries.count() )
    {
      for ( int i = 0; i < newEntriesSize; ++i )
      {
        if ( compareEntries( entries.at( i ), mEntries.at( mExtraValueIndex ) ) )
        {
          mEntries.replace( mExtraValueIndex, entries.at( i ) );
          currentEntryInNewList = i;
          setExtraValueDoesNotExist( false );
          break;
        }
      }
    }

    int firstRow = 0;

    // Move current entry to the first position if this is a fixed entry or because
    // the entry exists in the new list
    if ( mExtraValueIndex > -1 && ( mExtraValueIndex < nbFixedEntry || currentEntryInNewList != -1 ) )
    {
      if ( mExtraValueIndex != 0 )
      {
        beginMoveRows( QModelIndex(), mExtraValueIndex, mExtraValueIndex, QModelIndex(), 0 );
        mEntries.move( mExtraValueIndex, 0 );
        endMoveRows();
      }
      firstRow = 1;
    }

    // Remove all entries (except for extra entry if existent)
    beginRemoveRows( QModelIndex(), firstRow, mEntries.size() - firstRow );
    mEntries.remove( firstRow, mEntries.size() - firstRow );

    // if we remove all rows before endRemoveRows, setExtraIdentifierValuesUnguarded will be called
    // and a null value will be added to mEntries, so we block setExtraIdentifierValuesUnguarded call

    mIsSettingExtraIdentifierValue = true;
    endRemoveRows();
    mIsSettingExtraIdentifierValue = false;

    if ( currentEntryInNewList == -1 )
    {
      beginInsertRows( QModelIndex(), firstRow, entries.size() + 1 );
      mEntries += entries;
      endInsertRows();

      // if all entries have been cleaned (firstRow == 0)
      // and there is a value in entries, prefer this value over NULL
      // else chose the first one (the previous one)
      setExtraIdentifierValueIndex( firstRow == 0 && mAllowNull && !entries.isEmpty() ? 1 : 0, firstRow == 0 );
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
      setExtraIdentifierValueIndex( currentEntryInNewList );
    }

    emit filterJobCompleted();

    mKeepCurrentEntry = false;
  }
  emit endUpdate();

  // scheduleReload and updateCompleter lives in the same thread so if the gatherer hasn't been stopped
  // (checked before), mGatherer still references the current gatherer
  Q_ASSERT( gatherer == mGatherer );
  delete mGatherer;
  mGatherer = nullptr;
  emit isLoadingChanged();
}


void QgsFeaturePickerModelBase::scheduledReload()
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

  if ( mShouldReloadCurrentFeature )
  {
    requestToReloadCurrentFeature( request );
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
    {
      request.setFilterExpression( filterClause );
      request.expressionContext()->appendScopes( QgsExpressionContextUtils::globalProjectLayerScopes( sourceLayer() ) );
    }
  }
  QSet<QString> attributes = requestedAttributes();
  if ( !attributes.isEmpty() )
  {
    if ( auto *lFilterExpression = request.filterExpression() )
      attributes += lFilterExpression->referencedColumns();
    attributes += requestedAttributesForStyle();

    request.setSubsetOfAttributes( attributes, mSourceLayer->fields() );
  }

  if ( !mFetchGeometry )
    request.setFlags( QgsFeatureRequest::NoGeometry );
  if ( mFetchLimit > 0 )
    request.setLimit( mFetchLimit );

  mGatherer = createValuesGatherer( request );
  mGatherer->setData( mShouldReloadCurrentFeature );
  connect( mGatherer, &QgsFeatureExpressionValuesGatherer::finished, this, &QgsFeaturePickerModelBase::updateCompleter );

  mGatherer->start();
  if ( !wasLoading )
    emit isLoadingChanged();
}


QSet<QString> QgsFeaturePickerModelBase::requestedAttributesForStyle() const
{
  QSet<QString> requestedAttrs;

  const auto rowStyles = mSourceLayer->conditionalStyles()->rowStyles();

  for ( const QgsConditionalStyle &style : rowStyles )
  {
    const QgsExpression exp( style.rule() );
    requestedAttrs += exp.referencedColumns();
  }

  if ( mDisplayExpression.isField() )
  {
    const QString fieldName = *mDisplayExpression.referencedColumns().constBegin();
    const auto constFieldStyles = mSourceLayer->conditionalStyles()->fieldStyles( fieldName );
    for ( const QgsConditionalStyle &style : constFieldStyles )
    {
      const QgsExpression exp( style.rule() );
      requestedAttrs += exp.referencedColumns();
    }
  }

  return requestedAttrs;
}


void QgsFeaturePickerModelBase::setExtraIdentifierValueIndex( int index, bool force )
{
  if ( mExtraValueIndex == index && !force )
    return;

  mExtraValueIndex = index;
  emit extraIdentifierValueIndexChanged( index );
}


void QgsFeaturePickerModelBase::reloadCurrentFeature()
{
  mShouldReloadCurrentFeature = true;
  mReloadTimer.start();
}


void QgsFeaturePickerModelBase::setExtraIdentifierValueUnguarded( const QVariant &identifierValue )
{
  const QVector<QgsFeatureExpressionValuesGatherer::Entry> entries = mEntries;

  int index = 0;
  for ( const QgsFeatureExpressionValuesGatherer::Entry &entry : entries )
  {
    if ( compareEntries( entry, createEntry( identifierValue ) ) )
    {
      setExtraIdentifierValueIndex( index );
      break;
    }

    index++;
  }

  // Value not found in current entries
  if ( mExtraValueIndex != index )
  {
    const bool isNull = identifierIsNull( identifierValue );
    if ( !isNull || mAllowNull )
    {
      beginInsertRows( QModelIndex(), 0, 0 );
      if ( !isNull )
      {
        mEntries.prepend( createEntry( identifierValue ) );
        setExtraValueDoesNotExist( true );
        reloadCurrentFeature();
      }
      else
      {
        mEntries.prepend( QgsFeatureExpressionValuesGatherer::nullEntry( mSourceLayer ) );
        setExtraValueDoesNotExist( false );
      }
      endInsertRows();

      setExtraIdentifierValueIndex( 0, true );
    }
  }
}


QgsConditionalStyle QgsFeaturePickerModelBase::featureStyle( const QgsFeature &feature ) const
{
  if ( !mSourceLayer )
    return QgsConditionalStyle();

  QgsVectorLayer *layer = mSourceLayer;
  const QgsFeatureId fid = feature.id();
  mExpressionContext.setFeature( feature );

  auto styles = QgsConditionalStyle::matchingConditionalStyles( layer->conditionalStyles()->rowStyles(), QVariant(),  mExpressionContext );

  if ( mDisplayExpression.referencedColumns().count() == 1 )
  {
    // Style specific for this field
    const QString fieldName = *mDisplayExpression.referencedColumns().constBegin();
    const auto allStyles = layer->conditionalStyles()->fieldStyles( fieldName );
    const auto matchingFieldStyles = QgsConditionalStyle::matchingConditionalStyles( allStyles, feature.attribute( fieldName ),  mExpressionContext );

    styles += matchingFieldStyles;
  }

  QgsConditionalStyle style;
  style = QgsConditionalStyle::compressStyles( styles );
  mEntryStylesMap.insert( fid, style );

  return style;
}


bool QgsFeaturePickerModelBase::allowNull() const
{
  return mAllowNull;
}


void QgsFeaturePickerModelBase::setAllowNull( bool allowNull )
{
  if ( mAllowNull == allowNull )
    return;

  mAllowNull = allowNull;
  emit allowNullChanged();

  reload();
}

bool QgsFeaturePickerModelBase::fetchGeometry() const
{
  return mFetchGeometry;
}

void QgsFeaturePickerModelBase::setFetchGeometry( bool fetchGeometry )
{
  if ( mFetchGeometry == fetchGeometry )
    return;

  mFetchGeometry = fetchGeometry;
  reload();
}

int QgsFeaturePickerModelBase::fetchLimit() const
{
  return mFetchLimit;
}

void QgsFeaturePickerModelBase::setFetchLimit( int fetchLimit )
{
  if ( fetchLimit == mFetchLimit )
    return;

  mFetchLimit = fetchLimit;
  emit fetchLimitChanged();
}


bool QgsFeaturePickerModelBase::extraValueDoesNotExist() const
{
  return mExtraValueDoesNotExist;
}


void QgsFeaturePickerModelBase::setExtraValueDoesNotExist( bool extraValueDoesNotExist )
{
  if ( mExtraValueDoesNotExist == extraValueDoesNotExist )
    return;

  mExtraValueDoesNotExist = extraValueDoesNotExist;
  emit extraValueDoesNotExistChanged();
}


int QgsFeaturePickerModelBase::extraIdentifierValueIndex() const
{
  return mExtraValueIndex;
}


void QgsFeaturePickerModelBase::reload()
{
  mReloadTimer.start();
}


void QgsFeaturePickerModelBase::setExtraIdentifierValue( const QVariant &extraIdentifierValue )
{
  if ( extraIdentifierValue == mExtraIdentifierValue && !identifierIsNull( extraIdentifierValue ) && !identifierIsNull( mExtraIdentifierValue ) )
    return;

  if ( mIsSettingExtraIdentifierValue )
    return;

  mIsSettingExtraIdentifierValue = true;

  mExtraIdentifierValue = extraIdentifierValue;

  setExtraIdentifierValueUnguarded( extraIdentifierValue );

  mIsSettingExtraIdentifierValue = false;

  emit extraIdentifierValueChanged();
}

