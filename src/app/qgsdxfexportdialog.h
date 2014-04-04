/***************************************************************************
                         qgsdxfexportdialog.h
                         --------------------
    begin                : September 2013
    copyright            : (C) 2013 by Marco Hugentobler
    email                : marco at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSDXFEXPORTDIALOG_H
#define QGSDXFEXPORTDIALOG_H

#include "ui_qgsdxfexportdialogbase.h"
#include "qgsdxfexport.h"
#include "qgsmaplayerproxymodel.h"

class QgsDxfExportDialog: public QDialog, private Ui::QgsDxfExportDialogBase
{
    Q_OBJECT
  public:
    QgsDxfExportDialog( QWidget * parent = 0, Qt::WindowFlags f = 0 );
    ~QgsDxfExportDialog();

    QList<QgsMapLayer *> layers() const;
    double symbologyScale() const;
    QgsDxfExport::SymbologyExport symbologyMode() const;
    QString saveFile() const;
    bool exportMapExtent() const;

  public slots:
    /** change the selection of layers in the list */
    void selectAll();
    void unSelectAll();

  private slots:
    void on_mFileSelectionButton_clicked();
    void setOkEnabled();
    void saveSettings();

  private:
    QgsMapLayerProxyModel* mModel;
};

#endif // QGSDXFEXPORTDIALOG_H
