/***************************************************************************
  qgsosmimportdialog.h
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

#ifndef QGSOSMIMPORTDIALOG_H
#define QGSOSMIMPORTDIALOG_H

#include <QDialog>

#include "ui_qgsosmimportdialog.h"

class QgsOSMXmlImport;

class QgsOSMImportDialog : public QDialog, private Ui::QgsOSMImportDialog
{
    Q_OBJECT
  public:
    explicit QgsOSMImportDialog( QWidget* parent = 0 );
    ~QgsOSMImportDialog();

  private slots:
    void onBrowseXml();
    void onBrowseDb();

    void xmlFileNameChanged( const QString& fileName );
    void dbFileNameChanged( const QString& fileName );

    void onOK();
    void onClose();

    void onProgress( int percent );

  private:
    QgsOSMXmlImport* mImport;
};

#endif // QGSOSMIMPORTDIALOG_H
