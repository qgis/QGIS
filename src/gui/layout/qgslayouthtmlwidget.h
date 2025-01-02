/***************************************************************************
    qgslayouthtmlwidget.h
    ---------------------
    begin                : November 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSLAYOUTHTMLWIDGET_H
#define QGSLAYOUTHTMLWIDGET_H

// We don't want to expose this in the public API
#define SIP_NO_FILE

#include "qgis_gui.h"
#include "ui_qgslayouthtmlwidgetbase.h"
#include "qgslayoutitemwidget.h"

class QgsLayoutItemHtml;
class QgsLayoutFrame;
class QgsCodeEditorHTML;
class QgsCodeEditorCSS;

/**
 * \ingroup gui
 * \brief A widget for configuring layout html items.
 *
 * \note This class is not a part of public API
 * \since QGIS 3.12
 */
class GUI_EXPORT QgsLayoutHtmlWidget : public QgsLayoutItemBaseWidget, private Ui::QgsLayoutHtmlWidgetBase
{
    Q_OBJECT
  public:
    QgsLayoutHtmlWidget() = delete;
    //! constructor
    QgsLayoutHtmlWidget( QgsLayoutFrame *frame );
    void setMasterLayout( QgsMasterLayoutInterface *masterLayout ) override;

  protected:
    bool setNewItem( QgsLayoutItem *item ) override;

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
    void mAddFramePushButton_clicked();
    void mEmptyFrameCheckBox_toggled( bool checked );
    void mHideEmptyBgCheckBox_toggled( bool checked );

    //! Sets the GUI elements to the values of mHtmlItem
    void setGuiElementValues();

  protected slots:
    //! Initializes data defined buttons to current atlas coverage layer
    void populateDataDefinedButtons();

  private:
    void blockSignals( bool block );

    QPointer<QgsLayoutItemHtml> mHtml;
    QPointer<QgsLayoutFrame> mFrame;
    QgsLayoutItemPropertiesWidget *mItemPropertiesWidget = nullptr;

    QgsCodeEditorHTML *mHtmlEditor = nullptr;
    QgsCodeEditorCSS *mStylesheetEditor = nullptr;
};

#endif // QGSLAYOUTHTMLWIDGET_H
