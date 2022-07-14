/***************************************************************************
  qgsdataitemguiprovider.cpp
  --------------------------------------
  Date                 : October 2018
  Copyright            : (C) 2018 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdataitemguiprovider.h"
#include "qgsdataitem.h"
#include "qgsproviderregistry.h"
#include "qgsprovidermetadata.h"
#include "qgsabstractdatabaseproviderconnection.h"
#include "qgsmessagebar.h"
#include <QMessageBox>
//
// QgsDataItemGuiContext
//

QgsMessageBar *QgsDataItemGuiContext::messageBar() const
{
  return mMessageBar;
}

void QgsDataItemGuiContext::setMessageBar( QgsMessageBar *messageBar )
{
  mMessageBar = messageBar;
}

QgsBrowserTreeView *QgsDataItemGuiContext::view() const
{
  return mView;
}

void QgsDataItemGuiContext::setView( QgsBrowserTreeView *view )
{
  mView = view;
}

//
// QgsDataItemGuiProvider
//

void QgsDataItemGuiProvider::populateContextMenu( QgsDataItem *, QMenu *, const QList<QgsDataItem *> &, QgsDataItemGuiContext )
{

}

int QgsDataItemGuiProvider::precedenceWhenPopulatingMenus() const
{
  return 0;
}

bool QgsDataItemGuiProvider::rename( QgsDataItem *, const QString &, QgsDataItemGuiContext )
{
  return false;
}

bool QgsDataItemGuiProvider::deleteLayer( QgsLayerItem *, QgsDataItemGuiContext )
{
  return false;
}

bool QgsDataItemGuiProvider::handleDoubleClick( QgsDataItem *, QgsDataItemGuiContext )
{
  return false;
}

bool QgsDataItemGuiProvider::acceptDrop( QgsDataItem *, QgsDataItemGuiContext )
{
  return false;
}

bool QgsDataItemGuiProvider::handleDrop( QgsDataItem *, QgsDataItemGuiContext, const QMimeData *, Qt::DropAction )
{
  return false;
}

QWidget *QgsDataItemGuiProvider::createParamWidget( QgsDataItem *, QgsDataItemGuiContext )
{
  return nullptr;
}

void QgsDataItemGuiProvider::notify( const QString &title, const QString &message, QgsDataItemGuiContext context, Qgis::MessageLevel level, int duration, QWidget *parent )
{
  if ( QgsMessageBar *bar = context.messageBar() )
  {
    bar->pushMessage( title, message, level, duration );
  }
  else
  {
    switch ( level )
    {
      case Qgis::MessageLevel::Info:
      case Qgis::MessageLevel::NoLevel:
      {
        QMessageBox::information( parent, title, message );
        break;
      }
      case Qgis::MessageLevel::Warning:
      {
        QMessageBox::warning( parent, title, message );
        break;
      }
      case Qgis::MessageLevel::Critical:
      {
        QMessageBox::critical( parent, title, message );
        break;
      }
      case Qgis::MessageLevel::Success:
      {
        // There is no "success" in message box, let's use information instead
        QMessageBox::information( parent, title, message );
        break;
      }
    }
  }
}
