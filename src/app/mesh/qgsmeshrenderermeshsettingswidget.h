/***************************************************************************
    qgsmeshrenderermeshsettingswidget.h
    -------------------------------------
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

#ifndef QGSMESHRENDERERMESHSETTINGSWIDGET_H
#define QGSMESHRENDERERMESHSETTINGSWIDGET_H

#include "ui_qgsmeshrenderermeshsettingswidgetbase.h"
#include "qgsmeshrenderersettings.h"
#include "qgis_app.h"

#include <QWidget>

class QgsMeshLayer;

/**
 * A widget for setup of the mesh frame settings of
 * the mesh layer. Can be used for both native and
 * triangular mesh renderer settings.
 */
class APP_EXPORT QgsMeshRendererMeshSettingsWidget : public QWidget, private Ui::QgsMeshRendererMeshSettingsWidgetBase
{
    Q_OBJECT

  public:

    /**
     * A widget to hold the renderer mesh settings for a mesh layer.
     * \param parent Parent object
     */
    QgsMeshRendererMeshSettingsWidget( QWidget *parent = nullptr );

    /**
     * Associates mesh layer with the widget
     * \param layer mesh layer that contains mesh frame
     * \param isTriangularMesh whether use settings for triangular mesh or native mesh
     */
    void setLayer( QgsMeshLayer *layer, bool isTriangularMesh );

    //! Returns the mesh rendering settings (native or triangular)
    QgsMeshRendererMeshSettings settings() const;

    //! Synchronizes widgets state with associated mesh layer
    void syncToLayer();

  signals:
    //! Mesh rendering settings changed
    void widgetChanged();

  private:
    QgsMeshLayer *mMeshLayer = nullptr; // not owned

    /**
     * If true, the widget works for triangular (derived) mesh
     * If false, the widget works for native mesh
     */
    bool mIsTriangularMesh = true;
};

#endif // QGSMESHRENDERERMESHSETTINGSWIDGET_H
