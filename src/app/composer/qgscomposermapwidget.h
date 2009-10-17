/***************************************************************************
                         qgscomposermapwidget.h
                         ----------------------
    begin                : May 26, 2008
    copyright            : (C) 2008 by Marco Hugentobler
    email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCOMPOSERMAPWIDGET_H
#define QGSCOMPOSERMAPWIDGET_H

#include "ui_qgscomposermapwidgetbase.h"

class QgsComposerMap;

/** \ingroup MapComposer
 * Input widget for the configuration of QgsComposerMap
 * */
class QgsComposerMapWidget: public QWidget, private Ui::QgsComposerMapWidgetBase
{
    Q_OBJECT

  public:

    QgsComposerMapWidget( QgsComposerMap* composerMap );
    ~QgsComposerMapWidget();

  public slots:
    void on_mWidthLineEdit_editingFinished();
    void on_mHeightLineEdit_editingFinished();
    void on_mPreviewModeComboBox_activated( int i );
    void on_mScaleLineEdit_editingFinished();
    void on_mSetToMapCanvasExtentButton_clicked();
    void on_mUpdatePreviewButton_clicked();
    void on_mKeepLayerListCheckBox_stateChanged( int state );

    void on_mXMinLineEdit_editingFinished();
    void on_mXMaxLineEdit_editingFinished();
    void on_mYMinLineEdit_editingFinished();
    void on_mYMaxLineEdit_editingFinished();

    void on_mGridCheckBox_stateChanged( int state );
    void on_mIntervalXSpinBox_editingFinished();
    void on_mIntervalYSpinBox_editingFinished();
    void on_mOffsetXSpinBox_editingFinished();
    void on_mOffsetYSpinBox_editingFinished();
    void on_mLineWidthSpinBox_valueChanged( double d );
    void on_mLineColorButton_clicked();
    void on_mGridTypeComboBox_currentIndexChanged( const QString& text );
    void on_mAnnotationFontButton_clicked();
    void on_mDistanceToMapFrameSpinBox_valueChanged( double d );
    void on_mAnnotationPositionComboBox_currentIndexChanged( const QString& text );
    void on_mDrawAnnotationCheckBox_stateChanged( int state );
    void on_mAnnotationDirectionComboBox_currentIndexChanged( const QString& text );
    void on_mAnnotationTypeComboBox_currentIndexChanged( const QString& text );
    void on_mShowGridDialogCheckBox_stateChanged( int state );

    /**Updates width and height without notify the composer map (to avoid infinite recursion)*/
    void updateSettingsNoSignals();

  private:
    QgsComposerMap* mComposerMap;

    /**Sets the current composer map values to the GUI elements*/
    void updateGuiElements();

    /**Sets extent of composer map from line edits*/
    void updateComposerExtentFromGui();

    /**Blocks / unblocks the signals of all GUI elements*/
    void blockAllSignals( bool b );
};

#endif
