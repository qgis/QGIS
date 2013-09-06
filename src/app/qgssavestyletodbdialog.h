/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSAVESTYLETODBDIALOG_H
#define QGSSAVESTYLETODBDIALOG_H

#include "ui_qgssavetodbdialog.h"
#include "qgisgui.h"
#include "qgsfield.h"

class APP_EXPORT QgsSaveStyleToDbDialog : public QDialog, private Ui::QgsSaveToDBDialog
{
    QString mUIFileContent;
    Q_OBJECT
  public:
    explicit QgsSaveStyleToDbDialog( QWidget *parent = 0 );

  signals:

  public slots:
    QString getUIFileContent();
    QString getName();
    QString getDescription();
    bool isDefault();
    void on_mFilePickButton_clicked();
    void accept();
};

#endif // QGSSAVESTYLETODBDIALOG_H
