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
#ifndef QgsHTMLAnnotationDialog_H
#define QgsHTMLAnnotationDialog_H

#include "ui_qgsformannotationdialogbase.h"
#include "qgshtmlannotationitem.h"

class QgsAnnotationWidget;

class APP_EXPORT QgsHtmlAnnotationDialog: public QDialog, private Ui::QgsFormAnnotationDialogBase
{
    Q_OBJECT
  public:
    QgsHtmlAnnotationDialog( QgsHtmlAnnotationItem* item, QWidget * parent = 0, Qt::WindowFlags f = 0 );
    ~QgsHtmlAnnotationDialog();

  private:
    QgsHtmlAnnotationItem* mItem;
    QgsAnnotationWidget* mEmbeddedWidget;

  private slots:
    void applySettingsToItem();
    void on_mBrowseToolButton_clicked();
    void deleteItem();
};

#endif // QgsHTMLAnnotationDialog_H
