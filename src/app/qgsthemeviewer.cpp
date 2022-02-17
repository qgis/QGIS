/***************************************************************************
  qgsthemeviewer.cpp
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

#include "qgsthemeviewer.h"
#include "qgsmaplayer.h"
#include <QDrag>
#include <QDragEnterEvent>
#include <QContextMenuEvent>
#include <QDropEvent>
#include <QWidget>
#include <QMimeData>
#include <QSignalBlocker>
#include <QCoreApplication>

QgsThemeViewer::QgsThemeViewer( QWidget *parent )
  : QgsLayerTreeView( parent )
{
}


QStringList QgsThemeViewer::mimeTypes() const
{
  QStringList types;
  types << QStringLiteral( "application/qgis.thememanagerdata" );
  return types;
}

void QgsThemeViewer::dragEnterEvent( QDragEnterEvent *event )
{
  if ( event->mimeData()->hasFormat( QStringLiteral( "application/qgis.layertreemodeldata" ) ) )
    event->acceptProposedAction();
}

void QgsThemeViewer::dropEvent( QDropEvent *event )
{
  if ( event->mimeData()->hasFormat( QStringLiteral( "application/qgis.layertreemodeldata" ) ) )
    emit layersAdded();

}

Qt::DropActions QgsThemeViewer::supportedDropActions() const
{
  return Qt::CopyAction | Qt::LinkAction;
}


QMimeData *QgsThemeViewer::mimeData() const
{
  QMimeData *mimeData = new QMimeData();
  QList<QgsMapLayer *> layers = selectedLayers();
  //QList<QgsLayerTreeNode *> nodesFinal = indexes2nodes( sortedIndexes, true );

  if ( layers.isEmpty() )
    return mimeData;

  QStringList ids;
  for ( QgsMapLayer *layer : std::as_const( layers ) )
  {
    ids << layer->id().toUtf8();
  }

  mimeData->setData( QStringLiteral( "application/qgis.thememanagerdata" ), ids.join( "+" ).toUtf8() );
  mimeData->setData( QStringLiteral( "application/qgis.application.pid" ), QString::number( QCoreApplication::applicationPid() ).toUtf8() );

  return mimeData;
}



void QgsThemeViewer::startDrag( Qt::DropActions )
{
  QMimeData *mimeDat = mimeData();
  QDrag *drag = new QDrag( this );
  drag->setMimeData( mimeDat );

  if ( !( drag->target() == this ) )
    emit layersDropped();

}

void QgsThemeViewer::contextMenuEvent( QContextMenuEvent *event )
{
  emit showMenu( event->pos() );
}

