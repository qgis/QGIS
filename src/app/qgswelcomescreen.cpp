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

#include <QAbstractButton>
#include <QMessageBox>
#include <QQmlContext>
#include <QUrl>
#include <QVBoxLayout>

#define FEED_URL "https://feed.qgis.org/"


QgsWelcomeScreenController::QgsWelcomeScreenController( QgsWelcomeScreen *welcomeScreen )
  : QObject( welcomeScreen )
  , mWelcomeScreen( welcomeScreen )
{
}

void QgsWelcomeScreenController::openProject( const QString &path )
{
  QgisApp::instance()->openProject( path );
}

void QgsWelcomeScreenController::createProjectFromTemplate( const QString &path )
{
  if ( !path.isEmpty() )
  {
    QgisApp::instance()->fileNewFromTemplate( path );
  }
  else
  {
    QgisApp::instance()->newProject();
  }
}

void QgsWelcomeScreenController::clearRecentProjects()
{
  if ( mWelcomeScreen )
  {
    mWelcomeScreen->clearRecentProjects();
  }
}


QgsWelcomeScreen::QgsWelcomeScreen( QWidget *parent )
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

  rootContext()->setContextProperty( QStringLiteral( "recentProjectsModel" ), mRecentProjectsModel );
  rootContext()->setContextProperty( QStringLiteral( "templateProjectsModel" ), mTemplateProjectsModel );
  rootContext()->setContextProperty( QStringLiteral( "newsFeedParser" ), mNewsFeedParser );
  rootContext()->setContextProperty( QStringLiteral( "newsFeedModel" ), mNewsFeedModel );
  rootContext()->setContextProperty( QStringLiteral( "welcomeScreenController" ), mWelcomeScreenController );

  setResizeMode( QQuickWidget::ResizeMode::SizeRootObjectToView );
  setSource( QUrl( "qrc:/qt/qml/org/qgis/app/qml/WelcomeScreen.qml" ) );

  if ( parent )
  {
    parent->installEventFilter( this );
  }
}

bool QgsWelcomeScreen::eventFilter( QObject *object, QEvent *event )
{
  bool result = QWidget::eventFilter( object, event );

  if ( object == parent() )
  {
    refreshGeometry();
  }

  return result;
}

void QgsWelcomeScreen::refreshGeometry()
{
  if ( QWidget *parentWidget = qobject_cast<QWidget *>( parent() ) )
  {
    setGeometry( 20, 20, parentWidget->width() - 40, parentWidget->height() - 40 );
  }
}

QString QgsWelcomeScreen::newsFeedUrl()
{
  return QStringLiteral( FEED_URL );
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
  QMessageBox messageBox( QMessageBox::Question, tr( "Recent Projects" ), tr( "Are you sure you want to clear the list of recent projects?" ), QMessageBox::No | QMessageBox::Yes | QMessageBox::YesToAll, this );
  messageBox.button( QMessageBox::YesToAll )->setText( tr( "Yes, including pinned projects" ) );
  int answer = messageBox.exec();
  if ( answer != QMessageBox::No )
  {
    const bool clearPinned = ( answer == QMessageBox::YesToAll );
    mRecentProjectsModel->clear( clearPinned );
    emit projectsCleared( clearPinned );
  }
}
