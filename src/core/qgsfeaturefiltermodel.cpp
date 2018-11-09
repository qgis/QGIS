/***************************************************************************
  qgsfeaturefiltermodel.cpp - QgsFeatureFilterModel

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
#include "qgsfeaturefiltermodel.h"
#include "qgsfeaturefiltermodel_p.h"

#include "qgsvectorlayer.h"
#include "qgsconditionalstyle.h"

QgsFeatureFilterModel::QgsFeatureFilterModel( QObject *parent )
  : QAbstractItemModel( parent )
{
  mReloadTimer.setInterval( 100 );
  mReloadTimer.setSingleShot( true );
  connect( &mReloadTimer, &QTimer::timeout, this, &QgsFeatureFilterModel::scheduledReload );
  setExtraIdentifierValueUnguarded( QVariant() );
}

QgsFeatureFilterModel::~QgsFeatureFilterModel()
{
  if ( mGatherer )
    connect( mGatherer, &QgsFieldExpressionValuesGatherer::finished, mGatherer, &QgsFieldExpressionValuesGatherer::deleteLater );
}

QgsVectorLayer *QgsFeatureFilterModel::sourceLayer() const
{
  return mSourceLayer;
}

void QgsFeatureFilterModel::setSourceLayer( QgsVectorLayer *sourceLayer )
{
  if ( mSourceLayer == sourceLayer )
    return;

  mSourceLayer = sourceLayer;
  mExpressionContext = sourceLayer->createExpressionContext();
  reload();
  emit sourceLayerChanged();
}

QString QgsFeatureFilterModel::displayExpression() const
{
  return mDisplayExpression.expression();
}

void QgsFeatureFilterModel::setDisplayExpression( const QString &displayExpression )
{
  if ( mDisplayExpression.expression() == displayExpression )
    return;

  mDisplayExpression = QgsExpression( displayExpression );
  reload();
  emit displayExpressionChanged();
}

QString QgsFeatureFilterModel::filterValue() const
{
  return mFilterValue;
}

void QgsFeatureFilterModel::setFilterValue( const QString &filterValue )
{
  if ( mFilterValue == filterValue )
    return;

  mFilterValue = filterValue;
  reload();
  emit filterValueChanged();
}

QString QgsFeatureFilterModel::filterExpression() const
{
  return mFilterExpression;
}

void QgsFeatureFilterModel::setFilterExpression( const QString &filterExpression )
{
  if ( mFilterExpression == filterExpression )
    return;

  mFilterExpression = filterExpression;
  reload();
  emit filterExpressionChanged();
}

bool QgsFeatureFilterModel::isLoading() const
{
  return mGatherer;
}

QModelIndex QgsFeatureFilterModel::index( int row, int column, const QModelIndex &parent ) const
{
  Q_UNUSED( parent )
  return createIndex( row, column, nullptr );
}

QModelIndex QgsFeatureFilterModel::parent( const QModelIndex &child ) const
{
  Q_UNUSED( child )
  return QModelIndex();
}

int QgsFeatureFilterModel::rowCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent );

  return mEntries.size();
}

int QgsFeatureFilterModel::columnCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent )
  return 1;
}

QVariant QgsFeatureFilterModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() )
    return QVariant();

  switch ( role )
  {
    case Qt::DisplayRole:
    case Qt::EditRole:
    case ValueRole:
      return mEntries.value( index.row() ).value;

    case IdentifierValueRole:
      return mEntries.value( index.row() ).identifierValue;

    case Qt::BackgroundColorRole:
    case Qt::TextColorRole:
    case Qt::DecorationRole:
    case Qt::FontRole:
    {
      if ( mEntries.value( index.row() ).identifierValue.isNull() )
      {
        // Representation for NULL value
        if ( role == Qt::TextColorRole )
        {
          return QBrush( QColor( Qt::gray ) );
        }
        else if ( role == Qt::FontRole )
        {
          QFont font = QFont();
          if ( index.row() == mExtraIdentifierValueIndex )
            font.setBold( true );

          if ( mEntries.value( index.row() ).identifierValue.isNull() )
          {
            font.setItalic( true );
          }
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

void QgsFeatureFilterModel::updateCompleter()
{
  emit beginUpdate();
  QVector<Entry> entries = mGatherer->entries();

  if ( mExtraIdentifierValueIndex == -1 )
    setExtraIdentifierValueUnguarded( QVariant() );

  // Only reloading the current entry?
  if ( mGatherer->data().toBool() )
  {
    if ( !entries.isEmpty() )
    {
      mEntries.replace( mExtraIdentifierValueIndex, entries.at( 0 ) );
      emit dataChanged( index( mExtraIdentifierValueIndex, 0, QModelIndex() ), index( mExtraIdentifierValueIndex, 0, QModelIndex() ) );
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
    std::sort( entries.begin(), entries.end(), []( const Entry & a, const Entry & b ) { return a.value.localeAwareCompare( b.value ) < 0; } );

    if ( mAllowNull )
      entries.prepend( Entry( QVariant( QVariant::Int ), QgsApplication::nullRepresentation(), QgsFeature() ) );

    const int newEntriesSize = entries.size();

    // Find the index of the extra entry in the new list
    int currentEntryInNewList = -1;
    if ( mExtraIdentifierValueIndex != -1 )
    {
      for ( int i = 0; i < newEntriesSize; ++i )
      {
        if ( entries.at( i ).identifierValue == mExtraIdentifierValue )
        {
          currentEntryInNewList = i;
          mEntries.replace( mExtraIdentifierValueIndex, entries.at( i ) );
          emit dataChanged( index( mExtraIdentifierValueIndex, 0, QModelIndex() ), index( mExtraIdentifierValueIndex, 0, QModelIndex() ) );
          setExtraValueDoesNotExist( false );
          break;
        }
      }
    }
    else
    {
      Q_ASSERT_X( false, "QgsFeatureFilterModel::updateCompleter", "No extra identifier value generated. Should not get here." );
    }

    int firstRow = 0;

    // Move the extra entry to the first position
    if ( mExtraIdentifierValueIndex != -1 )
    {
      if ( mExtraIdentifierValueIndex != 0 )
      {
        beginMoveRows( QModelIndex(), mExtraIdentifierValueIndex, mExtraIdentifierValueIndex, QModelIndex(), 0 );
        mEntries.move( mExtraIdentifierValueIndex, 0 );
        endMoveRows();
      }
      firstRow = 1;
    }

    // Remove all entries (except for extra entry if existent)
    beginRemoveRows( QModelIndex(), firstRow, mEntries.size() - firstRow );
    mEntries.remove( firstRow, mEntries.size() - firstRow );
    endRemoveRows();

    if ( currentEntryInNewList == -1 )
    {
      beginInsertRows( QModelIndex(), 1, entries.size() + 1 );
      mEntries += entries;
      endInsertRows();
      setExtraIdentifierValueIndex( 0 );
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

      emit dataChanged( index( currentEntryInNewList, 0, QModelIndex() ), index( currentEntryInNewList, 0, QModelIndex() ) );

      beginInsertRows( QModelIndex(), currentEntryInNewList + 1, newEntriesSize - currentEntryInNewList - 1 );
      mEntries += entries.mid( currentEntryInNewList + 1 );
      endInsertRows();
      setExtraIdentifierValueIndex( currentEntryInNewList );
    }

    emit filterJobCompleted();
  }
  emit endUpdate();
}

void QgsFeatureFilterModel::gathererThreadFinished()
{
  delete mGatherer;
  mGatherer = nullptr;
  emit isLoadingChanged();
}

void QgsFeatureFilterModel::scheduledReload()
{
  if ( !mSourceLayer )
    return;

  bool wasLoading = false;

  if ( mGatherer )
  {
    // Send the gatherer thread to the graveyard:
    //   forget about it, tell it to stop and delete when finished
    disconnect( mGatherer, &QgsFieldExpressionValuesGatherer::collectedValues, this, &QgsFeatureFilterModel::updateCompleter );
    disconnect( mGatherer, &QgsFieldExpressionValuesGatherer::finished, this, &QgsFeatureFilterModel::gathererThreadFinished );
    connect( mGatherer, &QgsFieldExpressionValuesGatherer::finished, mGatherer, &QgsFieldExpressionValuesGatherer::deleteLater );
    mGatherer->stop();
    wasLoading = true;
  }

  QgsFeatureRequest request;

  if ( mShouldReloadCurrentFeature )
  {
    request.setFilterExpression( QStringLiteral( "%1 = %2" ).arg( QgsExpression::quotedColumnRef( mIdentifierField ), QgsExpression::quotedValue( mExtraIdentifierValue ) ) );
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
  QSet<QString> attributes;
  if ( request.filterExpression() )
    attributes = request.filterExpression()->referencedColumns();
  attributes << mIdentifierField;
  request.setSubsetOfAttributes( attributes, mSourceLayer->fields() );
  request.setFlags( QgsFeatureRequest::NoGeometry );

  request.setLimit( 100 );

  mGatherer = new QgsFieldExpressionValuesGatherer( mSourceLayer, mDisplayExpression, mIdentifierField, request );
  mGatherer->setData( mShouldReloadCurrentFeature );

  connect( mGatherer, &QgsFieldExpressionValuesGatherer::collectedValues, this, &QgsFeatureFilterModel::updateCompleter );
  connect( mGatherer, &QgsFieldExpressionValuesGatherer::finished, this, &QgsFeatureFilterModel::gathererThreadFinished );

  mGatherer->start();
  if ( !wasLoading )
    emit isLoadingChanged();
}

QSet<QString> QgsFeatureFilterModel::requestedAttributes() const
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
    Q_FOREACH ( const QgsConditionalStyle &style, mSourceLayer->conditionalStyles()->fieldStyles( fieldName ) )
    {
      QgsExpression exp( style.rule() );
      requestedAttrs += exp.referencedColumns();
    }
  }

  return requestedAttrs;
}

void QgsFeatureFilterModel::setExtraIdentifierValueIndex( int index, bool force )
{
  if ( mExtraIdentifierValueIndex == index && !force )
    return;

  mExtraIdentifierValueIndex = index;
  emit extraIdentifierValueIndexChanged( index );
}

void QgsFeatureFilterModel::reloadCurrentFeature()
{
  mShouldReloadCurrentFeature = true;
  mReloadTimer.start();
}

void QgsFeatureFilterModel::setExtraIdentifierValueUnguarded( const QVariant &extraIdentifierValue )
{
  const QVector<Entry> entries = mEntries;

  int index = 0;
  for ( const Entry &entry : entries )
  {
    if ( entry.identifierValue == extraIdentifierValue
         && entry.identifierValue.isNull() == extraIdentifierValue.isNull()
         && entry.identifierValue.isValid() == extraIdentifierValue.isValid() )
    {
      setExtraIdentifierValueIndex( index );
      break;
    }

    index++;
  }

  // Value not found in current entries
  if ( mExtraIdentifierValueIndex != index )
  {
    beginInsertRows( QModelIndex(), 0, 0 );
    if ( extraIdentifierValue.isNull() )
      mEntries.prepend( Entry( QVariant( QVariant::Int ), QgsApplication::nullRepresentation( ), QgsFeature() ) );
    else
      mEntries.prepend( Entry( extraIdentifierValue, QStringLiteral( "(%1)" ).arg( extraIdentifierValue.toString() ), QgsFeature() ) );
    endInsertRows();

    setExtraIdentifierValueIndex( 0, true );

    reloadCurrentFeature();
  }
}

QgsConditionalStyle QgsFeatureFilterModel::featureStyle( const QgsFeature &feature ) const
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

bool QgsFeatureFilterModel::allowNull() const
{
  return mAllowNull;
}

void QgsFeatureFilterModel::setAllowNull( bool allowNull )
{
  if ( mAllowNull == allowNull )
    return;

  mAllowNull = allowNull;
  emit allowNullChanged();

  reload();
}

bool QgsFeatureFilterModel::extraValueDoesNotExist() const
{
  return mExtraValueDoesNotExist;
}

void QgsFeatureFilterModel::setExtraValueDoesNotExist( bool extraValueDoesNotExist )
{
  if ( mExtraValueDoesNotExist == extraValueDoesNotExist )
    return;

  mExtraValueDoesNotExist = extraValueDoesNotExist;
  emit extraValueDoesNotExistChanged();
}

int QgsFeatureFilterModel::extraIdentifierValueIndex() const
{
  return mExtraIdentifierValueIndex;
}

QString QgsFeatureFilterModel::identifierField() const
{
  return mIdentifierField;
}

void QgsFeatureFilterModel::setIdentifierField( const QString &identifierField )
{
  if ( mIdentifierField == identifierField )
    return;

  mIdentifierField = identifierField;
  emit identifierFieldChanged();
}

void QgsFeatureFilterModel::reload()
{
  mReloadTimer.start();
}

QVariant QgsFeatureFilterModel::extraIdentifierValue() const
{
  return mExtraIdentifierValue;
}

void QgsFeatureFilterModel::setExtraIdentifierValue( const QVariant &extraIdentifierValue )
{
  if ( qgsVariantEqual( extraIdentifierValue, mExtraIdentifierValue ) && mExtraIdentifierValue.isValid() )
    return;

  if ( mIsSettingExtraIdentifierValue )
    return;

  mIsSettingExtraIdentifierValue = true;

  mExtraIdentifierValue = extraIdentifierValue;

  setExtraIdentifierValueUnguarded( extraIdentifierValue );

  mIsSettingExtraIdentifierValue = false;

  emit extraIdentifierValueChanged();
}
