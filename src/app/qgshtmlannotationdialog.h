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
#include "qgis_app.h"

class QgsAnnotationWidget;
class QgsMapCanvasAnnotationItem;

class APP_EXPORT QgsHtmlAnnotationDialog: public QDialog, private Ui::QgsFormAnnotationDialogBase
{
    Q_OBJECT
  public:
    QgsHtmlAnnotationDialog( QgsMapCanvasAnnotationItem *item, QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags() );

  private:
    QgsMapCanvasAnnotationItem *mItem = nullptr;
    QgsAnnotationWidget *mEmbeddedWidget = nullptr;

  private slots:
    void applySettingsToItem();
    void mBrowseToolButton_clicked();
    void deleteItem();
    void mButtonBox_clicked( QAbstractButton *button );
    void fileRadioButtonToggled( bool checked );
    void sourceRadioButtonToggled( bool checked );
    void showHelp();
};

#endif // QgsHTMLAnnotationDialog_H
