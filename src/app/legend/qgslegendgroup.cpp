/***************************************************************************
 *   Copyright (C) 2005 by Tim Sutton   *
 *   aps02ts@macbuntu   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "qgsapplication.h"
#include "qgisapp.h"
#include "qgslegendgroup.h"
#include "qgslegendlayer.h"
#include "qgslegendlayerfile.h"
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


bool QgsLegendGroup::isLeafNode()
{
  return mLeafNodeFlag;
}

QgsLegendItem::DRAG_ACTION QgsLegendGroup::accept( LEGEND_ITEM_TYPE type )
{
  if ( type == LEGEND_GROUP )
  {
    return REORDER;
  }
  if ( type == LEGEND_LAYER )
  {
    return INSERT;
  }
  else
  {
    return NO_ACTION;
  }
}

QgsLegendItem::DRAG_ACTION QgsLegendGroup::accept( const QgsLegendItem* li ) const
{
  if ( li )
  {
    LEGEND_ITEM_TYPE type = li->type();
    if ( type == LEGEND_GROUP )
    {
      return REORDER;
    }
    if ( type == LEGEND_LAYER )
    {
      return INSERT;
    }
  }
  return NO_ACTION;
}

bool QgsLegendGroup::insert( QgsLegendItem* theItem )
{
  if ( theItem->type() == LEGEND_LAYER )
  {
    // Always insert at top of list
    insertChild( 0, theItem );
  }
  // XXX - mloskot - I don't know what to return
  // but this function must return a value
  return true;
}

std::list<QgsLegendLayerFile*> QgsLegendGroup::legendLayerFiles()
{
  std::list<QgsLegendLayerFile*> result;
  for ( int i = 0; i < childCount(); ++i )
  {
    QgsLegendLayer* childItem = dynamic_cast<QgsLegendLayer *>( child( i ) );
    if ( childItem )
    {
      std::list<QgsLegendLayerFile*> childList = childItem->legendLayerFiles();
      result.splice( result.end(), childList );
    }
  }
  return result;
}

void QgsLegendGroup::updateCheckState()
{
  std::list<QgsLegendLayerFile*> llfiles = legendLayerFiles();
  if ( llfiles.size() < 1 )
  {
    return;
  }

  std::list<QgsLegendLayerFile*>::iterator iter = llfiles.begin();
  Qt::CheckState theState = ( *iter )->checkState( 0 );
  for ( ; iter != llfiles.end(); ++iter )
  {
    if ( theState != ( *iter )->checkState( 0 ) )
    {
      theState = Qt::PartiallyChecked;
      break;
    }
  }

  if ( theState != checkState( 0 ) )
  {
    treeWidget()->blockSignals( true );
    setCheckState( 0, theState );
    treeWidget()->blockSignals( false );
  }
}
