/***************************************************************************
    qgsmeshrenderer3daveragingwidget.h
    ----------------------------------
    begin                : November 2019
    copyright            : (C) 2019 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMESHRENDERER3DAVERAGINGWIDGET_H
#define QGSMESHRENDERER3DAVERAGINGWIDGET_H

#include "ui_qgsmeshrenderer3daveragingwidgetbase.h"
#include "qgis_gui.h"
#include "qgsmeshrenderersettings.h"
#include "qgsmeshdataprovider.h"

#include <memory>
#include <QWidget>

SIP_NO_FILE

class QgsMeshLayer;
class QgsMesh3dAveragingMethod;

/**
 * \ingroup gui
 * \class QgsMeshRenderer3dAveragingWidget
 *
 * \brief A widget for setup of the averaging method from 3d to 2d datasets on 3d stacked mesh.
 * The mesh layer must be connected
 */
class QgsMeshRenderer3dAveragingWidget : public QWidget, private Ui::QgsMeshRenderer3dAveragingWidgetBase
{
    Q_OBJECT

  public:

    /**
     * A widget to hold the renderer Vector settings for a mesh layer.
     * \param parent Parent object
     */
    QgsMeshRenderer3dAveragingWidget( QWidget *parent = nullptr );

    //! Associates mesh layer with the widget
    void setLayer( QgsMeshLayer *layer );

    /**
     * Returns selected averaging method
     *
     * Caller takes ownership
     */
    std::unique_ptr<QgsMesh3dAveragingMethod> averagingMethod() const;

    //! Synchronizes widgets state with associated mesh layer
    void syncToLayer();

  signals:
    //! Mesh rendering settings changed
    void widgetChanged();

  private slots:
    void onAveragingMethodChanged( int methodIndex );

  private:
    QgsMeshLayer *mMeshLayer = nullptr; //not owned
};

#endif // QGSMESHRENDERER3DAVERAGINGWIDGET_H
