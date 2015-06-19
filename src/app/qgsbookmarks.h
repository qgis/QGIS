/***************************************************************************
               QgsBookmarks.h  - Spatial Bookmarks
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
#ifndef QGSBOOKMARKS_H
#define QGSBOOKMARKS_H

#include <QDockWidget>

#include "ui_qgsbookmarksbase.h"
#include "qgscontexthelp.h"

class APP_EXPORT QgsBookmarks : public QDockWidget, private Ui::QgsBookmarksBase
{
    Q_OBJECT

  public:
    QgsBookmarks( QWidget *parent = 0 );
    ~QgsBookmarks();

  public slots:
    void addClicked();

  private slots:
    void deleteClicked();
    void zoomToBookmark();
    void exportToXML();
    void importFromXML();

    void on_lstBookmarks_doubleClicked( const QModelIndex & );
    void on_actionHelp_triggered() { QgsContextHelp::run( metaObject()->className() ); }

  private:
    void saveWindowLocation();
    void restorePosition();

};


#endif // QGSBOOKMARKS_H

