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
#include "qgis_app.h"

class QgsMapCanvasAnnotationItem;
class QgsAnnotationWidget;

class APP_EXPORT QgsSvgAnnotationDialog: public QDialog, private Ui::QgsFormAnnotationDialogBase
{
    Q_OBJECT
  public:
    QgsSvgAnnotationDialog( QgsMapCanvasAnnotationItem *item, QWidget *parent = nullptr, Qt::WindowFlags f = 0 );

  private slots:
    void mBrowseToolButton_clicked();
    void applySettingsToItem();
    void deleteItem();
    void mButtonBox_clicked( QAbstractButton *button );

  private:
    QgsSvgAnnotationDialog() = delete; //forbidden

    QgsMapCanvasAnnotationItem *mItem = nullptr;
    QgsAnnotationWidget *mEmbeddedWidget = nullptr;
};

#endif // QGSSVGANNOTATIONDIALOG_H
