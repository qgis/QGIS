/***************************************************************************
    qgsdevtoolspanelwidget.cpp
    ---------------------
    Date                 : March 2020
    Copyright            : (C) 2020 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsdevtoolspanelwidget.h"
#include "moc_qgsdevtoolspanelwidget.cpp"
#include "qgisapp.h"
#include "qgsdevtoolwidgetfactory.h"
#include "qgsdevtoolwidget.h"
#include "qgspanelwidgetstack.h"
#include "qgssettingsentryimpl.h"
#include "qgsapplication.h"
#include "qgsdockwidget.h"
#include "devtools/documentation/qgsdocumentationpanelwidget.h"

const QgsSettingsEntryString *QgsDevToolsPanelWidget::settingLastActiveTab = new QgsSettingsEntryString( QStringLiteral( "last-active-tab" ), QgsDevToolsPanelWidget::sTreeDevTools, QString(), QStringLiteral( "Last visible tab in developer tools panel" ) );


QgsDevToolsPanelWidget::QgsDevToolsPanelWidget( const QList<QgsDevToolWidgetFactory *> &factories, QWidget *parent )
  : QWidget( parent )
{
  setupUi( this );

  mOptionsListWidget->setIconSize( QgisApp::instance()->iconSize( false ) );
  mOptionsListWidget->setMaximumWidth( static_cast<int>( mOptionsListWidget->iconSize().width() * 1.18 ) );


  // Add embedded documentation
  mDocumentationPanel = new QgsDocumentationPanelWidget( this );
  addToolWidget( mDocumentationPanel );

  for ( QgsDevToolWidgetFactory *factory : factories )
    addToolFactory( factory );

  connect( mOptionsListWidget, &QListWidget::currentRowChanged, this, [=]( int row ) {
    setCurrentTool( row );
    settingLastActiveTab->setValue( mOptionsListWidget->currentItem()->data( Qt::UserRole ).toString() );
  } );
}

QgsDevToolsPanelWidget::~QgsDevToolsPanelWidget() = default;

void QgsDevToolsPanelWidget::addToolWidget( QgsDevToolWidget *widget )
{
  mStackedWidget->addWidget( widget );

  QListWidgetItem *item = new QListWidgetItem( widget->windowIcon(), QString() );
  item->setToolTip( widget->windowTitle() );
  item->setData( Qt::UserRole, widget->objectName() );
  mOptionsListWidget->addItem( item );
  if ( mOptionsListWidget->count() == 1 )
  {
    setCurrentTool( 0 );
  }
}


void QgsDevToolsPanelWidget::addToolFactory( QgsDevToolWidgetFactory *factory )
{
  if ( QgsDevToolWidget *toolWidget = factory->createWidget( this ) )
  {
    QgsPanelWidgetStack *toolStack = new QgsPanelWidgetStack();
    toolStack->setMainPanel( toolWidget );
    mStackedWidget->addWidget( toolStack );

    QListWidgetItem *item = new QListWidgetItem( factory->icon(), QString() );
    item->setToolTip( factory->title() );
    item->setData( Qt::UserRole, factory->title() );

    mOptionsListWidget->addItem( item );
    const int row = mOptionsListWidget->row( item );
    mFactoryPages[factory] = row;

    if ( mOptionsListWidget->count() == 1 )
    {
      setCurrentTool( 0 );
    }
  }
}

void QgsDevToolsPanelWidget::removeToolFactory( QgsDevToolWidgetFactory *factory )
{
  if ( mFactoryPages.contains( factory ) )
  {
    const int currentRow = mStackedWidget->currentIndex();
    const int row = mFactoryPages.value( factory );
    if ( QWidget *widget = mStackedWidget->widget( row ) )
    {
      mStackedWidget->removeWidget( widget );
    }
    mOptionsListWidget->removeItemWidget( mOptionsListWidget->item( row ) );
    mFactoryPages.remove( factory );
    if ( currentRow == row )
      setCurrentTool( 0 );
  }
}

void QgsDevToolsPanelWidget::setActiveTab( const QString &title )
{
  if ( !title.isEmpty() )
  {
    for ( int row = 0; row < mOptionsListWidget->count(); ++row )
    {
      if ( mOptionsListWidget->item( row )->data( Qt::UserRole ).toString() == title )
      {
        setCurrentTool( row );
        break;
      }
    }
  }
}

void QgsDevToolsPanelWidget::setCurrentTool( int row )
{
  whileBlocking( mOptionsListWidget )->setCurrentRow( row );
  mStackedWidget->setCurrentIndex( row );
}

void QgsDevToolsPanelWidget::showApiDocumentation(
  Qgis::DocumentationApi api, Qgis::DocumentationBrowser browser, const QString &object, const QString &module
)
{
  bool useQgisDocDirectory = false;
  QString baseUrl;
  QString version;

  if ( api == Qgis::DocumentationApi::Qt )
  {
    version = QString( qVersion() ).split( '.' ).mid( 0, 2 ).join( '.' );
    baseUrl = QString( "https://doc.qt.io/qt-%1/" ).arg( version );
  }
  else
  {
    if ( Qgis::version().toLower().contains( QStringLiteral( "master" ) ) )
    {
      version = QStringLiteral( "master" );
    }
    else
    {
      version = QString( Qgis::version() ).split( '.' ).mid( 0, 2 ).join( '.' );
    }

    if ( api == Qgis::DocumentationApi::PyQgis || api == Qgis::DocumentationApi::PyQgisSearch )
    {
      QgsSettings settings;
      baseUrl = settings.value( QStringLiteral( "qgis/PyQgisApiUrl" ), QString( "https://qgis.org/pyqgis/%1/" ).arg( version ) ).toString();
    }
    else
    {
      if ( QFileInfo::exists( QgsApplication::pkgDataPath() + "/doc/api/index.html" ) )
      {
        useQgisDocDirectory = true;
        baseUrl = "api/";
      }
      else
      {
        QgsSettings settings;
        baseUrl = settings.value( QStringLiteral( "qgis/QgisApiUrl" ), QString( "https://qgis.org/api/%1/" ).arg( version ) ).toString();
      }
    }
  }


  QString url;
  if ( object.isEmpty() )
  {
    url = baseUrl == "api/" ? baseUrl + "index.html" : baseUrl;
  }
  else
  {
    switch ( api )
    {
      case Qgis::DocumentationApi::PyQgis:
        url = baseUrl + QString( "%1/%2.html" ).arg( module, object );
        break;
      case Qgis::DocumentationApi::PyQgisSearch:
        url = baseUrl + QString( "search.html?q=%2" ).arg( object );
        break;
      case Qgis::DocumentationApi::CppQgis:
        url = baseUrl + QString( "class%1.html" ).arg( object );
        break;
      case Qgis::DocumentationApi::Qt:
        url = baseUrl + QString( "%1.html" ).arg( object.toLower() );
        break;
    }
  }
#ifndef HAVE_WEBENGINE
  // QWebView does not support the search function from the PyQGIS documentation homepage
  if ( api == Qgis::DocumentationApi::PyQgisSearch )
  {
    browser = Qgis::DocumentationBrowser::SystemWebBrowser;
  }
#endif

  switch ( browser )
  {
    case Qgis::DocumentationBrowser::SystemWebBrowser:
      QgisApp::instance()->openURL( url, useQgisDocDirectory );
      break;
    case Qgis::DocumentationBrowser::DeveloperToolsPanel:
      if ( useQgisDocDirectory )
      {
        url = "file://" + QgsApplication::pkgDataPath() + "/doc/" + url;
      }
      if ( QgsDockWidget *dock = QgisApp::instance()->findChild<QgsDockWidget *>( "DevTools" ) )
      {
        dock->setUserVisible( true );
      }
      showUrl( QUrl( url ) );
      break;
  }
}

void QgsDevToolsPanelWidget::showUrl( const QUrl &url )
{
  setActiveTab( mDocumentationPanel->objectName() );
  mDocumentationPanel->showUrl( url );
}
