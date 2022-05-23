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
#include "qgsexpressioncontextutils.h"
#include <QIcon>
#include <QBuffer>
#include <QDir>

#if QT_VERSION >= QT_VERSION_CHECK(5, 13, 0)
#include "qgscombinedstylemodel.h"
#endif

const double ICON_PADDING_FACTOR = 0.16;

const auto ENTITIES = { QgsStyle::SymbolEntity, QgsStyle::ColorrampEntity, QgsStyle::TextFormatEntity, QgsStyle::LabelSettingsEntity, QgsStyle::LegendPatchShapeEntity, QgsStyle::Symbol3DEntity };

QgsAbstractStyleEntityIconGenerator *QgsStyleModel::sIconGenerator = nullptr;

//
// QgsAbstractStyleEntityIconGenerator
//

QgsAbstractStyleEntityIconGenerator::QgsAbstractStyleEntityIconGenerator( QObject *parent )
  : QObject( parent )
{

}

void QgsAbstractStyleEntityIconGenerator::setIconSizes( const QList<QSize> &sizes )
{
  mIconSizes = sizes;
}

QList<QSize> QgsAbstractStyleEntityIconGenerator::iconSizes() const
{
  return mIconSizes;
}


//
// QgsStyleModel
//

QgsStyleModel::QgsStyleModel( QgsStyle *style, QObject *parent )
  : QAbstractItemModel( parent )
  , mStyle( style )
{
  Q_ASSERT( mStyle );

  for ( QgsStyle::StyleEntity entity : ENTITIES )
  {
    mEntityNames.insert( entity, mStyle->allNames( entity ) );
  }

  connect( mStyle, &QgsStyle::entityAdded, this, &QgsStyleModel::onEntityAdded );
  connect( mStyle, &QgsStyle::entityRemoved, this, &QgsStyleModel::onEntityRemoved );
  connect( mStyle, &QgsStyle::entityRenamed, this, &QgsStyleModel::onEntityRename );
  connect( mStyle, &QgsStyle::entityChanged, this, &QgsStyleModel::onEntityChanged );
  connect( mStyle, &QgsStyle::favoritedChanged, this, &QgsStyleModel::onFavoriteChanged );
  connect( mStyle, &QgsStyle::entityTagsChanged, this, &QgsStyleModel::onTagsChanged );
  connect( mStyle, &QgsStyle::rebuildIconPreviews, this, &QgsStyleModel::rebuildSymbolIcons );

  // when a remote svg or image has been fetched, update the model's decorations.
  // this is required if a symbol utilizes remote svgs, and the current icons
  // have been generated using the temporary "downloading" svg. In this case
  // we require the preview to be regenerated to use the correct fetched
  // svg
  connect( QgsApplication::svgCache(), &QgsSvgCache::remoteSvgFetched, this, &QgsStyleModel::rebuildSymbolIcons );
  connect( QgsApplication::imageCache(), &QgsImageCache::remoteImageFetched, this, &QgsStyleModel::rebuildSymbolIcons );

  if ( sIconGenerator )
    connect( sIconGenerator, &QgsAbstractStyleEntityIconGenerator::iconGenerated, this, &QgsStyleModel::iconGenerated, Qt::QueuedConnection );
}

QVariant QgsStyleModel::data( const QModelIndex &index, int role ) const
{
  if ( index.row() < 0 || index.row() >= rowCount( QModelIndex() ) )
    return QVariant();


  QgsStyle::StyleEntity entityType = entityTypeFromRow( index.row() );

  QString name;
  switch ( entityType )
  {
    case QgsStyle::TagEntity:
    case QgsStyle::SmartgroupEntity:
      break;

    default:
      name = mEntityNames[ entityType ].value( index.row() - offsetForEntity( entityType ) );
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
                              tags.count() > 0 ? tags.join( QLatin1String( ", " ) ) : tr( "Not tagged" ) );

            switch ( entityType )
            {
              case QgsStyle::SymbolEntity:
              {
                // create very large preview image
                std::unique_ptr< QgsSymbol > symbol( mStyle->symbol( name ) );
                if ( symbol )
                {
                  int width = static_cast< int >( Qgis::UI_SCALE_FACTOR * QFontMetrics( data( index, Qt::FontRole ).value< QFont >() ).horizontalAdvance( 'X' ) * 23 );
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
                int width = static_cast< int >( Qgis::UI_SCALE_FACTOR * QFontMetrics( data( index, Qt::FontRole ).value< QFont >() ).horizontalAdvance( 'X' ) * 23 );
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
                int width = static_cast< int >( Qgis::UI_SCALE_FACTOR * QFontMetrics( data( index, Qt::FontRole ).value< QFont >() ).horizontalAdvance( 'X' ) * 23 );
                int height = static_cast< int >( width / 1.61803398875 ); // golden ratio
                const QgsPalLayerSettings settings = mStyle->labelSettings( name );
                QPixmap pm = QgsPalLayerSettings::labelSettingsPreviewPixmap( settings, QSize( width, height ), QString(), height / 20 );
                QByteArray data;
                QBuffer buffer( &data );
                pm.save( &buffer, "PNG", 100 );
                tooltip += QStringLiteral( "<p><img src='data:image/png;base64, %3'>" ).arg( QString( data.toBase64() ) );
                break;
              }

              case QgsStyle::LegendPatchShapeEntity:
              {
                int width = static_cast< int >( Qgis::UI_SCALE_FACTOR * QFontMetrics( data( index, Qt::FontRole ).value< QFont >() ).horizontalAdvance( 'X' ) * 23 );
                int height = static_cast< int >( width / 1.61803398875 ); // golden ratio

                const QgsLegendPatchShape shape = mStyle->legendPatchShape( name );
                if ( const QgsSymbol *symbol = mStyle->previewSymbolForPatchShape( shape ) )
                {
                  QPixmap pm = QgsSymbolLayerUtils::symbolPreviewPixmap( symbol, QSize( width, height ), height / 20, nullptr, false, nullptr, &shape );
                  QByteArray data;
                  QBuffer buffer( &data );
                  pm.save( &buffer, "PNG", 100 );
                  tooltip += QStringLiteral( "<p><img src='data:image/png;base64, %3'>" ).arg( QString( data.toBase64() ) );
                }
                break;
              }

              case QgsStyle::ColorrampEntity:
              case QgsStyle::TagEntity:
              case QgsStyle::SmartgroupEntity:
              case QgsStyle::Symbol3DEntity:
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
          return mStyle->tagsOfSymbol( entityType, name ).join( QLatin1String( ", " ) );
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
        mExpressionContext = std::make_unique< QgsExpressionContext >();
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
              QIcon icon = mIconCache[ entityType ].value( name );
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
              mIconCache[ entityType ].insert( name, icon );
              return icon;
            }
            case QgsStyle::ColorrampEntity:
            {
              // use cached icon if possible
              QIcon icon = mIconCache[ entityType ].value( name );
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
              mIconCache[ entityType ].insert( name, icon );
              return icon;
            }

            case QgsStyle::TextFormatEntity:
            {
              // use cached icon if possible
              QIcon icon = mIconCache[ entityType ].value( name );
              if ( !icon.isNull() )
                return icon;

              const QgsTextFormat format( mStyle->textFormat( name ) );
              if ( mAdditionalSizes.isEmpty() )
                icon.addPixmap( QgsTextFormat::textFormatPreviewPixmap( format, QSize( 24, 24 ), QString(),  1 ) );
              for ( const QSize &s : mAdditionalSizes )
              {
                icon.addPixmap( QgsTextFormat::textFormatPreviewPixmap( format, s, QString(),  static_cast< int >( s.width() * ICON_PADDING_FACTOR ) ) );
              }
              mIconCache[ entityType ].insert( name, icon );
              return icon;
            }

            case QgsStyle::LabelSettingsEntity:
            {
              // use cached icon if possible
              QIcon icon = mIconCache[ entityType ].value( name );
              if ( !icon.isNull() )
                return icon;

              const QgsPalLayerSettings settings( mStyle->labelSettings( name ) );
              if ( mAdditionalSizes.isEmpty() )
                icon.addPixmap( QgsPalLayerSettings::labelSettingsPreviewPixmap( settings, QSize( 24, 24 ), QString(),  1 ) );
              for ( const QSize &s : mAdditionalSizes )
              {
                icon.addPixmap( QgsPalLayerSettings::labelSettingsPreviewPixmap( settings, s, QString(),  static_cast< int >( s.width() * ICON_PADDING_FACTOR ) ) );
              }
              mIconCache[ entityType ].insert( name, icon );
              return icon;
            }

            case QgsStyle::LegendPatchShapeEntity:
            {
              // use cached icon if possible
              QIcon icon = mIconCache[ entityType ].value( name );
              if ( !icon.isNull() )
                return icon;

              const QgsLegendPatchShape shape = mStyle->legendPatchShape( name );
              if ( !shape.isNull() )
              {
                if ( const QgsSymbol *symbol = mStyle->previewSymbolForPatchShape( shape ) )
                {
                  if ( mAdditionalSizes.isEmpty() )
                    icon.addPixmap( QgsSymbolLayerUtils::symbolPreviewPixmap( symbol, QSize( 24, 24 ), 1, nullptr, false, mExpressionContext.get(), &shape ) );

                  for ( const QSize &s : mAdditionalSizes )
                  {
                    icon.addPixmap( QgsSymbolLayerUtils::symbolPreviewPixmap( symbol, s, static_cast< int >( s.width() * ICON_PADDING_FACTOR ), nullptr, false, mExpressionContext.get(), &shape ) );
                  }
                }
              }
              mIconCache[ entityType ].insert( name, icon );
              return icon;
            }

            case QgsStyle::Symbol3DEntity:
            {
              // hack for now -- we just use a generic "3d icon" svg file.
              // TODO - render proper thumbnails

              // use cached icon if possible
              QIcon icon = mIconCache[ entityType ].value( name );
              if ( !icon.isNull() )
                return icon;

              if ( sIconGenerator && !mPending3dSymbolIcons.contains( name ) )
              {
                mPending3dSymbolIcons.insert( name );
                sIconGenerator->generateIcon( mStyle, QgsStyle::Symbol3DEntity, name );
              }

              // TODO - use hourglass icon
              if ( mAdditionalSizes.isEmpty() )
                icon.addFile( QgsApplication::defaultThemePath() + QDir::separator() + QStringLiteral( "3d.svg" ), QSize( 24, 24 ) );
              for ( const QSize &s : mAdditionalSizes )
              {
                icon.addFile( QgsApplication::defaultThemePath() + QDir::separator() + QStringLiteral( "3d.svg" ), s );
              }
              mIconCache[ entityType ].insert( name, icon );
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
      switch ( entityType )
      {
        case QgsStyle::SymbolEntity:
        {
          const QgsSymbol *symbol = mStyle->symbolRef( name );
          return symbol ? static_cast< int >( symbol->type() ) : QVariant();
        }

        case QgsStyle::LegendPatchShapeEntity:
          return static_cast< int >( mStyle->legendPatchShapeSymbolType( name ) );

        case QgsStyle::TagEntity:
        case QgsStyle::ColorrampEntity:
        case QgsStyle::SmartgroupEntity:
        case QgsStyle::LabelSettingsEntity:
        case QgsStyle::TextFormatEntity:
        case QgsStyle::Symbol3DEntity:
          return QVariant();
      }
      return QVariant();
    }

    case LayerTypeRole:
    {
      switch ( entityType )
      {
        case QgsStyle::LabelSettingsEntity:
          return mStyle->labelSettingsLayerType( name );

        case QgsStyle::Symbol3DEntity:
        case QgsStyle::SymbolEntity:
        case QgsStyle::LegendPatchShapeEntity:
        case QgsStyle::TagEntity:
        case QgsStyle::ColorrampEntity:
        case QgsStyle::SmartgroupEntity:
        case QgsStyle::TextFormatEntity:
          return QVariant();
      }
      return QVariant();
    }

    case CompatibleGeometryTypesRole:
    {
      switch ( entityType )
      {
        case QgsStyle::Symbol3DEntity:
        {
          QVariantList res;
          const QList< QgsWkbTypes::GeometryType > types = mStyle->symbol3DCompatibleGeometryTypes( name );
          res.reserve( types.size() );
          for ( QgsWkbTypes::GeometryType type : types )
          {
            res << static_cast< int >( type );
          }
          return res;
        }

        case QgsStyle::LabelSettingsEntity:
        case QgsStyle::SymbolEntity:
        case QgsStyle::LegendPatchShapeEntity:
        case QgsStyle::TagEntity:
        case QgsStyle::ColorrampEntity:
        case QgsStyle::SmartgroupEntity:
        case QgsStyle::TextFormatEntity:
          return QVariant();
      }
      return QVariant();
    }

    case EntityName:
      return name;

    case StyleName:
      return mStyle->name();

    case StyleFileName:
      return mStyle->fileName();

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
        case QgsStyle::TagEntity:
        case QgsStyle::SmartgroupEntity:
          return false;

        default:
          name = mEntityNames[ entityType ].value( index.row() - offsetForEntity( entityType ) );
          break;
      }

      const QString newName = value.toString();
      return mStyle->renameEntity( entityType, name, newName );
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
  return headerDataStatic( section, orientation, role );
}

QVariant QgsStyleModel::headerDataStatic( int section, Qt::Orientation orientation, int role )
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
    int count = 0;
    for ( QgsStyle::StyleEntity type : ENTITIES )
      count += mEntityNames[ type ].size();
    return count;
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

  if ( sIconGenerator )
    sIconGenerator->setIconSizes( mAdditionalSizes );

  mIconCache.clear();
}

void QgsStyleModel::setIconGenerator( QgsAbstractStyleEntityIconGenerator *generator )
{
  sIconGenerator = generator;
  connect( sIconGenerator, &QgsAbstractStyleEntityIconGenerator::iconGenerated, QgsApplication::defaultStyleModel(), &QgsStyleModel::iconGenerated, Qt::QueuedConnection );
}

void QgsStyleModel::onEntityAdded( QgsStyle::StyleEntity type, const QString &name )
{
  mIconCache[ type ].remove( name );
  const QStringList oldSymbolNames = mEntityNames[ type ];
  const QStringList newSymbolNames = mStyle->allNames( type );

  // find index of newly added symbol
  const int newNameIndex = newSymbolNames.indexOf( name );
  if ( newNameIndex < 0 )
    return; // shouldn't happen

  const int offset = offsetForEntity( type );
  beginInsertRows( QModelIndex(), newNameIndex + offset, newNameIndex + offset );
  mEntityNames[ type ] = newSymbolNames;
  endInsertRows();
}

void QgsStyleModel::onEntityRemoved( QgsStyle::StyleEntity type, const QString &name )
{
  mIconCache[ type ].remove( name );
  const QStringList oldSymbolNames = mEntityNames[ type ];
  const QStringList newSymbolNames = mStyle->allNames( type );

  // find index of removed symbol
  const int oldNameIndex = oldSymbolNames.indexOf( name );
  if ( oldNameIndex < 0 )
    return; // shouldn't happen

  const int offset = offsetForEntity( type );
  beginRemoveRows( QModelIndex(), oldNameIndex + offset, oldNameIndex + offset );
  mEntityNames[ type ] = newSymbolNames;
  endRemoveRows();
}

void QgsStyleModel::onEntityChanged( QgsStyle::StyleEntity type, const QString &name )
{
  mIconCache[ type ].remove( name );

  const int offset = offsetForEntity( type );
  QModelIndex i = index( offset + mEntityNames[ type ].indexOf( name ), Tags );
  emit dataChanged( i, i, QVector< int >() << Qt::DecorationRole );
}

void QgsStyleModel::onFavoriteChanged( QgsStyle::StyleEntity type, const QString &name, bool )
{
  const int offset = offsetForEntity( type );
  QModelIndex i = index( offset + mEntityNames[ type ].indexOf( name ), Name );
  emit dataChanged( i, i, QVector< int >() << Role::IsFavoriteRole );
}

void QgsStyleModel::onEntityRename( QgsStyle::StyleEntity type, const QString &oldName, const QString &newName )
{
  mIconCache[ type ].remove( oldName );
  const QStringList oldSymbolNames = mEntityNames[ type ];
  const QStringList newSymbolNames = mStyle->allNames( type );

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
    mEntityNames[ type ] = newSymbolNames;
    return;
  }

  const int offset = offsetForEntity( type );
  beginMoveRows( QModelIndex(), oldNameIndex + offset, oldNameIndex + offset, QModelIndex(), ( newNameIndex > oldNameIndex ? newNameIndex + 1 : newNameIndex ) + offset );
  mEntityNames[ type ] = newSymbolNames;
  endMoveRows();
}

void QgsStyleModel::onTagsChanged( int entity, const QString &name, const QStringList & )
{
  QgsStyle::StyleEntity type = static_cast< QgsStyle::StyleEntity >( entity );
  int row = mEntityNames[type].indexOf( name ) + offsetForEntity( type );
  switch ( static_cast< QgsStyle::StyleEntity >( entity ) )
  {
    case QgsStyle::TagEntity:
    case QgsStyle::SmartgroupEntity:
      return;

    default:
      break;
  }
  emit dataChanged( index( row, Name ), index( row, Tags ) );
}

void QgsStyleModel::rebuildSymbolIcons()
{
  mIconCache[ QgsStyle::SymbolEntity ].clear();
  mExpressionContext.reset();
  emit dataChanged( index( 0, 0 ), index( mEntityNames[ QgsStyle::SymbolEntity ].count() - 1, 0 ), QVector<int>() << Qt::DecorationRole );
}

void QgsStyleModel::iconGenerated( QgsStyle::StyleEntity type, const QString &name, const QIcon &icon )
{
  int row = mEntityNames[type].indexOf( name ) + offsetForEntity( type );

  switch ( type )
  {
    case QgsStyle::Symbol3DEntity:
      mPending3dSymbolIcons.remove( name );
      mIconCache[ QgsStyle::Symbol3DEntity ].insert( name, icon );
      emit dataChanged( index( row, 0 ), index( row, 0 ) );
      break;

    case QgsStyle::SymbolEntity:
    case QgsStyle::TagEntity:
    case QgsStyle::ColorrampEntity:
    case QgsStyle::LegendPatchShapeEntity:
    case QgsStyle::TextFormatEntity:
    case QgsStyle::SmartgroupEntity:
    case QgsStyle::LabelSettingsEntity:
      break;
  }
}

QgsStyle::StyleEntity QgsStyleModel::entityTypeFromRow( int row ) const
{
  int maxRowForEntity = 0;
  for ( QgsStyle::StyleEntity type : ENTITIES )
  {
    maxRowForEntity += mEntityNames[ type ].size();
    if ( row < maxRowForEntity )
      return type;
  }

  // should never happen
  Q_ASSERT( false );
  return QgsStyle::SymbolEntity;
}

int QgsStyleModel::offsetForEntity( QgsStyle::StyleEntity entity ) const
{
  int offset = 0;
  for ( QgsStyle::StyleEntity type : ENTITIES )
  {
    if ( type == entity )
      return offset;

    offset += mEntityNames[ type ].size();
  }
  return 0;
}

//
// QgsStyleProxyModel
//

QgsStyleProxyModel::QgsStyleProxyModel( QgsStyle *style, QObject *parent )
  : QSortFilterProxyModel( parent )
  , mStyle( style )
{
  mModel = new QgsStyleModel( mStyle, this );
  setSourceModel( mModel );
  initialize();
}

void QgsStyleProxyModel::initialize()
{
  setSortCaseSensitivity( Qt::CaseInsensitive );
//  setSortLocaleAware( true );
  setDynamicSortFilter( true );
  sort( 0 );

  if ( mStyle )
  {
    connect( mStyle, &QgsStyle::entityTagsChanged, this, [ = ]
    {
      // update tagged symbols if filtering by tag
      if ( mTagId >= 0 )
        setTagId( mTagId );
      if ( mSmartGroupId >= 0 )
        setSmartGroupId( mSmartGroupId );
    } );

    connect( mStyle, &QgsStyle::entityRenamed, this, [ = ]( QgsStyle::StyleEntity entity, const QString &, const QString & )
    {
      switch ( entity )
      {
        case QgsStyle::SmartgroupEntity:
        case QgsStyle::TagEntity:
          return;

        default:
          break;
      }

      if ( mSmartGroupId >= 0 )
        setSmartGroupId( mSmartGroupId );
    } );
  }
}

QgsStyleProxyModel::QgsStyleProxyModel( QgsStyleModel *model, QObject *parent )
  : QSortFilterProxyModel( parent )
  , mModel( model )
  , mStyle( model->style() )
{
  setSourceModel( mModel );
  initialize();
}

#if QT_VERSION >= QT_VERSION_CHECK(5, 13, 0)
QgsStyleProxyModel::QgsStyleProxyModel( QgsCombinedStyleModel *model, QObject *parent )
  : QSortFilterProxyModel( parent )
  , mCombinedModel( model )
{
  setSourceModel( mCombinedModel );
  initialize();
}
#endif

bool QgsStyleProxyModel::filterAcceptsRow( int source_row, const QModelIndex &source_parent ) const
{
  if ( mFilterString.isEmpty()  && !mEntityFilterEnabled && !mSymbolTypeFilterEnabled && mTagId < 0 && mSmartGroupId < 0 && !mFavoritesOnly && mTagFilter.isEmpty() )
    return true;

  const QModelIndex index = sourceModel()->index( source_row, 0, source_parent );

  if ( sourceModel()->data( index, QgsStyleModel::IsTitleRole ).toBool() )
    return true;

  const QString name = sourceModel()->data( index ).toString();
  const QStringList tags = sourceModel()->data( index, QgsStyleModel::TagRole ).toStringList();

  QgsStyle::StyleEntity styleEntityType = static_cast< QgsStyle::StyleEntity >( sourceModel()->data( index, QgsStyleModel::TypeRole ).toInt() );
  if ( mEntityFilterEnabled && ( mEntityFilters.empty() || !mEntityFilters.contains( styleEntityType ) ) )
    return false;

  Qgis::SymbolType symbolType = static_cast< Qgis::SymbolType >( sourceModel()->data( index, QgsStyleModel::SymbolTypeRole ).toInt() );
  if ( mSymbolTypeFilterEnabled && symbolType != mSymbolType )
    return false;

  if ( mLayerType != QgsWkbTypes::UnknownGeometry )
  {
    switch ( styleEntityType )
    {
      case QgsStyle::SymbolEntity:
      case QgsStyle::TextFormatEntity:
      case QgsStyle::TagEntity:
      case QgsStyle::ColorrampEntity:
      case QgsStyle::SmartgroupEntity:
      case QgsStyle::LegendPatchShapeEntity:
        break;

      case QgsStyle::LabelSettingsEntity:
      {
        if ( mLayerType != static_cast< QgsWkbTypes::GeometryType >( sourceModel()->data( index, QgsStyleModel::LayerTypeRole ).toInt() ) )
          return false;
        break;
      }

      case QgsStyle::Symbol3DEntity:
      {
        const QVariantList types = sourceModel()->data( index, QgsStyleModel::CompatibleGeometryTypesRole ).toList();
        if ( !types.empty() && !types.contains( mLayerType ) )
          return false;
        break;
      }
    }
  }

  if ( mTagId >= 0 && !mTaggedSymbolNames.contains( name ) )
    return false;

  if ( mSmartGroupId >= 0 && !mSmartGroupSymbolNames.contains( name ) )
    return false;

  if ( !mTagFilter.isEmpty() && !tags.contains( mTagFilter, Qt::CaseInsensitive ) )
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
      for ( const QString &partToSearch : std::as_const( partsToSearch ) )
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

bool QgsStyleProxyModel::lessThan( const QModelIndex &left, const QModelIndex &right ) const
{
  const QString leftSource = sourceModel()->data( left, QgsStyleModel::StyleFileName ).toString();
  const QString rightSource = sourceModel()->data( right, QgsStyleModel::StyleFileName ).toString();
  if ( leftSource != rightSource )
    return QString::localeAwareCompare( leftSource, rightSource ) < 0;

  const QString leftName = sourceModel()->data( left, QgsStyleModel::EntityName ).toString();
  const QString rightName = sourceModel()->data( right, QgsStyleModel::EntityName ).toString();
  return QString::localeAwareCompare( leftName, rightName ) < 0;
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
  if ( mModel )
    mModel->addDesiredIconSize( size );
#if QT_VERSION >= QT_VERSION_CHECK(5, 13, 0)
  if ( mCombinedModel )
    mCombinedModel->addDesiredIconSize( size );
#endif
}

bool QgsStyleProxyModel::symbolTypeFilterEnabled() const
{
  return mSymbolTypeFilterEnabled;
}

void QgsStyleProxyModel::setSymbolTypeFilterEnabled( bool enabled )
{
  mSymbolTypeFilterEnabled = enabled;
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
  if ( !mStyle )
    return;
  mTagId = id;

  mTaggedSymbolNames.clear();
  if ( mTagId >= 0 )
  {
    for ( QgsStyle::StyleEntity entity : ENTITIES )
      mTaggedSymbolNames.append( mStyle->symbolsWithTag( entity, mTagId ) );
  }

  invalidateFilter();
}

int QgsStyleProxyModel::tagId() const
{
  return mTagId;
}

void QgsStyleProxyModel::setTagString( const QString &tag )
{
  mTagFilter = tag;

  invalidateFilter();
}

QString QgsStyleProxyModel::tagString() const
{
  return mTagFilter;
}

void QgsStyleProxyModel::setSmartGroupId( int id )
{
  if ( !mStyle )
    return;

  mSmartGroupId = id;

  mSmartGroupSymbolNames.clear();
  if ( mSmartGroupId >= 0 )
  {
    for ( QgsStyle::StyleEntity entity : ENTITIES )
      mSmartGroupSymbolNames.append( mStyle->symbolsOfSmartgroup( entity, mSmartGroupId ) );
  }

  invalidateFilter();
}

int QgsStyleProxyModel::smartGroupId() const
{
  return mSmartGroupId;
}

Qgis::SymbolType QgsStyleProxyModel::symbolType() const
{
  return mSymbolType;
}

void QgsStyleProxyModel::setSymbolType( const Qgis::SymbolType symbolType )
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

