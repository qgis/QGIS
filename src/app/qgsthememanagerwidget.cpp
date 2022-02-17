/***************************************************************************
  qgsthememanagerwidget.cpp
  --------------------------------------
  Date                 : April 2021
  Copyright            : (C) 2021 by Alex RL
  Email                : ping me on github
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsthememanagerwidget.h"
#include "qgsdockwidget.h"
#include "qgsproject.h"
#include "qgsmaplayer.h"
#include "qgsmapthemecollection.h"
#include "qgslayertreemodel.h"
#include "qgslayertreeview.h"
#include "qgslayertreenode.h"
#include "qgslayertreegroup.h"
#include "qgsthemeviewer.h"
#include "qgsmapthemes.h"
#include <QMessageBox>
#include <QWidget>
#include <QToolButton>
#include <QComboBox>
#include <QMenu>
#include <QAction>
#include "qgisapp.h"

QgsThemeManagerWidget::QgsThemeManagerWidget( QWidget *parent )
  : QgsDockWidget( parent )
{
  setupUi( this );
  connect( this, &QgsDockWidget::opened, this, &QgsThemeManagerWidget::showWidget );
  connect( QgsProject::instance(), &QgsProject::readProject, this, &QgsThemeManagerWidget::projectLoaded );
  connect( mThemePrev, &QToolButton::clicked, this, &QgsThemeManagerWidget::previousTheme );
  connect( mThemeNext, &QToolButton::clicked, this, &QgsThemeManagerWidget::nextTheme );
  connect( mCreateTheme, &QToolButton::clicked, this, &QgsThemeManagerWidget::createTheme );
  connect( mDeleteTheme, &QToolButton::clicked, this, &QgsThemeManagerWidget::removeTheme );
  connect( mAddThemeLayer, &QToolButton::clicked, this, &QgsThemeManagerWidget::addSelectedLayers );
  connect( mRemoveThemeLayer, &QToolButton::clicked, this, &QgsThemeManagerWidget::removeSelectedLayers );
  connect( mThemeList, qOverload<int>( &QComboBox::activated ), [ = ]( int index ) { setTheme( index ); } );
  connect( mThemeViewer, &QgsThemeViewer::layersAdded, this, &QgsThemeManagerWidget::addSelectedLayers );
  connect( mThemeViewer, &QgsThemeViewer::showMenu, this, &QgsThemeManagerWidget::showContextMenu );
  //connect( mThemeViewer, &QgsThemeViewer::layersDropped, this, &QgsThemeManagerWidget::removeSelectedLayers );
  connect( this, &QgsThemeManagerWidget::droppedLayers, this, &QgsThemeManagerWidget::removeSelectedLayers );
  connect( this, &QgsThemeManagerWidget::addLayerTreeLayers, this, &QgsThemeManagerWidget::addSelectedLayers );
}

void QgsThemeManagerWidget::projectLoaded()
{
  mThemeCollection = QgsProject::instance()->mapThemeCollection();
  connect( mThemeCollection, &QgsMapThemeCollection::mapThemesChanged, this, &QgsThemeManagerWidget::populateCombo, Qt::UniqueConnection );
  QgsLayerTreeModel *mModel = QgisApp::instance()->layerTreeView()->layerTreeModel();
  if ( mThemeViewer && mModel )
  {
    mThemeViewer->setModel( mModel );
    mThemeViewer->disconnectProxyModel();
    mThemeViewer->showAllNodes( mShowAllLayers );
  }
  if ( mThemeCollection && mThemeCollection->mapThemes().length() > 0 )
  {
    if ( !mThemeCollection->hasMapTheme( mCurrentTheme ) )
    {
      mCurrentTheme = mThemeCollection->mapThemes()[0];
      mThemeList->setCurrentText( mCurrentTheme );
    }
    populateCombo();
    viewCurrentTheme();
  }
}

void QgsThemeManagerWidget::showWidget()
{
  if ( mCurrentTheme.isEmpty() )
    projectLoaded();
}

void QgsThemeManagerWidget::setTheme( const int index )
{
  QString themename;
  if ( index > -1 && mThemeCollection->mapThemes().size() > index )
  {
    QString themename = mThemeCollection->mapThemes().at( index );
    if ( !mThemeCollection->hasMapTheme( themename ) )
      return;
    mCurrentTheme = themename;
    emit updateComboBox();
  }
}

void QgsThemeManagerWidget::createTheme()
{
  QgsMapThemes::instance()->addPreset();
  emit updateComboBox();
}

void QgsThemeManagerWidget::removeTheme()
{
  int res = QMessageBox::question( QgisApp::instance(), tr( "Remove Theme" ),
                                   tr( "Are you sure you want to remove the existing theme “%1”?" ).arg( mCurrentTheme ),
                                   QMessageBox::Yes | QMessageBox::No, QMessageBox::No );  if ( res == QMessageBox::Yes )
  {
    mThemeCollection->removeMapTheme( mCurrentTheme );
    mCurrentTheme = mThemeCollection->mapThemes()[0];
    emit updateComboBox();
  }

}

void QgsThemeManagerWidget::previousTheme()
{
  QStringList themes = mThemeCollection->mapThemes();
  int idx = themes.indexOf( mCurrentTheme );
  if ( idx > 0 )
  {
    mCurrentTheme = themes.at( idx - 1 );
    emit updateComboBox();
  }
}

void QgsThemeManagerWidget::nextTheme()
{
  QStringList themes = mThemeCollection->mapThemes();
  int idx = 1 + themes.indexOf( mCurrentTheme );
  if ( idx < themes.size() )
  {
    mCurrentTheme = themes.at( idx );
    emit updateComboBox();
  }
}

void QgsThemeManagerWidget::populateCombo()
{
  mThemeList->clear();
  QStringList themes = QgsProject::instance()->mapThemeCollection()->mapThemes();
  if ( !themes.isEmpty() )
  {
    mThemeList->addItems( themes );
    if ( themes.contains( mCurrentTheme ) )
      mThemeList->setCurrentText( mCurrentTheme );
  }

}

void QgsThemeManagerWidget::updateComboBox()
{
  mThemeList->setCurrentText( mCurrentTheme );
  viewCurrentTheme();
}

void QgsThemeManagerWidget::viewCurrentTheme() const
{
  if ( !mThemeCollection->hasMapTheme( mCurrentTheme ) )
    return;
  QStringList themeIds;
  const QList<QgsMapLayer *> constMapThemeVisibleLayers = mThemeCollection->mapThemeVisibleLayers( mCurrentTheme );
  // for symbol legend nodes support, maybe look into ./src/core/qgsmapthemecollection.cpp#L47-L65
  for ( QgsMapLayer *layer : constMapThemeVisibleLayers )
  {
    if ( layer->isValid() && layer->isSpatial() )
    {
      themeIds << layer->id();
    }
  }
  mThemeViewer->proxyModel()->setApprovedIds( themeIds );
}


void QgsThemeManagerWidget::appendLayers( const QList<QgsMapLayer *> &layers )
{
  if ( !mThemeCollection->hasMapTheme( mCurrentTheme ) )
    return;

  QgsMapThemeCollection::MapThemeRecord theme = mThemeCollection->mapThemeState( mCurrentTheme );
  for ( QgsMapLayer *layer : std::as_const( layers ) )
  {
    QgsMapThemeCollection::MapThemeLayerRecord newRecord( layer );
    theme.addLayerRecord( newRecord );
  }
  mThemeCollection->update( mCurrentTheme, theme );
  viewCurrentTheme();
}

void QgsThemeManagerWidget::addSelectedLayers()
{
  const QList<QgsMapLayer *> &selectedLayers = QgisApp::instance()->layerTreeView()->selectedLayers();
  appendLayers( selectedLayers );
}

void QgsThemeManagerWidget::removeSelectedLayers()
{
  const QList<QgsMapLayer *> &selectedLayers = mThemeViewer->selectedLayers();
  removeThemeLayers( selectedLayers );
}

void QgsThemeManagerWidget::removeThemeLayers( const QList<QgsMapLayer *> &layers )
{
  if ( !mThemeCollection->hasMapTheme( mCurrentTheme ) )
    return;
  QgsMapThemeCollection::MapThemeRecord theme = mThemeCollection->mapThemeState( mCurrentTheme );
  for ( QgsMapLayer *layer : std::as_const( layers ) )
  {
    theme.removeLayerRecord( layer );
  }
  mThemeCollection->update( mCurrentTheme, theme );
  viewCurrentTheme();
}

void QgsThemeManagerWidget::changeVisibility()
{
  mShowAllLayers = !mShowAllLayers;
  if ( mThemeViewer )
    mThemeViewer->showAllNodes( mShowAllLayers );
}

void QgsThemeManagerWidget::showContextMenu( const QPoint &pos )
{
  QMenu *menu = new QMenu();
  QAction *hideToggle = new QAction( tr( "Display layers only" ), this );
  hideToggle->setCheckable( true );
  hideToggle->setChecked( !mShowAllLayers );
  menu->addAction( hideToggle );
  connect( hideToggle, &QAction::triggered, this, [ = ]() { changeVisibility(); } );
  menu->exec( mapToGlobal( pos ) );
  delete menu;
}



