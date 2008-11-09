
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
#ifndef QGSBOOKMARKITEM_H
#define QGSBOOKMARKITEM_H

#include <QString>
#include "qgsrectangle.h"

/*!
 * \class QgsBookmarkItem
 * \brief A spatial bookmark record that is stored in a sqlite3
 * database.
 */
class QgsBookmarkItem
{
  public:
    //! Constructs a bookmark item
    QgsBookmarkItem( QString name, QString projectTitle,
                     QgsRectangle viewExtent, int srid, QString databasePath );
    //! Default destructor
    ~QgsBookmarkItem();
    //! Store the bookmark in the database
    void store();
  private:
    //! Name of the bookmark
    QString mName;
    //! Project that this bookmark was created from
    QString mProjectTitle;
    //! Extent of the view for the bookmark
    QgsRectangle mViewExtent;
    //! SRID of the canvas coordinate system when the bookmark was created
    int mSrid;
    //! Full path to the user database
    QString mUserDbPath;

};

#endif // QGSBOOKMARKITEM_H

