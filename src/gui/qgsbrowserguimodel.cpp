/***************************************************************************
    qgsbrowserguimodel.cpp
    ----------------------
    begin                : June 2019
    copyright            : (C) 2019 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsbrowserguimodel.h"
#include "moc_qgsbrowserguimodel.cpp"
#include "qgslogger.h"
#include "qgsdataitemguiproviderregistry.h"
#include "qgsdataitemguiprovider.h"
#include "qgsgui.h"
#include "qgsmessagebar.h"
#include "qgsdataitem.h"

QgsBrowserGuiModel::QgsBrowserGuiModel( QObject *parent )
  : QgsBrowserModel( parent )
{
}

QgsDataItemGuiContext QgsBrowserGuiModel::createDataItemContext() const
{
  QgsDataItemGuiContext context;
  context.setMessageBar( mMessageBar );
  return context;
}

struct QgsBrowserGuiModelCachedAcceptDropValue
{
    bool acceptDrop;
    int numberOfProviders;
};
Q_DECLARE_METATYPE( QgsBrowserGuiModelCachedAcceptDropValue )

Qt::ItemFlags QgsBrowserGuiModel::flags( const QModelIndex &index ) const
{
  if ( !index.isValid() )
    return Qt::ItemFlags();

  Qt::ItemFlags flags = QgsBrowserModel::flags( index );
  QgsDataItem *ptr = dataItem( index );

  if ( !ptr )
  {
    QgsDebugMsgLevel( QStringLiteral( "FLAGS PROBLEM!" ), 4 );
    return Qt::ItemFlags();
  }

  Q_NOWARN_DEPRECATED_PUSH
  const bool legacyAcceptDrop = ptr->acceptDrop();
  Q_NOWARN_DEPRECATED_POP

  if ( legacyAcceptDrop )
    // legacy support for data items
    flags |= Qt::ItemIsDropEnabled;
  else
  {
    // Cache the value of acceptDrop(), as it can be slow to evaluate.
    // e.g. for a OGR datasource, this requires to open it. And this method
    // is called each time the browser is redrawn.
    // We cache the number of providers too, to be able to invalidate the
    // cached value if new providers are installed.
    QVariant cachedProperty = ptr->property( "_qgs_accept_drop_cached" );
    const QList<QgsDataItemGuiProvider *> providers = QgsGui::dataItemGuiProviderRegistry()->providers();
    bool refreshAcceptDrop = true;
    if ( cachedProperty.isValid() )
    {
      QgsBrowserGuiModelCachedAcceptDropValue cached = cachedProperty.value<QgsBrowserGuiModelCachedAcceptDropValue>();
      if ( cached.numberOfProviders == providers.size() )
      {
        refreshAcceptDrop = false;
        if ( cached.acceptDrop )
          flags |= Qt::ItemIsDropEnabled;
      }
    }

    if ( refreshAcceptDrop )
    {
      // new support
      for ( QgsDataItemGuiProvider *provider : providers )
      {
        if ( provider->acceptDrop( ptr, createDataItemContext() ) )
        {
          flags |= Qt::ItemIsDropEnabled;
          break;
        }
      }

      QgsBrowserGuiModelCachedAcceptDropValue cached;
      cached.acceptDrop = ( flags & Qt::ItemIsDropEnabled ) != 0;
      cached.numberOfProviders = providers.size();
      QVariant var;
      var.setValue( cached );
      ptr->setProperty( "_qgs_accept_drop_cached", var );
    }
  }
  return flags;
}

bool QgsBrowserGuiModel::dropMimeData( const QMimeData *data, Qt::DropAction action, int, int, const QModelIndex &parent )
{
  QgsDataItem *destItem = dataItem( parent );
  if ( !destItem )
  {
    QgsDebugMsgLevel( QStringLiteral( "DROP PROBLEM!" ), 4 );
    return false;
  }

  Q_NOWARN_DEPRECATED_PUSH
  const bool legacyAcceptDrop = destItem->acceptDrop();
  Q_NOWARN_DEPRECATED_POP

  // legacy support for data items
  if ( legacyAcceptDrop )
  {
    Q_NOWARN_DEPRECATED_PUSH
    return destItem->handleDrop( data, action );
    Q_NOWARN_DEPRECATED_POP
  }
  else
  {
    // new support
    const QList<QgsDataItemGuiProvider *> providers = QgsGui::dataItemGuiProviderRegistry()->providers();
    for ( QgsDataItemGuiProvider *provider : providers )
    {
      if ( provider->handleDrop( destItem, createDataItemContext(), data, action ) )
      {
        return true;
      }
    }
  }
  return false;
}

bool QgsBrowserGuiModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  QgsDataItem *item = dataItem( index );
  if ( !item )
  {
    QgsDebugMsgLevel( QStringLiteral( "RENAME PROBLEM!" ), 4 );
    return false;
  }

  if ( !( item->capabilities2() & Qgis::BrowserItemCapability::Rename )
       && !( item->capabilities2() & Qgis::BrowserItemCapability::ItemRepresentsFile ) )
    return false;

  switch ( role )
  {
    case Qt::EditRole:
    {
      // new support
      const QList<QgsDataItemGuiProvider *> providers = QgsGui::dataItemGuiProviderRegistry()->providers();
      for ( QgsDataItemGuiProvider *provider : providers )
      {
        if ( provider->rename( item, value.toString(), createDataItemContext() ) )
        {
          return true;
        }
      }

      Q_NOWARN_DEPRECATED_PUSH
      return item->rename( value.toString() );
      Q_NOWARN_DEPRECATED_POP
    }
  }
  return false;
}

void QgsBrowserGuiModel::setMessageBar( QgsMessageBar *bar )
{
  mMessageBar = bar;
}
