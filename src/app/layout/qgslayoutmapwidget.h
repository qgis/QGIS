/***************************************************************************
                         qgslayoutmapwidget.h
                         ----------------------
    begin                : October 2017
    copyright            : (C) 2017 by Nyall Dawson
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

#ifndef QGSLAYOUTMAPWIDGET_H
#define QGSLAYOUTMAPWIDGET_H

#include "ui_qgslayoutmapwidgetbase.h"
#include "qgslayoutitemwidget.h"

class QgsMapLayer;
class QgsLayoutItemMap;
class QgsLayoutMapGrid;
class QgsLayoutMapOverview;

/**
 * \ingroup app
 * Input widget for the configuration of QgsLayoutItemMap
*/
class QgsLayoutMapWidget: public QgsLayoutItemBaseWidget, private Ui::QgsLayoutMapWidgetBase
{
    Q_OBJECT

  public:
    explicit QgsLayoutMapWidget( QgsLayoutItemMap *item );

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

    QgsLayoutMapGrid *currentGrid();
    void mDrawGridCheckBox_toggled( bool state );
    void mGridListWidget_currentItemChanged( QListWidgetItem *current, QListWidgetItem *previous );
    void mGridListWidget_itemChanged( QListWidgetItem *item );
    void mGridPropertiesButton_clicked();

    //overviews
    void mAddOverviewPushButton_clicked();
    void mRemoveOverviewPushButton_clicked();
    void mOverviewUpButton_clicked();
    void mOverviewDownButton_clicked();
    QgsLayoutMapOverview *currentOverview();
    void mOverviewCheckBox_toggled( bool state );
    void mOverviewListWidget_currentItemChanged( QListWidgetItem *current, QListWidgetItem *previous );
    void mOverviewListWidget_itemChanged( QListWidgetItem *item );
    void setOverviewItemsEnabled( bool enabled );
    void setOverviewItems( const QgsLayoutMapOverview *overview );
    void blockOverviewItemsSignals( bool block );

  protected:
    bool setNewItem( QgsLayoutItem *item ) override;

  protected slots:
    //! Initializes data defined buttons to current atlas coverage layer
    void populateDataDefinedButtons();

  private slots:

    //! Sets the current composer map values to the GUI elements
    void updateGuiElements();

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
    QgsLayoutItemMap *mMapItem = nullptr;
    QgsLayoutItemPropertiesWidget *mItemPropertiesWidget = nullptr;

    //! Sets extent of composer map from line edits
    void updateComposerExtentFromGui();

    //! Blocks / unblocks the signals of all GUI elements
    void blockAllSignals( bool b );

    void rotationChanged( double value );

#if 0 //TODO
    void handleChangedFrameDisplay( QgsComposerMapGrid::BorderSide border, const QgsComposerMapGrid::DisplayMode mode );
    void handleChangedAnnotationDisplay( QgsComposerMapGrid::BorderSide border, const QString &text );
    void handleChangedAnnotationPosition( QgsComposerMapGrid::BorderSide border, const QString &text );
    void handleChangedAnnotationDirection( QgsComposerMapGrid::BorderSide border, QgsComposerMapGrid::AnnotationDirection direction );
#endif
    void insertFrameDisplayEntries( QComboBox *c );
    void insertAnnotationDisplayEntries( QComboBox *c );
    void insertAnnotationPositionEntries( QComboBox *c );
    void insertAnnotationDirectionEntries( QComboBox *c );

#if 0 //TODO
    void initFrameDisplayBox( QComboBox *c, QgsComposerMapGrid::DisplayMode display );
    void initAnnotationDisplayBox( QComboBox *c, QgsComposerMapGrid::DisplayMode display );
    void initAnnotationPositionBox( QComboBox *c, QgsComposerMapGrid::AnnotationPosition pos );
    void initAnnotationDirectionBox( QComboBox *c, QgsComposerMapGrid::AnnotationDirection dir );
#endif
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

    void updateOverviewFrameSymbolMarker( const QgsLayoutMapOverview *overview );

    void storeCurrentLayerSet();

};

#endif
