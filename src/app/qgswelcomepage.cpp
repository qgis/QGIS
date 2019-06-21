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
#include "qgsstringutils.h"
#include "qgsfileutils.h"
#include "qgstemplateprojectsmodel.h"
#include "qgsprojectlistitemdelegate.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QListView>
#include <QDesktopServices>
#include <QTextBrowser>
#include <QMessageBox>

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

  QWidget *centerContainer = new QWidget;
  QGridLayout *centerLayout = new QGridLayout;
  centerContainer->setLayout( centerLayout );

  centerLayout->setContentsMargins( 0, 0, 0, 0 );
  centerLayout->setMargin( 0 );

  int titleSize = static_cast<int>( QApplication::fontMetrics().height() * 1.4 );
  mRecentProjectsTitle = new QLabel( QStringLiteral( "<div style='font-size:%1px;font-weight:bold'>%2</div>" ).arg( QString::number( titleSize ), tr( "Recent Projects" ) ) );
  mRecentProjectsTitle->setContentsMargins( titleSize / 2, titleSize / 6, 0, 0 );
  centerLayout->addWidget( mRecentProjectsTitle, 0, 0 );

  mRecentProjectsListView = new QListView();
  mRecentProjectsListView->setResizeMode( QListView::Adjust );
  mRecentProjectsListView->setContextMenuPolicy( Qt::CustomContextMenu );
  connect( mRecentProjectsListView, &QListView::customContextMenuRequested, this, &QgsWelcomePage::showContextMenuForProjects );

  mRecentProjectsModel = new QgsRecentProjectItemsModel( mRecentProjectsListView );
  mRecentProjectsListView->setModel( mRecentProjectsModel );
  QgsProjectListItemDelegate *recentProjectsDelegate = new QgsProjectListItemDelegate( mRecentProjectsListView );
  mRecentProjectsListView->setItemDelegate( recentProjectsDelegate );
  connect( mRecentProjectsModel, &QAbstractItemModel::rowsRemoved, this, [this]
  {
    updateRecentProjectsVisibility();
  }
         );

  centerLayout->addWidget( mRecentProjectsListView, 1, 0 );

  layout->addWidget( centerContainer );

  QLabel *templatesTitle = new QLabel( QStringLiteral( "<div style='font-size:%1px;font-weight:bold'>%2</div>" ).arg( titleSize ).arg( tr( "Project Templates" ) ) );
  templatesTitle->setContentsMargins( titleSize / 2, titleSize / 6, 0, 0 );
  centerLayout->addWidget( templatesTitle, 0, 1 );

  mTemplateProjectsModel = new QgsTemplateProjectsModel( this );
  mTemplateProjectsListView = new QListView();
  mTemplateProjectsListView->setResizeMode( QListView::Adjust );
  mTemplateProjectsListView->setModel( mTemplateProjectsModel );
  QgsProjectListItemDelegate *templateProjectsDelegate = new QgsProjectListItemDelegate( mTemplateProjectsListView );
  templateProjectsDelegate->setShowPath( false );
  mTemplateProjectsListView->setItemDelegate( templateProjectsDelegate );
  mTemplateProjectsListView->setContextMenuPolicy( Qt::CustomContextMenu );
  connect( mTemplateProjectsListView, &QListView::customContextMenuRequested, this, &QgsWelcomePage::showContextMenuForTemplates );
  centerLayout->addWidget( mTemplateProjectsListView, 1, 1 );

  mVersionInformation = new QTextBrowser;
  mVersionInformation->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Maximum );
  mVersionInformation->setReadOnly( true );
  mVersionInformation->setOpenExternalLinks( true );
  mVersionInformation->setStyleSheet( QStringLiteral( "QTextEdit { background-color: #dff0d8; border: 1px solid #8e998a; padding-top: 0.25em; max-height: 1.75em; min-height: 1.75em; } "
                                      "QScrollBar { background-color: rgba(0,0,0,0); } "
                                      "QScrollBar::add-page,QScrollBar::sub-page,QScrollBar::handle { background-color: rgba(0,0,0,0); color: rgba(0,0,0,0); } "
                                      "QScrollBar::up-arrow,QScrollBar::down-arrow { color: rgb(0,0,0); } " ) );

  mainLayout->addWidget( mVersionInformation );
  mVersionInformation->setVisible( false );

  mVersionInfo = new QgsVersionInfo();
  if ( !QgsApplication::isRunningFromBuildDir() && settings.value( QStringLiteral( "/qgis/allowVersionCheck" ), true ).toBool()
       && settings.value( QStringLiteral( "qgis/checkVersion" ), true ).toBool() && !skipVersionCheck )
  {
    connect( mVersionInfo, &QgsVersionInfo::versionInfoAvailable, this, &QgsWelcomePage::versionInfoReceived );
    mVersionInfo->checkVersion();
  }

  connect( mRecentProjectsListView, &QAbstractItemView::activated, this, &QgsWelcomePage::recentProjectItemActivated );
  connect( mTemplateProjectsListView, &QAbstractItemView::activated, this, &QgsWelcomePage::templateProjectItemActivated );
}

QgsWelcomePage::~QgsWelcomePage()
{
  delete mVersionInfo;
}

void QgsWelcomePage::setRecentProjects( const QList<QgsRecentProjectItemsModel::RecentProjectData> &recentProjects )
{
  mRecentProjectsModel->setRecentProjects( recentProjects );
  updateRecentProjectsVisibility();
}

void QgsWelcomePage::recentProjectItemActivated( const QModelIndex &index )
{
  QgisApp::instance()->openProject( mRecentProjectsModel->data( index, Qt::ToolTipRole ).toString() );
}

void QgsWelcomePage::templateProjectItemActivated( const QModelIndex &index )
{
  if ( index.data( QgsProjectListItemDelegate::NativePathRole ).isNull() )
    QgisApp::instance()->newProject();
  QgisApp::instance()->fileNewFromTemplate( index.data( QgsProjectListItemDelegate::NativePathRole ).toString() );
}

void QgsWelcomePage::versionInfoReceived()
{
  QgsVersionInfo *versionInfo = qobject_cast<QgsVersionInfo *>( sender() );
  Q_ASSERT( versionInfo );

  if ( versionInfo->newVersionAvailable() )
  {
    mVersionInformation->setVisible( true );
    mVersionInformation->setText( QStringLiteral( "<style> a, a:visited, a:hover { color:#268300; } </style><b>%1</b>: %2" )
                                  .arg( tr( "New QGIS version available" ),
                                        QgsStringUtils::insertLinks( versionInfo->downloadInfo() ) ) );
  }
}

void QgsWelcomePage::showContextMenuForProjects( QPoint point )
{
  QModelIndex index = mRecentProjectsListView->indexAt( point );
  if ( !index.isValid() )
    return;

  bool pin = mRecentProjectsModel->data( index, QgsProjectListItemDelegate::PinRole ).toBool();
  QString path = mRecentProjectsModel->data( index, QgsProjectListItemDelegate::PathRole ).toString();
  if ( path.isEmpty() )
    return;

  bool enabled = mRecentProjectsModel->flags( index ) & Qt::ItemIsEnabled;

  QMenu *menu = new QMenu( this );

  if ( enabled )
  {
    if ( !pin )
    {
      QAction *pinAction = new QAction( tr( "Pin to List" ), menu );
      connect( pinAction, &QAction::triggered, this, [this, index]
      {
        mRecentProjectsModel->pinProject( index );
        emit projectPinned( index.row() );
      } );
      menu->addAction( pinAction );
    }
    else
    {
      QAction *pinAction = new QAction( tr( "Unpin from List" ), menu );
      connect( pinAction, &QAction::triggered, this, [this, index]
      {
        mRecentProjectsModel->unpinProject( index );
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
      mRecentProjectsModel->recheckProject( index );
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
    mRecentProjectsModel->removeProject( index );
    emit projectRemoved( index.row() );
  } );
  menu->addAction( removeProjectAction );

  menu->popup( mapToGlobal( point ) );
  connect( menu, &QMenu::aboutToHide, menu, &QMenu::deleteLater );
}

void QgsWelcomePage::showContextMenuForTemplates( QPoint point )
{
  QModelIndex index = mTemplateProjectsListView->indexAt( point );
  if ( !index.isValid() )
    return;

  QFileInfo fileInfo( index.data( QgsProjectListItemDelegate::NativePathRole ).toString() );

  QMenu *menu = new QMenu();

  if ( fileInfo.isWritable() )
  {
    QAction *deleteFileAction = new QAction( tr( "Delete Template…" ), menu );
    connect( deleteFileAction, &QAction::triggered, this, [this, fileInfo, index]
    {
      QMessageBox msgBox( this );
      msgBox.setWindowTitle( tr( "Delete Template" ) );
      msgBox.setText( tr( "Do you want to delete the template %1? This action can not be undone." ).arg( index.data( QgsProjectListItemDelegate::TitleRole ).toString() ) );
      auto deleteButton = msgBox.addButton( tr( "Delete" ), QMessageBox::YesRole );
      msgBox.addButton( QMessageBox::Cancel );
      msgBox.setIcon( QMessageBox::Question );
      msgBox.exec();
      if ( msgBox.clickedButton() == deleteButton )
      {
        QFile file( fileInfo.filePath() );
        file.remove();
      }
    } );
    menu->addAction( deleteFileAction );
  }

  menu->popup( mTemplateProjectsListView->mapToGlobal( point ) );
}

void QgsWelcomePage::updateRecentProjectsVisibility()
{
  bool visible = mRecentProjectsModel->rowCount() > 0;
  mRecentProjectsListView->setVisible( visible );
  mRecentProjectsTitle->setVisible( visible );
}
