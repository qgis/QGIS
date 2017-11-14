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
#include "qgis_app.h"
#include <memory>

class QgsAnnotationWidget;
class QgsMapCanvasAnnotationItem;

class APP_EXPORT QgsTextAnnotationDialog: public QDialog, private Ui::QgsTextAnnotationDialogBase
{
    Q_OBJECT
  public:
    QgsTextAnnotationDialog( QgsMapCanvasAnnotationItem *item, QWidget *parent = nullptr, Qt::WindowFlags f = 0 );

  protected:

    virtual void showEvent( QShowEvent *event ) override;

  private:
    QgsMapCanvasAnnotationItem *mItem = nullptr;
    //! Text document (a clone of the annotation items document)
    std::unique_ptr< QTextDocument > mTextDocument;
    QgsAnnotationWidget *mEmbeddedWidget = nullptr;

    void blockAllSignals( bool block );

  private slots:
    void applyTextToItem();
    void changeCurrentFormat();
    void mFontColorButton_colorChanged( const QColor &color );
    void setCurrentFontPropertiesToGui();
    void deleteItem();
    void mButtonBox_clicked( QAbstractButton *button );
    void backgroundColorChanged( const QColor &color );
};

#endif // QGSTEXTANNOTATIONDIALOG_H
