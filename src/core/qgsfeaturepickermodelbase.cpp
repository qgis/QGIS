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

template <class T>
QgsFeaturePickerModelBase<T>::QgsFeaturePickerModelBase( QObject *parent )
  : QAbstractItemModel( parent )
{
  mReloadTimer.setInterval( 100 );
  mReloadTimer.setSingleShot( true );
  connect( &mReloadTimer, &QTimer::timeout, this, &QgsFeaturePickerModelBase::scheduledReload );
}

template <class T>
QgsFeaturePickerModelBase<T>::~QgsFeaturePickerModelBase()
{
  if ( mGatherer )
    connect( mGatherer, &T::finished, mGatherer, &T::deleteLater );
}

template <class T>
QgsVectorLayer *QgsFeaturePickerModelBase<T>::sourceLayer() const
{
  return mSourceLayer;
}

template <class T>
void QgsFeaturePickerModelBase<T>::setSourceLayer( QgsVectorLayer *sourceLayer )
{
  if ( mSourceLayer == sourceLayer )
    return;

  mSourceLayer = sourceLayer;
  mExpressionContext = sourceLayer->createExpressionContext();
  reload();
  emit sourceLayerChanged();

  setDisplayExpression( sourceLayer->displayExpression() );
}

template <class T>
QString QgsFeaturePickerModelBase<T>::displayExpression() const
{
  return mDisplayExpression.expression();
}

template <class T>
void QgsFeaturePickerModelBase<T>::setDisplayExpression( const QString &displayExpression )
{
  if ( mDisplayExpression.expression() == displayExpression )
    return;

  mDisplayExpression = QgsExpression( displayExpression );
  reload();
  emit displayExpressionChanged();
}

template <class T>
QString QgsFeaturePickerModelBase<T>::filterValue() const
{
  return mFilterValue;
}

template <class T>
void QgsFeaturePickerModelBase<T>::setFilterValue( const QString &filterValue )
{
  if ( mFilterValue == filterValue )
    return;

  mFilterValue = filterValue;
  reload();
  emit filterValueChanged();
}

template <class T>
QString QgsFeaturePickerModelBase<T>::filterExpression() const
{
  return mFilterExpression;
}

template <class T>
void QgsFeaturePickerModelBase<T>::setFilterExpression( const QString &filterExpression )
{
  if ( mFilterExpression == filterExpression )
    return;

  mFilterExpression = filterExpression;
  reload();
  emit filterExpressionChanged();
}

template <class T>
bool QgsFeaturePickerModelBase<T>::isLoading() const
{
  return mGatherer;
}

template <class T>
QModelIndex QgsFeaturePickerModelBase<T>::index( int row, int column, const QModelIndex &parent ) const
{
  Q_UNUSED( parent )
  return createIndex( row, column, nullptr );
}

template <class T>
QModelIndex QgsFeaturePickerModelBase<T>::parent( const QModelIndex &child ) const
{
  Q_UNUSED( child )
  return QModelIndex();
}

template <class T>
int QgsFeaturePickerModelBase<T>::rowCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent )
  return mEntries.size();
}


template <class T>
QVariant QgsFeaturePickerModelBase<T>::data( const QModelIndex &index, int role ) const
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
      const QVariantList values = mEntries.value( index.row() ).identifierValues;
      return values.value( 0 );
    }

    case IdentifierValuesRole:
      return mEntries.value( index.row() ).identifierValues;

    case Qt::BackgroundColorRole:
    case Qt::TextColorRole:
    case Qt::DecorationRole:
    case Qt::FontRole:
    {
      bool isNull = true;
      const QVariantList values = mEntries.value( index.row() ).identifierValues;
      for ( const QVariant &value : values )
      {
        if ( !value.isNull() )
        {
          isNull = false;
          break;
        }
      }
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

template <class T>
void QgsFeaturePickerModelBase<T>::updateCompleter()
{
  emit beginUpdate();

  T *gatherer = qobject_cast<T *>( sender() );
  if ( gatherer->wasCanceled() )
  {
    delete gatherer;
    return;
  }

  QVector<typename T::Entry> entries = mGatherer->entries();

  if ( mExtraValueIndex == -1 )
  {
    setExtraIdentifierValueUnguarded( T::nullEntry() );
  }

  // Only reloading the current entry?
  bool reloadCurrentFeatureOnly = mGatherer->data().toBool();
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

    mShouldReloadCurrentFeature = false;

    if ( mFilterValue.isEmpty() )
      reload();
  }
  else
  {
    // We got strings for a filter selection
    std::sort( entries.begin(), entries.end(), []( const EntryType & a, const EntryType & b ) { return a.value.localeAwareCompare( b.value ) < 0; } );

    if ( mAllowNull )
    {
      entries.prepend( T::nullEntry() );
    }

    const int newEntriesSize = entries.size();

    // fixed entry is either NULL or extra value
    const int nbFixedEntry = ( mExtraValueDoesNotExist ? 1 : 0 ) + ( mAllowNull ? 1 : 0 );

    // Find the index of the current entry in the new list
    int currentEntryInNewList = -1;
    if ( mExtraValueIndex != -1 && mExtraValueIndex < mEntries.count() )
    {
      for ( int i = 0; i < newEntriesSize; ++i )
      {
        if ( mGatherer->compareEntries( entries.at( i ), mEntries.at( mExtraValueIndex ) ) )
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
  }
  emit endUpdate();


  // scheduleReload and updateCompleter lives in the same thread so if the gatherer hasn't been stopped
  // (checked before), mGatherer still references the current gatherer
  Q_ASSERT( gatherer == mGatherer );
  delete mGatherer;
  mGatherer = nullptr;
  emit isLoadingChanged();
}

template<class T>
void QgsFeaturePickerModelBase<T>::scheduledReload()
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
      request.setFilterExpression( filterClause );
  }
  QSet<QString> attributes = requestedAttributes();
  if ( !attributes.isEmpty() )
  {
    if ( request.filterExpression() )
      attributes += request.filterExpression()->referencedColumns();
    attributes += requestedAttributesForStyle();

    request.setSubsetOfAttributes( attributes, mSourceLayer->fields() );
  }

  request.setFlags( QgsFeatureRequest::NoGeometry );
  request.setLimit( QgsSettings().value( QStringLiteral( "maxEntriesRelationWidget" ), 100, QgsSettings::Gui ).toInt() );

  mGatherer = new QgsFeatureExpressionValuesGatherer( mSourceLayer, mDisplayExpression, request, mIdentifierFields );
  mGatherer->setData( mShouldReloadCurrentFeature );
  connect( mGatherer, &T::finished, this, &T::updateCompleter );


  mGatherer->start();
  if ( !wasLoading )
    emit isLoadingChanged();
}

template<class T>
T QgsFeaturePickerModelBase<T>::createValuesGatherer( const QgsFeatureRequest &request ) const
{
  new QgsFeatureExpressionValuesGatherer( mSourceLayer, mDisplayExpression, request );
}

template<class T>
QSet<QString> QgsFeaturePickerModelBase<T>::requestedAttributesForStyle() const
{
  QSet<QString> requestedAttrs;

  const auto rowStyles = mSourceLayer->conditionalStyles()->rowStyles();

  for ( const QgsConditionalStyle &style : rowStyles )
  {
    QgsExpression exp( style.rule() );
    requestedAttrs += exp.referencedColumns();
  }

  if ( mDisplayExpression.isField() )
  {
    QString fieldName = *mDisplayExpression.referencedColumns().constBegin();
    const auto constFieldStyles = mSourceLayer->conditionalStyles()->fieldStyles( fieldName );
    for ( const QgsConditionalStyle &style : constFieldStyles )
    {
      QgsExpression exp( style.rule() );
      requestedAttrs += exp.referencedColumns();
    }
  }

  return requestedAttrs;
}

template<class T>
void QgsFeaturePickerModelBase<T>::setExtraIdentifierValueIndex( int index, bool force )
{
  if ( mExtraValueIndex == index && !force )
    return;

  mExtraValueIndex = index;
  emit extraIdentifierValueIndexChanged( index );
}

template<class T>
void QgsFeaturePickerModelBase<T>::reloadCurrentFeature()
{
  mShouldReloadCurrentFeature = true;
  mReloadTimer.start();
}

template<class T>
void QgsFeaturePickerModelBase<T>::setExtraIdentifierValueUnguarded( const IdentifierType &identifierValue )
{
  const QVector<EntryType> entries = mEntries;

  int index = 0;
  for ( const EntryType &entry : entries )
  {
    if ( entry.identifier == identifierValue )
    {
      setExtraIdentifierValueIndex( index );
      break;
    }

    index++;
  }

  // Value not found in current entries
  if ( mExtraValueIndex != index )
  {
    bool identifierIsNull = mGatherer->identifierIsNull( identifierValue );
    if ( !identifierIsNull || mAllowNull )
    {
      beginInsertRows( QModelIndex(), 0, 0 );
      if ( !identifierIsNull )
      {
        mEntries.prepend( EntryType( identifierValue, mGatherer->identifierToString( identifierValue ), QgsFeature() ) );
        reloadCurrentFeature();
      }
      else
      {
        mEntries.prepend( T::nullEntry() );
      }
      endInsertRows();

      setExtraIdentifierValueIndex( 0, true );
    }
  }
}

template<class T>
QgsConditionalStyle QgsFeaturePickerModelBase<T>::featureStyle( const QgsFeature &feature ) const
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

template<class T>
bool QgsFeaturePickerModelBase<T>::allowNull() const
{
  return mAllowNull;
}

template<class T>
void QgsFeaturePickerModelBase<T>::setAllowNull( bool allowNull )
{
  if ( mAllowNull == allowNull )
    return;

  mAllowNull = allowNull;
  emit allowNullChanged();

  reload();
}

template<class T>
bool QgsFeaturePickerModelBase<T>::extraValueDoesNotExist() const
{
  return mExtraValueDoesNotExist;
}

template<class T>
void QgsFeaturePickerModelBase<T>::setExtraValueDoesNotExist( bool extraValueDoesNotExist )
{
  if ( mExtraValueDoesNotExist == extraValueDoesNotExist )
    return;

  mExtraValueDoesNotExist = extraValueDoesNotExist;
  emit extraValueDoesNotExistChanged();
}

template<class T>
int QgsFeaturePickerModelBase<T>::extraIdentifierValueIndex() const
{
  return mExtraValueIndex;
}

template<class T>
void QgsFeaturePickerModelBase<T>::reload()
{
  mReloadTimer.start();
}

template<class T>
void QgsFeaturePickerModelBase<T>::setExtraIdentifierValue( const IdentifierType &extraIdentifierValue )
{
  if ( extraIdentifierValue == mExtraIdentifierValue && !mGatherer->identifierIsNull( extraIdentifierValue ) )
    return;

  if ( mIsSettingExtraIdentifierValue )
    return;

  mIsSettingExtraIdentifierValue = true;

  mExtraIdentifierValue = extraIdentifierValue;

  setExtraIdentifierValueUnguarded( extraIdentifierValue );

  mIsSettingExtraIdentifierValue = false;

  emit extraIdentifierValueChanged();
}

