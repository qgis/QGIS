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

#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayer.h"
#include "qgsmaplayerstylemanager.h"


QgsMapLayerStyleGuiUtils* QgsMapLayerStyleGuiUtils::instance()
{
  static QgsMapLayerStyleGuiUtils sInstance;
  return &sInstance;
}

QAction* QgsMapLayerStyleGuiUtils::actionAddStyle( QgsMapLayer* layer, QObject* parent )
{
  QAction* a = new QAction( tr( "Add..." ), parent );
  a->setData( QVariant::fromValue<QObject*>( layer ) );
  connect( a, SIGNAL( triggered() ), this, SLOT( addStyle() ) );
  return a;
}

QAction* QgsMapLayerStyleGuiUtils::actionRemoveStyle( QgsMapLayer* layer, QObject* parent )
{
  QAction* a = new QAction( tr( "Remove Current" ), parent );
  a->connect( a, SIGNAL( triggered() ), this, SLOT( removeStyle() ) );
  a->setData( QVariant::fromValue<QObject*>( layer ) );
  a->setEnabled( layer->styleManager()->styles().count() > 1 );
  return a;
}

QAction* QgsMapLayerStyleGuiUtils::actionRenameStyle( QgsMapLayer* layer, QObject* parent )
{
  QAction* a = new QAction( tr( "Rename Current..." ), parent );
  a->connect( a, SIGNAL( triggered() ), this, SLOT( renameStyle() ) );
  a->setData( QVariant::fromValue<QObject*>( layer ) );
  return a;
}

QList<QAction*> QgsMapLayerStyleGuiUtils::actionsUseStyle( QgsMapLayer* layer, QObject* parent )
{
  QgsMapLayerStyleManager* mgr = layer->styleManager();
  bool onlyOneStyle = mgr->styles().count() == 1;

  QList<QAction*> actions;
  Q_FOREACH ( QString name, mgr->styles() )
  {
    bool active = name == mgr->currentStyle();
    if ( name.isEmpty() )
      name = defaultStyleName();
    QAction* actionUse = new QAction( name, parent );
    connect( actionUse, SIGNAL( triggered() ), this, SLOT( useStyle() ) );
    actionUse->setCheckable( true );
    actionUse->setChecked( active );
    actionUse->setEnabled( !onlyOneStyle );

    actionUse->setData( QVariant::fromValue<QObject*>( layer ) );
    actions << actionUse;
  }
  return actions;
}

void QgsMapLayerStyleGuiUtils::addStyleManagerActions( QMenu* m, QgsMapLayer* layer )
{
  m->addAction( actionAddStyle( layer ) );
  if ( layer->styleManager()->styles().count() > 1 )
    m->addAction( actionRemoveStyle( layer ) );
  m->addAction( actionRenameStyle( layer ) );
  m->addSeparator();
  Q_FOREACH ( QAction* a, actionsUseStyle( layer ) )
    m->addAction( a );
}

QString QgsMapLayerStyleGuiUtils::defaultStyleName()
{
  return tr( "(default)" );
}


void QgsMapLayerStyleGuiUtils::addStyle()
{
  QAction* a = qobject_cast<QAction*>( sender() );
  if ( !a )
    return;
  QgsMapLayer* layer = qobject_cast<QgsMapLayer*>( a->data().value<QObject*>() );
  if ( !layer )
    return;

  bool ok;
  QString text = QInputDialog::getText( 0, tr( "New style" ),
                                        tr( "Style name:" ), QLineEdit::Normal,
                                        "new style", &ok );
  if ( !ok || text.isEmpty() )
    return;

  bool res = layer->styleManager()->addStyleFromLayer( text );

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
  QAction* a = qobject_cast<QAction*>( sender() );
  if ( !a )
    return;
  QgsMapLayer* layer = qobject_cast<QgsMapLayer*>( a->data().value<QObject*>() );
  if ( !layer )
    return;
  QString name = a->text();
  if ( name == defaultStyleName() )
    name.clear();

  bool res = layer->styleManager()->setCurrentStyle( name );
  if ( !res )
  {
    QgsDebugMsg( "Failed to set current style: " + name );
  }
}


void QgsMapLayerStyleGuiUtils::removeStyle()
{
  QAction* a = qobject_cast<QAction*>( sender() );
  if ( !a )
    return;
  QgsMapLayer* layer = qobject_cast<QgsMapLayer*>( a->data().value<QObject*>() );
  if ( !layer )
    return;

  bool res = layer->styleManager()->removeStyle( layer->styleManager()->currentStyle() );
  if ( !res )
  {
    QgsDebugMsg( "Failed to remove current style" );
  }
}


void QgsMapLayerStyleGuiUtils::renameStyle()
{
  QAction* a = qobject_cast<QAction*>( sender() );
  if ( !a )
    return;
  QgsMapLayer* layer = qobject_cast<QgsMapLayer*>( a->data().value<QObject*>() );
  if ( !layer )
    return;

  QString name = layer->styleManager()->currentStyle();

  bool ok;
  QString text = QInputDialog::getText( 0, tr( "Rename style" ),
                                        tr( "Style name:" ), QLineEdit::Normal,
                                        name, &ok );
  if ( !ok )
    return;

  bool res = layer->styleManager()->renameStyle( name, text );
  if ( !res )
  {
    QgsDebugMsg( "Failed to rename style: " + name );
  }
}
