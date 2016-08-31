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
#include "qgisgui.h"
#include "qgscontexthelp.h"
#include "qgsmaplayerstylemanager.h"
#include "qgsvectorlayer.h"
#include "layertree/qgslayertreemodel.h"
#include "layertree/qgslayertreegroup.h"

class QgsMapLayer;

class QgsAttributeActionDialog;
class QgsApplyDialog;
class QgsVectorLayer;
class QgsLabelingWidget;
class QgsDiagramProperties;
class QgsFieldsProperties;
class QgsRendererPropertiesDialog;
class QgsMapLayerConfigWidgetFactory;
class QgsMapLayerConfigWidget;
class QgsPanelWidget;

class APP_EXPORT QgsVectorLayerProperties : public QgsOptionsDialogBase, private Ui::QgsVectorLayerPropertiesBase, private QgsExpressionContextGenerator
{
    Q_OBJECT

  public:
    enum StyleType
    {
      QML = 0,
      SLD,
      DB,
    };

    QgsVectorLayerProperties( QgsVectorLayer *lyr = nullptr, QWidget *parent = nullptr, Qt::WindowFlags fl = QgisGui::ModalDialogFlags );
    ~QgsVectorLayerProperties();
    /** Returns the display name entered in the dialog*/
    QString displayName();
    void setRendererDirty( bool ) {}

    /** Adds an attribute to the table (but does not commit it yet)
    @param field the field to add
    @return false in case of a name conflict, true in case of success */
    bool addAttribute( const QgsField &field );

    /** Deletes an attribute (but does not commit it)
      @param name attribute name
      @return false in case of a non-existing attribute.*/
    bool deleteAttribute( int attr );

    /** Adds a properties page factory to the vector layer properties dialog. */
    void addPropertiesPageFactory( QgsMapLayerConfigWidgetFactory *factory );

  public slots:

    void insertFieldOrExpression();

    /** Reset to original (vector layer) values */
    void syncToLayer();

    /** Get metadata about the layer in nice formatted html */
    QString metadata();

    /** Slot to update layer display name as original is edited */
    void on_mLayerOrigNameLineEdit_textEdited( const QString& text );

    /** Called when apply button is pressed or dialog is accepted */
    void apply();

    /** Called when cancel button is pressed */
    void onCancel();

    //
    //methods reimplemented from qt designer base class
    //

    void on_pbnQueryBuilder_clicked();
    void on_pbnIndex_clicked();
    void on_mCrsSelector_crsChanged( const QgsCoordinateReferenceSystem& crs );
    void loadDefaultStyle_clicked();
    void saveDefaultStyle_clicked();
    void loadStyle_clicked();
    void saveStyleAs_clicked();
    void mOptionsStackedWidget_CurrentChanged( int indx );
    void on_buttonBox_helpRequested() { QgsContextHelp::run( metaObject()->className() ); }
    void on_pbnUpdateExtents_clicked();

    void on_mButtonAddJoin_clicked();
    void on_mButtonEditJoin_clicked();
    void on_mJoinTreeWidget_itemDoubleClicked( QTreeWidgetItem *item, int column );
    void on_mButtonRemoveJoin_clicked();

    void on_mSimplifyDrawingGroupBox_toggled( bool checked );

  signals:

    /** Emitted when changes to layer were saved to update legend */
    void refreshLegend( const QString& layerID, bool expandItem );
    void refreshLegend( const QString& layerID );

    void toggleEditing( QgsMapLayer * );

  private slots:
    /** Toggle editing of layer */
    void toggleEditing();

    /** Save the style based on selected format from the menu */
    void saveStyleAsMenuTriggered( QAction * );

    /** Called when is possible to choice if load the style from filesystem or from db */
    void loadStyleMenuTriggered( QAction * );

    void aboutToShowStyleMenu();

    /** Updates the variable editor to reflect layer changes
     */
    void updateVariableEditor();

    /**
     * @brief updates the FieldsPropertiesDialog when syncing the layer properties
     */
    void updateFieldsPropertiesDialog();

  private:

    void saveStyleAs( StyleType styleType );

    /** When provider supports, it will list all the styles relative the layer in a dialog */
    void showListOfStylesFromDatabase();

    void updateSymbologyPage();

    void setPbnQueryBuilderEnabled();

    QgsVectorLayer *mLayer;

    bool mMetadataFilled;

    QString mOriginalSubsetSQL;

    QMenu *mSaveAsMenu;
    QMenu *mLoadStyleMenu;

    QAction* mActionLoadStyle;
    QAction* mActionSaveStyleAs;

    /** Renderer dialog which is shown*/
    QgsRendererPropertiesDialog* mRendererDialog;
    /** Labeling dialog. If apply is pressed, options are applied to vector's QgsLabel */
    QgsLabelingWidget* labelingDialog;
    /** Actions dialog. If apply is pressed, the actions are stored for later use */
    QgsAttributeActionDialog* mActionDialog;
    /** Diagram dialog. If apply is pressed, options are applied to vector's diagrams*/
    QgsDiagramProperties* diagramPropertiesDialog;
    /** Fields dialog. If apply is pressed, options are applied to vector's diagrams*/
    QgsFieldsProperties* mFieldsPropertiesDialog;

    //! List of joins of a layer at the time of creation of the dialog. Used to return joins to previous state if dialog is cancelled
    QList< QgsVectorJoinInfo > mOldJoins;

    //! A list of additional pages provided by plugins
    QList<QgsMapLayerConfigWidget*> mLayerPropertiesPages;

    /** Previous layer style. Used to reset style to previous state if new style
     * was loaded but dialog is cancelled */
    QgsMapLayerStyle mOldStyle;

    void initDiagramTab();

    /** Adds a new join to mJoinTreeWidget*/
    void addJoinToTreeWidget( const QgsVectorJoinInfo& join , const int insertIndex = -1 );

    QgsExpressionContext mContext;

    QgsExpressionContext createExpressionContext() const override;

    QScopedPointer<QgsLayerTreeGroup> mLayersDependenciesTreeGroup;
    QScopedPointer<QgsLayerTreeModel> mLayersDependenciesTreeModel;

  private slots:
    void openPanel( QgsPanelWidget* panel );
};

inline QString QgsVectorLayerProperties::displayName()
{
  return txtDisplayName->text();
}
#endif
