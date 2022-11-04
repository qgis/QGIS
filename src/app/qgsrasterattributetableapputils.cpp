/***************************************************************************
  qgsrasterattributetableapputils.cpp - QgsRasterAttributeTableAppUtils

 ---------------------
 begin                : 3.11.2022
 copyright            : (C) 2022 by ale
 email                : [your-email-here]
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsrasterattributetableapputils.h"

#include "qgsmaplayer.h"
#include "qgsrasterlayer.h"
#include "qgslayertreeview.h"
#include "qgsrasterattributetabledialog.h"
#include "qgsloadrasterattributetabledialog.h"
#include "qgscreaterasterattributetabledialog.h"

void QgsRasterAttributeTableAppUtils::openRasterAttributeTable( QgsLayerTreeView *treeView )
{
  if ( !treeView )
    return;

  //find current Layer
  QgsMapLayer *currentLayer = treeView->currentLayer();
  if ( !currentLayer )
    return;

  QgsRasterLayer *layer = qobject_cast<QgsRasterLayer *>( currentLayer );
  if ( layer && layer->attributeTableCount() > 0 )
  {
    QgsRasterAttributeTableDialog *dlg = new QgsRasterAttributeTableDialog( layer );
    dlg->setAttribute( Qt::WA_DeleteOnClose );
    dlg->show();
  }
}

void QgsRasterAttributeTableAppUtils::loadRasterAttributeTableFromFile( QgsLayerTreeView *treeView, QgsMessageBar *messageBar )
{
  if ( !treeView )
    return;

  //find current Layer
  QgsMapLayer *currentLayer = treeView->currentLayer();
  if ( !currentLayer )
    return;

  if ( QgsRasterLayer *layer = qobject_cast<QgsRasterLayer *>( currentLayer ); layer )
  {
    QgsLoadRasterAttributeTableDialog dlg { layer };
    dlg.setMessageBar( messageBar );
    if ( dlg.exec() == QDialog::Accepted && dlg.openWhenDone() )
    {
      openRasterAttributeTable( treeView );
    }
  }
}

void QgsRasterAttributeTableAppUtils::createRasterAttributeTable( QgsLayerTreeView *treeView, QgsMessageBar *messageBar )
{
  if ( !treeView )
    return;

  //find current Layer
  QgsMapLayer *currentLayer = treeView->currentLayer();
  if ( !currentLayer )
    return;

  if ( QgsRasterLayer *layer = qobject_cast<QgsRasterLayer *>( currentLayer ); layer && layer->canCreateRasterAttributeTable() )
  {
    // Create the attribute table from the renderer and open it
    QgsCreateRasterAttributeTableDialog dlg { layer };
    dlg.setMessageBar( messageBar );
    if ( dlg.exec() == QDialog::Accepted && dlg.openWhenDone() )
    {
      openRasterAttributeTable( treeView );
    }
  }
}
