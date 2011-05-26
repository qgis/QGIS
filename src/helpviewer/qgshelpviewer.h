/***************************************************************************
                             qgshelpviewer.h
                             Simple help browser
                             -------------------
    begin                : 2005-07-02
    copyright            : (C) 2005 by Gary E.Sherman
    email                : sherman at mrcc.com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSHELPVIEWER_H
#define QGSHELPVIEWER_H
# include "ui_qgshelpviewerbase.h"
class QString;
struct sqlite3;
class QgsHelpViewer : public QDialog, private Ui::QgsHelpViewerBase
{
    Q_OBJECT
  public:
    QgsHelpViewer( const QString &contextId = QString::null, QWidget *parent = 0, Qt::WFlags = 0 );
    ~QgsHelpViewer();
  public slots:
    void setContext( const QString &contextId );
    void fileExit();
  protected:
    void moveEvent( QMoveEvent *event );
    void resizeEvent( QResizeEvent *event );
  private:
    void restorePosition();
    void saveWindowLocation();
    void loadContext( const QString &contextId );
    void loadContextFromSqlite( const QString &contextId );
    int connectDb( const QString &helpDbPath );
    sqlite3 *db;
};
#endif // QGSHELPVIEWER_H
