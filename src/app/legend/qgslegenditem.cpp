/***************************************************************************
    qgslegenditem.cpp
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
#include "qgslegenditem.h"
#include <QCoreApplication>
#include "qgslegend.h"
#include "qgslogger.h"


QgsLegendItem::QgsLegendItem( QTreeWidgetItem * theItem, QString theName )
    : QTreeWidgetItem( theItem )
{
  setText( 0, theName );
}

QgsLegendItem::QgsLegendItem( QTreeWidget* theListView, QString theString )
    : QTreeWidgetItem( theListView )
{
  setText( 0, theString );
}

QgsLegendItem::QgsLegendItem(): QTreeWidgetItem()
{
}

QgsLegendItem::~QgsLegendItem()
{
}


void QgsLegendItem::print( QgsLegendItem * theItem )
{
#if 0 //todo: adapt to qt4
  Q3ListViewItemIterator myIterator( theItem );
  while ( myIterator.current() )
  {
    LEGEND_ITEM_TYPE curtype = qobject_cast<QgsLegendItem *>( myIterator.current() )->type();
    QgsDebugMsg( QString( "%1 - %2" ).arg( myIterator.current()->text( 0 ) ).arg( curtype ) );
    if ( myIterator.current()->childCount() > 0 )
    {
      //print(qobject_cast<QgsLegendItem *>(myIterator.current()));
    }
    ++myIterator;
  }
#else
  Q_UNUSED( theItem );
#endif
}

QgsLegendItem* QgsLegendItem::firstChild()
{
  return dynamic_cast<QgsLegendItem *>( child( 0 ) );
}

QgsLegendItem* QgsLegendItem::nextSibling()
{
  return dynamic_cast<QgsLegendItem *>( dynamic_cast<QgsLegend*>( treeWidget() )->nextSibling( this ) );
}

QgsLegendItem* QgsLegendItem::findYoungerSibling()
{
  return dynamic_cast<QgsLegendItem *>( dynamic_cast<QgsLegend*>( treeWidget() )->previousSibling( this ) );
}

void QgsLegendItem::moveItem( QgsLegendItem* after )
{
  qobject_cast<QgsLegend *>( treeWidget() )->moveItem( this, after );
}

void QgsLegendItem::removeAllChildren()
{
  while ( child( 0 ) )
  {
    takeChild( 0 );
  }
}

void QgsLegendItem::storeAppearanceSettings()
{
  mExpanded = treeWidget()->isItemExpanded( this );
  mHidden = treeWidget()->isItemHidden( this );
  //call recursively for all subitems
  for ( int i = 0; i < childCount(); ++i )
  {
    static_cast<QgsLegendItem*>( child( i ) )->storeAppearanceSettings();
  }
}

void QgsLegendItem::restoreAppearanceSettings()
{
  treeWidget()->setItemExpanded( this, mExpanded );
  treeWidget()->setItemHidden( this, mHidden );
  //call recursively for all subitems
  for ( int i = 0; i < childCount(); ++i )
  {
    static_cast<QgsLegendItem*>( child( i ) )->restoreAppearanceSettings();
  }
}

QgsLegend* QgsLegendItem::legend() const
{
  QTreeWidget* treeWidgetPtr = treeWidget();
  QgsLegend* legendPtr = qobject_cast<QgsLegend *>( treeWidgetPtr );
  return legendPtr;
}

QTreeWidgetItem* QgsLegendItem::child( int i ) const
{
  return QTreeWidgetItem::child( i );
}

QTreeWidgetItem* QgsLegendItem::parent() const
{
  return QTreeWidgetItem::parent();
}

void QgsLegendItem::insertChild( int index, QTreeWidgetItem *child )
{
  QTreeWidgetItem::insertChild( index, child );
}
