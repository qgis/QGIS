/***************************************************************************
                         qgsinstallgridshiftdialog.h
                             -------------------
    begin                : September 2019
    copyright            : (C) 2019 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSINSTALLGRIDSHIFTDIALOG_H
#define QGSINSTALLGRIDSHIFTDIALOG_H

#include "ui_qgsinstallgridshiftdialog.h"
#include <QDialog>
#include "qgis_app.h"

class QgsInstallGridShiftFileDialog: public QDialog, private Ui::QgsInstallGridShiftFileDialogBase
{
    Q_OBJECT
  public:
    QgsInstallGridShiftFileDialog( const QString &gridName, QWidget *parent = nullptr );

    void setDescription( const QString &html );
    void setDownloadMessage( const QString &message );

  private slots:

    void installFromFile();

  private:
    QString mGridName;

};

#endif // QGSINSTALLGRIDSHIFTDIALOG_H
