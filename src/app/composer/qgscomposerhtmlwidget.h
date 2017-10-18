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
    QgsComposerHtmlWidget( QgsComposerHtml *html, QgsComposerFrame *frame );

  private slots:
    void mUrlLineEdit_editingFinished();
    void mFileToolButton_clicked();
    void mResizeModeComboBox_currentIndexChanged( int index );
    void mEvaluateExpressionsCheckbox_toggled( bool checked );
    void mUseSmartBreaksCheckBox_toggled( bool checked );
    void mMaxDistanceSpinBox_valueChanged( double val );
    void htmlEditorChanged();
    void stylesheetEditorChanged();
    void mUserStylesheetCheckBox_toggled( bool checked );
    void mRadioManualSource_clicked( bool checked );
    void mRadioUrlSource_clicked( bool checked );
    void mInsertExpressionButton_clicked();

    void mReloadPushButton_clicked();
    void mReloadPushButton2_clicked();
    void mAddFramePushButton_clicked();
    void mEmptyFrameCheckBox_toggled( bool checked );
    void mHideEmptyBgCheckBox_toggled( bool checked );

    //! Sets the GUI elements to the values of mHtmlItem
    void setGuiElementValues();

  protected slots:
    //! Initializes data defined buttons to current atlas coverage layer
    void populateDataDefinedButtons();

  private:
    QgsComposerHtmlWidget();
    void blockSignals( bool block );

    QgsComposerHtml *mHtml = nullptr;
    QgsComposerFrame *mFrame = nullptr;
    QgsCodeEditorHTML *mHtmlEditor = nullptr;
    QgsCodeEditorCSS *mStylesheetEditor = nullptr;
};

#endif // QGSCOMPOSERHTMLWIDGET_H
