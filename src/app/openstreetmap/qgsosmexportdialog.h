/***************************************************************************
  qgsosmexportdialog.h
  --------------------------------------
  Date                 : February 2013
  Copyright            : (C) 2013 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSOSMEXPORTDIALOG_H
#define QGSOSMEXPORTDIALOG_H

#include <QDialog>

#include "ui_qgsosmexportdialog.h"

class QgsOSMDatabase;

class QStandardItemModel;

class QgsOSMExportDialog : public QDialog, private Ui::QgsOSMExportDialog
{
    Q_OBJECT
  public:
    explicit QgsOSMExportDialog( QWidget *parent = 0 );
    ~QgsOSMExportDialog();

  protected:
    bool openDatabase();

  private slots:
    void onBrowse();
    void updateLayerName();
    void onLoadTags();

    void onOK();
    void onClose();

  private:
    QgsOSMDatabase* mDatabase;
    QStandardItemModel* mTagsModel;
};

#endif // QGSOSMEXPORTDIALOG_H
