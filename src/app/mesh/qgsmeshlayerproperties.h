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

#include "qgsoptionsdialogbase.h"
#include "qgsguiutils.h"
#include "qgis_app.h"

class QgsMapLayer;
class QgsMapCanvas;
class QgsMeshLayer;
class QgsRendererMeshPropertiesWidget;

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
    //! Synchronize widgets state with associated mesh layer
    void syncToLayer();

    //!Applies the settings made in the dialog without closing the box
    void apply();
    //! \brief Slot to update layer display name as original is edited.
    void updateLayerName( const QString &text );
    //! Synchronize GUI state with associated mesh layer and trigger repaint
    void syncAndRepaint();
    //! Change layer coordinate reference system
    void changeCrs( const QgsCoordinateReferenceSystem &crs );
    //! Associate dataset to the mesh layer
    void addDataset();

  private:
    //! Pointer to the mesh styling widget
    QgsRendererMeshPropertiesWidget *mRendererMeshPropertiesWidget = nullptr;

    //! \brief Pointer to the mesh layer that this property dilog changes the behavior of.
    QgsMeshLayer *mMeshLayer = nullptr;

    friend class TestQgsMeshLayerPropertiesDialog;
};


#endif // QGSMESHLAYERPROPERTIES_H
