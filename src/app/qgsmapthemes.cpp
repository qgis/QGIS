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
#include "moc_qgsmapthemes.cpp"
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
#include "qgshelp.h"

#include <QMessageBox>

QgsMapThemes *QgsMapThemes::sInstance;


QgsMapThemes::QgsMapThemes()
  : mMenu( new QMenu )
{
  mMenu->addAction( QgisApp::instance()->actionShowAllLayers() );
  mMenu->addAction( QgisApp::instance()->actionHideAllLayers() );
  mMenu->addAction( QgisApp::instance()->actionShowSelectedLayers() );
  mMenu->addAction( QgisApp::instance()->actionHideSelectedLayers() );
  mMenu->addAction( QgisApp::instance()->actionToggleSelectedLayers() );
  mMenu->addAction( QgisApp::instance()->actionToggleSelectedLayersIndependently() );
  mMenu->addAction( QgisApp::instance()->actionHideDeselectedLayers() );
  mMenu->addSeparator();

  mReplaceMenu = new QMenu( tr( "Replace Theme" ) );
  mMenu->addMenu( mReplaceMenu );
  mActionRenameCurrentPreset = mMenu->addAction( tr( "Rename Current Theme…" ), this, &QgsMapThemes::renameCurrentPreset );
  mActionAddPreset = mMenu->addAction( tr( "Add Theme…" ), this, [=] { addPreset(); } );
  mMenuSeparator = mMenu->addSeparator();

  mActionRemoveCurrentPreset = mMenu->addAction( tr( "Remove Current Theme" ), this, &QgsMapThemes::removeCurrentPreset );

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

QMenu *QgsMapThemes::menu()
{
  return mMenu;
}


void QgsMapThemes::addPreset()
{
  QStringList existingNames = QgsProject::instance()->mapThemeCollection()->mapThemes();
  QgsNewNameDialog dlg( tr( "theme" ), tr( "Theme" ), QStringList(), existingNames, Qt::CaseInsensitive, mMenu );
  dlg.setWindowTitle( tr( "Map Themes" ) );
  dlg.setHintString( tr( "Name of the new theme" ) );
  dlg.setOverwriteEnabled( false );
  dlg.setConflictingNameWarning( tr( "A theme with this name already exists." ) );
  dlg.buttonBox()->addButton( QDialogButtonBox::Help );
  connect( dlg.buttonBox(), &QDialogButtonBox::helpRequested, this, &QgsMapThemes::showHelp );
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

  int res = QMessageBox::question( QgisApp::instance(), tr( "Replace Theme" ), tr( "Are you sure you want to replace the existing theme “%1”?" ).arg( actionPreset->text() ), QMessageBox::Yes | QMessageBox::No, QMessageBox::No );
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

void QgsMapThemes::renameCurrentPreset()
{
  QgsMapThemeCollection::MapThemeRecord mapTheme = currentState();
  QStringList existingNames = QgsProject::instance()->mapThemeCollection()->mapThemes();

  for ( QAction *actionPreset : std::as_const( mMenuPresetActions ) )
  {
    if ( actionPreset->isChecked() )
    {
      QgsNewNameDialog dlg(
        tr( "theme" ),
        tr( "%1" ).arg( actionPreset->text() ),
        QStringList(), existingNames, Qt::CaseInsensitive, mMenu
      );

      dlg.setWindowTitle( tr( "Rename Map Theme" ) );
      dlg.setHintString( tr( "Enter the new name of the map theme" ) );
      dlg.setOverwriteEnabled( false );
      dlg.setConflictingNameWarning( tr( "A theme with this name already exists." ) );
      dlg.buttonBox()->addButton( QDialogButtonBox::Help );
      connect( dlg.buttonBox(), &QDialogButtonBox::helpRequested, this, &QgsMapThemes::showHelp );
      if ( dlg.exec() != QDialog::Accepted || dlg.name().isEmpty() )
        return;

      QgsProject::instance()->mapThemeCollection()->renameMapTheme( actionPreset->text(), dlg.name() );
    }
  }
}

void QgsMapThemes::removeCurrentPreset()
{
  for ( QAction *actionPreset : std::as_const( mMenuPresetActions ) )
  {
    if ( actionPreset->isChecked() )
    {
      int res = QMessageBox::question( QgisApp::instance(), tr( "Remove Theme" ), tr( "Are you sure you want to remove the existing theme “%1”?" ).arg( actionPreset->text() ), QMessageBox::Yes | QMessageBox::No, QMessageBox::No );
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

  const auto constMapThemes = QgsProject::instance()->mapThemeCollection()->mapThemes();
  for ( const QString &grpName : constMapThemes )
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
  mActionRenameCurrentPreset->setEnabled( hasCurrent );
}

void QgsMapThemes::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "introduction/general_tools.html#configuring-map-themes" ) );
}
