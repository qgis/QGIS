/***************************************************************************
                             qgshistorywidget.cpp
                             ------------------
    Date                 : April 2023
    Copyright            : (C) 2023 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgshistorywidget.h"
#include "qgsgui.h"
#include "qgshistoryentrymodel.h"
#include "qgshistoryentrynode.h"

#include <QTextBrowser>

QgsHistoryWidget::QgsHistoryWidget( const QString &providerId, Qgis::HistoryProviderBackends backends, QgsHistoryProviderRegistry *registry, QWidget *parent )
  : QgsPanelWidget( parent )
{
  setupUi( this );

  mModel = new QgsHistoryEntryModel( providerId, backends, registry, this );
  mTreeView->setModel( mModel );

  connect( mTreeView->selectionModel(), &QItemSelectionModel::currentChanged, this, &QgsHistoryWidget::currentItemChanged );
}

void QgsHistoryWidget::currentItemChanged( const QModelIndex &selected, const QModelIndex & )
{
  QWidget *newWidget = nullptr;
  if ( QgsHistoryEntryNode *node = mModel->index2node( selected ) )
  {
    newWidget = node->createWidget();
    if ( !newWidget )
    {
      const QString html = node->html();
      if ( !html.isEmpty() )
      {
        QTextBrowser *htmlBrowser = new QTextBrowser();
        htmlBrowser->setOpenExternalLinks( true );
        htmlBrowser->setHtml( html );
        newWidget = htmlBrowser;
      }
    }
    if ( newWidget )
    {
      mContainerStackedWidget->addWidget( newWidget );
      mContainerStackedWidget->setCurrentWidget( newWidget );
    }
  }

  if ( !newWidget )
  {
    //remove current widget, if any
    if ( mContainerStackedWidget->count() > 1 )
    {
      mContainerStackedWidget->removeWidget( mContainerStackedWidget->widget( 1 ) );
      mContainerStackedWidget->setCurrentIndex( 0 );
    }
  }
}
