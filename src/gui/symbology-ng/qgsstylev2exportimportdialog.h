/***************************************************************************
    qgsstylev2exportimportdialog.h
    ---------------------
    begin                : Jan 2011
    copyright            : (C) 2011 by Alexander Bruy
    email                : alexander dot bruy at gmail dot com

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/* $Id: qgsstylev2exportimportdialog.h 13187 2010-03-28 22:14:44Z jef $ */

#ifndef QGSSTYLEV2EXPORTIMPORTDIALOG_H
#define QGSSTYLEV2EXPORTIMPORTDIALOG_H

#include <QDialog>

#include "ui_qgsstylev2exportimportdialogbase.h"

class QgsStyleV2;

class QgsStyleV2ExportImportDialog : public QDialog, private Ui::QgsStyleV2ExportImportDialogBase
{
    Q_OBJECT

  public:
    enum Mode
    {
      Export,
      Import
    };

    // constructor
    // mode argument must be 0 for saving and 1 for loading
    QgsStyleV2ExportImportDialog( QgsStyleV2* style, QWidget *parent = NULL, Mode mode = Export, QString fileName = "" );
    ~QgsStyleV2ExportImportDialog();

  public slots:
    void doExportImport();
    void selectAll();
    void clearSelection();

  private:
    bool populateStyles( QgsStyleV2* style );
    void moveStyles( QModelIndexList* selection, QgsStyleV2* src, QgsStyleV2* dst );

    QString mFileName;
    Mode mDialogMode;

    QgsStyleV2* mQgisStyle;
    QgsStyleV2* mTempStyle;
};

#endif // QGSSTYLEV2EXPORTIMPORTDIALOG_H
