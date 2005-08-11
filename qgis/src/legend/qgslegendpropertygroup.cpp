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
#include "qgslegendpropertygroup.h"
#include <qlistview.h>
#include <qstring.h>
#include <qpixmap.h>
QgsLegendPropertyGroup::QgsLegendPropertyGroup(QListViewItem * theLegendItem, QString theString)
    : QgsLegendItem(theLegendItem,theString )
{
  mType=LEGEND_PROPERTY_GROUP;
  QPixmap myPixmap(QString(PKGDATAPATH)+QString("/images/icons/properties.png"));
  setPixmap(0,myPixmap);
}


QgsLegendPropertyGroup::~QgsLegendPropertyGroup()
{

}

bool QgsLegendPropertyGroup::accept(DRAG_TYPE dt, LEGEND_ITEM_TYPE type)
{
    return false;
}

