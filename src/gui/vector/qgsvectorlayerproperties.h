/***************************************************************************
                          qgsdlgvectorlayerproperties.h
                   Unified property dialog for vector layers
                             -------------------
    begin                : 2004-01-28
    copyright            : (C) 2004 by Gary E.Sherman
    email                : sherman at mrcc.com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVECTORLAYERPROPERTIES
#define QGSVECTORLAYERPROPERTIES

#include <QStandardItemModel>

#include "ui_qgsvectorlayerpropertiesbase.h"
#include "qgsguiutils.h"
#include "qgsmaplayerserverproperties.h"
#include "qgsvectorlayerjoininfo.h"
#include "qgslayertreefilterproxymodel.h"
#include "qgslayerpropertiesdialog.h"

class QgsMapLayer;

class QgsAttributeActionDialog;
class QgsVectorLayer;
class QgsLabelingWidget;
class QgsDiagramWidget;
class QgsSourceFieldsProperties;
class QgsAttributesFormProperties;
class QgsRendererPropertiesDialog;
class QgsMapLayerConfigWidgetFactory;
class QgsMapLayerConfigWidget;
class QgsMessageBar;
class QgsMetadataWidget;
class QgsPanelWidget;
class QgsVectorLayer3DRendererWidget;
class QgsMapLayerComboBox;
class QgsDoubleSpinBox;
class QgsMaskingWidget;
class QgsVectorLayerTemporalPropertiesWidget;
class QgsProviderSourceWidget;
class QgsWebView;

/**
 * \ingroup gui
 * \class QgsVectorLayerProperties
 */
class GUI_EXPORT QgsVectorLayerProperties : public QgsLayerPropertiesDialog, private Ui::QgsVectorLayerPropertiesBase, private QgsExpressionContextGenerator
{
    Q_OBJECT

  public:
    QgsVectorLayerProperties( QgsMapCanvas *canvas, QgsMessageBar *messageBar, QgsVectorLayer *lyr = nullptr, QWidget *parent = nullptr, Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags );

    bool eventFilter( QObject *obj, QEvent *ev ) override;

  protected slots:
    void optionsStackedWidget_CurrentChanged( int index ) final;
    void syncToLayer() FINAL;
    void apply() FINAL;
    void rollback() FINAL;

  private slots:

    void insertField();
    void insertOrEditExpression();

    //! Gets metadata about the layer in nice formatted html
    QString htmlMetadata();

    //
    //methods reimplemented from qt designer base class
    //

    void pbnQueryBuilder_clicked();
    void pbnIndex_clicked();
    void mCrsSelector_crsChanged( const QgsCoordinateReferenceSystem &crs );
    void pbnUpdateExtents_clicked();

    void mButtonAddJoin_clicked();
    void mButtonEditJoin_clicked();
    void mJoinTreeWidget_itemDoubleClicked( QTreeWidgetItem *item, int column );
    void mButtonRemoveJoin_clicked();

    // Server properties
    void addMetadataUrl();
    void removeSelectedMetadataUrl();
    void mButtonAddWmsDimension_clicked();
    void mButtonEditWmsDimension_clicked();
    void mWmsDimensionsTreeWidget_itemDoubleClicked( QTreeWidgetItem *item, int column );
    void mButtonRemoveWmsDimension_clicked();

    void mSimplifyDrawingGroupBox_toggled( bool checked );

  signals:

    void toggleEditing( QgsMapLayer * ) SIP_SKIP;

    void exportAuxiliaryLayer( QgsAuxiliaryLayer *layer ) SIP_SKIP;

  private slots:
    //! Toggle editing of layer
    void toggleEditing();

    //! Save multiple styles
    void saveMultipleStylesAs();

    void aboutToShowStyleMenu();

    /**
     * Updates the variable editor to reflect layer changes
     */
    void updateVariableEditor();

    void onAuxiliaryLayerNew();

    void onAuxiliaryLayerClear();

    void onAuxiliaryLayerDelete();

    void onAuxiliaryLayerDeleteField();

    void onAuxiliaryLayerAddField();

    // Update the preview of the map tip
    void updateMapTipPreview();
    // Resize the map tip preview
    void resizeMapTip();

  private:
    enum PropertyType
    {
      Style = 0,
      Metadata,
    };

    void updateSymbologyPage();

    void setPbnQueryBuilderEnabled();

    QgsMessageBar *mMessageBar = nullptr;
    QgsVectorLayer *mLayer = nullptr;

    bool mMetadataFilled = false;

    QString mOriginalSubsetSQL;

    QAction *mActionLoadMetadata = nullptr;
    QAction *mActionSaveMetadataAs = nullptr;

    QAction *mActionLoadStyle = nullptr;
    QAction *mActionSaveStyle = nullptr;
    QAction *mActionSaveMultipleStyles = nullptr;

    //! Renderer dialog which is shown
    QgsRendererPropertiesDialog *mRendererDialog = nullptr;
    //! Labeling dialog. If apply is pressed, options are applied to vector's QgsLabel
    QgsLabelingWidget *labelingDialog = nullptr;
    //! Masking widget
    QgsMaskingWidget *mMaskingWidget = nullptr;
    //! Actions dialog. If apply is pressed, the actions are stored for later use
    QgsAttributeActionDialog *mActionDialog = nullptr;
    //! Diagram dialog. If apply is pressed, options are applied to vector's diagrams
    QgsDiagramWidget *diagramPropertiesDialog = nullptr;
    //! SourceFields dialog. If apply is pressed, options are applied to vector's diagrams
    QgsSourceFieldsProperties *mSourceFieldsPropertiesDialog = nullptr;
    //! AttributesForm dialog. If apply is pressed, options are applied to vector's diagrams
    QgsAttributesFormProperties *mAttributesFormPropertiesDialog = nullptr;

    //! List of joins of a layer at the time of creation of the dialog. Used to return joins to previous state if dialog is canceled
    QList<QgsVectorLayerJoinInfo> mOldJoins;

    //! Adds a new join to mJoinTreeWidget
    void addJoinToTreeWidget( const QgsVectorLayerJoinInfo &join, int insertIndex = -1 );

    //! Adds a QGIS Server WMS dimension to mWmsDimensionTreeWidget
    void addWmsDimensionInfoToTreeWidget( const QgsMapLayerServerProperties::WmsDimensionInfo &wmsDim, int insertIndex = -1 );
    QStandardItemModel *mMetadataUrlModel = nullptr;

    void updateAuxiliaryStoragePage();
    void deleteAuxiliaryField( int index );

    QgsExpressionContext mContext;

    QgsExpressionContext createExpressionContext() const override;

    QgsLayerTreeFilterProxyModel *mLayersDependenciesTreeModel;

    void showHelp();

    QgsMetadataWidget *mMetadataWidget = nullptr;

    QAction *mAuxiliaryLayerActionNew = nullptr;
    QAction *mAuxiliaryLayerActionClear = nullptr;
    QAction *mAuxiliaryLayerActionDelete = nullptr;
    QAction *mAuxiliaryLayerActionExport = nullptr;
    QAction *mAuxiliaryLayerActionDeleteField = nullptr;
    QAction *mAuxiliaryLayerActionAddField = nullptr;

    QgsVectorLayer3DRendererWidget *mVector3DWidget = nullptr;

    QgsVectorLayerTemporalPropertiesWidget *mTemporalWidget = nullptr;

    QgsProviderSourceWidget *mSourceWidget = nullptr;

    QgsCoordinateReferenceSystem mBackupCrs;

    std::unique_ptr<QgsProjectDirtyBlocker> mProjectDirtyBlocker;

    void initMapTipPreview();

    QgsWebView *mMapTipPreview = nullptr;

  private slots:
    void openPanel( QgsPanelWidget *panel );

    friend class QgsAppScreenShots;
    friend class TestQgsLayerPropertiesDialogs;
};

#endif
