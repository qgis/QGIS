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
/* $Id$ */

#ifndef QGSVECTORLAYERPROPERTIES
#define QGSVECTORLAYERPROPERTIES

#include "ui_qgsvectorlayerpropertiesbase.h"
#include "qgisgui.h"
#include "qgsrenderer.h"
#include "qgsaddattrdialog.h"
#include "qgsdelattrdialog.h"
#include "qgsattributetypedialog.h"
#include "qgsfield.h"
#include "qgsmapcanvas.h"
#include "qgscontexthelp.h"

class QgsMapLayer;

class QgsAttributeActionDialog;
class QgsApplyDialog;
class QgsLabelDialog;
class QgsVectorLayer;
class QgsVectorOverlayPlugin;

class QgsVectorLayerProperties : public QDialog, private Ui::QgsVectorLayerPropertiesBase
{
    Q_OBJECT

  public:
    QgsVectorLayerProperties( QgsVectorLayer *lyr = 0, QWidget *parent = 0, Qt::WFlags fl = QgisGui::ModalDialogFlags );
    ~QgsVectorLayerProperties();
    /**Sets the legend type to "single symbol", "graduated symbol" or "continuous color"*/
    void setLegendType( QString type );
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

    void attributeTypeDialog();

    void alterLayerDialog( const QString& string );

    /** Reset to original (vector layer) values */
    void reset();

    /** Get metadata about the layer in nice formatted html */
    QString metadata();

    /** Set transparency based on slider position */
    void sliderTransparency_valueChanged( int theValue );

    /** Toggles on the label check box */
    void setLabelCheckBox();

    /** Called when apply button is pressed or dialog is accepted */
    void apply();

    /** toggle editing of layer */
    void toggleEditing();

    /** editing of layer was toggled */
    void editingToggled();

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
    void on_tblAttributes_cellChanged( int row, int column );
    void on_mCalculateFieldButton_clicked();
    void on_pbnSelectEditForm_clicked();
    void on_stackedWidget_currentChanged( int idx );
    void on_buttonBox_helpRequested() { QgsContextHelp::run( metaObject()->className() ); }

    void addAttribute();
    void deleteAttribute();

    void attributeAdded( int idx );
    void attributeDeleted( int idx );

    void useNewSymbology();
    void setUsingNewSymbology( bool useNewSymbology );

    void on_mButtonAddJoin_clicked();
    void on_mButtonRemoveJoin_clicked();

  signals:

    /** emitted when changes to layer were saved to update legend */
    void refreshLegend( QString layerID, bool expandItem );

    void toggleEditing( QgsMapLayer * );

  protected:

    void updateSymbologyPage();

    enum attrColumns
    {
      attrIdCol = 0,
      attrNameCol,
      attrTypeCol,
      attrLengthCol,
      attrPrecCol,
      attrCommentCol,
      attrEditTypeCol,
      attrAliasCol,
      attrColCount,
    };

    QgsVectorLayer *layer;

    bool mMetadataFilled;

    /**Renderer dialog which is shown*/
    QDialog* mRendererDialog;
    /**Buffer renderer, which is assigned to the vector layer when apply is pressed*/
    //QgsRenderer* bufferRenderer;
    /**Label dialog. If apply is pressed, options are applied to vector's QgsLabel */
    QgsLabelDialog* labelDialog;
    /**Actions dialog. If apply is pressed, the actions are stored for later use */
    QgsAttributeActionDialog* actionDialog;

    QList<QgsApplyDialog*> mOverlayDialogs;
    QMap<int, QPushButton*> mButtonMap;
    QMap<int, QgsVectorLayer::EditType> mEditTypeMap;
    QMap<int, QMap<QString, QVariant> > mValueMaps;
    QMap<int, QgsVectorLayer::RangeData> mRanges;
    QMap<int, QPair<QString, QString> > mCheckedStates;

    void updateButtons();
    void loadRows();
    void setRow( int row, int idx, const QgsField &field );

    /**Requests all overlay plugis from the plugin registry. Useful for inserting their dialogs as new tabs*/
    QList<QgsVectorOverlayPlugin*> overlayPlugins() const;

    /**Buffer pixmap which takes the picture of renderers before they are assigned to the vector layer*/
    //QPixmap bufferPixmap;

    /**Adds a new join to mJoinTreeWidget*/
    void addJoinToTreeWidget( QgsVectorJoinInfo& join );

    static QMap< QgsVectorLayer::EditType, QString > editTypeMap;
    static void setupEditTypes();
    static QString editTypeButtonText( QgsVectorLayer::EditType type );
    static QgsVectorLayer::EditType editTypeFromButtonText( QString text );
};

inline QString QgsVectorLayerProperties::displayName()
{
  return txtDisplayName->text();
}
#endif
