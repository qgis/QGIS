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
#include "qgscomposermap.h"
class QgsMapLayer;

/** \ingroup MapComposer
 * Input widget for the configuration of QgsComposerMap
 * */
class QgsComposerMapWidget: public QWidget, private Ui::QgsComposerMapWidgetBase
{
    Q_OBJECT

  public:

    QgsComposerMapWidget( QgsComposerMap* composerMap );
    virtual ~QgsComposerMapWidget();

  public slots:
    void on_mPreviewModeComboBox_activated( int i );
    void on_mScaleLineEdit_editingFinished();
    void on_mRotationSpinBox_valueChanged( double value );
    void on_mSetToMapCanvasExtentButton_clicked();
    void on_mUpdatePreviewButton_clicked();
    void on_mKeepLayerListCheckBox_stateChanged( int state );
    void on_mDrawCanvasItemsCheckBox_stateChanged( int state );
    void on_mOverviewFrameMapComboBox_currentIndexChanged( const QString& text );
    void on_mOverviewFrameStyleButton_clicked();
    void on_mOverviewBlendModeComboBox_currentIndexChanged( int index );
    void on_mOverviewInvertCheckbox_toggled( bool state );

    void on_mXMinLineEdit_editingFinished();
    void on_mXMaxLineEdit_editingFinished();
    void on_mYMinLineEdit_editingFinished();
    void on_mYMaxLineEdit_editingFinished();

    void on_mGridCheckBox_toggled( bool state );
    void on_mIntervalXSpinBox_editingFinished();
    void on_mIntervalYSpinBox_editingFinished();
    void on_mOffsetXSpinBox_editingFinished();
    void on_mOffsetYSpinBox_editingFinished();
    void on_mLineWidthSpinBox_valueChanged( double d );
    void on_mGridLineStyleButton_clicked();
    void on_mGridTypeComboBox_currentIndexChanged( const QString& text );
    void on_mCrossWidthSpinBox_valueChanged( double d );
    void on_mAnnotationFontButton_clicked();
    void on_mDistanceToMapFrameSpinBox_valueChanged( double d );

    void on_mAnnotationFormatComboBox_currentIndexChanged( int index );

    //annotation position
    void on_mAnnotationPositionLeftComboBox_currentIndexChanged( const QString& text );
    void on_mAnnotationPositionRightComboBox_currentIndexChanged( const QString& text );
    void on_mAnnotationPositionTopComboBox_currentIndexChanged( const QString& text );
    void on_mAnnotationPositionBottomComboBox_currentIndexChanged( const QString& text );

    //annotation direction
    void on_mAnnotationDirectionComboBoxLeft_currentIndexChanged( const QString& text );
    void on_mAnnotationDirectionComboBoxRight_currentIndexChanged( const QString& text );
    void on_mAnnotationDirectionComboBoxTop_currentIndexChanged( const QString& text );
    void on_mAnnotationDirectionComboBoxBottom_currentIndexChanged( const QString& text );

    void on_mDrawAnnotationCheckableGroupBox_toggled( bool state );
    void on_mCoordinatePrecisionSpinBox_valueChanged( int value );

    void on_mFrameStyleComboBox_currentIndexChanged( const QString& text );
    void on_mFrameWidthSpinBox_valueChanged( double d );

  protected:
    void showEvent( QShowEvent * event );

    void addPageToToolbox( QWidget * widget, const QString& name );

    /**Sets the current composer map values to the GUI elements*/
    virtual void updateGuiElements();

  private slots:

    /**Sets the GUI elements to the values of mPicture*/
    void setGuiElementValues();

  private:
    QgsComposerMap* mComposerMap;

    /**Sets extent of composer map from line edits*/
    void updateComposerExtentFromGui();

    /**Blocks / unblocks the signals of all GUI elements*/
    void blockAllSignals( bool b );

    void handleChangedAnnotationPosition( QgsComposerMap::Border border, const QString& text );
    void handleChangedAnnotationDirection( QgsComposerMap::Border border, const QString& text );

    void insertAnnotationPositionEntries( QComboBox* c );
    void insertAnnotationDirectionEntries( QComboBox* c );

    void initAnnotationPositionBox( QComboBox* c, QgsComposerMap::GridAnnotationPosition pos );
    void initAnnotationDirectionBox( QComboBox* c, QgsComposerMap::GridAnnotationDirection dir );

    void updateOverviewSymbolMarker();
    void updateLineSymbolMarker();

    /**Updates the map combo box with the current composer map ids*/
    void refreshMapComboBox();
};

#endif
