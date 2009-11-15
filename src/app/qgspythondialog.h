/***************************************************************************
    qgspythondialog.h - dialog with embedded python console
    ---------------------
    begin                : October 2006
    copyright            : (C) 2006 by Martin Dobias
    email                : wonder.sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

#ifndef QGSPYTHONDIALOG_H
#define QGSPYTHONDIALOG_H

#include "ui_qgspythondialog.h"

class QgisInterface;
class QgsPythonUtils;
class QCloseEvent;
class QShowEvent;

class QgsPythonDialog : public QDialog, private Ui::QgsPythonDialog
{
    Q_OBJECT

  public:
    QgsPythonDialog( QgisInterface* pIface, QgsPythonUtils* pythonUtils, QWidget *parent = 0 );

    ~QgsPythonDialog();

    QString escapeHtml( QString text );

  public slots:

    void on_pbnPrev_clicked();
    void on_pbnExecute_clicked();
    void on_pbnEval_clicked();
    void on_pbnNext_clicked();

  protected:

    void closeEvent( QCloseEvent *event );
    void showEvent( QShowEvent *event );

  private:
    void execute( bool single );

    QgisInterface* mIface;
    QgsPythonUtils* mPythonUtils;

    QStringList history;
    int pos;
};

#endif
