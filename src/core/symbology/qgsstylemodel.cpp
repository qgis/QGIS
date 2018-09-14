/***************************************************************************
    qgsstylemodel.cpp
    ---------------
    begin                : September 2018
    copyright            : (C) 2018 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsstylemodel.h"
#include "qgsstyle.h"
#include "qgssymbollayerutils.h"
#include "qgsapplication.h"
#include "qgssvgcache.h"
#include <QIcon>

const double ICON_PADDING_FACTOR = 0.16;

QgsStyleModel::QgsStyleModel( QgsStyle *style, QObject *parent )
  : QAbstractItemModel( parent )
  , mStyle( style )
{
  Q_ASSERT( mStyle );
  mSymbolNames = mStyle->symbolNames();
  mRampNames = mStyle->colorRampNames();

  connect( mStyle, &QgsStyle::symbolSaved, this, &QgsStyleModel::onSymbolAdded );
  connect( mStyle, &QgsStyle::symbolRemoved, this, &QgsStyleModel::onSymbolRemoved );
  connect( mStyle, &QgsStyle::symbolRenamed, this, &QgsStyleModel::onSymbolRename );
  connect( mStyle, &QgsStyle::symbolChanged, this, &QgsStyleModel::onSymbolChanged );
  connect( mStyle, &QgsStyle::rampAdded, this, &QgsStyleModel::onRampAdded );
  connect( mStyle, &QgsStyle::rampChanged, this, &QgsStyleModel::onRampChanged );
  connect( mStyle, &QgsStyle::rampRemoved, this, &QgsStyleModel::onRampRemoved );
  connect( mStyle, &QgsStyle::rampRenamed, this, &QgsStyleModel::onRampRename );

  connect( mStyle, &QgsStyle::entityTagsChanged, this, &QgsStyleModel::onTagsChanged );

  // when a remote svg has been fetched, update the model's decorations.
  // this is required if a symbol utilizes remote svgs, and the current icons
  // have been generated using the temporary "downloading" svg. In this case
  // we require the preview to be regenerated to use the correct fetched
  // svg
  connect( QgsApplication::svgCache(), &QgsSvgCache::remoteSvgFetched, this, &QgsStyleModel::rebuildSymbolIcons );
}

QVariant QgsStyleModel::data( const QModelIndex &index, int role ) const
{
  if ( index.row() < 0 || index.row() >= rowCount( QModelIndex() ) )
    return QVariant();


  const bool isColorRamp = index.row() >= mStyle->symbolCount();
  const QString name = !isColorRamp
                       ? mSymbolNames.at( index.row() )
                       : mRampNames.at( index.row() - mSymbolNames.size() );

  switch ( role )
  {
    case Qt::DisplayRole:
    case Qt::ToolTipRole:
    case Qt::EditRole:
    {
      switch ( index.column() )
      {
        case Name:
        {
          const QStringList tags = mStyle->tagsOfSymbol( isColorRamp ? QgsStyle::ColorrampEntity : QgsStyle::SymbolEntity, name );
          return role != Qt::ToolTipRole ? name
                 : QStringLiteral( "<b>%1</b><br><i>%2</i>" ).arg( name, tags.count() > 0 ? tags.join( QStringLiteral( ", " ) ) : tr( "Not tagged" ) );
        }
        case Tags:
          return mStyle->tagsOfSymbol( isColorRamp ? QgsStyle::ColorrampEntity : QgsStyle::SymbolEntity, name ).join( QStringLiteral( ", " ) );
      }
      return QVariant();
    }

    case Qt::DecorationRole:
    {
      // Generate icons at all additional sizes specified for the model.
      // This allows the model to have size responsive icons.
      switch ( index.column() )
      {
        case Name:
          if ( !isColorRamp )
          {
            // use cached icon if possible
            QIcon icon = mSymbolIconCache.value( name );
            if ( !icon.isNull() )
              return icon;

            std::unique_ptr< QgsSymbol > symbol( mStyle->symbol( name ) );
            if ( mAdditionalSizes.isEmpty() )
              icon.addPixmap( QgsSymbolLayerUtils::symbolPreviewPixmap( symbol.get(), QSize( 24, 24 ), 1 ) );

            for ( const QVariant &size : mAdditionalSizes )
            {
              QSize s = size.toSize();
              icon.addPixmap( QgsSymbolLayerUtils::symbolPreviewPixmap( symbol.get(), s, static_cast< int >( s.width() * ICON_PADDING_FACTOR ) ) );
            }

            mSymbolIconCache.insert( name, icon );
            return icon;
          }
          else
          {
            // use cached icon if possible
            QIcon icon = mColorRampIconCache.value( name );
            if ( !icon.isNull() )
              return icon;

            std::unique_ptr< QgsColorRamp > ramp( mStyle->colorRamp( name ) );
            if ( mAdditionalSizes.isEmpty() )
              icon.addPixmap( QgsSymbolLayerUtils::colorRampPreviewPixmap( ramp.get(), QSize( 24, 24 ), 1 ) );
            for ( const QVariant &size : mAdditionalSizes )
            {
              QSize s = size.toSize();
              icon.addPixmap( QgsSymbolLayerUtils::colorRampPreviewPixmap( ramp.get(), s, static_cast< int >( s.width() * ICON_PADDING_FACTOR ) ) );
            }

            mColorRampIconCache.insert( name, icon );
            return icon;
          }
        case Tags:
          return QVariant();
      }
      return QVariant();
    }

    case TypeRole:
      return isColorRamp ? QgsStyle::ColorrampEntity : QgsStyle::SymbolEntity;

    case TagRole:
      return mStyle->tagsOfSymbol( isColorRamp ? QgsStyle::ColorrampEntity : QgsStyle::SymbolEntity, name );

    case SymbolTypeRole:
    {
      if ( isColorRamp )
        return QVariant();

      const QgsSymbol *symbol = mStyle->symbolRef( name );
      return symbol ? symbol->type() : QVariant();
    }

    default:
      return QVariant();
  }

  return QVariant();
}

bool QgsStyleModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  if ( index.row() < 0 || index.row() >= rowCount( QModelIndex() ) || role != Qt::EditRole )
    return false;

  switch ( index.column() )
  {
    case Name:
    {
      const bool isColorRamp = index.row() >= mStyle->symbolCount();
      const QString name = !isColorRamp
                           ? mSymbolNames.at( index.row() )
                           : mRampNames.at( index.row() - mSymbolNames.size() );
      const QString newName = value.toString();

      return isColorRamp
             ? mStyle->renameColorRamp( name, newName )
             : mStyle->renameSymbol( name, newName );
    }

    case Tags:
      return false;
  }

  return false;
}

Qt::ItemFlags QgsStyleModel::flags( const QModelIndex &index ) const
{
  Qt::ItemFlags flags = QAbstractItemModel::flags( index );
  if ( index.isValid() && index.column() == Name )
  {
    return flags | Qt::ItemIsEditable;
  }
  else
  {
    return flags;
  }
}

QVariant QgsStyleModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  if ( role == Qt::DisplayRole )
  {
    if ( orientation == Qt::Vertical ) //row
    {
      return QVariant( section );
    }
    else
    {
      switch ( section )
      {
        case Name:
          return QVariant( tr( "Name" ) );

        case Tags:
          return QVariant( tr( "Tags" ) );

        default:
          return QVariant();
      }
    }
  }
  else
  {
    return QVariant();
  }
}

QModelIndex QgsStyleModel::index( int row, int column, const QModelIndex &parent ) const
{
  if ( !hasIndex( row, column, parent ) )
    return QModelIndex();

  if ( !parent.isValid() )
  {
    return createIndex( row, column );
  }

  return QModelIndex();
}

QModelIndex QgsStyleModel::parent( const QModelIndex & ) const
{
  //all items are top level for now
  return QModelIndex();
}

int QgsStyleModel::rowCount( const QModelIndex &parent ) const
{
  if ( !parent.isValid() )
  {
    return mSymbolNames.count() + mRampNames.count();
  }
  return 0;
}

int QgsStyleModel::columnCount( const QModelIndex & ) const
{
  return 2;
}

void QgsStyleModel::addDesiredIconSize( QSize size )
{
  mAdditionalSizes << size;
  mSymbolIconCache.clear();
  mColorRampIconCache.clear();
}

void QgsStyleModel::onSymbolAdded( const QString &name, QgsSymbol * )
{
  mSymbolIconCache.remove( name );
  const QStringList oldSymbolNames = mSymbolNames;
  const QStringList newSymbolNames = mStyle->symbolNames();

  // find index of newly added symbol
  const int newNameIndex = newSymbolNames.indexOf( name );
  if ( newNameIndex < 0 )
    return; // shouldn't happen

  beginInsertRows( QModelIndex(), newNameIndex, newNameIndex );
  mSymbolNames = newSymbolNames;
  endInsertRows();
}

void QgsStyleModel::onSymbolRemoved( const QString &name )
{
  mSymbolIconCache.remove( name );
  const QStringList oldSymbolNames = mSymbolNames;
  const QStringList newSymbolNames = mStyle->symbolNames();

  // find index of removed symbol
  const int oldNameIndex = oldSymbolNames.indexOf( name );
  if ( oldNameIndex < 0 )
    return; // shouldn't happen

  beginRemoveRows( QModelIndex(), oldNameIndex, oldNameIndex );
  mSymbolNames = newSymbolNames;
  endRemoveRows();
}

void QgsStyleModel::onSymbolChanged( const QString &name )
{
  mSymbolIconCache.remove( name );

  QModelIndex i = index( mSymbolNames.indexOf( name ), Tags );
  emit dataChanged( i, i, QVector< int >() << Qt::DecorationRole );
}

void QgsStyleModel::onSymbolRename( const QString &oldName, const QString &newName )
{
  mSymbolIconCache.remove( oldName );
  const QStringList oldSymbolNames = mSymbolNames;
  const QStringList newSymbolNames = mStyle->symbolNames();

  // find index of removed symbol
  const int oldNameIndex = oldSymbolNames.indexOf( oldName );
  if ( oldNameIndex < 0 )
    return; // shouldn't happen

  // find index of added symbol
  const int newNameIndex = newSymbolNames.indexOf( newName );
  if ( newNameIndex < 0 )
    return; // shouldn't happen

  if ( newNameIndex == oldNameIndex )
  {
    mSymbolNames = newSymbolNames;
    return;
  }

  beginMoveRows( QModelIndex(), oldNameIndex, oldNameIndex, QModelIndex(), newNameIndex > oldNameIndex ? newNameIndex + 1 : newNameIndex );
  mSymbolNames = newSymbolNames;
  endMoveRows();
}

void QgsStyleModel::onRampAdded( const QString &name )
{
  mColorRampIconCache.remove( name );
  const QStringList oldRampNames = mRampNames;
  const QStringList newRampNames = mStyle->colorRampNames();

  // find index of newly added symbol
  const int newNameIndex = newRampNames.indexOf( name );
  if ( newNameIndex < 0 )
    return; // shouldn't happen

  beginInsertRows( QModelIndex(), newNameIndex + mSymbolNames.count(), newNameIndex + mSymbolNames.count() );
  mRampNames = newRampNames;
  endInsertRows();
}

void QgsStyleModel::onRampRemoved( const QString &name )
{
  mColorRampIconCache.remove( name );
  const QStringList oldRampNames = mRampNames;
  const QStringList newRampNames = mStyle->colorRampNames();

  // find index of removed symbol
  const int oldNameIndex = oldRampNames.indexOf( name );
  if ( oldNameIndex < 0 )
    return; // shouldn't happen

  beginRemoveRows( QModelIndex(), oldNameIndex + mSymbolNames.count(), oldNameIndex + mSymbolNames.count() );
  mRampNames = newRampNames;
  endRemoveRows();
}

void QgsStyleModel::onRampChanged( const QString &name )
{
  mColorRampIconCache.remove( name );

  QModelIndex i = index( mSymbolNames.count() + mRampNames.indexOf( name ), Tags );
  emit dataChanged( i, i, QVector< int >() << Qt::DecorationRole );
}

void QgsStyleModel::onRampRename( const QString &oldName, const QString &newName )
{
  mColorRampIconCache.remove( oldName );
  const QStringList oldRampNames = mRampNames;
  const QStringList newRampNames = mStyle->colorRampNames();

  // find index of removed ramp
  const int oldNameIndex = oldRampNames.indexOf( oldName );
  if ( oldNameIndex < 0 )
    return; // shouldn't happen

  // find index of newly added ramp
  const int newNameIndex = newRampNames.indexOf( newName );
  if ( newNameIndex < 0 )
    return; // shouldn't happen

  if ( newNameIndex == oldNameIndex )
  {
    mRampNames = newRampNames;
    return;
  }

  beginMoveRows( QModelIndex(), oldNameIndex + mSymbolNames.count(), oldNameIndex + mSymbolNames.count(),
                 QModelIndex(), ( newNameIndex > oldNameIndex ? newNameIndex + 1 : newNameIndex ) + mSymbolNames.count() );
  mRampNames = newRampNames;
  endMoveRows();
}

void QgsStyleModel::onTagsChanged( int entity, const QString &name, const QStringList & )
{
  QModelIndex i;
  if ( entity == QgsStyle::SymbolEntity )
  {
    i = index( mSymbolNames.indexOf( name ), Tags );
  }
  else if ( entity == QgsStyle::ColorrampEntity )
  {
    i = index( mSymbolNames.count() + mRampNames.indexOf( name ), Tags );
  }
  emit dataChanged( i, i );
}

void QgsStyleModel::rebuildSymbolIcons()
{
  mSymbolIconCache.clear();
  emit dataChanged( index( 0, 0 ), index( mSymbolNames.count() - 1, 0 ), QVector<int>() << Qt::DecorationRole );
}

//
// QgsStyleProxyModel
//

QgsStyleProxyModel::QgsStyleProxyModel( QgsStyle *style, QObject *parent )
  : QSortFilterProxyModel( parent )
  , mStyle( style )
{
  mModel = new QgsStyleModel( mStyle, this );
  setSortCaseSensitivity( Qt::CaseInsensitive );
//  setSortLocaleAware( true );
  setSourceModel( mModel );
  setDynamicSortFilter( true );
  sort( 0 );

  connect( mStyle, &QgsStyle::entityTagsChanged, this, [ = ]
  {
    // update tagged symbols if filtering by tag
    if ( mTagId >= 0 )
      setTagId( mTagId );
    if ( mSmartGroupId >= 0 )
      setSmartGroupId( mSmartGroupId );
  } );

  connect( mStyle, &QgsStyle::favoritedChanged, this, [ = ]
  {
    // update favorited symbols if filtering by favorite
    if ( mFavoritesOnly )
      setFavoritesOnly( mFavoritesOnly );
  } );

  connect( mStyle, &QgsStyle::rampRenamed, this, [ = ]
  {
    if ( mSmartGroupId >= 0 )
      setSmartGroupId( mSmartGroupId );
  } );
  connect( mStyle, &QgsStyle::symbolRenamed, this, [ = ]
  {
    if ( mSmartGroupId >= 0 )
      setSmartGroupId( mSmartGroupId );
  } );
}

bool QgsStyleProxyModel::filterAcceptsRow( int source_row, const QModelIndex &source_parent ) const
{
  if ( mFilterString.isEmpty() && !mEntityFilterEnabled && !mSymbolTypeFilterEnabled && mTagId < 0 && mSmartGroupId < 0 && !mFavoritesOnly )
    return true;

  QModelIndex index = sourceModel()->index( source_row, 0, source_parent );
  const QString name = sourceModel()->data( index ).toString();
  const QStringList tags = sourceModel()->data( index, QgsStyleModel::TagRole ).toStringList();

  QgsStyle::StyleEntity styleEntityType = static_cast< QgsStyle::StyleEntity >( sourceModel()->data( index, QgsStyleModel::TypeRole ).toInt() );
  if ( mEntityFilterEnabled && styleEntityType != mEntityFilter )
    return false;

  QgsSymbol::SymbolType symbolType = static_cast< QgsSymbol::SymbolType >( sourceModel()->data( index, QgsStyleModel::SymbolTypeRole ).toInt() );
  if ( mSymbolTypeFilterEnabled && symbolType != mSymbolType )
    return false;

  if ( mTagId >= 0 && !mTaggedSymbolNames.contains( name ) )
    return false;

  if ( mSmartGroupId >= 0 && !mSmartGroupSymbolNames.contains( name ) )
    return false;

  if ( mFavoritesOnly && !mFavoritedSymbolNames.contains( name ) )
    return false;

  if ( !mFilterString.isEmpty() )
  {
    // filter by word, in both filter string and style entity name/tags
    // this allows matching of a filter string "hash line" to the symbol "hashed red lines"
    const QStringList partsToMatch = mFilterString.trimmed().split( ' ' );

    QStringList partsToSearch = name.split( ' ' );
    for ( const QString &tag : tags )
    {
      partsToSearch.append( tag.split( ' ' ) );
    }

    for ( const QString &part : partsToMatch )
    {
      bool found = false;
      for ( const QString &partToSearch : qgis::as_const( partsToSearch ) )
      {
        if ( partToSearch.contains( part, Qt::CaseInsensitive ) )
        {
          found = true;
          break;
        }
      }
      if ( !found )
        return false; // couldn't find a match for this word, so hide entity
    }
  }

  return true;
}

void QgsStyleProxyModel::setFilterString( const QString &filter )
{
  mFilterString = filter;
  invalidateFilter();
}

bool QgsStyleProxyModel::favoritesOnly() const
{
  return mFavoritesOnly;
}

void QgsStyleProxyModel::setFavoritesOnly( bool favoritesOnly )
{
  mFavoritesOnly = favoritesOnly;

  if ( mFavoritesOnly )
  {
    mFavoritedSymbolNames = mStyle->symbolsOfFavorite( QgsStyle::SymbolEntity );
    mFavoritedSymbolNames.append( mStyle->symbolsOfFavorite( QgsStyle::ColorrampEntity ) );
  }
  else
  {
    mFavoritedSymbolNames.clear();
  }
  invalidateFilter();
}

void QgsStyleProxyModel::addDesiredIconSize( QSize size )
{
  mModel->addDesiredIconSize( size );
}

bool QgsStyleProxyModel::symbolTypeFilterEnabled() const
{
  return mSymbolTypeFilterEnabled;
}

void QgsStyleProxyModel::setSymbolTypeFilterEnabled( bool symbolTypeFilterEnabled )
{
  mSymbolTypeFilterEnabled = symbolTypeFilterEnabled;
  invalidateFilter();
}

void QgsStyleProxyModel::setTagId( int id )
{
  mTagId = id;

  if ( mTagId >= 0 )
  {
    mTaggedSymbolNames = mStyle->symbolsWithTag( QgsStyle::SymbolEntity, mTagId );
    mTaggedSymbolNames.append( mStyle->symbolsWithTag( QgsStyle::ColorrampEntity, mTagId ) );
  }
  else
  {
    mTaggedSymbolNames.clear();
  }

  invalidateFilter();
}

int QgsStyleProxyModel::tagId() const
{
  return mTagId;
}

void QgsStyleProxyModel::setSmartGroupId( int id )
{
  mSmartGroupId = id;

  if ( mSmartGroupId >= 0 )
  {
    mSmartGroupSymbolNames = mStyle->symbolsOfSmartgroup( QgsStyle::SymbolEntity, mSmartGroupId );
    mSmartGroupSymbolNames.append( mStyle->symbolsOfSmartgroup( QgsStyle::ColorrampEntity, mSmartGroupId ) );
  }
  else
  {
    mSmartGroupSymbolNames.clear();
  }

  invalidateFilter();
}

int QgsStyleProxyModel::smartGroupId() const
{
  return mSmartGroupId;
}

QgsSymbol::SymbolType QgsStyleProxyModel::symbolType() const
{
  return mSymbolType;
}

void QgsStyleProxyModel::setSymbolType( const QgsSymbol::SymbolType symbolType )
{
  mSymbolType = symbolType;
  invalidateFilter();
}

bool QgsStyleProxyModel::entityFilterEnabled() const
{
  return mEntityFilterEnabled;
}

void QgsStyleProxyModel::setEntityFilterEnabled( bool entityFilterEnabled )
{
  mEntityFilterEnabled = entityFilterEnabled;
  invalidateFilter();
}

QgsStyle::StyleEntity QgsStyleProxyModel::entityFilter() const
{
  return mEntityFilter;
}

void QgsStyleProxyModel::setEntityFilter( const QgsStyle::StyleEntity entityFilter )
{
  mEntityFilter = entityFilter;
  invalidateFilter();
}
