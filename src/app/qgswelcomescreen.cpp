/***************************************************************************
                             qgswelcomescreen.cpp
                             -------------------
    begin                : December 2025
    copyright            : (C) 2025 by Mathieu Pellerin
    email                : mathieu at opengis dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgswelcomescreen.h"

#include "qgisapp.h"
#include "qgsapplication.h"
#include "qgspluginmanager.h"
#include "qgssettings.h"
#include "qgssettingsentryimpl.h"
#include "qgssettingstree.h"

#include <QAbstractButton>
#include <QMessageBox>
#include <QQmlContext>
#include <QString>
#include <QTimer>
#include <QUrl>
#include <QVBoxLayout>

#include "moc_qgswelcomescreen.cpp"

using namespace Qt::StringLiterals;

#define FEED_URL "https://feed.qgis.org/"


QgsWelcomeScreenController::QgsWelcomeScreenController( QgsWelcomeScreen *welcomeScreen )
  : QObject( welcomeScreen )
  , mWelcomeScreen( welcomeScreen )
{}

void QgsWelcomeScreenController::openProject( const QString &path )
{
  // QTimer needed to prevent crashes when the Item bound to the calling function is deleted by the ListView
  QTimer::singleShot( 1, this, [path]() { QgisApp::instance()->openProject( path ); } );
}

void QgsWelcomeScreenController::createBlankProject()
{
  // QTimer needed to prevent crashes when the Item bound to the calling function is deleted by the ListView
  QTimer::singleShot( 1, this, []() { QgisApp::instance()->newProject(); } );
}

void QgsWelcomeScreenController::createProjectFromBasemap()
{
  // QTimer needed to prevent crashes when the Item bound to the calling function is deleted by the ListView
  QTimer::singleShot( 1, this, []() { QgisApp::instance()->fileNewWithBasemap(); } );
}

void QgsWelcomeScreenController::createProjectFromTemplate( const QString &path )
{
  // QTimer needed to prevent crashes when the Item bound to the calling function is deleted by the ListView
  QTimer::singleShot( 1, this, [path]() { QgisApp::instance()->fileNewFromTemplate( path ); } );
}

void QgsWelcomeScreenController::clearRecentProjects()
{
  if ( mWelcomeScreen )
  {
    mWelcomeScreen->clearRecentProjects();
  }
}

void QgsWelcomeScreenController::removeTemplateProject( int row )
{
  if ( mWelcomeScreen )
  {
    mWelcomeScreen->removeTemplateProject( row );
  }
}

void QgsWelcomeScreenController::showPluginManager()
{
  QgisApp::instance()->showPluginManager( static_cast<int>( QgsPluginManager::Tabs::UpgradeablePlugins ) );
}

void QgsWelcomeScreenController::openSettings()
{
  QgisApp::instance()->showOptionsDialog();
}

void QgsWelcomeScreenController::importQgisProfile()
{
  QgisApp::instance()->importQgisProfile();
}

void QgsWelcomeScreenController::hideScene()
{
  if ( mWelcomeScreen )
  {
    mWelcomeScreen->hideScene();
  }
}

void QgsWelcomeScreenController::forwardDrop( const QString &text, const QStringList &urls, const QVariantMap &formatsData )
{
  QMimeData mimeData;
  const QStringList formats = formatsData.keys();
  for ( const QString &format : formats )
  {
    mimeData.setData( format, formatsData[format].toByteArray() );
  }

  QList<QUrl> mimeDataUrls;
  for ( const QString &url : urls )
  {
    mimeDataUrls << QUrl( url );
  }
  mimeData.setUrls( mimeDataUrls );
  mimeData.setText( text );

  QDropEvent dropEvent( QPointF( 0, 0 ), Qt::CopyAction, &mimeData, Qt::LeftButton, Qt::NoModifier );
  QgisApp::instance()->dropEvent( &dropEvent );
}


const QgsSettingsEntryBool *QgsWelcomeScreen::settingsCheckVersion
  = new QgsSettingsEntryBool( u"check-version"_s, QgsSettingsTree::sTreeApp, true, u"Whether the welcome screen should check for a newer QGIS version online"_s );

QgsWelcomeScreen::QgsWelcomeScreen( bool skipVersionCheck, QWidget *parent )
  : QQuickWidget( parent )
{
  setAttribute( Qt::WA_AlwaysStackOnTop );
  setAttribute( Qt::WA_TranslucentBackground );
  setClearColor( Qt::transparent );

  mRecentProjectsModel = new QgsRecentProjectItemsModel( this );
  connect( mRecentProjectsModel, &QgsRecentProjectItemsModel::projectPinned, this, &QgsWelcomeScreen::projectPinned );
  connect( mRecentProjectsModel, &QgsRecentProjectItemsModel::projectUnpinned, this, &QgsWelcomeScreen::projectUnpinned );
  connect( mRecentProjectsModel, &QgsRecentProjectItemsModel::projectRemoved, this, &QgsWelcomeScreen::projectRemoved );
  connect( mRecentProjectsModel, &QgsRecentProjectItemsModel::projectsCleared, this, &QgsWelcomeScreen::projectsCleared );

  mTemplateProjectsModel = new QgsTemplateProjectsModel( this );

  mNewsFeedParser = new QgsNewsFeedParser( QUrl( QStringLiteral( FEED_URL ) ), QString(), this );
  mNewsFeedModel = new QgsNewsFeedProxyModel( mNewsFeedParser, this );

  mWelcomeScreenController = new QgsWelcomeScreenController( this );

  rootContext()->setContextProperty( u"recentProjectsModel"_s, mRecentProjectsModel );
  rootContext()->setContextProperty( u"templateProjectsModel"_s, mTemplateProjectsModel );
  rootContext()->setContextProperty( u"newsFeedParser"_s, mNewsFeedParser );
  rootContext()->setContextProperty( u"newsFeedModel"_s, mNewsFeedModel );
  rootContext()->setContextProperty( u"welcomeScreenController"_s, mWelcomeScreenController );

  setResizeMode( QQuickWidget::ResizeMode::SizeRootObjectToView );

  if ( parent )
  {
    parent->installEventFilter( this );
  }

  QgsSettings settings;
  mVersionInfo = new QgsVersionInfo();
  if ( !QgsApplication::isRunningFromBuildDir() && settings.value( u"/qgis/allowVersionCheck"_s, true ).toBool() && settingsCheckVersion->value() && !skipVersionCheck )
  {
    connect( mVersionInfo, &QgsVersionInfo::versionInfoAvailable, this, &QgsWelcomeScreen::versionInfoReceived );
    mVersionInfo->checkVersion();
  }
}

bool QgsWelcomeScreen::eventFilter( QObject *object, QEvent *event )
{
  bool result = QWidget::eventFilter( object, event );

  if ( event->type() == QEvent::Resize )
  {
    if ( isVisible() && object == parent() )
    {
      refreshGeometry();
    }
  }

  return result;
}

void QgsWelcomeScreen::refreshGeometry()
{
  if ( QWidget *parentWidget = qobject_cast<QWidget *>( parent() ) )
  {
    const int adjustedWidth = std::min( mOriginalWidth, parentWidget->width() - 10 );
    const int adjustedHeight = std::min( mOriginalHeight, parentWidget->height() - 60 );
    const int adjustedX = ( parentWidget->width() - adjustedWidth ) / 2;
    const int adjustedY = ( parentWidget->height() - adjustedHeight ) / 2;
    setGeometry( adjustedX, adjustedY, adjustedWidth, adjustedHeight );
  }
}

void QgsWelcomeScreen::showScene()
{
  if ( source().isEmpty() )
  {
    setSource( QUrl( "qrc:/qt/qml/org/qgis/app/qml/WelcomeScreen.qml" ) );
    mOriginalWidth = width();
    mOriginalHeight = height();
  }
  refreshGeometry();
  show();
}

void QgsWelcomeScreen::hideScene()
{
  if ( isVisible() )
  {
    hide();
  }
}

QString QgsWelcomeScreen::newsFeedUrl()
{
  return QStringLiteral( FEED_URL );
}

void QgsWelcomeScreen::registerTypes()
{
  qmlRegisterType<QgsTemplateProjectsModel>( "org.qgis.app", 1, 0, "TemplateProjectsModel" );
  qmlRegisterType<QgsRecentProjectItemsModel>( "org.qgis.app", 1, 0, "RecentProjectItemsModel" );
  qmlRegisterType<QgsNewsFeedModel>( "org.qgis.app", 1, 0, "NewsFeedModel" );
}

void QgsWelcomeScreen::setRecentProjects( const QList<QgsRecentProjectItemsModel::RecentProjectData> &recentProjects )
{
  mRecentProjectsModel->setRecentProjects( recentProjects );
}

QgsRecentProjectItemsModel *QgsWelcomeScreen::recentProjectsModel()
{
  return mRecentProjectsModel;
}

QgsTemplateProjectsModel *QgsWelcomeScreen::templateProjectsModel()
{
  return mTemplateProjectsModel;
}

void QgsWelcomeScreen::clearRecentProjects()
{
  QMessageBox
    messageBox( QMessageBox::Question, tr( "Recent Projects" ), tr( "Are you sure you want to clear the list of recent projects?" ), QMessageBox::No | QMessageBox::Yes | QMessageBox::YesToAll, this );
  messageBox.button( QMessageBox::YesToAll )->setText( tr( "Yes, including pinned projects" ) );
  int answer = messageBox.exec();
  if ( answer != QMessageBox::No )
  {
    const bool clearPinned = ( answer == QMessageBox::YesToAll );
    mRecentProjectsModel->clear( clearPinned );
    emit projectsCleared( clearPinned );
  }
}

void QgsWelcomeScreen::removeTemplateProject( int row )
{
  if ( row < 0 || row >= mTemplateProjectsModel->rowCount() )
  {
    return;
  }

  QStandardItem *templateItem = mTemplateProjectsModel->item( row );
  const QFileInfo fileInfo( templateItem->data( static_cast<int>( QgsTemplateProjectsModel::CustomRole::NativePathRole ) ).toString() );
  if ( fileInfo.isWritable() )
  {
    QMessageBox msgBox;
    msgBox.setWindowTitle( tr( "Delete Template" ) );
    msgBox.setText(
      tr( "Do you want to delete the template %1? This action can not be undone." ).arg( templateItem->data( static_cast<int>( QgsTemplateProjectsModel::CustomRole::TitleRole ) ).toString() )
    );
    auto deleteButton = msgBox.addButton( tr( "Delete" ), QMessageBox::YesRole );
    msgBox.addButton( QMessageBox::Cancel );
    msgBox.setIcon( QMessageBox::Question );
    msgBox.exec();
    if ( msgBox.clickedButton() == deleteButton )
    {
      mTemplateProjectsModel->removeRow( row );
      QFile file( fileInfo.filePath() );
      file.remove();
    }
  }
}

void QgsWelcomeScreen::versionInfoReceived()
{
  if ( !mWelcomeScreenController )
  {
    return;
  }

  QgsVersionInfo *versionInfo = qobject_cast<QgsVersionInfo *>( sender() );
  Q_ASSERT( versionInfo );

  if ( versionInfo->newVersionAvailable() )
  {
    emit mWelcomeScreenController->newVersionAvailable( versionInfo->latestVersion(), versionInfo->releaseUrl() );
  }
}

void QgsWelcomeScreen::pluginUpdatesAvailableReceived( const QStringList &plugins )
{
  if ( !mWelcomeScreenController )
  {
    return;
  }

  emit mWelcomeScreenController->pluginUpdatesAvailable( plugins );
}
