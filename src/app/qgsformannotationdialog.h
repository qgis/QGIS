/***************************************************************************
    qgsformannotationdialog.h
    ---------------------
    begin                : March 2010
    copyright            : (C) 2010 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSFORMANNOTATIONDIALOG_H
#define QGSFORMANNOTATIONDIALOG_H

#include "ui_qgsformannotationdialogbase.h"
#include "qgsformannotationitem.h"

class QgsAnnotationWidget;

class QgsFormAnnotationDialog: public QDialog, private Ui::QgsFormAnnotationDialogBase
{
    Q_OBJECT
  public:
    QgsFormAnnotationDialog( QgsFormAnnotationItem* item, QWidget * parent = 0, Qt::WindowFlags f = 0 );
    ~QgsFormAnnotationDialog();

  private:
    QgsFormAnnotationItem* mItem;
    QgsAnnotationWidget* mEmbeddedWidget;

  private slots:
    void applySettingsToItem();
    void on_mBrowseToolButton_clicked();
    void deleteItem();
};

#endif // QGSFORMANNOTATIONDIALOG_H
