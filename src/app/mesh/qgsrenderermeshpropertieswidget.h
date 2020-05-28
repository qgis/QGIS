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
#include "qgis_app.h"
#include <memory>

class QgsMeshLayer;
class QgsMapCanvas;

/**
 * Widget for renderer properties of the mesh, contours (scalars)
 * and vectors data associated with the mesh layer
 */
class APP_EXPORT QgsRendererMeshPropertiesWidget : public QgsMapLayerConfigWidget, private Ui::QgsRendererMeshPropsWidgetBase
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

  public slots:
    //! Applies the settings made in the dialog
    void apply() override;

    //! Synchronize widgets state with associated mesh layer
    void syncToLayer();

  private slots:
    void onActiveScalarGroupChanged( int groupIndex );
    void onActiveVectorGroupChanged( int groupIndex );

  private:
    QgsMeshLayer *mMeshLayer = nullptr; //not owned
};

#endif // QGSRENDERERMESHPROPERTIESWIDGET_H
