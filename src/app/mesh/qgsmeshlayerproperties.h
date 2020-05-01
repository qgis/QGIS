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
#include "qgis_app.h"

class QgsMapLayer;
class QgsMapCanvas;
class QgsMeshLayer;
class QgsRendererMeshPropertiesWidget;
class QgsMeshLayer3DRendererWidget;
class QgsMeshStaticDatasetWidget;

/**
 * Property sheet for a mesh map layer.
 * Contains information, source and style tabs
 */
class APP_EXPORT QgsMeshLayerProperties : public QgsOptionsDialogBase, private Ui::QgsMeshLayerPropertiesBase
{
    Q_OBJECT

  public:

    /**
     * \brief Constructor
     * \param lyr Mesh map layer for which properties will be displayed
     */
    QgsMeshLayerProperties( QgsMapLayer *lyr, QgsMapCanvas *canvas, QWidget *parent = nullptr, Qt::WindowFlags = QgsGuiUtils::ModalDialogFlags );

  private slots:
    //! Synchronizes widgets state with associated mesh layer
    void syncToLayer();

    //!Applies the settings made in the dialog without closing the box
    void apply();
    //! \brief Slot to update layer display name as original is edited.
    void updateLayerName( const QString &text );
    //! Synchronizes GUI state with associated mesh layer and trigger repaint
    void syncAndRepaint();
    //! Changes layer coordinate reference system
    void changeCrs( const QgsCoordinateReferenceSystem &crs );
    //! Associates dataset to the mesh layer
    void addDataset();
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

    void onTimeReferenceChange();

    void onStaticDatasetCheckBoxChanged();
  private:
    //! Pointer to the mesh styling widget
    QgsRendererMeshPropertiesWidget *mRendererMeshPropertiesWidget = nullptr;

    //! Pointer to the mesh layer that this property dialog changes the behavior of.
    QgsMeshLayer *mMeshLayer = nullptr;

    //! Pointer to mesh 3d styling widget
    QgsMeshLayer3DRendererWidget *mMesh3DWidget = nullptr;

    /**
     * Previous layer style. Used to reset style to previous state if new style
     * was loaded but dialog is canceled */
    QgsMapLayerStyle mOldStyle;

    friend class TestQgsMeshLayerPropertiesDialog;

    void showHelp();
};


#endif // QGSMESHLAYERPROPERTIES_H
