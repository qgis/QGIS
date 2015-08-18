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

  QListView* welcomeScreenListView = new QListView();
  mModel = new QgsWelcomePageItemsModel( welcomeScreenListView );
  welcomeScreenListView->setModel( mModel );
  welcomeScreenListView->setSizePolicy( QSizePolicy::Ignored, QSizePolicy::MinimumExpanding );
  welcomeScreenListView->setStyleSheet( "QListView::item {"
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

  recentProjctsContainer->layout()->addWidget( welcomeScreenListView );

  layout->addWidget( recentProjctsContainer );

  QWidget* whatsNewContainer = new QWidget;
  whatsNewContainer->setLayout( new QVBoxLayout );
  QLabel* whatsNewTitle = new QLabel( QString( "<h1>%1</h1>" ).arg( tr( "QGIS News" ) ) );
  whatsNewContainer->layout()->addWidget( whatsNewTitle );

  QgsWebView* whatsNewPage = new QgsWebView();
  whatsNewPage->setUrl( QUrl::fromLocalFile( QgsApplication::whatsNewFilePath() ) );
  whatsNewPage->page()->setLinkDelegationPolicy( QWebPage::DelegateAllLinks );
  whatsNewPage->setContextMenuPolicy( Qt::NoContextMenu );
  whatsNewPage->setSizePolicy( QSizePolicy::Ignored, QSizePolicy::MinimumExpanding );

  whatsNewContainer->layout()->addWidget( whatsNewPage );
  layout->addWidget( whatsNewContainer );

  connect( whatsNewPage, SIGNAL( linkClicked( QUrl ) ), this, SLOT( whatsNewLinkClicked( QUrl ) ) );

  mVersionInformation = new QLabel;
  mainLayout->addWidget( mVersionInformation );
  mVersionInformation->setVisible( false );

  QgsVersionInfo* versionInfo = new QgsVersionInfo();
  connect( versionInfo, SIGNAL( versionInfoAvailable() ), this, SLOT( versionInfoReceived() ) );
  versionInfo->checkVersion();

  connect( welcomeScreenListView, SIGNAL( doubleClicked( QModelIndex ) ), this, SLOT( itemDoubleClicked( QModelIndex ) ) );
}

void QgsWelcomePage::setRecentProjects( const QList<QgsWelcomePageItemsModel::RecentProjectData>& recentProjects )
{
  mModel->setRecentProjects( recentProjects );
}

void QgsWelcomePage::itemDoubleClicked( const QModelIndex& index )
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
