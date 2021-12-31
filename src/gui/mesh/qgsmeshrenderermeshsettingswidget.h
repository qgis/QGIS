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
#include "qgis_gui.h"

#include <QWidget>

SIP_NO_FILE

class QgsMeshLayer;

/**
 * \ingroup gui
 * \class QgsMeshRendererMeshSettingsWidget
 *
 * \brief A widget for setup of the mesh frame settings of
 * the mesh layer. Can be used for native,
 * triangular and edge mesh renderer settings.
 */
class QgsMeshRendererMeshSettingsWidget : public QWidget, private Ui::QgsMeshRendererMeshSettingsWidgetBase
{
    Q_OBJECT

  public:

    enum MeshType
    {
      Native,
      Triangular,
      Edge
    };

    /**
     * A widget to hold the renderer mesh settings for a mesh layer.
     * \param parent Parent object
     */
    QgsMeshRendererMeshSettingsWidget( QWidget *parent = nullptr );

    /**
     * Associates mesh layer with the widget
     * \param layer mesh layer that contains mesh frame
     * \param meshType whether use settings for triangular mesh or native mesh or edge mesh
     */
    void setLayer( QgsMeshLayer *layer, MeshType meshType );

    //! Returns the mesh rendering settings (native or triangular or edge)
    QgsMeshRendererMeshSettings settings() const;

    //! Synchronizes widgets state with associated mesh layer
    void syncToLayer();

  signals:
    //! Mesh rendering settings changed
    void widgetChanged();

  private:
    QgsMeshLayer *mMeshLayer = nullptr; // not owned
    MeshType mMeshType = MeshType::Native;
};

#endif // QGSMESHRENDERERMESHSETTINGSWIDGET_H
