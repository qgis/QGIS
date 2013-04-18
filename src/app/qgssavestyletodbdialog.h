/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSADDATTRDIALOG_H
#define QGSADDATTRDIALOG_H

#include "ui_qgssavetodbdialog.h"
#include "qgisgui.h"
#include "qgsfield.h"

class QgsSaveStyleToDbDialog : public QDialog, private Ui::QgsSaveToDBDialog
{
    Q_OBJECT
public:
    explicit QgsSaveStyleToDbDialog(QWidget *parent = 0);
    
signals:
public slots:
    QString getName();
    QString getDescription();
    bool isDefault();
};

#endif // QGSSAVESTYLETODBDIALOG_H
