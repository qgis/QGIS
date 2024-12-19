/***************************************************************************
    qgsmeshrendereractivedatasetwidget.h
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

#ifndef QGSMESHRENDERERACTIVEDATASETWIDGET_H
#define QGSMESHRENDERERACTIVEDATASETWIDGET_H

#include "ui_qgsmeshrendereractivedatasetwidgetbase.h"
#include "qgis_gui.h"
#include "qgsmeshdataprovider.h"

#include <QWidget>
#include <QIcon>

SIP_NO_FILE

class QgsMeshLayer;

/**
 * \ingroup gui
 * \class QgsMeshRendererActiveDatasetWidget
 *
 * \brief Widget for selection of active dataset group from tree view.
 * Also selects the active scalar and vector dataset by slider
 *
 * User can choose different scalar and vector dataset.
 * Time slider is deactivated when no dataset is selected or
 * when all selected datasets are non-temporal.
 */
class GUI_EXPORT QgsMeshRendererActiveDatasetWidget : public QWidget, private Ui::QgsMeshRendererActiveDatasetWidgetBase
{
    Q_OBJECT

  public:

    /**
     * A widget to hold the renderer scalar settings for a mesh layer.
     * \param parent Parent object
     */
    QgsMeshRendererActiveDatasetWidget( QWidget *parent = nullptr );
    ~QgsMeshRendererActiveDatasetWidget() override;

    //! Associates mesh layer with the widget
    void setLayer( QgsMeshLayer *layer );

    //! Returns index of the active scalar dataset group
    int activeScalarDatasetGroup() const;

    //! Returns index of the active vector dataset group
    int activeVectorDatasetGroup() const;

    //! Synchronizes widgets state with associated mesh layer
    void syncToLayer();

  signals:

    //! Emitted when the current scalar group gets changed
    void activeScalarGroupChanged( int groupIndex );

    //! Emitted when the current vector group gets changed
    void activeVectorGroupChanged( int groupIndex );

    //! Emitted when any settings related to rendering changed
    void widgetChanged();

  private slots:
    void onActiveScalarGroupChanged( int groupIndex );
    void onActiveVectorGroupChanged( int groupIndex );
    QString metadata( QgsMeshDatasetIndex datasetIndex );

  private:
    void updateMetadata();

    QgsMeshLayer *mMeshLayer = nullptr; // not owned
    int mActiveScalarDatasetGroup = -1;
    int mActiveVectorDatasetGroup = -1;
};

#endif // QGSMESHRENDERERSCALARSETTINGSWIDGET_H
