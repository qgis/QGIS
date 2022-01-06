/***************************************************************************
    qgsrenderermeshpropertieswidget.h
    ---------------------
    begin                : June 2018
    copyright            : (C) 2018 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSRENDERERMESHPROPERTIESWIDGET_H
#define QGSRENDERERMESHPROPERTIESWIDGET_H

#include <QObject>
#include <QDialog>

#include "ui_qgsrenderermeshpropswidgetbase.h"

#include "qgsmaplayerconfigwidget.h"
#include "qgis_gui.h"
#include <memory>

SIP_NO_FILE

class QgsMeshLayer;
class QgsMapCanvas;

/**
 * \ingroup gui
 * \class QgsRendererMeshPropertiesWidget
 *
 * \brief Widget for renderer properties of the mesh, contours (scalars)
 * and vectors data associated with the mesh layer
 */
class GUI_EXPORT QgsRendererMeshPropertiesWidget : public QgsMapLayerConfigWidget, private Ui::QgsRendererMeshPropsWidgetBase
{
    Q_OBJECT

  public:

    /**
     * A widget to hold the renderer properties for a mesh layer.
     * \param layer The mesh layer to style
     * \param canvas The canvas object used
     * \param parent Parent object
     */
    QgsRendererMeshPropertiesWidget( QgsMeshLayer *layer, QgsMapCanvas *canvas, QWidget *parent = nullptr );

    /**
     * Synchronize widgets state with associated map layer
     *
     * \since QGIS 3.22, replace syncToLayer() without argument
     */
    void syncToLayer( QgsMapLayer *mapLayer ) override;

  public slots:
    //! Applies the settings made in the dialog
    void apply() override;

  private slots:
    void onActiveScalarGroupChanged( int groupIndex );
    void onActiveVectorGroupChanged( int groupIndex );

    void syncToLayerPrivate();

  private:
    QgsMeshLayer *mMeshLayer = nullptr; //not owned
};

#endif // QGSRENDERERMESHPROPERTIESWIDGET_H
