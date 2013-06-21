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
#include "qgsaddattrdialog.h"
#include "qgsdelattrdialog.h"
#include "qgsattributetypedialog.h"
#include "qgsfield.h"
#include "qgslegenditem.h"
#include "qgsmapcanvas.h"
#include "qgscontexthelp.h"
#include "qgsexpressionbuilderdialog.h"

class QgsMapLayer;

class QgsAttributeActionDialog;
class QgsApplyDialog;
class QgsLabelDialog;
class QgsVectorLayer;
class QgsVectorOverlayPlugin;
class QgsLabelingGui;
class QgsDiagramProperties;
class QgsFieldsProperties;

class QgsVectorLayerProperties : public QgsOptionsDialogBase, private Ui::QgsVectorLayerPropertiesBase
{
    Q_OBJECT

  public:
    enum StyleType
    {
      QML = 0,
      SLD,
      DB,
    };

    QgsVectorLayerProperties( QgsVectorLayer *lyr = 0, QWidget *parent = 0, Qt::WFlags fl = QgisGui::ModalDialogFlags );
    ~QgsVectorLayerProperties();
    /**Returns the display name entered in the dialog*/
    QString displayName();
    void setRendererDirty( bool ) {}

    /**Sets the attribute that is used in the Identify Results dialog box*/
    void setDisplayField( QString name );

    /**Adds an attribute to the table (but does not commit it yet)
    @param field the field to add
    @return false in case of a name conflict, true in case of success */
    bool addAttribute( const QgsField &field );

    /**Deletes an attribute (but does not commit it)
      @param name attribute name
      @return false in case of a non-existing attribute.*/
    bool deleteAttribute( int attr );

  public slots:

    /** Insert a field in the expression text for the map tip **/
    void insertField();

    void insertExpression();

    /** Reset to original (vector layer) values */
    void syncToLayer();

    /** Get metadata about the layer in nice formatted html */
    QString metadata();

    /** Slot to update layer display name as original is edited
     * @note added in QGIS 1.9
     */
    void on_mLayerOrigNameLineEdit_textEdited( const QString& text );

    /** Toggles on the label check box */
    void setLabelCheckBox();

    /** Called when apply button is pressed or dialog is accepted */
    void apply();

    //
    //methods reimplemented from qt designer base class
    //

    void on_pbnQueryBuilder_clicked();
    void on_pbnIndex_clicked();
    void on_pbnChangeSpatialRefSys_clicked();
    void on_pbnLoadDefaultStyle_clicked();
    void on_pbnSaveDefaultStyle_clicked();
    void on_pbnLoadStyle_clicked();
    void on_pbnSaveStyleAs_clicked();
    void mOptionsStackedWidget_CurrentChanged( int indx );
    void on_buttonBox_helpRequested() { QgsContextHelp::run( metaObject()->className() ); }
    void on_pbnUpdateExtents_clicked();

    void enableLabelOptions( bool theFlag );

    void on_mButtonAddJoin_clicked();
    void on_mButtonRemoveJoin_clicked();

    void on_mMinimumScaleSetCurrentPushButton_clicked();
    void on_mMaximumScaleSetCurrentPushButton_clicked();

  signals:

    /** emitted when changes to layer were saved to update legend */
    void refreshLegend( QString layerID, bool expandItem );
    void refreshLegend( QString layerID, QgsLegendItem::Expansion expandItem );

    void toggleEditing( QgsMapLayer * );

  private slots:
    /** toggle editing of layer */
    void toggleEditing();

    /** save the style based on selected format from the menu */
    void saveStyleAsMenuTriggered( QAction * );

    /** called when is possible to choice if load the style from filesystem or from db */
    void loadStyleMenuTriggered( QAction * );


  protected:

    void saveStyleAs( StyleType styleType );

    /** when provider supports, it will list all the styles relative the layer in a dialog */
    void showListOfStylesFromDatabase();

    void updateSymbologyPage();

    QgsVectorLayer *layer;

    bool mMetadataFilled;

    QMenu *mSaveAsMenu;
    QMenu *mLoadStyleMenu;

    /**Renderer dialog which is shown*/
    QDialog* mRendererDialog;
    /**Labeling dialog. If apply is pressed, options are applied to vector's QgsLabel */
    QgsLabelingGui* labelingDialog;
    /**Label dialog. If apply is pressed, options are applied to vector's QgsLabel */
    QgsLabelDialog* labelDialog;
    /**Actions dialog. If apply is pressed, the actions are stored for later use */
    QgsAttributeActionDialog* actionDialog;
    /**Diagram dialog. If apply is pressed, options are applied to vector's diagrams*/
    QgsDiagramProperties* diagramPropertiesDialog;
    /**Fields dialog. If apply is pressed, options are applied to vector's diagrams*/
    QgsFieldsProperties* mFieldsPropertiesDialog;


    QList<QgsApplyDialog*> mOverlayDialogs;

    void initDiagramTab();

    /**Requests all overlay plugis from the plugin registry. Useful for inserting their dialogs as new tabs*/
    QList<QgsVectorOverlayPlugin*> overlayPlugins() const;

    /**Buffer pixmap which takes the picture of renderers before they are assigned to the vector layer*/
    //QPixmap bufferPixmap;

    /**Adds a new join to mJoinTreeWidget*/
    void addJoinToTreeWidget( const QgsVectorJoinInfo& join );
};

inline QString QgsVectorLayerProperties::displayName()
{
  return txtDisplayName->text();
}
#endif
