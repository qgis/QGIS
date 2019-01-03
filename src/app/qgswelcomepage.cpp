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
#include "qgssettings.h"
#include "qgsgui.h"
#include "qgsnative.h"
#include "qgsfileutils.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QListView>
#include <QDesktopServices>

QgsWelcomePage::QgsWelcomePage( bool skipVersionCheck, QWidget *parent )
  : QWidget( parent )
{
  QgsSettings settings;

  QVBoxLayout *mainLayout = new QVBoxLayout;
  mainLayout->setMargin( 0 );
  mainLayout->setContentsMargins( 0, 0, 0, 0 );
  setLayout( mainLayout );

  QHBoxLayout *layout = new QHBoxLayout();
  layout->setMargin( 0 );

  mainLayout->addLayout( layout );

  QWidget *recentProjectsContainer = new QWidget;
  recentProjectsContainer->setLayout( new QVBoxLayout );
  recentProjectsContainer->layout()->setContentsMargins( 0, 0, 0, 0 );
  recentProjectsContainer->layout()->setMargin( 0 );

  int titleSize = QApplication::fontMetrics().height() * 1.4;
  QLabel *recentProjectsTitle = new QLabel( QStringLiteral( "<div style='font-size:%1px;font-weight:bold'>%2</div>" ).arg( titleSize ).arg( tr( "Recent Projects" ) ) );
  recentProjectsTitle->setContentsMargins( titleSize / 2, titleSize / 6, 0, 0 );

  recentProjectsContainer->layout()->addWidget( recentProjectsTitle );

  mRecentProjectsListView = new QListView();
  mRecentProjectsListView->setResizeMode( QListView::Adjust );
  mRecentProjectsListView->setContextMenuPolicy( Qt::CustomContextMenu );
  connect( mRecentProjectsListView, &QListView::customContextMenuRequested, this, &QgsWelcomePage::showContextMenuForProjects );

  mModel = new QgsWelcomePageItemsModel( mRecentProjectsListView );
  mRecentProjectsListView->setModel( mModel );
  mRecentProjectsListView->setItemDelegate( new QgsWelcomePageItemDelegate( mRecentProjectsListView ) );

  recentProjectsContainer->layout()->addWidget( mRecentProjectsListView );

  layout->addWidget( recentProjectsContainer );

  mVersionInformation = new QLabel;
  mainLayout->addWidget( mVersionInformation );
  mVersionInformation->setVisible( false );

  mVersionInfo = new QgsVersionInfo();
  if ( !QgsApplication::isRunningFromBuildDir() && settings.value( QStringLiteral( "/qgis/allowVersionCheck" ), true ).toBool()
       && settings.value( QStringLiteral( "qgis/checkVersion" ), true ).toBool() && !skipVersionCheck )
  {
    connect( mVersionInfo, &QgsVersionInfo::versionInfoAvailable, this, &QgsWelcomePage::versionInfoReceived );
    mVersionInfo->checkVersion();
  }

  connect( mRecentProjectsListView, &QAbstractItemView::activated, this, &QgsWelcomePage::itemActivated );
}

QgsWelcomePage::~QgsWelcomePage()
{
  delete mVersionInfo;
}

void QgsWelcomePage::setRecentProjects( const QList<QgsWelcomePageItemsModel::RecentProjectData> &recentProjects )
{
  mModel->setRecentProjects( recentProjects );
}

void QgsWelcomePage::itemActivated( const QModelIndex &index )
{
  QgisApp::instance()->openProject( mModel->data( index, Qt::ToolTipRole ).toString() );
}

void QgsWelcomePage::versionInfoReceived()
{
  QgsVersionInfo *versionInfo = qobject_cast<QgsVersionInfo *>( sender() );
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

void QgsWelcomePage::showContextMenuForProjects( QPoint point )
{
  QModelIndex index = mRecentProjectsListView->indexAt( point );
  if ( !index.isValid() )
    return;

  bool pin = mModel->data( index, QgsWelcomePageItemsModel::PinRole ).toBool();
  QString path = mModel->data( index, QgsWelcomePageItemsModel::PathRole ).toString();
  if ( path.isEmpty() )
    return;

  bool enabled = mModel->flags( index ) & Qt::ItemIsEnabled;

  QMenu *menu = new QMenu( this );

  if ( enabled )
  {
    if ( !pin )
    {
      QAction *pinAction = new QAction( tr( "Pin to List" ), menu );
      connect( pinAction, &QAction::triggered, this, [this, index]
      {
        mModel->pinProject( index );
        emit projectPinned( index.row() );
      } );
      menu->addAction( pinAction );
    }
    else
    {
      QAction *pinAction = new QAction( tr( "Unpin from List" ), menu );
      connect( pinAction, &QAction::triggered, this, [this, index]
      {
        mModel->unpinProject( index );
        emit projectUnpinned( index.row() );
      } );
      menu->addAction( pinAction );
    }
    QAction *openFolderAction = new QAction( tr( "Open Directory…" ), menu );
    connect( openFolderAction, &QAction::triggered, this, [path]
    {
      QgsGui::instance()->nativePlatformInterface()->openFileExplorerAndSelectFile( path );
    } );
    menu->addAction( openFolderAction );
  }
  else
  {
    QAction *rescanAction = new QAction( tr( "Refresh" ), menu );
    connect( rescanAction, &QAction::triggered, this, [this, index]
    {
      mModel->recheckProject( index );
    } );
    menu->addAction( rescanAction );

    // add an entry to open the closest existing path to the original project file location
    // to help users re-find moved/renamed projects!
    const QString closestPath = QgsFileUtils::findClosestExistingPath( path );
    QAction *openFolderAction = new QAction( tr( "Open “%1”…" ).arg( QDir::toNativeSeparators( closestPath ) ), menu );
    connect( openFolderAction, &QAction::triggered, this, [closestPath]
    {
      QDesktopServices::openUrl( QUrl::fromLocalFile( closestPath ) );
    } );
    menu->addAction( openFolderAction );
  }
  QAction *removeProjectAction = new QAction( tr( "Remove from List" ), menu );
  connect( removeProjectAction, &QAction::triggered, this, [this, index]
  {
    mModel->removeProject( index );
    emit projectRemoved( index.row() );
  } );
  menu->addAction( removeProjectAction );

  menu->popup( mapToGlobal( point ) );
}
