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
#include "qgscomposeritemwidget.h"
#include "qgscomposermapgrid.h"

class QgsMapLayer;
class QgsComposerMapOverview;

/**
 * \ingroup app
 * Input widget for the configuration of QgsComposerMap
 * */
class QgsComposerMapWidget: public QgsComposerItemBaseWidget, private Ui::QgsComposerMapWidgetBase
{
    Q_OBJECT

  public:
    explicit QgsComposerMapWidget( QgsComposerMap *composerMap );

  public slots:
    void mScaleLineEdit_editingFinished();
    void mSetToMapCanvasExtentButton_clicked();
    void mViewExtentInCanvasButton_clicked();
    void mUpdatePreviewButton_clicked();
    void mFollowVisibilityPresetCheckBox_stateChanged( int state );
    void mKeepLayerListCheckBox_stateChanged( int state );
    void mKeepLayerStylesCheckBox_stateChanged( int state );
    void mDrawCanvasItemsCheckBox_stateChanged( int state );
    void overviewMapChanged( QgsComposerItem *item );
    void mOverviewFrameStyleButton_clicked();
    void mOverviewBlendModeComboBox_currentIndexChanged( int index );
    void mOverviewInvertCheckbox_toggled( bool state );
    void mOverviewCenterCheckbox_toggled( bool state );

    void mXMinLineEdit_editingFinished();
    void mXMaxLineEdit_editingFinished();
    void mYMinLineEdit_editingFinished();
    void mYMaxLineEdit_editingFinished();

    void mAtlasMarginRadio_toggled( bool checked );

    void mAtlasCheckBox_toggled( bool checked );
    void mAtlasMarginSpinBox_valueChanged( int value );
    void mAtlasFixedScaleRadio_toggled( bool checked );
    void mAtlasPredefinedScaleRadio_toggled( bool checked );

    void mAddGridPushButton_clicked();
    void mRemoveGridPushButton_clicked();
    void mGridUpButton_clicked();
    void mGridDownButton_clicked();

    QgsComposerMapGrid *currentGrid();
    void mDrawGridCheckBox_toggled( bool state );
    void mGridListWidget_currentItemChanged( QListWidgetItem *current, QListWidgetItem *previous );
    void mGridListWidget_itemChanged( QListWidgetItem *item );
    void mGridPropertiesButton_clicked();

    //overviews
    void mAddOverviewPushButton_clicked();
    void mRemoveOverviewPushButton_clicked();
    void mOverviewUpButton_clicked();
    void mOverviewDownButton_clicked();
    QgsComposerMapOverview *currentOverview();
    void mOverviewCheckBox_toggled( bool state );
    void mOverviewListWidget_currentItemChanged( QListWidgetItem *current, QListWidgetItem *previous );
    void mOverviewListWidget_itemChanged( QListWidgetItem *item );
    void setOverviewItemsEnabled( bool enabled );
    void setOverviewItems( const QgsComposerMapOverview *overview );
    void blockOverviewItemsSignals( bool block );

  protected:

    void addPageToToolbox( QWidget *widget, const QString &name );

    //! Sets the current composer map values to the GUI elements
    virtual void updateGuiElements();

  protected slots:
    //! Initializes data defined buttons to current atlas coverage layer
    void populateDataDefinedButtons();

  private slots:

    //! Sets the GUI elements to the values of mPicture
    void setGuiElementValues();

    //! Enables or disables the atlas margin around feature option depending on coverage layer type
    void atlasLayerChanged( QgsVectorLayer *layer );

    //! Enables or disables the atlas controls when composer atlas is toggled on/off
    void compositionAtlasToggled( bool atlasEnabled );

    void aboutToShowKeepLayersVisibilityPresetsMenu();

    void followVisibilityPresetSelected( int currentIndex );
    void keepLayersVisibilityPresetSelected();

    void onMapThemesChanged();

    void updateOverviewFrameStyleFromWidget();
    void cleanUpOverviewFrameStyleSelector( QgsPanelWidget *container );

    void mapCrsChanged( const QgsCoordinateReferenceSystem &crs );

  private:
    QgsComposerMap *mComposerMap = nullptr;

    //! Sets extent of composer map from line edits
    void updateComposerExtentFromGui();

    //! Blocks / unblocks the signals of all GUI elements
    void blockAllSignals( bool b );

    void rotationChanged( double value );

    void handleChangedFrameDisplay( QgsComposerMapGrid::BorderSide border, const QgsComposerMapGrid::DisplayMode mode );
    void handleChangedAnnotationDisplay( QgsComposerMapGrid::BorderSide border, const QString &text );
    void handleChangedAnnotationPosition( QgsComposerMapGrid::BorderSide border, const QString &text );
    void handleChangedAnnotationDirection( QgsComposerMapGrid::BorderSide border, QgsComposerMapGrid::AnnotationDirection direction );

    void insertFrameDisplayEntries( QComboBox *c );
    void insertAnnotationDisplayEntries( QComboBox *c );
    void insertAnnotationPositionEntries( QComboBox *c );
    void insertAnnotationDirectionEntries( QComboBox *c );

    void initFrameDisplayBox( QComboBox *c, QgsComposerMapGrid::DisplayMode display );
    void initAnnotationDisplayBox( QComboBox *c, QgsComposerMapGrid::DisplayMode display );
    void initAnnotationPositionBox( QComboBox *c, QgsComposerMapGrid::AnnotationPosition pos );
    void initAnnotationDirectionBox( QComboBox *c, QgsComposerMapGrid::AnnotationDirection dir );

    //! Enables or disables the atlas margin and predefined scales radio depending on the atlas coverage layer type
    void toggleAtlasScalingOptionsByLayerType();

    //! Recalculates the bounds for an atlas map when atlas properties change
    void updateMapForAtlas();

    //! Is there some predefined scales, globally or as project's options ?
    bool hasPredefinedScales() const;

    QListWidgetItem *addGridListItem( const QString &id, const QString &name );

    void loadGridEntries();

    QListWidgetItem *addOverviewListItem( const QString &id, const QString &name );

    void loadOverviewEntries();

    void updateOverviewFrameSymbolMarker( const QgsComposerMapOverview *overview );

    void storeCurrentLayerSet();

};

#endif
