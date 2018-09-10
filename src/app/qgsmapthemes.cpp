/***************************************************************************
  qgsmapthemes.cpp
  --------------------------------------
  Date                 : September 2014
  Copyright            : (C) 2014 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmapthemes.h"
#include "qgsmapthemecollection.h"

#include "qgslayertree.h"
#include "qgslayertreemapcanvasbridge.h"
#include "qgslayertreemodel.h"
#include "qgslayertreemodellegendnode.h"
#include "qgslayertreeview.h"
#include "qgsmaplayerstylemanager.h"
#include "qgsproject.h"
#include "qgsrenderer.h"
#include "qgsvectorlayer.h"
#include "qgisapp.h"
#include "qgsnewnamedialog.h"

#include <QInputDialog>
#include <QMessageBox>

QgsMapThemes *QgsMapThemes::sInstance;


QgsMapThemes::QgsMapThemes()
  : mMenu( new QMenu )
{

  mMenu->addAction( QgisApp::instance()->actionShowAllLayers() );
  mMenu->addAction( QgisApp::instance()->actionHideAllLayers() );
  mMenu->addAction( QgisApp::instance()->actionShowSelectedLayers() );
  mMenu->addAction( QgisApp::instance()->actionHideSelectedLayers() );
  mMenu->addAction( QgisApp::instance()->actionHideDeselectedLayers() );
  mMenu->addSeparator();

  mReplaceMenu = new QMenu( tr( "Replace Theme" ) );
  mMenu->addMenu( mReplaceMenu );
  mActionAddPreset = mMenu->addAction( tr( "Add Theme…" ), this, SLOT( addPreset() ) );
  mMenuSeparator = mMenu->addSeparator();

  mActionRemoveCurrentPreset = mMenu->addAction( tr( "Remove Current Theme" ), this, SLOT( removeCurrentPreset() ) );

  connect( mMenu, &QMenu::aboutToShow, this, &QgsMapThemes::menuAboutToShow );
}


QgsMapThemeCollection::MapThemeRecord QgsMapThemes::currentState()
{
  QgsLayerTreeGroup *root = QgsProject::instance()->layerTreeRoot();
  QgsLayerTreeModel *model = QgisApp::instance()->layerTreeView()->layerTreeModel();
  return QgsMapThemeCollection::createThemeFromCurrentState( root, model );
}

QgsMapThemes *QgsMapThemes::instance()
{
  if ( !sInstance )
    sInstance = new QgsMapThemes();

  return sInstance;
}

void QgsMapThemes::addPreset( const QString &name )
{
  QgsProject::instance()->mapThemeCollection()->insert( name, currentState() );
}

void QgsMapThemes::updatePreset( const QString &name )
{
  QgsProject::instance()->mapThemeCollection()->update( name, currentState() );
}

QList<QgsMapLayer *> QgsMapThemes::orderedPresetVisibleLayers( const QString &name ) const
{
  QStringList visibleIds = QgsProject::instance()->mapThemeCollection()->mapThemeVisibleLayerIds( name );

  // also make sure to order the layers according to map canvas order
  QList<QgsMapLayer *> lst;
  Q_FOREACH ( QgsMapLayer *layer, QgsProject::instance()->layerTreeRoot()->layerOrder() )
  {
    if ( visibleIds.contains( layer->id() ) )
    {
      lst << layer;
    }
  }
  return lst;
}

QMenu *QgsMapThemes::menu()
{
  return mMenu;
}


void QgsMapThemes::addPreset()
{
  QStringList existingNames = QgsProject::instance()->mapThemeCollection()->mapThemes();
  QgsNewNameDialog dlg( tr( "theme" ), tr( "Theme" ), QStringList(), existingNames, QRegExp(), Qt::CaseInsensitive, mMenu );
  dlg.setWindowTitle( tr( "Map Themes" ) );
  dlg.setHintString( tr( "Name of the new theme" ) );
  dlg.setOverwriteEnabled( false );
  dlg.setConflictingNameWarning( tr( "A theme with this name already exists." ) );
  if ( dlg.exec() != QDialog::Accepted || dlg.name().isEmpty() )
    return;

  addPreset( dlg.name() );
}


void QgsMapThemes::presetTriggered()
{
  QAction *actionPreset = qobject_cast<QAction *>( sender() );
  if ( !actionPreset )
    return;

  applyState( actionPreset->text() );
}

void QgsMapThemes::replaceTriggered()
{
  QAction *actionPreset = qobject_cast<QAction *>( sender() );
  if ( !actionPreset )
    return;

  int res = QMessageBox::question( QgisApp::instance(), tr( "Replace Theme" ),
                                   tr( "Are you sure you want to replace the existing theme “%1”?" ).arg( actionPreset->text() ),
                                   QMessageBox::Yes | QMessageBox::No, QMessageBox::No );
  if ( res != QMessageBox::Yes )
    return;

  //adding preset with same name is effectively a replace
  addPreset( actionPreset->text() );
}


void QgsMapThemes::applyState( const QString &presetName )
{
  if ( !QgsProject::instance()->mapThemeCollection()->hasMapTheme( presetName ) )
    return;

  QgsLayerTreeGroup *root = QgsProject::instance()->layerTreeRoot();
  QgsLayerTreeModel *model = QgisApp::instance()->layerTreeView()->layerTreeModel();
  QgsProject::instance()->mapThemeCollection()->applyTheme( presetName, root, model );
}

void QgsMapThemes::removeCurrentPreset()
{
  for ( QAction *actionPreset : qgis::as_const( mMenuPresetActions ) )
  {
    if ( actionPreset->isChecked() )
    {
      int res = QMessageBox::question( QgisApp::instance(), tr( "Remove Theme" ),
                                       tr( "Are you sure you want to remove the existing theme “%1”?" ).arg( actionPreset->text() ),
                                       QMessageBox::Yes | QMessageBox::No, QMessageBox::No );
      if ( res == QMessageBox::Yes )
        QgsProject::instance()->mapThemeCollection()->removeMapTheme( actionPreset->text() );
      break;
    }
  }
}

void QgsMapThemes::menuAboutToShow()
{
  qDeleteAll( mMenuPresetActions );
  mMenuPresetActions.clear();
  mReplaceMenu->clear();
  qDeleteAll( mMenuReplaceActions );
  mMenuReplaceActions.clear();

  QgsMapThemeCollection::MapThemeRecord rec = currentState();
  bool hasCurrent = false;

  Q_FOREACH ( const QString &grpName, QgsProject::instance()->mapThemeCollection()->mapThemes() )
  {
    QAction *a = new QAction( grpName, mMenu );
    a->setCheckable( true );
    if ( !hasCurrent && rec == QgsProject::instance()->mapThemeCollection()->mapThemeState( grpName ) )
    {
      a->setChecked( true );
      hasCurrent = true;
    }
    connect( a, &QAction::triggered, this, &QgsMapThemes::presetTriggered );
    mMenuPresetActions.append( a );

    QAction *replaceAction = new QAction( grpName, mReplaceMenu );
    connect( replaceAction, &QAction::triggered, this, &QgsMapThemes::replaceTriggered );
    mReplaceMenu->addAction( replaceAction );
  }
  mMenu->insertActions( mMenuSeparator, mMenuPresetActions );
  mReplaceMenu->addActions( mMenuReplaceActions );

  mActionAddPreset->setEnabled( !hasCurrent );
  mActionRemoveCurrentPreset->setEnabled( hasCurrent );
}
