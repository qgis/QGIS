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
#include "qgslegenditem.h"
#include <qpixmap.h>
#include <iostream.h>
QgsLegendItem::QgsLegendItem(QListViewItem * theItem ,QString theName)
 : QListViewItem(theItem,theName)
{
  QPixmap myPixmap(QString(PKGDATAPATH)+QString("/images/icons/group.png"));
  setPixmap(0,myPixmap);
}

QgsLegendItem::QgsLegendItem(QListView * theListView,QString theString)
 : QListViewItem(theListView,theString)
{
  QPixmap myPixmap(QString(PKGDATAPATH)+QString("/images/icons/group.png"));
  setPixmap(0,myPixmap);
}

QgsLegendItem::~QgsLegendItem()
{
}


void QgsLegendItem::print(QgsLegendItem * theItem)
{
    QListViewItemIterator myIterator (theItem);
    while (myIterator.current())
    {
      LEGEND_ITEM_TYPE curtype = dynamic_cast<QgsLegendItem *>(myIterator.current())->type();
      std::cout << myIterator.current()->text(0) << " - " << curtype << std::endl;
      if (myIterator.current()->childCount() > 0)
      {
        //print(dynamic_cast<QgsLegendItem *>(myIterator.current()));
      }
      ++myIterator;
    }
}
