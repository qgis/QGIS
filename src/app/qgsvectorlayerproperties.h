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

#include "qgsoptionsdialogbase.h"
#include "ui_qgsvectorlayerpropertiesbase.h"
#include "qgsguiutils.h"
#include "qgshelp.h"
#include "qgsmaplayerstylemanager.h"
#include "qgsvectorlayerjoininfo.h"
#include "layertree/qgslayertree.h"
#include "layertree/qgslayertreemodel.h"
#include "layertree/qgslayertreegroup.h"
#include "qgis_app.h"

class QgsMapLayer;

class QgsAttributeActionDialog;
class QgsApplyDialog;
class QgsVectorLayer;
class QgsLabelingWidget;
class QgsDiagramProperties;
class QgsSourceFieldsProperties;
class QgsAttributesFormProperties;
class QgsRendererPropertiesDialog;
class QgsMapLayerConfigWidgetFactory;
class QgsMapLayerConfigWidget;
class QgsMetadataWidget;
class QgsPanelWidget;
class QgsVectorLayer3DRendererWidget;

class APP_EXPORT QgsVectorLayerProperties : public QgsOptionsDialogBase, private Ui::QgsVectorLayerPropertiesBase, private QgsExpressionContextGenerator
{
    Q_OBJECT

  public:
    enum StyleType
    {
      QML,
      SLD,
      DB,
    };
    Q_ENUM( StyleType )

    QgsVectorLayerProperties( QgsVectorLayer *lyr = nullptr, QWidget *parent = nullptr, Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags );

    //! Returns the display name entered in the dialog
    QString displayName()
    {
      return txtDisplayName->text();
    }

    /**
     * Adds an attribute to the layer.
     * The layer will need to be in edit mode. It will only be added to the provider when the edit buffer
     * is committed.
     * \param field the field to add
     * \returns false in case of a name conflict, true in case of success
     */
    bool addAttribute( const QgsField &field );

    /**
     * Deletes an attribute.
     * The layer will need to be in edit mode. It will only be added to the provider when the edit buffer
     * is committed.
     * \param name attribute name
     * \returns false in case of a non-existing attribute.
     */
    bool deleteAttribute( int attr );

    //! Adds a properties page factory to the vector layer properties dialog.
    void addPropertiesPageFactory( QgsMapLayerConfigWidgetFactory *factory );

  public slots:

    void insertFieldOrExpression();

    //! Reset to original (vector layer) values
    void syncToLayer();

    //! Gets metadata about the layer in nice formatted html
    QString htmlMetadata();

    //! Slot to update layer display name as original is edited
    void mLayerOrigNameLineEdit_textEdited( const QString &text );

    //! Called when apply button is pressed or dialog is accepted
    void apply();

    //! Called when cancel button is pressed
    void onCancel();

    //
    //methods reimplemented from qt designer base class
    //

    void pbnQueryBuilder_clicked();
    void pbnIndex_clicked();
    void mCrsSelector_crsChanged( const QgsCoordinateReferenceSystem &crs );
    void loadDefaultStyle_clicked();
    void saveDefaultStyle_clicked();
    void loadMetadata();
    void saveMetadataAs();
    void saveDefaultMetadata();
    void loadDefaultMetadata();
    void optionsStackedWidget_CurrentChanged( int index ) override;
    void pbnUpdateExtents_clicked();

    void mButtonAddJoin_clicked();
    void mButtonEditJoin_clicked();
    void mJoinTreeWidget_itemDoubleClicked( QTreeWidgetItem *item, int column );
    void mButtonRemoveJoin_clicked();

    void mSimplifyDrawingGroupBox_toggled( bool checked );

  signals:

    void toggleEditing( QgsMapLayer * );

  private slots:
    //! Toggle editing of layer
    void toggleEditing();

    //! Save the style
    void saveStyleAs();

    //! Load the style
    void loadStyle();

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

    void onAuxiliaryLayerExport();

    void urlClicked( const QUrl &url );

  private:

    enum PropertyType
    {
      Style = 0,
      Metadata,
    };

    void updateSymbologyPage();

    void setPbnQueryBuilderEnabled();

    QgsVectorLayer *mLayer = nullptr;

    bool mMetadataFilled = false;

    QString mOriginalSubsetSQL;

    QPushButton *mBtnStyle = nullptr;
    QPushButton *mBtnMetadata = nullptr;
    QAction *mActionLoadMetadata = nullptr;
    QAction *mActionSaveMetadataAs = nullptr;

    QMenu *mLoadStyleMenu = nullptr;

    QAction *mActionLoadStyle = nullptr;
    QAction *mActionSaveStyle = nullptr;

    //! Renderer dialog which is shown
    QgsRendererPropertiesDialog *mRendererDialog = nullptr;
    //! Labeling dialog. If apply is pressed, options are applied to vector's QgsLabel
    QgsLabelingWidget *labelingDialog = nullptr;
    //! Actions dialog. If apply is pressed, the actions are stored for later use
    QgsAttributeActionDialog *mActionDialog = nullptr;
    //! Diagram dialog. If apply is pressed, options are applied to vector's diagrams
    QgsDiagramProperties *diagramPropertiesDialog = nullptr;
    //! SourceFields dialog. If apply is pressed, options are applied to vector's diagrams
    QgsSourceFieldsProperties *mSourceFieldsPropertiesDialog = nullptr;
    //! AttributesForm dialog. If apply is pressed, options are applied to vector's diagrams
    QgsAttributesFormProperties *mAttributesFormPropertiesDialog = nullptr;

    //! List of joins of a layer at the time of creation of the dialog. Used to return joins to previous state if dialog is canceled
    QList< QgsVectorLayerJoinInfo > mOldJoins;

    //! A list of additional pages provided by plugins
    QList<QgsMapLayerConfigWidget *> mLayerPropertiesPages;

    /**
     * Previous layer style. Used to reset style to previous state if new style
     * was loaded but dialog is canceled */
    QgsMapLayerStyle mOldStyle;

    void initDiagramTab();

    //! Adds a new join to mJoinTreeWidget
    void addJoinToTreeWidget( const QgsVectorLayerJoinInfo &join, int insertIndex = -1 );

    void updateAuxiliaryStoragePage( bool reset = false );
    void deleteAuxiliaryField( int index );

    QgsExpressionContext mContext;

    QgsExpressionContext createExpressionContext() const override;

    std::unique_ptr<QgsLayerTree> mLayersDependenciesTreeGroup;
    std::unique_ptr<QgsLayerTreeModel> mLayersDependenciesTreeModel;

    void showHelp();

    QgsMetadataWidget *mMetadataWidget = nullptr;

    QAction *mAuxiliaryLayerActionNew = nullptr;
    QAction *mAuxiliaryLayerActionClear = nullptr;
    QAction *mAuxiliaryLayerActionDelete = nullptr;
    QAction *mAuxiliaryLayerActionExport = nullptr;
    QAction *mAuxiliaryLayerActionDeleteField = nullptr;
    QAction *mAuxiliaryLayerActionAddField = nullptr;

    QgsVectorLayer3DRendererWidget *mVector3DWidget = nullptr;

  private slots:
    void openPanel( QgsPanelWidget *panel );

    friend class QgsAppScreenShots;
};


#endif
