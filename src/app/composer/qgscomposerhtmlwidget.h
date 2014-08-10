/***************************************************************************
    qgscomposerhtmlwidget.h
    ---------------------
    begin                : July 2012
    copyright            : (C) 2012 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSCOMPOSERHTMLWIDGET_H
#define QGSCOMPOSERHTMLWIDGET_H

#include "ui_qgscomposerhtmlwidgetbase.h"
#include "qgscomposeritemwidget.h"

class QgsComposerHtml;
class QgsComposerFrame;
class QgsCodeEditorHTML;
class QgsCodeEditorCSS;

class QgsComposerHtmlWidget: public QgsComposerItemBaseWidget, private Ui::QgsComposerHtmlWidgetBase
{
    Q_OBJECT
  public:
    QgsComposerHtmlWidget( QgsComposerHtml* html, QgsComposerFrame* frame );
    ~QgsComposerHtmlWidget();

  private slots:
    void on_mUrlLineEdit_editingFinished();
    void on_mFileToolButton_clicked();
    void on_mResizeModeComboBox_currentIndexChanged( int index );
    void on_mEvaluateExpressionsCheckbox_toggled( bool checked );
    void on_mUseSmartBreaksCheckBox_toggled( bool checked );
    void on_mMaxDistanceSpinBox_valueChanged( double val );
    void htmlEditorChanged();
    void stylesheetEditorChanged();
    void on_mUserStylesheetCheckBox_toggled( bool checked );
    void on_mRadioManualSource_clicked( bool checked );
    void on_mRadioUrlSource_clicked( bool checked );
    void on_mInsertExpressionButton_clicked();

    void on_mReloadPushButton_clicked();
    void on_mReloadPushButton2_clicked();
    void on_mAddFramePushButton_clicked();

    /**Sets the GUI elements to the values of mHtmlItem*/
    void setGuiElementValues();

  protected:
    QgsComposerItem::DataDefinedProperty ddPropertyForWidget( QgsDataDefinedButton *widget );

  protected slots:
    /**Initializes data defined buttons to current atlas coverage layer*/
    void populateDataDefinedButtons();

  private:
    QgsComposerHtmlWidget();
    void blockSignals( bool block );

    QgsComposerHtml* mHtml;
    QgsComposerFrame* mFrame;
    QgsCodeEditorHTML *mHtmlEditor;
    QgsCodeEditorCSS *mStylesheetEditor;
};

#endif // QGSCOMPOSERHTMLWIDGET_H
