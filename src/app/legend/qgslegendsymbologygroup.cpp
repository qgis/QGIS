/***************************************************************************
    qgslegendsymbologygroup.cpp
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
#include "qgslegendsymbologygroup.h"
#include "qgsmaplayer.h"
#include <QCoreApplication>
#include <QIcon>

QgsLegendSymbologyGroup::QgsLegendSymbologyGroup( QTreeWidgetItem * theItem, QString theString )
    : QgsLegendItem( theItem, theString )
{
  mType = LEGEND_SYMBOL_GROUP;
  QIcon myIcon = QgsApplication::getThemeIcon( "/mIconSymbology.png" );
  setText( 0, theString );
  setIcon( 0, myIcon );
}


QgsLegendSymbologyGroup::~QgsLegendSymbologyGroup()
{}

/** Overloads cmpare function of QListViewItem
  * @note The symbology group must always be the second in the list
  */
int QgsLegendSymbologyGroup::compare( QTreeWidgetItem * i, int col, bool ascending )
{
  Q_UNUSED( col );
  Q_UNUSED( ascending );
  QgsLegendItem * myItem = dynamic_cast<QgsLegendItem *>( i ) ;
  if ( myItem->type() == QgsLegendItem::LEGEND_PROPERTY_GROUP )
  {
    return 1;
  }
  else
  {
    return -1;
  }
}

