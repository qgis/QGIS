/***************************************************************************

               ----------------------------------------------------
              date                 : 18.8.2015
              copyright            : (C) 2015 by Matthias Kuhn
              email                : matthias (at) opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgswelcomedialog.h"
#include "qgsproject.h"
#include "qgisapp.h"

#include <QHBoxLayout>
#include <QListView>
#include <QSettings>

QgsWelcomeDialog::QgsWelcomeDialog()
{
  QHBoxLayout* layout = new QHBoxLayout();
  setLayout( layout );

  QListView* welcomeScreenListView = new QListView();
  mModel = new QgsWelcomePageItemsModel();
  welcomeScreenListView->setModel( mModel );
  layout->addWidget( welcomeScreenListView );

  setWindowTitle( tr( "Recent Projects..." ) );

  QSettings settings;
  restoreGeometry( settings.value( "/Windows/WelcomeDialog/geometry" ).toByteArray() );

  connect( welcomeScreenListView, SIGNAL( doubleClicked( QModelIndex ) ), this, SLOT( itemDoubleClicked( QModelIndex ) ) );
}

void QgsWelcomeDialog::setRecentProjects(const QList<QgsWelcomePageItemsModel::RecentProjectData>& recentProjects)
{
  mModel->setRecentProjects( recentProjects );
}

void QgsWelcomeDialog::itemDoubleClicked( const QModelIndex& index )
{
  QgisApp::instance()->openProject( mModel->data( index, Qt::ToolTipRole ).toString() );
  accept();
}


void QgsWelcomeDialog::done( int result )
{
  QDialog::done( result );
  QSettings settings;
  settings.setValue( "/Windows/WelcomeDialog/geometry", saveGeometry() );
}
