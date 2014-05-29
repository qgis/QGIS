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

class QgsComposerHtml;
class QgsComposerFrame;

class QgsComposerHtmlWidget: public QWidget, private Ui::QgsComposerHtmlWidgetBase
{
    Q_OBJECT
  public:
    QgsComposerHtmlWidget( QgsComposerHtml* html, QgsComposerFrame* frame );
    ~QgsComposerHtmlWidget();

  private slots:
    void on_mUrlLineEdit_editingFinished();
    void on_mFileToolButton_clicked();
    void on_mResizeModeComboBox_currentIndexChanged( int index );
    void on_mUseSmartBreaksCheckBox_toggled( bool checked );
    void on_mMaxDistanceSpinBox_valueChanged( double val );

    void on_mReloadPushButton_clicked();
    void on_mAddFramePushButton_clicked();

    /**Sets the GUI elements to the values of mHtmlItem*/
    void setGuiElementValues();

  private:
    QgsComposerHtmlWidget();
    void blockSignals( bool block );

    QgsComposerHtml* mHtml;
    QgsComposerFrame* mFrame;
};

#endif // QGSCOMPOSERHTMLWIDGET_H
