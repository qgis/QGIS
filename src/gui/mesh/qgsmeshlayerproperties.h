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
#include "qgslayerpropertiesdialog.h"
#include "qgsguiutils.h"
#include "qgis_gui.h"

class QgsMapLayer;
class QgsMapCanvas;
class QgsRendererMeshPropertiesWidget;
class QgsMeshLabelingWidget;
class QgsMeshLayer3DRendererWidget;
class QgsMeshStaticDatasetWidget;
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
class GUI_EXPORT QgsMeshLayerProperties : public QgsLayerPropertiesDialog, private Ui::QgsMeshLayerPropertiesBase
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
     * Saves the default style when appropriate button is pressed
     *
     * \deprecated QGIS 3.40. Use saveStyleAsDefault() instead.
     */
    Q_DECL_DEPRECATED void saveDefaultStyle() SIP_DEPRECATED;

    /**
     * Loads a saved style when appropriate button is pressed
     *
     * \deprecated QGIS 3.40. Use loadStyleFromFile() instead.
     */
    Q_DECL_DEPRECATED void loadStyle() SIP_DEPRECATED;

    /**
     * Saves a style when appriate button is pressed
     *
     * \deprecated QGIS 3.40. Use saveStyleToFile() instead.
     */
    Q_DECL_DEPRECATED void saveStyleAs() SIP_DEPRECATED;

  protected slots:
    void syncToLayer() FINAL;
    void apply() FINAL;
    void rollback() FINAL;

  private slots:

    //! Synchronizes GUI state with associated mesh layer and trigger repaint
    void syncAndRepaint();
    //! Changes layer coordinate reference system
    void changeCrs( const QgsCoordinateReferenceSystem &crs );
    //! Prepares style menu
    void aboutToShowStyleMenu();
    //! Reloads temporal properties from the provider
    void reloadTemporalProperties();

    void onTimeReferenceChange();

  private:
    //! Pointer to the mesh styling widget
    QgsRendererMeshPropertiesWidget *mRendererMeshPropertiesWidget = nullptr;

    //! Pointer to the mesh layer that this property dialog changes the behavior of.
    QgsMeshLayer *mMeshLayer = nullptr;

    //! Pointer to mesh 3d styling widget
    QgsMeshLayer3DRendererWidget *mMesh3DWidget = nullptr;

    //! Labeling dialog. If apply is pressed, options are applied to mesh layer
    QgsMeshLabelingWidget *mLabelingDialog = nullptr;

    QAction *mActionLoadMetadata = nullptr;
    QAction *mActionSaveMetadataAs = nullptr;

    QgsMetadataWidget *mMetadataWidget = nullptr;

    bool mIsMapSettingsTemporal = false;

    friend class TestQgsMeshLayerPropertiesDialog;

    void showHelp();

    QgsCoordinateReferenceSystem mBackupCrs;
};


#endif // QGSMESHLAYERPROPERTIES_H
