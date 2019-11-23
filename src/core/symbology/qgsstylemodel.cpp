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
#include "qgsimagecache.h"
#include "qgsproject.h"
#include "qgsexpressioncontextutils.h"
#include <QIcon>

const double ICON_PADDING_FACTOR = 0.16;

QgsStyleModel::QgsStyleModel( QgsStyle *style, QObject *parent )
  : QAbstractItemModel( parent )
  , mStyle( style )
{
  Q_ASSERT( mStyle );
  mSymbolNames = mStyle->symbolNames();
  mRampNames = mStyle->colorRampNames();
  mTextFormatNames = mStyle->textFormatNames();
  mLabelSettingsNames = mStyle->labelSettingsNames();

  connect( mStyle, &QgsStyle::symbolSaved, this, &QgsStyleModel::onSymbolAdded );
  connect( mStyle, &QgsStyle::symbolRemoved, this, &QgsStyleModel::onSymbolRemoved );
  connect( mStyle, &QgsStyle::symbolRenamed, this, &QgsStyleModel::onSymbolRename );
  connect( mStyle, &QgsStyle::symbolChanged, this, &QgsStyleModel::onSymbolChanged );

  connect( mStyle, &QgsStyle::rampAdded, this, &QgsStyleModel::onRampAdded );
  connect( mStyle, &QgsStyle::rampChanged, this, &QgsStyleModel::onRampChanged );
  connect( mStyle, &QgsStyle::rampRemoved, this, &QgsStyleModel::onRampRemoved );
  connect( mStyle, &QgsStyle::rampRenamed, this, &QgsStyleModel::onRampRename );

  connect( mStyle, &QgsStyle::textFormatAdded, this, &QgsStyleModel::onTextFormatAdded );
  connect( mStyle, &QgsStyle::textFormatChanged, this, &QgsStyleModel::onTextFormatChanged );
  connect( mStyle, &QgsStyle::textFormatRemoved, this, &QgsStyleModel::onTextFormatRemoved );
  connect( mStyle, &QgsStyle::textFormatRenamed, this, &QgsStyleModel::onTextFormatRename );

  connect( mStyle, &QgsStyle::labelSettingsAdded, this, &QgsStyleModel::onLabelSettingsAdded );
  connect( mStyle, &QgsStyle::labelSettingsChanged, this, &QgsStyleModel::onLabelSettingsChanged );
  connect( mStyle, &QgsStyle::labelSettingsRemoved, this, &QgsStyleModel::onLabelSettingsRemoved );
  connect( mStyle, &QgsStyle::labelSettingsRenamed, this, &QgsStyleModel::onLabelSettingsRename );

  connect( mStyle, &QgsStyle::entityTagsChanged, this, &QgsStyleModel::onTagsChanged );

  // when a remote svg or image has been fetched, update the model's decorations.
  // this is required if a symbol utilizes remote svgs, and the current icons
  // have been generated using the temporary "downloading" svg. In this case
  // we require the preview to be regenerated to use the correct fetched
  // svg
  connect( QgsApplication::svgCache(), &QgsSvgCache::remoteSvgFetched, this, &QgsStyleModel::rebuildSymbolIcons );
  connect( QgsApplication::imageCache(), &QgsImageCache::remoteImageFetched, this, &QgsStyleModel::rebuildSymbolIcons );

  // if project color scheme changes, we need to redraw symbols - they may use project colors and accordingly
  // need updating to reflect the new colors
  connect( QgsProject::instance(), &QgsProject::projectColorsChanged, this, &QgsStyleModel::rebuildSymbolIcons );
}

QVariant QgsStyleModel::data( const QModelIndex &index, int role ) const
{
  if ( index.row() < 0 || index.row() >= rowCount( QModelIndex() ) )
    return QVariant();


  QgsStyle::StyleEntity entityType = entityTypeFromRow( index.row() );

  QString name;
  switch ( entityType )
  {
    case QgsStyle::SymbolEntity:
      name = mSymbolNames.value( index.row() );
      break;

    case QgsStyle::ColorrampEntity:
      name = mRampNames.value( index.row() - mSymbolNames.size() );
      break;

    case QgsStyle::TextFormatEntity:
      name = mTextFormatNames.value( index.row() - mSymbolNames.size() - mRampNames.size() );
      break;

    case QgsStyle::LabelSettingsEntity:
      name = mLabelSettingsNames.value( index.row() - mSymbolNames.size() - mRampNames.size() - mTextFormatNames.size() );
      break;

    case QgsStyle::TagEntity:
    case QgsStyle::SmartgroupEntity:
      break;
  }

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
          const QStringList tags = mStyle->tagsOfSymbol( entityType, name );

          if ( role == Qt::ToolTipRole )
          {
            QString tooltip = QStringLiteral( "<h3>%1</h3><p><i>%2</i>" ).arg( name,
                              tags.count() > 0 ? tags.join( QStringLiteral( ", " ) ) : tr( "Not tagged" ) );

            switch ( entityType )
            {
              case QgsStyle::SymbolEntity:
              {
                // create very large preview image
                std::unique_ptr< QgsSymbol > symbol( mStyle->symbol( name ) );
                if ( symbol )
                {
                  int width = static_cast< int >( Qgis::UI_SCALE_FACTOR * QFontMetrics( data( index, Qt::FontRole ).value< QFont >() ).width( 'X' ) * 23 );
                  int height = static_cast< int >( width / 1.61803398875 ); // golden ratio
                  QPixmap pm = QgsSymbolLayerUtils::symbolPreviewPixmap( symbol.get(), QSize( width, height ), height / 20, nullptr, false, mExpressionContext.get() );
                  QByteArray data;
                  QBuffer buffer( &data );
                  pm.save( &buffer, "PNG", 100 );
                  tooltip += QStringLiteral( "<p><img src='data:image/png;base64, %3'>" ).arg( QString( data.toBase64() ) );
                }
                break;
              }

              case QgsStyle::TextFormatEntity:
              {
                int width = static_cast< int >( Qgis::UI_SCALE_FACTOR * QFontMetrics( data( index, Qt::FontRole ).value< QFont >() ).width( 'X' ) * 23 );
                int height = static_cast< int >( width / 1.61803398875 ); // golden ratio
                const QgsTextFormat format = mStyle->textFormat( name );
                QPixmap pm = QgsTextFormat::textFormatPreviewPixmap( format, QSize( width, height ), QString(), height / 20 );
                QByteArray data;
                QBuffer buffer( &data );
                pm.save( &buffer, "PNG", 100 );
                tooltip += QStringLiteral( "<p><img src='data:image/png;base64, %3'>" ).arg( QString( data.toBase64() ) );
                break;
              }

              case QgsStyle::LabelSettingsEntity:
              {
                int width = static_cast< int >( Qgis::UI_SCALE_FACTOR * QFontMetrics( data( index, Qt::FontRole ).value< QFont >() ).width( 'X' ) * 23 );
                int height = static_cast< int >( width / 1.61803398875 ); // golden ratio
                const QgsPalLayerSettings settings = mStyle->labelSettings( name );
                QPixmap pm = QgsPalLayerSettings::labelSettingsPreviewPixmap( settings, QSize( width, height ), QString(), height / 20 );
                QByteArray data;
                QBuffer buffer( &data );
                pm.save( &buffer, "PNG", 100 );
                tooltip += QStringLiteral( "<p><img src='data:image/png;base64, %3'>" ).arg( QString( data.toBase64() ) );
                break;
              }

              case QgsStyle::ColorrampEntity:
              case QgsStyle::TagEntity:
              case QgsStyle::SmartgroupEntity:
                break;
            }
            return tooltip;
          }
          else
          {
            return name;
          }
        }
        case Tags:
          return mStyle->tagsOfSymbol( entityType, name ).join( QStringLiteral( ", " ) );
      }
      return QVariant();
    }

    case Qt::DecorationRole:
    {
      // Generate icons at all additional sizes specified for the model.
      // This allows the model to have size responsive icons.

      if ( !mExpressionContext )
      {
        // build the expression context once, and keep it around. Usually this is a no-no, but in this
        // case we want to avoid creating potentially thousands of contexts one-by-one (usually one context
        // is created for a batch of multiple evalutions like this), and we only use a very minimal context
        // anyway...
        mExpressionContext = qgis::make_unique< QgsExpressionContext >();
        mExpressionContext->appendScopes( QgsExpressionContextUtils::globalProjectLayerScopes( nullptr ) );
      }

      switch ( index.column() )
      {
        case Name:
          switch ( entityType )
          {
            case QgsStyle::SymbolEntity:
            {
              // use cached icon if possible
              QIcon icon = mSymbolIconCache.value( name );
              if ( !icon.isNull() )
                return icon;

              std::unique_ptr< QgsSymbol > symbol( mStyle->symbol( name ) );
              if ( symbol )
              {
                if ( mAdditionalSizes.isEmpty() )
                  icon.addPixmap( QgsSymbolLayerUtils::symbolPreviewPixmap( symbol.get(), QSize( 24, 24 ), 1, nullptr, false, mExpressionContext.get() ) );

                for ( const QSize &s : mAdditionalSizes )
                {
                  icon.addPixmap( QgsSymbolLayerUtils::symbolPreviewPixmap( symbol.get(), s, static_cast< int >( s.width() * ICON_PADDING_FACTOR ), nullptr, false, mExpressionContext.get() ) );
                }

              }
              mSymbolIconCache.insert( name, icon );
              return icon;
            }
            case QgsStyle::ColorrampEntity:
            {
              // use cached icon if possible
              QIcon icon = mColorRampIconCache.value( name );
              if ( !icon.isNull() )
                return icon;

              std::unique_ptr< QgsColorRamp > ramp( mStyle->colorRamp( name ) );
              if ( ramp )
              {
                if ( mAdditionalSizes.isEmpty() )
                  icon.addPixmap( QgsSymbolLayerUtils::colorRampPreviewPixmap( ramp.get(), QSize( 24, 24 ), 1 ) );
                for ( const QSize &s : mAdditionalSizes )
                {
                  icon.addPixmap( QgsSymbolLayerUtils::colorRampPreviewPixmap( ramp.get(), s, static_cast< int >( s.width() * ICON_PADDING_FACTOR ) ) );
                }

              }
              mColorRampIconCache.insert( name, icon );
              return icon;
            }

            case QgsStyle::TextFormatEntity:
            {
              // use cached icon if possible
              QIcon icon = mTextFormatIconCache.value( name );
              if ( !icon.isNull() )
                return icon;

              const QgsTextFormat format( mStyle->textFormat( name ) );
              if ( mAdditionalSizes.isEmpty() )
                icon.addPixmap( QgsTextFormat::textFormatPreviewPixmap( format, QSize( 24, 24 ), QString(),  1 ) );
              for ( const QSize &s : mAdditionalSizes )
              {
                icon.addPixmap( QgsTextFormat::textFormatPreviewPixmap( format, s, QString(),  static_cast< int >( s.width() * ICON_PADDING_FACTOR ) ) );
              }
              mTextFormatIconCache.insert( name, icon );
              return icon;
            }

            case QgsStyle::LabelSettingsEntity:
            {
              // use cached icon if possible
              QIcon icon = mLabelSettingsIconCache.value( name );
              if ( !icon.isNull() )
                return icon;

              const QgsPalLayerSettings settings( mStyle->labelSettings( name ) );
              if ( mAdditionalSizes.isEmpty() )
                icon.addPixmap( QgsPalLayerSettings::labelSettingsPreviewPixmap( settings, QSize( 24, 24 ), QString(),  1 ) );
              for ( const QSize &s : mAdditionalSizes )
              {
                icon.addPixmap( QgsPalLayerSettings::labelSettingsPreviewPixmap( settings, s, QString(),  static_cast< int >( s.width() * ICON_PADDING_FACTOR ) ) );
              }
              mLabelSettingsIconCache.insert( name, icon );
              return icon;
            }

            case QgsStyle::TagEntity:
            case QgsStyle::SmartgroupEntity:
              return QVariant();
          }
          break;

        case Tags:
          return QVariant();
      }
      return QVariant();
    }

    case TypeRole:
      return entityType;

    case TagRole:
      return mStyle->tagsOfSymbol( entityType, name );

    case IsFavoriteRole:
      return mStyle->isFavorite( entityType, name );

    case SymbolTypeRole:
    {
      if ( entityType != QgsStyle::SymbolEntity )
        return QVariant();

      const QgsSymbol *symbol = mStyle->symbolRef( name );
      return symbol ? symbol->type() : QVariant();
    }

    case LayerTypeRole:
    {
      if ( entityType != QgsStyle::LabelSettingsEntity )
        return QVariant();

      return mStyle->labelSettingsLayerType( name );
    }

    default:
      return QVariant();
  }
#ifndef _MSC_VER // avoid warning
  return QVariant();  // avoid warning
#endif
}

bool QgsStyleModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  if ( index.row() < 0 || index.row() >= rowCount( QModelIndex() ) || role != Qt::EditRole )
    return false;

  switch ( index.column() )
  {
    case Name:
    {
      QgsStyle::StyleEntity entityType = entityTypeFromRow( index.row() );
      QString name;
      switch ( entityType )
      {
        case QgsStyle::SymbolEntity:
          name = mSymbolNames.value( index.row() );
          break;

        case QgsStyle::ColorrampEntity:
          name = mRampNames.value( index.row() - mSymbolNames.size() );
          break;

        case QgsStyle::TextFormatEntity:
          name = mTextFormatNames.value( index.row() - mSymbolNames.size() - mRampNames.size() );
          break;

        case QgsStyle::LabelSettingsEntity:
          name = mLabelSettingsNames.value( index.row() - mSymbolNames.size() - mRampNames.size() - mTextFormatNames.size() );
          break;

        case QgsStyle::TagEntity:
        case QgsStyle::SmartgroupEntity:
          break;
      }

      const QString newName = value.toString();

      switch ( entityType )
      {
        case QgsStyle::SymbolEntity:
          return mStyle->renameSymbol( name, newName );

        case QgsStyle::ColorrampEntity:
          return mStyle->renameColorRamp( name, newName );

        case QgsStyle::TextFormatEntity:
          return mStyle->renameTextFormat( name, newName );

        case QgsStyle::LabelSettingsEntity:
          return mStyle->renameLabelSettings( name, newName );

        case QgsStyle::TagEntity:
        case QgsStyle::SmartgroupEntity:
          return false;
      }
      break;
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
    return mSymbolNames.count() + mRampNames.count() + mTextFormatNames.count() + mLabelSettingsNames.count();
  }
  return 0;
}

int QgsStyleModel::columnCount( const QModelIndex & ) const
{
  return 2;
}

void QgsStyleModel::addDesiredIconSize( QSize size )
{
  if ( mAdditionalSizes.contains( size ) )
    return;

  mAdditionalSizes << size;
  mSymbolIconCache.clear();
  mColorRampIconCache.clear();
  mTextFormatIconCache.clear();
  mLabelSettingsIconCache.clear();
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

void QgsStyleModel::onTextFormatAdded( const QString &name )
{
  mTextFormatIconCache.remove( name );
  const QStringList oldTextFormatNames = mTextFormatNames;
  const QStringList newTextFormatNames = mStyle->textFormatNames();

  // find index of newly added symbol
  const int newNameIndex = newTextFormatNames.indexOf( name );
  if ( newNameIndex < 0 )
    return; // shouldn't happen

  beginInsertRows( QModelIndex(), newNameIndex + mSymbolNames.count() + mRampNames.count(), newNameIndex + mSymbolNames.count() + mRampNames.count() );
  mTextFormatNames = newTextFormatNames;
  endInsertRows();
}

void QgsStyleModel::onTextFormatRemoved( const QString &name )
{
  mTextFormatIconCache.remove( name );
  const QStringList oldTextFormatNames = mTextFormatNames;
  const QStringList newTextFormatNames = mStyle->textFormatNames();

  // find index of removed symbol
  const int oldNameIndex = oldTextFormatNames.indexOf( name );
  if ( oldNameIndex < 0 )
    return; // shouldn't happen

  beginRemoveRows( QModelIndex(), oldNameIndex + mSymbolNames.count() + mRampNames.count(), oldNameIndex + mSymbolNames.count() + mRampNames.count() );
  mTextFormatNames = newTextFormatNames;
  endRemoveRows();
}

void QgsStyleModel::onTextFormatChanged( const QString &name )
{
  mTextFormatIconCache.remove( name );

  QModelIndex i = index( mSymbolNames.count() + mRampNames.count() + mTextFormatNames.indexOf( name ), Tags );
  emit dataChanged( i, i, QVector< int >() << Qt::DecorationRole );
}

void QgsStyleModel::onTextFormatRename( const QString &oldName, const QString &newName )
{
  mTextFormatIconCache.remove( oldName );
  const QStringList oldTextFormatNames = mTextFormatNames;
  const QStringList newTextFormatNames = mStyle->textFormatNames();

  // find index of removed format
  const int oldNameIndex = oldTextFormatNames.indexOf( oldName );
  if ( oldNameIndex < 0 )
    return; // shouldn't happen

  // find index of newly added format
  const int newNameIndex = newTextFormatNames.indexOf( newName );
  if ( newNameIndex < 0 )
    return; // shouldn't happen

  if ( newNameIndex == oldNameIndex )
  {
    mTextFormatNames = newTextFormatNames;
    return;
  }

  beginMoveRows( QModelIndex(), oldNameIndex + mSymbolNames.count() + mRampNames.count(), oldNameIndex + mSymbolNames.count() + mRampNames.count(),
                 QModelIndex(), ( newNameIndex > oldNameIndex ? newNameIndex + 1 : newNameIndex ) + mSymbolNames.count() + mRampNames.count() );
  mTextFormatNames = newTextFormatNames;
  endMoveRows();
}

void QgsStyleModel::onLabelSettingsAdded( const QString &name )
{
  mLabelSettingsIconCache.remove( name );
  const QStringList oldLabelSettingsNames = mLabelSettingsNames;
  const QStringList newLabelSettingsNames = mStyle->labelSettingsNames();

  // find index of newly added symbol
  const int newNameIndex = newLabelSettingsNames.indexOf( name );
  if ( newNameIndex < 0 )
    return; // shouldn't happen

  beginInsertRows( QModelIndex(), newNameIndex + mSymbolNames.count() + mRampNames.count() + mTextFormatNames.count(), newNameIndex + mSymbolNames.count() + mRampNames.count() + mTextFormatNames.count() );
  mLabelSettingsNames = newLabelSettingsNames;
  endInsertRows();
}

void QgsStyleModel::onLabelSettingsRemoved( const QString &name )
{
  mLabelSettingsIconCache.remove( name );
  const QStringList oldLabelSettingsNames = mLabelSettingsNames;
  const QStringList newLabelSettingsNames = mStyle->labelSettingsNames();

  // find index of removed symbol
  const int oldNameIndex = oldLabelSettingsNames.indexOf( name );
  if ( oldNameIndex < 0 )
    return; // shouldn't happen

  beginRemoveRows( QModelIndex(), oldNameIndex + mSymbolNames.count() + mRampNames.count() + mTextFormatNames.count(), oldNameIndex + mSymbolNames.count() + mRampNames.count()  + mTextFormatNames.count() );
  mLabelSettingsNames = newLabelSettingsNames;
  endRemoveRows();
}

void QgsStyleModel::onLabelSettingsChanged( const QString &name )
{
  mLabelSettingsIconCache.remove( name );

  QModelIndex i = index( mSymbolNames.count() + mRampNames.count() + mTextFormatNames.count() + mLabelSettingsNames.indexOf( name ), Tags );
  emit dataChanged( i, i, QVector< int >() << Qt::DecorationRole );
}

void QgsStyleModel::onLabelSettingsRename( const QString &oldName, const QString &newName )
{
  mLabelSettingsIconCache.remove( oldName );
  const QStringList oldLabelSettingsNames = mLabelSettingsNames;
  const QStringList newLabelSettingsNames = mStyle->labelSettingsNames();

  // find index of removed format
  const int oldNameIndex = oldLabelSettingsNames.indexOf( oldName );
  if ( oldNameIndex < 0 )
    return; // shouldn't happen

  // find index of newly added format
  const int newNameIndex = newLabelSettingsNames.indexOf( newName );
  if ( newNameIndex < 0 )
    return; // shouldn't happen

  if ( newNameIndex == oldNameIndex )
  {
    mLabelSettingsNames = newLabelSettingsNames;
    return;
  }

  beginMoveRows( QModelIndex(), oldNameIndex + mSymbolNames.count() + mRampNames.count() + mTextFormatNames.count(), oldNameIndex + mSymbolNames.count() + mRampNames.count() + mTextFormatNames.count(),
                 QModelIndex(), ( newNameIndex > oldNameIndex ? newNameIndex + 1 : newNameIndex ) + mSymbolNames.count() + mRampNames.count() + mTextFormatNames.count() );
  mLabelSettingsNames = newLabelSettingsNames;
  endMoveRows();
}

void QgsStyleModel::onTagsChanged( int entity, const QString &name, const QStringList & )
{
  QModelIndex i;
  switch ( static_cast< QgsStyle::StyleEntity >( entity ) )
  {
    case QgsStyle::SymbolEntity:
      i = index( mSymbolNames.indexOf( name ), Tags );
      break;

    case QgsStyle::ColorrampEntity:
      i = index( mSymbolNames.count() + mRampNames.indexOf( name ), Tags );
      break;

    case QgsStyle::TextFormatEntity:
      i = index( mSymbolNames.count() + mRampNames.count() + mTextFormatNames.indexOf( name ), Tags );
      break;

    case QgsStyle::LabelSettingsEntity:
      i = index( mSymbolNames.count() + mRampNames.count() + mTextFormatNames.count() + mLabelSettingsNames.indexOf( name ), Tags );
      break;

    case QgsStyle::TagEntity:
    case QgsStyle::SmartgroupEntity:
      return;
  }
  emit dataChanged( i, i );
}

void QgsStyleModel::rebuildSymbolIcons()
{
  mSymbolIconCache.clear();
  mExpressionContext.reset();
  emit dataChanged( index( 0, 0 ), index( mSymbolNames.count() - 1, 0 ), QVector<int>() << Qt::DecorationRole );
}

QgsStyle::StyleEntity QgsStyleModel::entityTypeFromRow( int row ) const
{
  if ( row >= mStyle->symbolCount() + mStyle->colorRampCount() + + mTextFormatNames.count() )
    return QgsStyle::LabelSettingsEntity;
  else if ( row >= mStyle->symbolCount() + mStyle->colorRampCount() )
    return QgsStyle::TextFormatEntity;
  else if ( row >= mStyle->symbolCount() )
    return QgsStyle::ColorrampEntity;
  return QgsStyle::SymbolEntity;
}

//
// QgsStyleProxyModel
//

QgsStyleProxyModel::QgsStyleProxyModel( QgsStyle *style, QObject *parent )
  : QSortFilterProxyModel( parent )
  , mStyle( style )
{
  mModel = new QgsStyleModel( mStyle, this );
  initialize();
}

void QgsStyleProxyModel::initialize()
{
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
  connect( mStyle, &QgsStyle::textFormatRenamed, this, [ = ]
  {
    if ( mSmartGroupId >= 0 )
      setSmartGroupId( mSmartGroupId );
  } );
  connect( mStyle, &QgsStyle::labelSettingsRenamed, this, [ = ]
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

QgsStyleProxyModel::QgsStyleProxyModel( QgsStyleModel *model, QObject *parent )
  : QSortFilterProxyModel( parent )
  , mModel( model )
  , mStyle( model->style() )
{
  initialize();
}

bool QgsStyleProxyModel::filterAcceptsRow( int source_row, const QModelIndex &source_parent ) const
{
  if ( mFilterString.isEmpty() && !mEntityFilterEnabled && !mSymbolTypeFilterEnabled && mTagId < 0 && mSmartGroupId < 0 && !mFavoritesOnly )
    return true;

  QModelIndex index = sourceModel()->index( source_row, 0, source_parent );
  const QString name = sourceModel()->data( index ).toString();
  const QStringList tags = sourceModel()->data( index, QgsStyleModel::TagRole ).toStringList();

  QgsStyle::StyleEntity styleEntityType = static_cast< QgsStyle::StyleEntity >( sourceModel()->data( index, QgsStyleModel::TypeRole ).toInt() );
  if ( mEntityFilterEnabled && ( mEntityFilters.empty() || !mEntityFilters.contains( styleEntityType ) ) )
    return false;

  QgsSymbol::SymbolType symbolType = static_cast< QgsSymbol::SymbolType >( sourceModel()->data( index, QgsStyleModel::SymbolTypeRole ).toInt() );
  if ( mSymbolTypeFilterEnabled && symbolType != mSymbolType )
    return false;

  if ( styleEntityType == QgsStyle::LabelSettingsEntity && mLayerType != QgsWkbTypes::UnknownGeometry &&
       mLayerType != static_cast< QgsWkbTypes::GeometryType >( sourceModel()->data( index, QgsStyleModel::LayerTypeRole ).toInt() ) )
    return false;

  if ( mTagId >= 0 && !mTaggedSymbolNames.contains( name ) )
    return false;

  if ( mSmartGroupId >= 0 && !mSmartGroupSymbolNames.contains( name ) )
    return false;

  if ( mFavoritesOnly && !sourceModel()->data( index, QgsStyleModel::IsFavoriteRole ).toBool() )
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

QgsWkbTypes::GeometryType QgsStyleProxyModel::layerType() const
{
  return mLayerType;
}

void QgsStyleProxyModel::setLayerType( QgsWkbTypes::GeometryType type )
{
  mLayerType = type;
  invalidateFilter();
}

void QgsStyleProxyModel::setTagId( int id )
{
  mTagId = id;

  if ( mTagId >= 0 )
  {
    mTaggedSymbolNames = mStyle->symbolsWithTag( QgsStyle::SymbolEntity, mTagId );
    mTaggedSymbolNames.append( mStyle->symbolsWithTag( QgsStyle::ColorrampEntity, mTagId ) );
    mTaggedSymbolNames.append( mStyle->symbolsWithTag( QgsStyle::TextFormatEntity, mTagId ) );
    mTaggedSymbolNames.append( mStyle->symbolsWithTag( QgsStyle::LabelSettingsEntity, mTagId ) );
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
    mSmartGroupSymbolNames.append( mStyle->symbolsOfSmartgroup( QgsStyle::TextFormatEntity, mSmartGroupId ) );
    mSmartGroupSymbolNames.append( mStyle->symbolsOfSmartgroup( QgsStyle::LabelSettingsEntity, mSmartGroupId ) );
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
  return mEntityFilters.empty() ? QgsStyle::SymbolEntity : mEntityFilters.at( 0 );
}

void QgsStyleProxyModel::setEntityFilter( const QgsStyle::StyleEntity entityFilter )
{
  mEntityFilters = QList< QgsStyle::StyleEntity >() << entityFilter;
  invalidateFilter();
}

void QgsStyleProxyModel::setEntityFilters( const QList<QgsStyle::StyleEntity> &filters )
{
  mEntityFilters = filters;
  invalidateFilter();
}
