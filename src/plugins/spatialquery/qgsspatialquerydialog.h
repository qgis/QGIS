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

#ifndef SPATIALQUERYDIALOG_H
#define SPATIALQUERYDIALOG_H

#include "qgsrubberselectid.h"

#include "ui_qgsspatialquerydialogbase.h"
#include "qgisinterface.h"
#include "qgsvectorlayer.h"

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

    //! Verify is possible execute the query
    static bool hasPossibleQuery( QString &msg );

  private slots:
    //! Slots for signs of Dialog
    void on_bbMain_clicked( QAbstractButton * button );
    void on_pbCreateLayerItems_clicked();
    void on_pbCreateLayerSelected_clicked();
    void on_cbTargetLayer_currentIndexChanged( int index );
    void on_cbReferenceLayer_currentIndexChanged( int index );
    void on_cbTypeItems_currentIndexChanged( int index );
    void on_cbResultFor_currentIndexChanged();
    void on_cbOperation_currentIndexChanged();
    void on_lwFeatures_currentItemChanged( QListWidgetItem * item );
    void on_ckbUsingSelectedTarget_toggled();
    void on_ckbLogProcessing_clicked( bool checked );
    void on_ckbZoomItem_clicked( bool checked );

    //! Slots for signs of QGIS
    void signal_qgis_layerWasAdded( QgsMapLayer* mapLayer );
    void signal_qgis_layerWillBeRemoved( QString idLayer );

    //! Slots for signs of Layers (Target or Reference)
    void signal_layerTarget_selectionFeaturesChanged();
    void signal_layerReference_selectionFeaturesChanged();

  private:
    //! Enum Type of items
    enum TypeItems { itemsResult, itemsInvalidTarget, itemsInvalidReference };
    //! Enum Type Result for
    enum TypeResultFor { selectedNew, selectedAdd, selectedRemove };
    //! Enum Type of verify subset
    enum TypeVerifyCreateSubset { verifyOk, verifyTry, verifyImpossible };

    //! Initialize the Gui
    void initGui();
    //! Apply Button
    void apply();
    //! Visible result
    void visibleResult( bool show );
    //! Set Layer (Target or Reference)
    void setLayer( bool isTarget, int index );
    //! Evaluate status of selected features from layer (Target or Reference)
    void evaluateCheckBoxLayer( bool isTarget );
    //! Run Query
    void runQuery();
    //! Show result of query
    void showResultQuery( QDateTime *datetimeStart, QDateTime *datetimeEnd );
    //! Get string subset with selected FID
    QString getSubsetFIDs( const QgsFeatureIds *fids, QString fieldFID );
    //! Verify can create layer subset
    TypeVerifyCreateSubset verifyCreateSubset( QString &msg, QString &fieldFID );
    //! Add layer target with subset
    bool addLayerSubset( QString name, QString subset );
    //! Get Description Layer to show result
    QString getDescriptionLayerShow( bool isTarget );
    //! Get Description Layer to show result
    QString getDescriptionInvalidFeaturesShow( bool isTarget );
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
    void addCbLayer( bool isTarget, QgsVectorLayer* lyr );
    //! Find Layer in combobox
    int getCbIndexLayer( bool isTarget, QgsVectorLayer* lyr );
    //! Remove layer in combobox and setting GUI
    void removeLayer( bool isTarget, QgsVectorLayer* lyr );
    //! Populate cbResultFor
    void populateCbResulFor();
    //! Populate cbTypeItems
    void populateTypeItems();
    //! Populates cbTargetLayer with all layers
    void populateCbTargetLayer();
    //! Populates cbReferenceLayer with all layers except the current target layer
    void populateCbReferenceLayer();
    //! Populates operationComboBox with the topological operations
    void populateCbOperation();
    //! Set selected GUI (lbStatusSelected and pbCreateLayerSelected)
    void setSelectedGui();
    //! Make action when change item in List feature
    void changeLwFeature( QgsVectorLayer *lyr, QgsFeatureId fid );
    //! Zoom mapcanvas to current feature in listbox target
    void zoomFeature( QgsVectorLayer *lyr, QgsFeatureId fid );
    //! Show rubber from feature
    void showRubberFeature( QgsVectorLayer *lyr, QgsFeatureId id );

    //! Pointer to Interface QGIS
    QgisInterface* mIface;
    //! Target Layer, the query will be performed on it
    QgsVectorLayer* mLayerTarget;
    //! Reference Layer, the query will be based on it
    QgsVectorLayer* mLayerReference;
    //! Stores ID's from spatial query
    QgsFeatureIds mFeatureResult;
    //! Stores ID's invalid of target layer
    QgsFeatureIds mFeatureInvalidTarget;
    //! Stores ID's invalid of reference layer
    QgsFeatureIds mFeatureInvalidReference;
    //! Map for Id name of vector layers (use in signal_qgis_layerWillBeRemoved)
    QMap<QString, QgsVectorLayer *> mMapIdVectorLayers;
    //! Rubber band for features result
    QgsRubberSelectId* mRubberSelectId;
    //! Text for source selected
    QString mSourceSelected;
    bool mIsSelectedOperator;

    void MsgDEBUG( QString sMSg );
};

#endif // SPATIALQUERYDIALOG_H
