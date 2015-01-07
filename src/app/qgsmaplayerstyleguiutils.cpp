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
  QAction* actionAdd = m->addAction( tr( "Add..." ), this, SLOT( addStyle() ) );
  actionAdd->setData( QVariant::fromValue<QObject*>( layer ) );

  QgsMapLayerStyleManager* mgr = layer->styleManager();

  if ( !mgr )
    return m;

  QMenu* mRemove = m->addMenu( tr( "Remove" ) );
  m->addSeparator();

  foreach ( QString name, mgr->styles() )
  {
    bool active = name == mgr->currentStyle();
    if ( name.isEmpty() )
      name = defaultStyleName();
    QAction* actionUse = m->addAction( name, this, SLOT( useStyle() ) );
    actionUse->setCheckable( true );
    actionUse->setChecked( active );
    actionUse->setData( QVariant::fromValue<QObject*>( layer ) );

    QAction* actionRemove = mRemove->addAction( name, this, SLOT( removeStyle() ) );
    actionRemove->setData( QVariant::fromValue<QObject*>( layer ) );
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

  layer->enableStyleManager(); // make sure it exists

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

  layer->triggerRepaint();
}


void QgsMapLayerStyleGuiUtils::removeStyle()
{
  QAction* a = qobject_cast<QAction*>( sender() );
  if ( !a )
    return;
  QgsMapLayer* layer = qobject_cast<QgsMapLayer*>( a->data().value<QObject*>() );
  if ( !layer )
    return;

  if ( layer->styleManager()->styles().count() == 1 )
  {
    // let's get rid of the style manager altogether
    layer->enableStyleManager( false );
    return;
  }

  QString name = a->text();
  if ( name == defaultStyleName() )
    name.clear();

  bool needsRefresh = ( layer->styleManager()->currentStyle() == name );

  bool res = layer->styleManager()->removeStyle( name );
  if ( !res )
    QgsDebugMsg( "Failed to remove style: " + name );

  if ( needsRefresh )
    layer->triggerRepaint();
}
