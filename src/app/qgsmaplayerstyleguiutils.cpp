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


QMenu* QgsMapLayerStyleGuiUtils::createStyleManagerMenu( QgsMapLayer* layer )
{
  QMenu* m = new QMenu( tr( "Styles" ) );
  QAction* actionAdd = m->addAction( tr( "Add" ), this, SLOT( addStyle() ) );
  actionAdd->setData( QVariant::fromValue<QObject*>( layer ) );

  QgsMapLayerStyleManager* mgr = layer->styleManager();
  bool onlyOneStyle = mgr->styles().count() == 1;

  QAction* actionRemove = m->addAction( tr( "Remove Current" ), this, SLOT( removeStyle() ) );
  actionRemove->setData( QVariant::fromValue<QObject*>( layer ) );
  actionRemove->setEnabled( !onlyOneStyle );

  QAction* actionRename = m->addAction( tr( "Rename Current" ), this, SLOT( renameStyle() ) );
  actionRename->setData( QVariant::fromValue<QObject*>( layer ) );

  m->addSeparator();

  foreach ( QString name, mgr->styles() )
  {
    bool active = name == mgr->currentStyle();
    if ( name.isEmpty() )
      name = defaultStyleName();
    QAction* actionUse = m->addAction( name, this, SLOT( useStyle() ) );
    actionUse->setCheckable( true );
    actionUse->setChecked( active );
    actionUse->setEnabled( !onlyOneStyle );

    actionUse->setData( QVariant::fromValue<QObject*>( layer ) );
  }

  return m;
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
    layer->styleManager()->setCurrentStyle( text );
  else
    QgsDebugMsg( "Failed to add style: " + text );
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
    QgsDebugMsg( "Failed to set current style: " + name );
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
    QgsDebugMsg( "Failed to remove current style" );
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
    QgsDebugMsg( "Failed to rename style: " + name );
}
