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
#include <QDialog>
#include <QSqlDatabase>
#include <QSqlTableModel>
#include <QTreeWidgetItem>
#include "qgscontexthelp.h"

class QString;
class QWidget;

const QString CRSDB = "CRSDB";


class QgsBookmarks : public QDialog, private Ui::QgsBookmarksBase
{
    Q_OBJECT

  public:
    QgsBookmarks( QWidget *parent = 0, Qt::WFlags fl = 0 );
    ~QgsBookmarks();
    void restorePosition();
  private slots:
    void saveWindowLocation();
    void on_btnDelete_clicked();
    void on_btnZoomTo_clicked();
    //void on_lstBookmarks_clicked( const QModelIndex & );
    void on_lstBookmarks_doubleClicked( const QModelIndex & );
    void refreshBookmarks();
    void on_buttonBox_helpRequested() { QgsContextHelp::run( metaObject()->className() ); }

  private:
    QWidget *mParent;
    void initialise();
    int connectDb();
    void zoomToBookmark();
    QSqlDatabase db;
};


#endif // QGSBOOKMARKS_H

