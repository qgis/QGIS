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
    // new support
    const QList<QgsDataItemGuiProvider *> providers = QgsGui::dataItemGuiProviderRegistry()->providers();
    for ( QgsDataItemGuiProvider *provider : providers )
    {
      if ( provider->acceptDrop( ptr, createDataItemContext() ) )
      {
        flags |= Qt::ItemIsDropEnabled;
        break;
      }
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
