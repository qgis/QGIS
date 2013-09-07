/***************************************************************************
                              qgstextannotationdialog.h
                              ------------------------
  begin                : February 24, 2010
  copyright            : (C) 2010 by Marco Hugentobler
  email                : marco dot hugentobler at hugis dot net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSTEXTANNOTATIONDIALOG_H
#define QGSTEXTANNOTATIONDIALOG_H

#include "ui_qgstextannotationdialogbase.h"

class QgsAnnotationWidget;
class QgsTextAnnotationItem;

class APP_EXPORT QgsTextAnnotationDialog: public QDialog, private Ui::QgsTextAnnotationDialogBase
{
    Q_OBJECT
  public:
    QgsTextAnnotationDialog( QgsTextAnnotationItem* item, QWidget * parent = 0, Qt::WindowFlags f = 0 );
    ~QgsTextAnnotationDialog();

  private:
    QgsTextAnnotationItem* mItem;
    /**Text document (a clone of the annotation items document)*/
    QTextDocument* mTextDocument;
    QgsAnnotationWidget* mEmbeddedWidget;

    void blockAllSignals( bool block );

  private slots:
    void applyTextToItem();
    void changeCurrentFormat();
    void on_mFontColorButton_colorChanged( const QColor& color );
    void setCurrentFontPropertiesToGui();
    void deleteItem();
};

#endif // QGSTEXTANNOTATIONDIALOG_H
