/***************************************************************************
                          qgsspatialquerydialog.h
                             -------------------
    begin                : Dec 29, 2009
    copyright            : (C) 2009 by Diego Moreira And Luiz Motta
    email                : moreira.geo at gmail.com And motta.luiz at gmail.com

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/*  $Id$ */

#ifndef SPATIALQUERYDIALOG_H
#define SPATIALQUERYDIALOG_H

#include "qgsrubberselectid.h"

#include "ui_qgsspatialquerydialogbase.h"
#include "qgisinterface.h"
#include "qgsvectorlayer.h"

/**
* \brief Enum with feature listwidget
* \enum Feature_Widget
*
*/
enum Feature_Widget
{
  FW_Result,
  FW_InvalidTarget,
  FW_InvalidRefence
};


/**
* \class QgsSpatialQueryDialog
* \brief Spatial Query dialog
*
*/
class QgsSpatialQueryDialog : public QDialog, private Ui::QgsSpatialQueryDialogBase
{
    Q_OBJECT
  public:
    /**
    * Constructor for a dialog. The QgisInterface pointer is passed by
    * QGIS when it attempts to instantiate the plugin.
    * @param iface Pointer to the QgisInterface object.
    */
    QgsSpatialQueryDialog( QWidget *parent = 0, QgisInterface* iface = 0 );
    //! Destructor
    ~QgsSpatialQueryDialog();

    //! Message about number layers less Two
    static void messageLayersLessTwo();

    //! Unload plugins by QGIS - Disconnect signal from QGIS
    void disconnectQGis();

    //! Override show for ajust size
    void show();

  private slots:
    //! Slots for signs of Dialog
    void on_buttonBoxMain_accepted();
    void on_buttonBoxMain_rejected();
    void on_targetLayerComboBox_currentIndexChanged( int index );
    void on_referenceLayerComboBox_currentIndexChanged( int index );
    void on_resultFeatureTargetListWidget_itemClicked( QListWidgetItem * item );
    void on_resultFeatureTargetListWidget_currentItemChanged( QListWidgetItem * item );
    void on_invalidFeatureTargetListWidget_itemClicked( QListWidgetItem * item );
    void on_invalidFeatureTargetListWidget_currentItemChanged( QListWidgetItem * item );
    void on_invalidFeatureReferenceListWidget_itemClicked( QListWidgetItem * item );
    void on_invalidFeatureReferenceListWidget_currentItemChanged( QListWidgetItem * item );
    void on_ckboxLogProcessing_clicked( bool checked );
    void on_pushButtonSelectResultTarget_clicked();
    void on_pushButtonSelectInvalidTarget_clicked();
    void on_pushButtonSelectInvalidReference_clicked();

    //! Slots for signs of QGIS
    void signal_qgis_layerWasAdded( QgsMapLayer* mapLayer );
    void signal_qgis_layerWillBeRemoved( QString idLayer );

    //! Slots for signs of Layers (Target or Reference)
    void signal_layerTarget_selectionFeaturesChanged();
    void signal_layerReference_selectionFeaturesChanged();

  private:
    //! Initialize the Gui
    void initGui();
    //! Set Color RGB for select - mRGBRubberSelect
    void setColorRubberSelect();
    //! Set Layer (Target or Reference)
    void setLayer( bool isTarget, int index );
    //! Evaluate status of selected features from layer (Target or Reference)
    void evaluateCheckBox( bool isTarget );
    //! Run Query
    void runQuery();
    //! Show Log Processing
    void showLogProcessing( bool hasShow );
    //! Show result of query
    void showResultQuery( QDateTime *datetimeStart, QDateTime *datetimeEnd );
    //! Get Description Layer to show result
    QString getDescriptionLayerShow( bool isTarget );
    //! Connect all slots
    void connectAll();
    //! Disconnect all slots
    void disconnectAll();
    //! reject - override
    void reject();
    //! Get Vector layer from combobox
    QgsVectorLayer * getLayerFromCombobox( bool isTarget, int index );
    //! Get Icon for vector layer
    QIcon getIconTypeGeometry( QGis::GeometryType geomType );
    //! Add layer in combobox (text, data and  tooltips)
    void addLayerCombobox( bool isTarget, QgsVectorLayer* vectorLayer );
    //! Find Layer in combobox
    int getIndexLayerCombobox( bool isTarget, QgsVectorLayer* vectorLayer );
    //! Remove layer in combobox and setting GUI
    void removeLayer( bool isTarget, QgsVectorLayer* lyrRemove );
    //! Populates targetLayerComboBox with all layers
    void populateTargetLayerComboBox();
    //! Populates referenceLayerComboBox with all layers except the current target layer
    void populateReferenceLayerComboBox();
    //! Populates operationComboBox with the topological operations
    void populateOperationComboBox();
    //! Populates the features in QListWidget (use by result, invalid target and reference)
    void populateFeatureListWidget( QListWidget *listWidget, QSet<int> & setFeatures, bool hasSetRow = true );
    //! Clear the features of QListWidget (use by result, invalid target and reference)
    void clearFeatureListWidget( QListWidget *listWidget );
    //! Make action when change item in ListWidget
    void changeFeatureListWidget( QListWidget *listWidget, QgsVectorLayer* vectorLayer, const QString& currentText );
    //! Show rubber from feature
    void showRubberFeature( QgsVectorLayer* vectorLayer, int id );

    //! Pointer to Interface QGIS
    QgisInterface* mIface;
    //! Target Layer, the query will be performed on it
    QgsVectorLayer* mLayerTarget;
    //! Reference Layer, the query will be based on it
    QgsVectorLayer* mLayerReference;
    //! Stores ID's from spatial query
    QSet<int> mFeatureResult;
    //! Stores ID's invalid of target layer
    QSet<int> mFeatureInvalidTarget;
    //! Stores ID's invalid of reference layer
    QSet<int> mFeatureInvalidReference;
    //! Map for Id name of vector layers (use in signal_qgis_layerWillBeRemoved)
    QMap<QString, QgsVectorLayer *> mMapIdVectorLayers;
    //! Rubber band for features result
    QgsRubberSelectId* mRubberSelectId;
    //! RGB select feature result
    int mRGBRubberSelect[3];
    //! Current Feature Widget
    Feature_Widget mCurrentFeatureWidget;

    // Message
    QString mMsgLayersLessTwo;

    void MsgDEBUG( QString sMSg );

    //! show/hide operation inputs: target, reference and operation group box
    void setLayoutOperationVisible( bool show );
    //! show/hide result of operation: result and invalid group box
    void setLayoutResultInvalid( bool show );
};

#endif // SPATIALQUERYDIALOG_H
