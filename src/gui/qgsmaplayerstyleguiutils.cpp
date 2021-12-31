/***************************************************************************
  qgsmaplayerstyleguiutils.cpp
  --------------------------------------
  Date                 : January 2015
  Copyright            : (C) 2015 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaplayerstyleguiutils.h"

#include <QAction>
#include <QInputDialog>
#include <QMenu>
#include <QActionGroup>

#include "qgshelp.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayer.h"
#include "qgsmaplayerstylemanager.h"
#include "qgsnewnamedialog.h"


QgsMapLayerStyleGuiUtils *QgsMapLayerStyleGuiUtils::instance()
{
  static QgsMapLayerStyleGuiUtils sInstance;
  return &sInstance;
}

QAction *QgsMapLayerStyleGuiUtils::actionAddStyle( QgsMapLayer *layer, QObject *parent )
{
  QAction *a = new QAction( tr( "Add…" ), parent );
  a->setData( QVariant::fromValue<QObject *>( layer ) );
  connect( a, &QAction::triggered, this, &QgsMapLayerStyleGuiUtils::addStyle );
  return a;
}

QAction *QgsMapLayerStyleGuiUtils::actionRemoveStyle( QgsMapLayer *layer, QObject *parent )
{
  QAction *a = new QAction( tr( "Remove Current" ), parent );
  connect( a, &QAction::triggered, this, &QgsMapLayerStyleGuiUtils::removeStyle );
  a->setData( QVariant::fromValue<QObject *>( layer ) );
  a->setEnabled( layer->styleManager()->styles().count() > 1 );
  return a;
}

QAction *QgsMapLayerStyleGuiUtils::actionRenameStyle( QgsMapLayer *layer, QObject *parent )
{
  QAction *a = new QAction( tr( "Rename Current…" ), parent );
  connect( a, &QAction::triggered, this, &QgsMapLayerStyleGuiUtils::renameStyle );
  a->setData( QVariant::fromValue<QObject *>( layer ) );
  return a;
}

QList<QAction *> QgsMapLayerStyleGuiUtils::actionsUseStyle( QgsMapLayer *layer, QObject *parent )
{
  QgsMapLayerStyleManager *mgr = layer->styleManager();
  const bool onlyOneStyle = mgr->styles().count() == 1;

  QList<QAction *> actions;
  QActionGroup *styleGroup = new QActionGroup( parent );
  const auto constStyles = mgr->styles();
  for ( const QString &name : constStyles )
  {
    const bool active = name == mgr->currentStyle();
    QAction *actionUse = new QAction( name, styleGroup );
    connect( actionUse, &QAction::triggered, this, &QgsMapLayerStyleGuiUtils::useStyle );
    actionUse->setCheckable( true );
    actionUse->setChecked( active );
    actionUse->setEnabled( !onlyOneStyle );

    actionUse->setData( QVariant::fromValue<QObject *>( layer ) );
    actions << actionUse;
  }
  return actions;
}

void QgsMapLayerStyleGuiUtils::addStyleManagerActions( QMenu *m, QgsMapLayer *layer )
{
  if ( ! layer )
    return;

  m->addAction( actionAddStyle( layer, m ) );
  if ( layer->styleManager()->styles().count() > 1 )
    m->addAction( actionRemoveStyle( layer, m ) );
  m->addAction( actionRenameStyle( layer, m ) );
  m->addSeparator();
  const auto actions {actionsUseStyle( layer, m )};
  for ( QAction *a : actions )
    m->addAction( a );
}

void QgsMapLayerStyleGuiUtils::removesExtraMenuSeparators( QMenu *m )
{
  if ( !m )
    return;

  // Get rid of previously added style manager actions (they are dynamic)
  bool gotFirstSeparator = false;
  QList<QAction *> actions = m->actions();
  for ( int i = 0; i < actions.count(); ++i )
  {
    if ( actions[i]->isSeparator() )
    {
      if ( gotFirstSeparator )
      {
        // remove all actions after second separator (including it)
        while ( actions.count() != i )
          delete actions.takeAt( i );
        break;
      }
      else
        gotFirstSeparator = true;
    }
  }

}

void QgsMapLayerStyleGuiUtils::addStyle()
{
  QAction *a = qobject_cast<QAction *>( sender() );
  if ( !a )
    return;
  QgsMapLayer *layer = qobject_cast<QgsMapLayer *>( a->data().value<QObject *>() );
  if ( !layer )
    return;

  const QStringList styles = layer->styleManager()->styles();

  QgsNewNameDialog dlg( nullptr, tr( "new style" ), QStringList(), styles, Qt::CaseSensitive );
  dlg.setWindowTitle( tr( "Create New Style" ) );
  dlg.setHintString( tr( "Name of the new style" ) );
  dlg.setOverwriteEnabled( false );
  dlg.setConflictingNameWarning( tr( "A style with this name already exists!" ) );

  dlg.buttonBox()->addButton( QDialogButtonBox::Help );
  connect( dlg.buttonBox(), &QDialogButtonBox::helpRequested, this, [ = ]
  {
    QgsHelp::openHelp( "introduction/general_tools.html#save-and-share-layer-properties" );
  } );

  if ( dlg.exec() != QDialog::Accepted )
  {
    return;
  }

  const QString text = dlg.name();
  const bool res = layer->styleManager()->addStyleFromLayer( text );
  if ( res ) // make it active!
  {
    layer->styleManager()->setCurrentStyle( text );
  }
  else
  {
    QgsDebugMsg( "Failed to add style: " + text );
  }
}

void QgsMapLayerStyleGuiUtils::useStyle()
{
  QAction *a = qobject_cast<QAction *>( sender() );
  if ( !a )
    return;
  QgsMapLayer *layer = qobject_cast<QgsMapLayer *>( a->data().value<QObject *>() );
  if ( !layer )
    return;
  const QString name = a->text();

  const bool res = layer->styleManager()->setCurrentStyle( name );
  if ( !res )
  {
    QgsDebugMsg( "Failed to set current style: " + name );
  }
}


void QgsMapLayerStyleGuiUtils::removeStyle()
{
  QAction *a = qobject_cast<QAction *>( sender() );
  if ( !a )
    return;
  QgsMapLayer *layer = qobject_cast<QgsMapLayer *>( a->data().value<QObject *>() );
  if ( !layer )
    return;

  const bool res = layer->styleManager()->removeStyle( layer->styleManager()->currentStyle() );
  if ( !res )
  {
    QgsDebugMsg( QStringLiteral( "Failed to remove current style" ) );
  }
}


void QgsMapLayerStyleGuiUtils::renameStyle()
{
  QAction *a = qobject_cast<QAction *>( sender() );
  if ( !a )
    return;
  QgsMapLayer *layer = qobject_cast<QgsMapLayer *>( a->data().value<QObject *>() );
  if ( !layer )
    return;

  const QString name = layer->styleManager()->currentStyle();
  const QStringList styles = layer->styleManager()->styles();

  QgsNewNameDialog dlg( nullptr, name, QStringList(), styles, Qt::CaseSensitive );
  dlg.setWindowTitle( tr( "Rename Current Style" ) );
  dlg.setHintString( tr( "New name of the style" ) );
  dlg.setOverwriteEnabled( false );
  dlg.setConflictingNameWarning( tr( "A style with this name already exists!" ) );

  dlg.buttonBox()->addButton( QDialogButtonBox::Help );
  connect( dlg.buttonBox(), &QDialogButtonBox::helpRequested, this, [ = ]
  {
    QgsHelp::openHelp( "introduction/general_tools.html#save-and-share-layer-properties" );
  } );

  if ( dlg.exec() != QDialog::Accepted )
  {
    return;
  }

  const QString text = dlg.name();
  const bool res = layer->styleManager()->renameStyle( name, text );
  if ( !res )
  {
    QgsDebugMsg( "Failed to rename style: " + name );
  }
}
