/***************************************************************************
    qgslegendgroup.cpp
    ---------------------
    begin                : January 2007
    copyright            : (C) 2007 by Martin Dobias
    email                : wonder.sk at gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsapplication.h"
#include "qgisapp.h"
#include "qgslegendgroup.h"
#include "qgslegendlayer.h"
#include <QCoreApplication>
#include <QIcon>

QgsLegendGroup::QgsLegendGroup( QTreeWidgetItem * theItem, QString theName )
    : QgsLegendItem( theItem, theName )
{
  mType = LEGEND_GROUP;
  setFlags( Qt::ItemIsEditable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable );
  setCheckState( 0, Qt::Checked );
  QIcon myIcon = QgisApp::getThemeIcon( "/mActionFolder.png" );
  setIcon( 0, myIcon );
}
QgsLegendGroup::QgsLegendGroup( QTreeWidget* theListView, QString theString )
    : QgsLegendItem( theListView, theString )
{
  mType = LEGEND_GROUP;
  setFlags( Qt::ItemIsEditable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable );
  setCheckState( 0, Qt::Checked );
  QIcon myIcon = QgisApp::getThemeIcon( "/mActionFolder.png" );
  setIcon( 0, myIcon );
}

QgsLegendGroup::QgsLegendGroup( QString name ): QgsLegendItem()
{
  mType = LEGEND_GROUP;
  setFlags( Qt::ItemIsEditable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable );
  setCheckState( 0, Qt::Checked );
  QIcon myIcon = QgisApp::getThemeIcon( + "/mActionFolder.png" );
  setText( 0, name );
  setIcon( 0, myIcon );
}

QgsLegendGroup::~QgsLegendGroup()
{}


bool QgsLegendGroup::insert( QgsLegendItem* theItem )
{
  if ( theItem->type() == LEGEND_GROUP ||
       theItem->type() == LEGEND_LAYER )
  {
    // Always insert at top of list
    insertChild( 0, theItem );
  }
  // XXX - mloskot - I don't know what to return
  // but this function must return a value
  return true;
}

QList<QgsLegendLayer*> QgsLegendGroup::legendLayers( bool recurse )
{
  QList<QgsLegendLayer*> result;
  for ( int i = 0; i < childCount(); ++i )
  {
    QgsLegendLayer *layer = dynamic_cast<QgsLegendLayer *>( child( i ) );
    if ( layer )
    {
      result.push_back( layer );
    }

    if ( !recurse )
      continue;

    QgsLegendGroup *group = dynamic_cast<QgsLegendGroup *>( child( i ) );
    if ( group )
    {
      result << group->legendLayers( true );
    }
  }
  return result;
}

Qt::CheckState QgsLegendGroup::pendingCheckState()
{
  QList<QgsLegendItem *> elements;

  for ( int i = 0; i < childCount(); i++ )
  {
    QgsLegendItem *li = dynamic_cast<QgsLegendItem *>( child( i ) );

    if ( !li )
      continue;

    if ( dynamic_cast<QgsLegendGroup *>( li ) || dynamic_cast<QgsLegendLayer *>( li ) )
    {
      elements << li;
    }
  }

  if ( elements.isEmpty() )
    return Qt::PartiallyChecked;

  Qt::CheckState theState = elements[0]->checkState( 0 );
  foreach( QgsLegendItem * li, elements )
  {
    if ( theState != li->checkState( 0 ) )
    {
      theState = Qt::PartiallyChecked;
      break;
    }
  }

  return theState;
}
