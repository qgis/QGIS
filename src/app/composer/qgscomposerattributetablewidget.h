/***************************************************************************
                         qgscomposerattributetablewidget.h
                         ---------------------------------
    begin                : September 2014
    copyright            : (C) 2014 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCOMPOSERATTRIBUTETABLEWIDGET_H
#define QGSCOMPOSERATTRIBUTETABLEWIDGET_H

#include "ui_qgscomposerattributetablewidgetbase.h"
#include "qgscomposeritemwidget.h"

class QgsComposerAttributeTableV2;
class QgsComposerFrame;

class QgsComposerAttributeTableWidget: public QgsComposerItemBaseWidget, private Ui::QgsComposerAttributeTableWidgetBase
{
    Q_OBJECT
  public:
    QgsComposerAttributeTableWidget( QgsComposerAttributeTableV2* table, QgsComposerFrame* frame );
    ~QgsComposerAttributeTableWidget();

  protected:
    void showEvent( QShowEvent * event ) override;

  private:
    QgsComposerAttributeTableV2* mComposerTable;
    QgsComposerFrame* mFrame;

    /** Blocks / unblocks the signals of all GUI elements*/
    void blockAllSignals( bool b );
    void refreshMapComboBox();

    void toggleSourceControls();

    void toggleAtlasSpecificControls( const bool atlasEnabled );

  private slots:
    void on_mRefreshPushButton_clicked();
    void on_mAttributesPushButton_clicked();
    void on_mComposerMapComboBox_activated( int index );
    void on_mMaximumRowsSpinBox_valueChanged( int i );
    void on_mMarginSpinBox_valueChanged( double d );
    void on_mGridStrokeWidthSpinBox_valueChanged( double d );
    void on_mGridColorButton_colorChanged( const QColor& newColor );
    void on_mBackgroundColorButton_colorChanged( const QColor &newColor );
    void on_mHeaderFontPushButton_clicked();
    void on_mHeaderFontColorButton_colorChanged( const QColor& newColor );
    void on_mContentFontPushButton_clicked();
    void on_mContentFontColorButton_colorChanged( const QColor& newColor );
    void on_mShowGridGroupCheckBox_toggled( bool state );
    void on_mShowOnlyVisibleFeaturesCheckBox_stateChanged( int state );
    void on_mFeatureFilterCheckBox_stateChanged( int state );
    void on_mFeatureFilterEdit_editingFinished();
    void on_mFeatureFilterButton_clicked();
    void on_mHeaderHAlignmentComboBox_currentIndexChanged( int index );
    void on_mHeaderModeComboBox_currentIndexChanged( int index );
    void on_mWrapStringLineEdit_editingFinished();
    void changeLayer( QgsMapLayer* layer );
    void on_mAddFramePushButton_clicked();
    void on_mResizeModeComboBox_currentIndexChanged( int index );
    void on_mSourceComboBox_currentIndexChanged( int index );
    void on_mRelationsComboBox_currentIndexChanged( int index );
    void on_mEmptyModeComboBox_currentIndexChanged( int index );
    void on_mDrawEmptyCheckBox_toggled( bool checked );
    void on_mEmptyMessageLineEdit_editingFinished();
    void on_mIntersectAtlasCheckBox_stateChanged( int state );
    void on_mUniqueOnlyCheckBox_stateChanged( int state );
    void on_mEmptyFrameCheckBox_toggled( bool checked );
    void on_mHideEmptyBgCheckBox_toggled( bool checked );
    void on_mWrapBehaviourComboBox_currentIndexChanged( int index );

    /** Inserts a new maximum number of features into the spin box (without the spinbox emitting a signal)*/
    void setMaximumNumberOfFeatures( int n );

    /** Sets the GUI elements to the values of mComposerTable*/
    void updateGuiElements();

    void atlasToggled();

    void updateRelationsCombo();

};

#endif // QGSCOMPOSERATTRIBUTETABLEWIDGET_H
