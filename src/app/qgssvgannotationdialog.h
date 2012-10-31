/***************************************************************************
                              qgssvgannotationdialog.h
                              ------------------------
  begin                : November, 2012
  copyright            : (C) 2012 by Marco Hugentobler
  email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSVGANNOTATIONDIALOG_H
#define QGSSVGANNOTATIONDIALOG_H

#include "ui_qgsformannotationdialogbase.h"

class QgsSVGAnnotationItem;
class QgsAnnotationWidget;

class QgsSVGAnnotationDialog: public QDialog, private Ui::QgsFormAnnotationDialogBase
{
    Q_OBJECT
    public:
        QgsSVGAnnotationDialog( QgsSVGAnnotationItem* item, QWidget * parent = 0, Qt::WindowFlags f = 0);
        ~QgsSVGAnnotationDialog();

    private slots:
        void on_mBrowseToolButton_clicked();
        void applySettingsToItem();
        void deleteItem();

    private:
        QgsSVGAnnotationDialog(); //forbidden

        QgsSVGAnnotationItem* mItem;
        QgsAnnotationWidget* mEmbeddedWidget;
};

#endif // QGSSVGANNOTATIONDIALOG_H
