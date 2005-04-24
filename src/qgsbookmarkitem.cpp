
/***************************************************************************
               QgsBookmarkItem.h  - Spatial Bookmark Item
                             -------------------
    begin                : 2005-04-23
    copyright            : (C) 2005 Gary Sherman
    email                : sherman at mrcc dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
 /* $Id$ */
#include <iostream>
#include <sqlite3.h>
#include <qstring.h>

#include "qgsrect.h"
#include "qgsbookmarkitem.h"

QgsBookmarkItem::QgsBookmarkItem(QString name, QString projectTitle, 
      QgsRect viewExtent, int srid, QString dbPath)
      : mName(name), mProjectTitle(projectTitle), mViewExtent(viewExtent),
      mSrid(srid), mDbPath(dbPath)
{
}
QgsBookmarkItem::~QgsBookmarkItem()
{
}
  void QgsBookmarkItem::store()
{
  std::cout << "Storing bookmark" << std::endl; 
}

