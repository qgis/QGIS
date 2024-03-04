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

// We don't want to expose this in the public API
#define SIP_NO_FILE

#include "qgis_gui.h"
#include "ui_qgslayoutmapwidgetbase.h"
#include "ui_qgslayoutmaplabelingwidgetbase.h"
#include "ui_qgslayoutmapclippingwidgetbase.h"
#include "qgslayoutitemwidget.h"
#include "qgslayoutitemmapgrid.h"

class QgsMapLayer;
class QgsLayoutItemMap;
class QgsLayoutItemMapOverview;
class QgsLayoutMapLabelingWidget;
class QgsLayoutMapClippingWidget;
class QgsBookmarkManagerProxyModel;

/**
 * \ingroup gui
 * \brief Input widget for the configuration of QgsLayoutItemMap
 *
 * \note This class is not a part of public API
 * \since QGIS 3.12
 */
class GUI_EXPORT QgsLayoutMapWidget: public QgsLayoutItemBaseWidget, private Ui::QgsLayoutMapWidgetBase
{
    Q_OBJECT

  public:
    //! constructor
    explicit QgsLayoutMapWidget( QgsLayoutItemMap *item, QgsMapCanvas *mapCanvas );
    void setMasterLayout( QgsMasterLayoutInterface *masterLayout ) override;

    void setReportTypeString( const QString &string ) override;
    void setDesignerInterface( QgsLayoutDesignerInterface *iface ) override;

  private slots:
    void mScaleLineEdit_editingFinished();
    void setToMapCanvasExtent();
    void setToMapCanvasScale();
    void viewExtentInCanvas();
    void viewScaleInCanvas();
    void updatePreview();
    void mFollowVisibilityPresetCheckBox_stateChanged( int state );
    void mKeepLayerListCheckBox_stateChanged( int state );
    void mKeepLayerStylesCheckBox_stateChanged( int state );
    void mDrawCanvasItemsCheckBox_stateChanged( int state );
    void overviewMapChanged( QgsLayoutItem *item );
    void mOverviewBlendModeComboBox_currentIndexChanged( int index );
    void mOverviewInvertCheckbox_toggled( bool state );
    void mOverviewCenterCheckbox_toggled( bool state );
    void overviewStackingChanged( int value );
    void overviewStackingLayerChanged( QgsMapLayer *layer );

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

    QgsLayoutItemMapGrid *currentGrid();
    void mGridListWidget_currentItemChanged( QListWidgetItem *current, QListWidgetItem *previous );
    void mGridListWidget_itemChanged( QListWidgetItem *item );
    void mGridPropertiesButton_clicked();

    //overviews
    void mAddOverviewPushButton_clicked();
    void mRemoveOverviewPushButton_clicked();
    void mOverviewUpButton_clicked();
    void mOverviewDownButton_clicked();
    QgsLayoutItemMapOverview *currentOverview();
    void mOverviewCheckBox_toggled( bool state );
    void mOverviewListWidget_currentItemChanged( QListWidgetItem *current, QListWidgetItem *previous );
    void mOverviewListWidget_itemChanged( QListWidgetItem *item );
    void setOverviewItemsEnabled( bool enabled );
    void setOverviewItems( QgsLayoutItemMapOverview *overview );
    void blockOverviewItemsSignals( bool block );

    void mTemporalCheckBox_toggled( bool checked );
    void updateTemporalExtent();

    void mElevationRangeCheckBox_toggled( bool checked );
    void updateZRange();

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

    void mapCrsChanged( const QgsCoordinateReferenceSystem &crs );
    void overviewSymbolChanged();
    void showLabelSettings();
    void showClipSettings();
    void switchToMoveContentTool();
    void aboutToShowBookmarkMenu();

  private:
    QPointer< QgsLayoutItemMap > mMapItem;
    QgsMapCanvas *mMapCanvas = nullptr;
    QgsLayoutItemPropertiesWidget *mItemPropertiesWidget = nullptr;
    QgsLayoutDesignerInterface *mInterface = nullptr;
    QPointer< QgsLayoutMapLabelingWidget > mLabelWidget;
    QPointer< QgsLayoutMapClippingWidget > mClipWidget;
    QMenu *mBookmarkMenu = nullptr;
    QgsBookmarkManagerProxyModel *mBookmarkModel = nullptr;
    QString mReportTypeString;
    int mBlockThemeComboChanges = 0;

    //! Sets extent of composer map from line edits
    void updateComposerExtentFromGui();

    //! Blocks / unblocks the signals of all GUI elements
    void blockAllSignals( bool b );

    void rotationChanged( double value );

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

    void storeCurrentLayerSet();

    /**
     * Returns list of layer IDs that should be visible for particular preset.
     * The order will match the layer order from the map canvas
     */
    QList<QgsMapLayer *> orderedPresetVisibleLayers( const QString &name ) const;

};

/**
 * \ingroup gui
 * \brief Model for label blocking items
 *
 * \note This class is not a part of public API
 * \since QGIS 3.12
 */
class GUI_EXPORT QgsLayoutMapItemBlocksLabelsModel : public QSortFilterProxyModel
{
    Q_OBJECT

  public:
    //! constructor
    explicit QgsLayoutMapItemBlocksLabelsModel( QgsLayoutItemMap *map, QgsLayoutModel *layoutModel, QObject *parent = nullptr );

    int columnCount( const QModelIndex &parent = QModelIndex() ) const override;
    QVariant data( const QModelIndex &index, int role ) const override;
    bool setData( const QModelIndex &index, const QVariant &value, int role ) override;
    Qt::ItemFlags flags( const QModelIndex &index ) const override;

  protected:

    bool filterAcceptsRow( int source_row, const QModelIndex &source_parent ) const override;

  private:
    QgsLayoutModel *mLayoutModel = nullptr;
    QPointer< QgsLayoutItemMap > mMapItem;

};

/**
 * \ingroup gui
 * \brief Allows configuration of layout map labeling settings.
 *
 * \note This class is not a part of public API
 * \since QGIS 3.12
 */
class GUI_EXPORT QgsLayoutMapLabelingWidget: public QgsLayoutItemBaseWidget, private Ui::QgsLayoutMapLabelingWidgetBase
{
    Q_OBJECT

  public:
    //! constructor
    explicit QgsLayoutMapLabelingWidget( QgsLayoutItemMap *map );

  protected:
    bool setNewItem( QgsLayoutItem *item ) final;

  private slots:
    void updateGuiElements();
    void labelMarginChanged( double val );
    void labelMarginUnitsChanged();
    void showPartialsToggled( bool checked );
    void showUnplacedToggled( bool checked );

  private:
    QPointer< QgsLayoutItemMap > mMapItem;
};

/**
 * \ingroup gui
 * \brief Allows configuration of layout map clipping settings.
 *
 * \note This class is not a part of public API
 * \since QGIS 3.16
 */
class GUI_EXPORT QgsLayoutMapClippingWidget: public QgsLayoutItemBaseWidget, private Ui::QgsLayoutMapClippingWidgetBase
{
    Q_OBJECT

  public:
    //! constructor
    explicit QgsLayoutMapClippingWidget( QgsLayoutItemMap *map );

    void setReportTypeString( const QString &string ) override;

  protected:
    bool setNewItem( QgsLayoutItem *item ) final;

  private slots:
    void updateGuiElements();
    void atlasLayerChanged( QgsVectorLayer *layer );
    void atlasToggled( bool atlasEnabled );
    void selectAll();
    void deselectAll();
    void invertSelection();
    void toggleLayersSelectionGui( bool toggled );

  private:
    QPointer< QgsLayoutItemMap > mMapItem;
    QgsMapLayerModel *mLayerModel = nullptr;

    bool mBlockUpdates = false;
};

#endif
