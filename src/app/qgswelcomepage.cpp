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
#include <QDesktopServices>

QgsWelcomePage::QgsWelcomePage( QWidget* parent )
    : QWidget( parent )
{
  QVBoxLayout* mainLayout = new QVBoxLayout;
  mainLayout->setMargin( 0 );
  setLayout( mainLayout );

  QHBoxLayout* layout = new QHBoxLayout();
  layout->setMargin( 9 );

  mainLayout->addLayout( layout );

  QWidget* recentProjctsContainer = new QWidget;
  recentProjctsContainer->setLayout( new QVBoxLayout );
  QLabel* recentProjectsTitle = new QLabel( QString( "<h1>%1</h1>" ).arg( tr( "Recent Projects" ) ) );
  recentProjctsContainer->layout()->addWidget( recentProjectsTitle );

  QListView* recentProjectsListView = new QListView();
  mModel = new QgsWelcomePageItemsModel( recentProjectsListView );
  recentProjectsListView->setModel( mModel );
  recentProjectsListView->setStyleSheet( "QListView::item {"
                                         "  margin-top: 5px;"
                                         "  margin-bottom: 5px;"
                                         "  margin-left: 15px;"
                                         "  margin-right: 15px;"
                                         "  border-width: 1px;"
                                         "  border-color: #999;"
                                         "  border-radius: 9px;"
                                         "  background: #eee;"
                                         "  padding: 10px;"
                                         "}"
                                         "QListView::item:selected:active {"
                                         "  background: #aaaaaa;"
                                         "}" );

  recentProjctsContainer->layout()->addWidget( recentProjectsListView );

  layout->addWidget( recentProjctsContainer );

  QWidget* whatsNewContainer = new QWidget;
  whatsNewContainer->setLayout( new QVBoxLayout );
  QLabel* whatsNewTitle = new QLabel( QString( "<h1>%1</h1>" ).arg( tr( "QGIS News" ) ) );
  whatsNewContainer->layout()->addWidget( whatsNewTitle );

  QgsWebView* whatsNewPage = new QgsWebView();
  whatsNewPage->setUrl( QUrl::fromLocalFile( QgsApplication::whatsNewFilePath() ) );
  whatsNewPage->page()->setLinkDelegationPolicy( QWebPage::DelegateAllLinks );
  whatsNewPage->setContextMenuPolicy( Qt::NoContextMenu );
  whatsNewPage->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Expanding );
  whatsNewPage->setStyleSheet( "background:transparent" );
  whatsNewPage->setAttribute( Qt::WA_TranslucentBackground );

  whatsNewContainer->layout()->addWidget( whatsNewPage );
  whatsNewContainer->setMaximumWidth( 250 );
  whatsNewContainer->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Preferred );
  layout->addWidget( whatsNewContainer );

  connect( whatsNewPage, SIGNAL( linkClicked( QUrl ) ), this, SLOT( whatsNewLinkClicked( QUrl ) ) );

  mVersionInformation = new QLabel;
  mainLayout->addWidget( mVersionInformation );
  mVersionInformation->setVisible( false );

  QgsVersionInfo* versionInfo = new QgsVersionInfo();
  connect( versionInfo, SIGNAL( versionInfoAvailable() ), this, SLOT( versionInfoReceived() ) );
  versionInfo->checkVersion();

  connect( recentProjectsListView, SIGNAL( activated( QModelIndex ) ), this, SLOT( itemActivated( QModelIndex ) ) );
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
    mVersionInformation->setText( QString( "<b>%1</b>: %2" )
                                  .arg( tr( "There is a new QGIS version available" ) )
                                  .arg( versionInfo->downloadInfo() ) );
    mVersionInformation->setStyleSheet( "QLabel{"
                                        "  background-color: #dddd00;"
                                        "  padding: 5px;"
                                        "}" );
  }
}

void QgsWelcomePage::whatsNewLinkClicked( const QUrl& url )
{
  QDesktopServices::openUrl( url );
}
