/***************************************************************************
                             qgsprovidersublayermodel.cpp
                             ----------------------
    begin                : June 2021
    copyright            : (C) 2021 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprovidersublayermodel.h"
#include "qgsprovidersublayerdetails.h"
#include "qgsiconutils.h"
#include <QLocale>

QgsProviderSublayerModel::QgsProviderSublayerModel( QObject *parent )
  : QAbstractItemModel( parent )
{

}

void QgsProviderSublayerModel::setSublayerDetails( const QList<QgsProviderSublayerDetails> &details )
{
  // remove layers which don't exist in new list
  for ( int i = mSublayers.count() - 1; i >= 0; --i )
  {
    if ( !details.contains( mSublayers.at( i ) ) )
    {
      beginRemoveRows( QModelIndex(), i, i );
      mSublayers.removeAt( i );
      endRemoveRows();
    }
  }

  // and add new layers which exist only in new list
  for ( const QgsProviderSublayerDetails &layer : details )
  {
    if ( !mSublayers.contains( layer ) )
    {
      beginInsertRows( QModelIndex(), mSublayers.count(), mSublayers.count() );
      mSublayers.append( layer );
      endInsertRows();
    }
  }
}

QList<QgsProviderSublayerDetails> QgsProviderSublayerModel::sublayerDetails() const
{
  return mSublayers;
}

QgsProviderSublayerDetails QgsProviderSublayerModel::indexToSublayer( const QModelIndex &index ) const
{
  if ( index.isValid() && index.row() < mSublayers.count() )
  {
    return mSublayers.at( index.row() );
  }

  return QgsProviderSublayerDetails();
}

QgsProviderSublayerModel::NonLayerItem QgsProviderSublayerModel::indexToNonLayerItem( const QModelIndex &index ) const
{
  if ( index.isValid() && index.row() >= mSublayers.count() && index.row() < mSublayers.count() + mNonLayerItems.count() )
  {
    return mNonLayerItems.at( index.row() - mSublayers.count() );
  }

  return QgsProviderSublayerModel::NonLayerItem();
}

void QgsProviderSublayerModel::addNonLayerItem( const QgsProviderSublayerModel::NonLayerItem &item )
{
  beginInsertRows( QModelIndex(), mSublayers.count() + mNonLayerItems.count(), mSublayers.count() + mNonLayerItems.count() );
  mNonLayerItems.append( item );
  endInsertRows();
}

QModelIndex QgsProviderSublayerModel::index( int row, int column, const QModelIndex &parent ) const
{
  if ( column < 0 || column >= columnCount() )
  {
    //column out of bounds
    return QModelIndex();
  }

  if ( !parent.isValid() && row >= 0 && row < mSublayers.size() + mNonLayerItems.size() )
  {
    //return an index for the sublayer at this position
    return createIndex( row, column );
  }

  //only top level supported for now
  return QModelIndex();
}

QModelIndex QgsProviderSublayerModel::parent( const QModelIndex &index ) const
{
  Q_UNUSED( index )

  //all items are top level for now
  return QModelIndex();
}

int QgsProviderSublayerModel::columnCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent )
  return static_cast< int >( Column::Description ) + 1;
}

int QgsProviderSublayerModel::rowCount( const QModelIndex &parent ) const
{
  if ( !parent.isValid() )
  {
    return mSublayers.size() + mNonLayerItems.size();
  }
  else
  {
    //no children for now
    return 0;
  }
}

QVariant QgsProviderSublayerModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() )
    return QVariant();

  if ( index.row() < 0 || index.row() >= rowCount( QModelIndex() ) )
    return QVariant();

  if ( index.row() < mSublayers.count() )
  {
    const QgsProviderSublayerDetails details = mSublayers.at( index.row() );

    switch ( role )
    {
      case Qt::DisplayRole:
      case Qt::ToolTipRole:
      case Qt::EditRole:
      {
        switch ( static_cast< Column >( index.column() ) )
        {
          case QgsProviderSublayerModel::Column::Name:
            return details.name();
          case QgsProviderSublayerModel::Column::Description:
          {
            switch ( details.type() )
            {
              case QgsMapLayerType::VectorLayer:
              {
                QString count;
                if ( details.featureCount() == static_cast< long long >( Qgis::FeatureCountState::Uncounted )
                     || details.featureCount() == static_cast< long long >( Qgis::FeatureCountState::UnknownCount ) )
                  count = tr( "Uncounted" );
                else
                  count = QLocale().toString( details.featureCount() );

                if ( !details.description().isEmpty() )
                  return QStringLiteral( "%1 - %2 (%3)" ).arg( details.description(),
                         QgsWkbTypes::displayString( details.wkbType() ),
                         count );
                else
                  return QStringLiteral( "%2 (%3)" ).arg(
                           QgsWkbTypes::displayString( details.wkbType() ),
                           count );
              }

              case QgsMapLayerType::RasterLayer:
              case QgsMapLayerType::PluginLayer:
              case QgsMapLayerType::MeshLayer:
              case QgsMapLayerType::VectorTileLayer:
              case QgsMapLayerType::AnnotationLayer:
              case QgsMapLayerType::PointCloudLayer:
              case QgsMapLayerType::GroupLayer:
                return details.description();
            }
            break;

          }
        }
        return details.name();

      }

      case Qt::DecorationRole:
      {
        if ( index.column() == 0 )
          return details.type() == QgsMapLayerType::VectorLayer
                 ? ( details.wkbType() != QgsWkbTypes::Unknown ? QgsIconUtils::iconForWkbType( details.wkbType() ) : QVariant() )
                 : QgsIconUtils::iconForLayerType( details.type() );
        else
          return QVariant();
      }

      case static_cast< int >( Role::IsNonLayerItem ):
        return false;

      case static_cast< int >( Role::ProviderKey ):
        return details.providerKey();

      case static_cast< int >( Role::LayerType ):
        return static_cast< int >( details.type() );

      case static_cast< int >( Role::Uri ):
        return details.uri();

      case static_cast< int >( Role::Name ):
        return details.name();

      case static_cast< int >( Role::Description ):
        return details.description();

      case static_cast< int >( Role::Path ):
        return details.path();

      case static_cast< int >( Role::FeatureCount ):
        return details.featureCount();

      case static_cast< int >( Role::WkbType ):
        return details.wkbType();

      case static_cast< int >( Role::GeometryColumnName ):
        return details.geometryColumnName();

      case static_cast< int >( Role::LayerNumber ):
        return details.layerNumber();

      case static_cast< int >( Role::Flags ):
        return static_cast< int >( details.flags() );

      default:
        return QVariant();
    }
  }
  else
  {
    const NonLayerItem details = mNonLayerItems.at( index.row() - mSublayers.count() );

    switch ( role )
    {
      case Qt::DisplayRole:
      case Qt::ToolTipRole:
      case Qt::EditRole:
      {
        switch ( static_cast< Column >( index.column() ) )
        {
          case QgsProviderSublayerModel::Column::Name:
            return details.name();
          case QgsProviderSublayerModel::Column::Description:
            return details.description();
        }
        return QVariant();
      }

      case Qt::DecorationRole:
      {
        if ( index.column() == 0 )
          return details.icon();
        else
          return QVariant();
      }

      case static_cast< int >( Role::IsNonLayerItem ):
        return true;

      case static_cast< int >( Role::Uri ):
        return details.uri();

      case static_cast< int >( Role::Name ):
        return details.name();

      case static_cast< int >( Role::Description ):
        return details.description();

      case static_cast< int >( Role::NonLayerItemType ):
        return details.type();

      default:
        return QVariant();
    }
  }
}

QVariant QgsProviderSublayerModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  switch ( orientation )
  {
    case Qt::Vertical:
      break;
    case Qt::Horizontal:
    {
      switch ( role )
      {
        case Qt::DisplayRole:
        case Qt::ToolTipRole:
        {
          switch ( static_cast< Column>( section ) )
          {
            case QgsProviderSublayerModel::Column::Name:
              return tr( "Item" );
            case QgsProviderSublayerModel::Column::Description:
              return tr( "Description" );
          }
          break;
        }
      }
      break;
    }
  }
  return QVariant();
}


//
// QgsProviderSublayerModel::NonLayerItem
//

QString QgsProviderSublayerModel::NonLayerItem::type() const
{
  return mType;
}

void QgsProviderSublayerModel::NonLayerItem::setType( const QString &type )
{
  mType = type;
}

QString QgsProviderSublayerModel::NonLayerItem::name() const
{
  return mName;
}

void QgsProviderSublayerModel::NonLayerItem::setName( const QString &name )
{
  mName = name;
}

QString QgsProviderSublayerModel::NonLayerItem::description() const
{
  return mDescription;
}

void QgsProviderSublayerModel::NonLayerItem::setDescription( const QString &description )
{
  mDescription = description;
}

QString QgsProviderSublayerModel::NonLayerItem::uri() const
{
  return mUri;
}

void QgsProviderSublayerModel::NonLayerItem::setUri( const QString &uri )
{
  mUri = uri;
}

QIcon QgsProviderSublayerModel::NonLayerItem::icon() const
{
  return mIcon;
}

void QgsProviderSublayerModel::NonLayerItem::setIcon( const QIcon &icon )
{
  mIcon = icon;
}

bool QgsProviderSublayerModel::NonLayerItem::operator==( const QgsProviderSublayerModel::NonLayerItem &other ) const
{
  return mType == other.mType
         && mName == other.mName
         && mDescription == other.mDescription
         && mUri == other.mUri;
}

bool QgsProviderSublayerModel::NonLayerItem::operator!=( const QgsProviderSublayerModel::NonLayerItem &other ) const
{
  return !( *this == other );
}

//
// QgsProviderSublayerProxyModel
//

QgsProviderSublayerProxyModel::QgsProviderSublayerProxyModel( QObject *parent )
  : QSortFilterProxyModel( parent )
{
  setDynamicSortFilter( true );
  sort( 0 );
}

bool QgsProviderSublayerProxyModel::filterAcceptsRow( int source_row, const QModelIndex &source_parent ) const
{
  const QModelIndex sourceIndex = sourceModel()->index( source_row, 0, source_parent );

  if ( !mIncludeSystemTables && static_cast< Qgis::SublayerFlags >( sourceModel()->data( sourceIndex, static_cast< int >( QgsProviderSublayerModel::Role::Flags ) ).toInt() ) & Qgis::SublayerFlag::SystemTable )
    return false;

  if ( mFilterString.trimmed().isEmpty() )
    return true;

  if ( sourceModel()->data( sourceIndex, static_cast< int >( QgsProviderSublayerModel::Role::Name ) ).toString().contains( mFilterString, Qt::CaseInsensitive ) )
    return true;

  if ( sourceModel()->data( sourceIndex, static_cast< int >( QgsProviderSublayerModel::Role::Description ) ).toString().contains( mFilterString, Qt::CaseInsensitive ) )
    return true;

  const QVariant wkbTypeVariant =  sourceModel()->data( sourceIndex, static_cast< int >( QgsProviderSublayerModel::Role::WkbType ) );
  if ( wkbTypeVariant.isValid() )
  {
    const QgsWkbTypes::Type wkbType = static_cast< QgsWkbTypes::Type >( wkbTypeVariant.toInt() );
    if ( QgsWkbTypes::displayString( wkbType ).contains( mFilterString, Qt::CaseInsensitive ) )
      return true;
  }

  return false;
}

bool QgsProviderSublayerProxyModel::lessThan( const QModelIndex &source_left, const QModelIndex &source_right ) const
{
  const bool leftIsNonLayer = sourceModel()->data( source_left, static_cast< int >( QgsProviderSublayerModel::Role::IsNonLayerItem ) ).toBool();
  const bool rightIsNonLayer = sourceModel()->data( source_right, static_cast< int >( QgsProviderSublayerModel::Role::IsNonLayerItem ) ).toBool();

  if ( leftIsNonLayer && !rightIsNonLayer )
    return true;
  else if ( rightIsNonLayer && !leftIsNonLayer )
    return false;

  const QString leftName = sourceModel()->data( source_left, static_cast< int >( QgsProviderSublayerModel::Role::Name ) ).toString();
  const QString rightName = sourceModel()->data( source_right, static_cast< int >( QgsProviderSublayerModel::Role::Name ) ).toString();

  return QString::localeAwareCompare( leftName, rightName ) < 0;
}

bool QgsProviderSublayerProxyModel::includeSystemTables() const
{
  return mIncludeSystemTables;
}

void QgsProviderSublayerProxyModel::setIncludeSystemTables( bool include )
{
  mIncludeSystemTables = include;
  invalidateFilter();
}

QString QgsProviderSublayerProxyModel::filterString() const
{
  return mFilterString;
}

void QgsProviderSublayerProxyModel::setFilterString( const QString &filter )
{
  mFilterString = filter;
  invalidateFilter();
}
