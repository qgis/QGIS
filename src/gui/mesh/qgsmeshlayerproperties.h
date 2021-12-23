/***************************************************************************
  qgsmeshlayerproperties.h
  ------------------------
    begin                : Jun 2018
    copyright            : (C) 2018 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSMESHLAYERPROPERTIES_H
#define QGSMESHLAYERPROPERTIES_H

#include "ui_qgsmeshlayerpropertiesbase.h"

#include "qgsmaplayerstylemanager.h"
#include "qgsoptionsdialogbase.h"
#include "qgsguiutils.h"
#include "qgis_gui.h"

class QgsMapLayer;
class QgsMapCanvas;
class QgsMeshLayer;
class QgsRendererMeshPropertiesWidget;
class QgsMapLayerConfigWidget;
class QgsMeshLayer3DRendererWidget;
class QgsMeshStaticDatasetWidget;
class QgsMapLayerConfigWidgetFactory;
class QgsMetadataWidget;

/**
 * \ingroup gui
 * \class QgsMeshLayerProperties
 *
 * \brief Property sheet for a mesh map layer.
 * Contains information, source and style tabs
 *
 * \since QGIS 3.16 in the GUI API
 */
class GUI_EXPORT QgsMeshLayerProperties : public QgsOptionsDialogBase, private Ui::QgsMeshLayerPropertiesBase
{
    Q_OBJECT

  public:

    /**
     * \brief Constructor
     * \param lyr Mesh map layer for which properties will be displayed
     * \param canvas The map canvas
     * \param parent The parent widget
     * \param fl Window flags
     */
    QgsMeshLayerProperties( QgsMapLayer *lyr, QgsMapCanvas *canvas, QWidget *parent = nullptr, Qt::WindowFlags = QgsGuiUtils::ModalDialogFlags );

    /**
     * Adds properties page from a factory
     *
     * \since QGIS 3.16
     */
    void addPropertiesPageFactory( const QgsMapLayerConfigWidgetFactory *factory );

  protected slots:
    void optionsStackedWidget_CurrentChanged( int index ) override SIP_SKIP ;

  private slots:
    //! Synchronizes widgets state with associated mesh layer
    void syncToLayer();

    //!Applies the settings made in the dialog without closing the box
    void apply();
    //! Synchronizes GUI state with associated mesh layer and trigger repaint
    void syncAndRepaint();
    //! Changes layer coordinate reference system
    void changeCrs( const QgsCoordinateReferenceSystem &crs );
    //! Loads the default style when appropriate button is pressed
    void loadDefaultStyle();
    //! Saves the default style when appropriate button is pressed
    void saveDefaultStyle();
    //! Loads a saved style when appropriate button is pressed
    void loadStyle();
    //! Saves a style when appriate button is pressed
    void saveStyleAs();
    //! Prepares style menu
    void aboutToShowStyleMenu();
    //! Reloads temporal properties from the provider
    void reloadTemporalProperties();
    //! \brief Called when cancel button is pressed
    void onCancel();

    void onTimeReferenceChange();

    void urlClicked( const QUrl &url );
    void loadMetadata();
    void saveMetadataAs();

  private:
    //! Pointer to the mesh styling widget
    QgsRendererMeshPropertiesWidget *mRendererMeshPropertiesWidget = nullptr;

    QList<QgsMapLayerConfigWidget *> mConfigWidgets;

    //! Pointer to the mesh layer that this property dialog changes the behavior of.
    QgsMeshLayer *mMeshLayer = nullptr;

    //! Pointer to mesh 3d styling widget
    QgsMeshLayer3DRendererWidget *mMesh3DWidget = nullptr;

    /**
     * Previous layer style. Used to reset style to previous state if new style
     * was loaded but dialog is canceled.
    */
    QgsMapLayerStyle mOldStyle;

    QPushButton *mBtnStyle = nullptr;
    QPushButton *mBtnMetadata = nullptr;
    QAction *mActionLoadMetadata = nullptr;
    QAction *mActionSaveMetadataAs = nullptr;

    QgsMapCanvas *mCanvas = nullptr;
    QgsMetadataWidget *mMetadataWidget = nullptr;

    bool mIsMapSettingsTemporal = false;

    friend class TestQgsMeshLayerPropertiesDialog;

    void showHelp();

    QgsCoordinateReferenceSystem mBackupCrs;
};


#endif // QGSMESHLAYERPROPERTIES_H
