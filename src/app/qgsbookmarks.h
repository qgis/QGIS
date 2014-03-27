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
#include "ui_qgsbookmarksbase.h"
#include "qgscontexthelp.h"

#include <QDialog>

class APP_EXPORT QgsBookmarks : public QDialog, private Ui::QgsBookmarksBase
{
    Q_OBJECT

  public:
    static void showBookmarks();
    static void newBookmark();

  private slots:
    void addClicked();
    void deleteClicked();
    void zoomToBookmark();

    void on_lstBookmarks_doubleClicked( const QModelIndex & );
    void on_buttonBox_helpRequested() { QgsContextHelp::run( metaObject()->className() ); }

  private:
    QgsBookmarks( QWidget *parent = 0, Qt::WindowFlags fl = 0 );
    ~QgsBookmarks();

    void saveWindowLocation();
    void restorePosition();

    static QgsBookmarks *sInstance;
};


#endif // QGSBOOKMARKS_H

