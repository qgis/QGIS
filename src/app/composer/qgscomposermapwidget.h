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
#include "qgscomposermapgrid.h"
#include "qgscomposeritemwidget.h"

class QgsMapLayer;

/** \ingroup MapComposer
 * Input widget for the configuration of QgsComposerMap
 * */
class QgsComposerMapWidget: public QgsComposerItemBaseWidget, private Ui::QgsComposerMapWidgetBase
{
    Q_OBJECT

  public:

    QgsComposerMapWidget( QgsComposerMap* composerMap );
    virtual ~QgsComposerMapWidget();

  public slots:
    void on_mPreviewModeComboBox_activated( int i );
    void on_mScaleLineEdit_editingFinished();
    void on_mMapRotationSpinBox_valueChanged( double value );
    void on_mSetToMapCanvasExtentButton_clicked();
    void on_mViewExtentInCanvasButton_clicked();
    void on_mUpdatePreviewButton_clicked();
    void on_mKeepLayerListCheckBox_stateChanged( int state );
    void on_mKeepLayerStylesCheckBox_stateChanged( int state );
    void on_mDrawCanvasItemsCheckBox_stateChanged( int state );
    void on_mOverviewFrameMapComboBox_currentIndexChanged( const QString& text );
    void on_mOverviewFrameStyleButton_clicked();
    void on_mOverviewBlendModeComboBox_currentIndexChanged( int index );
    void on_mOverviewInvertCheckbox_toggled( bool state );
    void on_mOverviewCenterCheckbox_toggled( bool state );

    void on_mXMinLineEdit_editingFinished();
    void on_mXMaxLineEdit_editingFinished();
    void on_mYMinLineEdit_editingFinished();
    void on_mYMaxLineEdit_editingFinished();

    void on_mAtlasMarginRadio_toggled( bool checked );

    void on_mAtlasCheckBox_toggled( bool checked );
    void on_mAtlasMarginSpinBox_valueChanged( int value );
    void on_mAtlasFixedScaleRadio_toggled( bool checked );
    void on_mAtlasPredefinedScaleRadio_toggled( bool checked );

    void on_mAddGridPushButton_clicked();
    void on_mRemoveGridPushButton_clicked();
    void on_mGridUpButton_clicked();
    void on_mGridDownButton_clicked();

    QgsComposerMapGrid* currentGrid();
    void on_mGridCheckBox_toggled( bool state );
    void on_mGridListWidget_currentItemChanged( QListWidgetItem* current, QListWidgetItem* previous );
    void on_mGridListWidget_itemChanged( QListWidgetItem* item );
    void setGridItemsEnabled( bool enabled );
    void setGridItems( const QgsComposerMapGrid* grid );
    void blockGridItemsSignals( bool block );
    void on_mGridLineStyleButton_clicked();
    void on_mGridMarkerStyleButton_clicked();
    void on_mIntervalXSpinBox_editingFinished();
    void on_mIntervalYSpinBox_editingFinished();
    void on_mOffsetXSpinBox_valueChanged( double value );
    void on_mOffsetYSpinBox_valueChanged( double value );
    void on_mCrossWidthSpinBox_valueChanged( double val );
    void on_mFrameWidthSpinBox_valueChanged( double val );
    void on_mFrameStyleComboBox_currentIndexChanged( const QString& text );
    void on_mGridFramePenSizeSpinBox_valueChanged( double d );
    void on_mGridFramePenColorButton_colorChanged( const QColor& newColor );
    void on_mGridFrameFill1ColorButton_colorChanged( const QColor& newColor );
    void on_mGridFrameFill2ColorButton_colorChanged( const QColor& newColor );
    void on_mGridTypeComboBox_currentIndexChanged( const QString& text );
    void on_mMapGridCRSButton_clicked();
    void on_mMapGridUnitComboBox_currentIndexChanged( const QString& text );
    void on_mGridBlendComboBox_currentIndexChanged( int index );
    void on_mCheckGridLeftSide_toggled( bool checked );
    void on_mCheckGridRightSide_toggled( bool checked );
    void on_mCheckGridTopSide_toggled( bool checked );
    void on_mCheckGridBottomSide_toggled( bool checked );

    //frame divisions display
    void on_mFrameDivisionsLeftComboBox_currentIndexChanged( int index );
    void on_mFrameDivisionsRightComboBox_currentIndexChanged( int index );
    void on_mFrameDivisionsTopComboBox_currentIndexChanged( int index );
    void on_mFrameDivisionsBottomComboBox_currentIndexChanged( int index );

    void on_mDrawAnnotationGroupBox_toggled( bool state );

    //annotation display
    void on_mAnnotationDisplayLeftComboBox_currentIndexChanged( const QString& text );
    void on_mAnnotationDisplayRightComboBox_currentIndexChanged( const QString& text );
    void on_mAnnotationDisplayTopComboBox_currentIndexChanged( const QString& text );
    void on_mAnnotationDisplayBottomComboBox_currentIndexChanged( const QString& text );

    //annotation position
    void on_mAnnotationPositionLeftComboBox_currentIndexChanged( const QString& text );
    void on_mAnnotationPositionRightComboBox_currentIndexChanged( const QString& text );
    void on_mAnnotationPositionTopComboBox_currentIndexChanged( const QString& text );
    void on_mAnnotationPositionBottomComboBox_currentIndexChanged( const QString& text );

    //annotation direction
    void on_mAnnotationDirectionComboBoxLeft_currentIndexChanged( int index );
    void on_mAnnotationDirectionComboBoxRight_currentIndexChanged( int index );
    void on_mAnnotationDirectionComboBoxTop_currentIndexChanged( int index );
    void on_mAnnotationDirectionComboBoxBottom_currentIndexChanged( int index );

    void on_mAnnotationFormatComboBox_currentIndexChanged( int index );
    void on_mCoordinatePrecisionSpinBox_valueChanged( int value );
    void on_mDistanceToMapFrameSpinBox_valueChanged( double d );
    void on_mAnnotationFontButton_clicked();
    void on_mAnnotationFontColorButton_colorChanged( const QColor &color );

    //overviews
    void on_mAddOverviewPushButton_clicked();
    void on_mRemoveOverviewPushButton_clicked();
    void on_mOverviewUpButton_clicked();
    void on_mOverviewDownButton_clicked();
    QgsComposerMapOverview* currentOverview();
    void on_mOverviewCheckBox_toggled( bool state );
    void on_mOverviewListWidget_currentItemChanged( QListWidgetItem* current, QListWidgetItem* previous );
    void on_mOverviewListWidget_itemChanged( QListWidgetItem* item );
    void setOverviewItemsEnabled( bool enabled );
    void setOverviewItems( const QgsComposerMapOverview* overview );
    void blockOverviewItemsSignals( bool block );

  protected:
    void showEvent( QShowEvent * event ) override;

    void addPageToToolbox( QWidget * widget, const QString& name );

    /**Sets the current composer map values to the GUI elements*/
    virtual void updateGuiElements();

    QgsComposerObject::DataDefinedProperty ddPropertyForWidget( QgsDataDefinedButton *widget ) override;

  protected slots:
    /**Initializes data defined buttons to current atlas coverage layer*/
    void populateDataDefinedButtons();

  private slots:

    /**Sets the GUI elements to the values of mPicture*/
    void setGuiElementValues();

    /**Enables or disables the atlas margin around feature option depending on coverage layer type*/
    void atlasLayerChanged( QgsVectorLayer* layer );

    /**Enables or disables the atlas controls when composer atlas is toggled on/off*/
    void compositionAtlasToggled( bool atlasEnabled );

    void aboutToShowVisibilityPresetsMenu();

    void visibilityPresetSelected();

  private:
    QgsComposerMap* mComposerMap;

    /**Sets extent of composer map from line edits*/
    void updateComposerExtentFromGui();

    /**Blocks / unblocks the signals of all GUI elements*/
    void blockAllSignals( bool b );

    void handleChangedFrameDisplay( QgsComposerMapGrid::BorderSide border, const QgsComposerMapGrid::DisplayMode mode );
    void handleChangedAnnotationDisplay( QgsComposerMapGrid::BorderSide border, const QString& text );
    void handleChangedAnnotationPosition( QgsComposerMapGrid::BorderSide border, const QString& text );
    void handleChangedAnnotationDirection( QgsComposerMapGrid::BorderSide border, const QgsComposerMapGrid::AnnotationDirection &direction );

    void insertFrameDisplayEntries( QComboBox* c );
    void insertAnnotationDisplayEntries( QComboBox* c );
    void insertAnnotationPositionEntries( QComboBox* c );
    void insertAnnotationDirectionEntries( QComboBox* c );

    void initFrameDisplayBox( QComboBox* c, QgsComposerMapGrid::DisplayMode display );
    void initAnnotationDisplayBox( QComboBox* c, QgsComposerMapGrid::DisplayMode display );
    void initAnnotationPositionBox( QComboBox* c, QgsComposerMapGrid::AnnotationPosition pos );
    void initAnnotationDirectionBox( QComboBox* c, QgsComposerMapGrid::AnnotationDirection dir );

    void updateGridLineSymbolMarker( const QgsComposerMapGrid* grid );
    void updateGridMarkerSymbolMarker( const QgsComposerMapGrid* grid );

    /**Updates the map combo box with the current composer map ids*/
    void refreshMapComboBox();

    /**Enables/disables grid frame related controls*/
    void toggleFrameControls( bool frameEnabled, bool frameFillEnabled, bool frameSizeEnabled );

    /**Enables or disables the atlas margin and predefined scales radio depending on the atlas coverage layer type*/
    void toggleAtlasScalingOptionsByLayerType();

    /**Recalculates the bounds for an atlas map when atlas properties change*/
    void updateMapForAtlas();

    /**Is there some predefined scales, globally or as project's options ?*/
    bool hasPredefinedScales() const;

    QListWidgetItem* addGridListItem( const QString& id, const QString& name );

    void loadGridEntries();

    QListWidgetItem* addOverviewListItem( const QString& id, const QString& name );

    void loadOverviewEntries();

    void updateOverviewFrameSymbolMarker( const QgsComposerMapOverview* overview );
};

#endif
