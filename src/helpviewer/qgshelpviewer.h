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

#include <QDialog>
#include <QThread>

#include "ui_qgshelpviewerbase.h"

class QString;
class QFile;

class QgsReaderThread : public QThread
{
    Q_OBJECT
  public:
    QgsReaderThread();

    virtual void run() override;

  signals:
    void helpRead( const QString& help );
};


class QgsHelpViewer : public QDialog, private Ui::QgsHelpViewerBase
{
    Q_OBJECT
  public:
    QgsHelpViewer( QWidget *parent = nullptr, const Qt::WindowFlags& = nullptr );
    ~QgsHelpViewer();
  public slots:
    void showHelp( const QString& );
    void fileExit();
  protected:
    void moveEvent( QMoveEvent *event ) override;
    void resizeEvent( QResizeEvent *event ) override;
  private:
    void restorePosition();
    void saveWindowLocation();
    QgsReaderThread *mThread;
};
#endif // QGSHELPVIEWER_H
