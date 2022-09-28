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
#include "qgsfocuskeeper.h"
#include "qgssettings.h"
#include "qgsgui.h"
#include "qgsnative.h"
#include "qgsstringutils.h"
#include "qgsfileutils.h"
#include "qgstemplateprojectsmodel.h"
#include "qgsprojectlistitemdelegate.h"
#include "qgsnewsfeedmodel.h"
#include "qgsnewsfeedparser.h"

#include "qgsprojectstorage.h"
#include "qgsprojectstorageguiprovider.h"
#include "qgsprojectstorageguiregistry.h"
#include "qgsprojectstorageregistry.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QListView>
#include <QDesktopServices>
#include <QTextBrowser>
#include <QMessageBox>
#include <QSplitter>
#include <QRegularExpression>
#include <QUrl>

#define FEED_URL "https://feed.qgis.org/"

QgsWelcomePage::QgsWelcomePage( bool skipVersionCheck, QWidget *parent )
  : QWidget( parent )
{
  const QgsSettings settings;

  QVBoxLayout *mainLayout = new QVBoxLayout;
  mainLayout->setContentsMargins( 0, 0, 0, 0 );
  setLayout( mainLayout );

  mSplitter = new QSplitter( Qt::Horizontal );
  mainLayout->addWidget( mSplitter, 1 );

  QWidget *leftContainer = new QWidget();
  QVBoxLayout *leftLayout = new QVBoxLayout;
  leftLayout->setContentsMargins( 0, 0, 0, 0 );

  const int titleSize = static_cast<int>( QApplication::fontMetrics().height() * 1.4 );
  mRecentProjectsTitle = new QLabel( QStringLiteral( "<div style='font-size:%1px;font-weight:bold'>%2</div>" ).arg( QString::number( titleSize ), tr( "Recent Projects" ) ) );
  mRecentProjectsTitle->setContentsMargins( titleSize / 2, titleSize / 6, 0, 0 );
  leftLayout->addWidget( mRecentProjectsTitle, 0 );

  mRecentProjectsListView = new QListView();
  mRecentProjectsListView->setResizeMode( QListView::Adjust );
  mRecentProjectsListView->setContextMenuPolicy( Qt::CustomContextMenu );
  connect( mRecentProjectsListView, &QListView::customContextMenuRequested, this, &QgsWelcomePage::showContextMenuForProjects );
  mRecentProjectsListView->setVerticalScrollMode( QAbstractItemView::ScrollPerPixel );

  mRecentProjectsModel = new QgsRecentProjectItemsModel( mRecentProjectsListView );
  mRecentProjectsListView->setModel( mRecentProjectsModel );
  QgsProjectListItemDelegate *recentProjectsDelegate = new QgsProjectListItemDelegate( mRecentProjectsListView );
  mRecentProjectsListView->setItemDelegate( recentProjectsDelegate );

  leftLayout->addWidget( mRecentProjectsListView, 1 );
  leftContainer->setLayout( leftLayout );
  mSplitter->addWidget( leftContainer );

  QWidget *rightContainer = new QWidget();
  QVBoxLayout *rightLayout = new QVBoxLayout;
  rightLayout->setContentsMargins( 0, 0, 0, 0 );

  if ( !QgsSettings().value( QStringLiteral( "%1/disabled" ).arg( QgsNewsFeedParser::keyForFeed( QStringLiteral( FEED_URL ) ) ), false, QgsSettings::Core ).toBool() )
  {
    mSplitter2 = new QSplitter( Qt::Vertical );
    rightLayout->addWidget( mSplitter2 );
    QWidget *newsContainer = new QWidget();
    QVBoxLayout *newsLayout = new QVBoxLayout();
    newsLayout->setContentsMargins( 0, 0, 0, 0 );
    mNewsFeedTitle = new QLabel( QStringLiteral( "<div style='font-size:%1px;font-weight:bold'>%2</div>" ).arg( titleSize ).arg( tr( "News" ) ) );
    mNewsFeedTitle->setContentsMargins( titleSize / 2, titleSize / 6, 0, 0 );
    newsLayout->addWidget( mNewsFeedTitle, 0 );

    mNewsFeedParser = new QgsNewsFeedParser( QUrl( QStringLiteral( FEED_URL ) ), QString(), this );
    mNewsFeedModel = new QgsNewsFeedProxyModel( mNewsFeedParser, this );
    mNewsFeedListView = new QListView();
    mNewsFeedListView->setResizeMode( QListView::Adjust );
    mNewsFeedListView->setModel( mNewsFeedModel );
    mNewsFeedListView->setVerticalScrollMode( QAbstractItemView::ScrollPerPixel );
    mNewsDelegate = new QgsNewsItemListItemDelegate( mNewsFeedListView );
    mNewsFeedListView->setItemDelegate( mNewsDelegate );
    mNewsFeedListView->setContextMenuPolicy( Qt::CustomContextMenu );
    mNewsFeedListView->viewport()->installEventFilter( this );
    connect( mNewsFeedListView, &QAbstractItemView::activated, this, &QgsWelcomePage::newsItemActivated );
    connect( mNewsFeedListView, &QListView::customContextMenuRequested, this, &QgsWelcomePage::showContextMenuForNews );
    connect( mNewsFeedParser, &QgsNewsFeedParser::entryDismissed, this, &QgsWelcomePage::updateNewsFeedVisibility );
    newsLayout->addWidget( mNewsFeedListView, 1 );
    connect( mNewsFeedParser, &QgsNewsFeedParser::fetched, this, &QgsWelcomePage::updateNewsFeedVisibility );
    mNewsFeedParser->fetch();
    newsContainer->setLayout( newsLayout );
    mSplitter2->addWidget( newsContainer );
  }

  QWidget *templateContainer = new QWidget();
  QVBoxLayout *templateLayout = new QVBoxLayout();
  templateLayout->setContentsMargins( 0, 0, 0, 0 );
  QLabel *templatesTitle = new QLabel( QStringLiteral( "<div style='font-size:%1px;font-weight:bold'>%2</div>" ).arg( titleSize ).arg( tr( "Project Templates" ) ) );
  templatesTitle->setContentsMargins( titleSize / 2, titleSize / 6, 0, 0 );
  templateLayout->addWidget( templatesTitle, 0 );

  mTemplateProjectsModel = new QgsTemplateProjectsModel( this );
  mTemplateProjectsListView = new QListView();
  mTemplateProjectsListView->setResizeMode( QListView::Adjust );
  mTemplateProjectsListView->setModel( mTemplateProjectsModel );
  mTemplateProjectsListView->setVerticalScrollMode( QAbstractItemView::ScrollPerPixel );
  QgsProjectListItemDelegate *templateProjectsDelegate = new QgsProjectListItemDelegate( mTemplateProjectsListView );
  templateProjectsDelegate->setShowPath( false );
  mTemplateProjectsListView->setItemDelegate( templateProjectsDelegate );
  mTemplateProjectsListView->setContextMenuPolicy( Qt::CustomContextMenu );
  connect( mTemplateProjectsListView, &QListView::customContextMenuRequested, this, &QgsWelcomePage::showContextMenuForTemplates );
  templateLayout->addWidget( mTemplateProjectsListView, 1 );
  templateContainer->setLayout( templateLayout );
  if ( mSplitter2 )
  {
    mSplitter2->addWidget( templateContainer );
  }
  else
  {
    rightLayout->addWidget( templateContainer );
  }

  rightContainer->setLayout( rightLayout );
  mSplitter->addWidget( rightContainer );
  mSplitter->setStretchFactor( 0, 4 );
  mSplitter->setStretchFactor( 1, 6 );

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

  mSplitter->restoreState( settings.value( QStringLiteral( "Windows/WelcomePage/SplitState" ), QVariant(), QgsSettings::App ).toByteArray() );
  if ( mSplitter2 )
    mSplitter2->restoreState( settings.value( QStringLiteral( "Windows/WelcomePage/SplitState2" ), QVariant(), QgsSettings::App ).toByteArray() );

  connect( mRecentProjectsListView, &QAbstractItemView::activated, this, &QgsWelcomePage::recentProjectItemActivated );
  connect( mTemplateProjectsListView, &QAbstractItemView::activated, this, &QgsWelcomePage::templateProjectItemActivated );

  updateNewsFeedVisibility();
}

QgsWelcomePage::~QgsWelcomePage()
{
  QgsSettings settings;
  settings.setValue( QStringLiteral( "Windows/WelcomePage/SplitState" ), mSplitter->saveState(), QgsSettings::App );
  if ( mSplitter2 && mNewsFeedTitle->isVisible() )
    settings.setValue( QStringLiteral( "Windows/WelcomePage/SplitState2" ), mSplitter2->saveState(), QgsSettings::App );

  delete mVersionInfo;
}

void QgsWelcomePage::setRecentProjects( const QList<QgsRecentProjectItemsModel::RecentProjectData> &recentProjects )
{
  mRecentProjectsModel->setRecentProjects( recentProjects );
}

QString QgsWelcomePage::newsFeedUrl()
{
  return QStringLiteral( FEED_URL );
}

void QgsWelcomePage::recentProjectItemActivated( const QModelIndex &index )
{
  QgisApp::instance()->openProject( mRecentProjectsModel->data( index, Qt::ToolTipRole ).toString() );
}

void QgsWelcomePage::templateProjectItemActivated( const QModelIndex &index )
{
  if ( !index.data( QgsProjectListItemDelegate::NativePathRole ).isValid() )
    QgisApp::instance()->newProject();
  else
    QgisApp::instance()->fileNewFromTemplate( index.data( QgsProjectListItemDelegate::NativePathRole ).toString() );
}

void QgsWelcomePage::newsItemActivated( const QModelIndex &index )
{
  if ( !index.isValid() )
    return;

  const QUrl link = index.data( QgsNewsFeedModel::Link ).toUrl();
  QDesktopServices::openUrl( link );
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
  const QModelIndex index = mRecentProjectsListView->indexAt( point );
  if ( !index.isValid() )
    return;

  const bool pin = mRecentProjectsModel->data( index, QgsProjectListItemDelegate::PinRole ).toBool();
  QString path = mRecentProjectsModel->data( index, QgsProjectListItemDelegate::PathRole ).toString();
  if ( path.isEmpty() )
    return;

  QgsProjectStorage *storage = QgsApplication::projectStorageRegistry()->projectStorageFromUri( path );
  const bool enabled = mRecentProjectsModel->flags( index ) & Qt::ItemIsEnabled;

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

    if ( storage )
    {
      path = storage->filePath( path );
    }

    if ( !path.isEmpty() )
    {
      QAction *openFolderAction = new QAction( tr( "Open Directory…" ), menu );
      connect( openFolderAction, &QAction::triggered, this, [path]
      {
        const QgsFocusKeeper focusKeeper;
        QgsGui::nativePlatformInterface()->openFileExplorerAndSelectFile( path );
      } );
      menu->addAction( openFolderAction );
    }
  }
  else
  {
    QAction *rescanAction = new QAction( tr( "Refresh" ), menu );
    connect( rescanAction, &QAction::triggered, this, [this, index]
    {
      mRecentProjectsModel->recheckProject( index );
    } );
    menu->addAction( rescanAction );

    bool showClosestPath = storage ? false : true;
    if ( storage && ( storage->type() == QLatin1String( "geopackage" ) ) )
    {
      const QRegularExpression reGpkg( "^(geopackage:)([^\?]+)\?(.+)$", QRegularExpression::CaseInsensitiveOption );
      const QRegularExpressionMatch matchGpkg = reGpkg.match( path );
      if ( matchGpkg.hasMatch() )
      {
        path = matchGpkg.captured( 2 );
        showClosestPath = true;
      }
    }

    if ( showClosestPath )
    {
      // add an entry to open the closest existing path to the original project file or geopackage location
      // to help users re-find moved/renamed projects!
      const QString closestPath = QgsFileUtils::findClosestExistingPath( path );
      QAction *openFolderAction = new QAction( tr( "Open “%1”…" ).arg( QDir::toNativeSeparators( closestPath ) ), menu );
      connect( openFolderAction, &QAction::triggered, this, [closestPath]
      {
        QDesktopServices::openUrl( QUrl::fromLocalFile( closestPath ) );
      } );
      menu->addAction( openFolderAction );
    }
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
  const QModelIndex index = mTemplateProjectsListView->indexAt( point );
  if ( !index.isValid() )
    return;

  const QFileInfo fileInfo( index.data( QgsProjectListItemDelegate::NativePathRole ).toString() );

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

void QgsWelcomePage::showContextMenuForNews( QPoint point )
{
  const QModelIndex index = mNewsFeedListView->indexAt( point );
  if ( !index.isValid() )
    return;

  const int key = index.data( QgsNewsFeedModel::Key ).toInt();

  QMenu *menu = new QMenu();

  QAction *dismissAction = new QAction( tr( "Dismiss" ), menu );
  connect( dismissAction, &QAction::triggered, this, [this, key]
  {
    mNewsFeedParser->dismissEntry( key );
  } );
  menu->addAction( dismissAction );
  QAction *dismissAllAction = new QAction( tr( "Dismiss All" ), menu );
  connect( dismissAllAction, &QAction::triggered, this, [this]
  {
    mNewsFeedParser->dismissAll();
  } );
  menu->addAction( dismissAllAction );
  menu->addSeparator();
  QAction *hideAction = new QAction( tr( "Hide QGIS News…" ), menu );
  connect( hideAction, &QAction::triggered, this, [this]
  {
    if ( QMessageBox::question( this,  tr( "QGIS News" ), tr( "Are you sure you want to hide QGIS news? (The news feed can be re-enabled from the QGIS settings dialog.)" ) ) == QMessageBox::Yes )
    {
      //...sad trombone...
      mNewsFeedParser->dismissAll();
      QgsSettings().setValue( QStringLiteral( "%1/disabled" ).arg( QgsNewsFeedParser::keyForFeed( QStringLiteral( FEED_URL ) ) ), true, QgsSettings::Core );
    }
  } );
  menu->addAction( hideAction );

  menu->popup( mNewsFeedListView->mapToGlobal( point ) );
}

void QgsWelcomePage::updateNewsFeedVisibility()
{
  if ( !mNewsFeedModel || !mNewsFeedListView || !mSplitter2 )
    return;

  const bool visible = mNewsFeedModel->rowCount() > 0;
  mNewsFeedListView->setVisible( visible );
  mNewsFeedTitle->setVisible( visible );
  if ( !visible )
  {
    mSplitter2->setSizes( QList<int>() << 0 << 99999999 );
  }
  else
  {
    mSplitter2->restoreState( QgsSettings().value( QStringLiteral( "Windows/WelcomePage/SplitState2" ), QVariant(), QgsSettings::App ).toByteArray() );
    if ( mSplitter2->sizes().first() == 0 )
    {
      const int splitSize = mSplitter2->height() / 2;
      mSplitter2->setSizes( QList< int > { splitSize, splitSize} );
    }
  }
}

bool QgsWelcomePage::eventFilter( QObject *obj, QEvent *event )
{
  if ( obj == mNewsFeedListView->viewport() && event->type() == QEvent::MouseButtonRelease )
  {
    QMouseEvent *mouseEvent = dynamic_cast< QMouseEvent *>( event );
    if ( mouseEvent->button() == Qt::LeftButton )
    {
      const QModelIndex index = mNewsFeedListView->indexAt( mouseEvent->pos() );
      if ( index.isValid() )
      {
        const QPoint itemClickPoint = mouseEvent->pos() - mNewsFeedListView->visualRect( index ).topLeft();
        if ( QRect( mNewsDelegate->dismissRect().left(), mNewsDelegate->dismissRect().top(), mNewsDelegate->dismissRectSize().width(), mNewsDelegate->dismissRectSize().height() ).contains( itemClickPoint ) )
        {
          mNewsFeedParser->dismissEntry( index.data( QgsNewsFeedModel::Key ).toInt() );
        }
        return true;
      }
    }
  }

  return QWidget::eventFilter( obj, event );
}

