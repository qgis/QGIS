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

#include "qgswelcomepage.h"
#include "qgsproject.h"
#include "qgisapp.h"
#include "qgsversioninfo.h"
#include "qgsapplication.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QListView>
#include <QSettings>

QgsWelcomePage::QgsWelcomePage( bool skipVersionCheck, QWidget* parent )
  : QWidget( parent )
{
  QSettings settings;

  QVBoxLayout* mainLayout = new QVBoxLayout;
  mainLayout->setMargin( 0 );
  mainLayout->setContentsMargins( 0, 0, 0, 0 );
  setLayout( mainLayout );

  QHBoxLayout* layout = new QHBoxLayout();
  layout->setMargin( 0 );

  mainLayout->addLayout( layout );

  QWidget* recentProjectsContainer = new QWidget;
  recentProjectsContainer->setLayout( new QVBoxLayout );
  recentProjectsContainer->layout()->setContentsMargins( 0, 0, 0, 0 );
  recentProjectsContainer->layout()->setMargin( 0 );
  QLabel* recentProjectsTitle = new QLabel( QStringLiteral( "<h1>%1</h1>" ).arg( tr( "Recent Projects" ) ) );
  recentProjectsContainer->layout()->addWidget( recentProjectsTitle );

  QListView* recentProjectsListView = new QListView();
  recentProjectsListView->setResizeMode( QListView::Adjust );

  mModel = new QgsWelcomePageItemsModel( recentProjectsListView );
  recentProjectsListView->setModel( mModel );
  recentProjectsListView->setItemDelegate( new QgsWelcomePageItemDelegate( recentProjectsListView ) );

  recentProjectsContainer->layout()->addWidget( recentProjectsListView );

  layout->addWidget( recentProjectsContainer );

  mVersionInformation = new QLabel;
  mainLayout->addWidget( mVersionInformation );
  mVersionInformation->setVisible( false );

  mVersionInfo = new QgsVersionInfo();
  if ( !QgsApplication::isRunningFromBuildDir() && settings.value( QStringLiteral( "/qgis/checkVersion" ), true ).toBool() && !skipVersionCheck )
  {
    connect( mVersionInfo, SIGNAL( versionInfoAvailable() ), this, SLOT( versionInfoReceived() ) );
    mVersionInfo->checkVersion();
  }

  connect( recentProjectsListView, SIGNAL( activated( QModelIndex ) ), this, SLOT( itemActivated( QModelIndex ) ) );
}

QgsWelcomePage::~QgsWelcomePage()
{
  delete mVersionInfo;
}

void QgsWelcomePage::setRecentProjects( const QList<QgsWelcomePageItemsModel::RecentProjectData>& recentProjects )
{
  mModel->setRecentProjects( recentProjects );
}

void QgsWelcomePage::itemActivated( const QModelIndex& index )
{
  QgisApp::instance()->openProject( mModel->data( index, Qt::ToolTipRole ).toString() );
}

void QgsWelcomePage::versionInfoReceived()
{
  QgsVersionInfo* versionInfo = qobject_cast<QgsVersionInfo*>( sender() );
  Q_ASSERT( versionInfo );

  if ( versionInfo->newVersionAvailable() )
  {
    mVersionInformation->setVisible( true );
    mVersionInformation->setText( QStringLiteral( "<b>%1</b>: %2" )
                                  .arg( tr( "There is a new QGIS version available" ),
                                        versionInfo->downloadInfo() ) );
    mVersionInformation->setStyleSheet( "QLabel{"
                                        "  background-color: #dddd00;"
                                        "  padding: 5px;"
                                        "}" );
  }
}
