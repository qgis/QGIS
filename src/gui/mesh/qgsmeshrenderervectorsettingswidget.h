/***************************************************************************
    qgsmeshrenderervectorsettingswidget.h
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

#ifndef QGSMESHRENDERERVECTORSETTINGSWIDGET_H
#define QGSMESHRENDERERVECTORSETTINGSWIDGET_H

#include "ui_qgsmeshrenderervectorsettingswidgetbase.h"
#include "qgis_gui.h"
#include "qgsmeshrenderersettings.h"
#include "qgsmeshdataprovider.h"
#include "qgsmeshdatasetgrouptreeview.h"

#include <memory>
#include <QWidget>

SIP_NO_FILE

class QgsMeshLayer;

/**
 * \ingroup gui
 * \class QgsMeshRendererVectorSettingsWidget
 *
 * \brief A widget for setup of the vector dataset renderer settings of
 * a mesh layer. The layer must be connected and an active dataset
 * must be selected.
 */
class QgsMeshRendererVectorSettingsWidget : public QWidget, private Ui::QgsMeshRendererVectorSettingsWidgetBase
{
    Q_OBJECT

  public:

    /**
     * A widget to hold the renderer Vector settings for a mesh layer.
     * \param parent Parent object
     */
    QgsMeshRendererVectorSettingsWidget( QWidget *parent = nullptr );

    //! Associates mesh layer with the widget
    void setLayer( QgsMeshLayer *layer );

    //! Associates a dataset group with the widget (should be set before syncToLayer())
    void setActiveDatasetGroup( int groupIndex ) { mActiveDatasetGroup = groupIndex; }

    //! Returns vector settings
    QgsMeshRendererVectorSettings settings() const;

    //! Synchronizes widgets state with associated mesh layer
    void syncToLayer();

  signals:
    //! Mesh rendering settings changed
    void widgetChanged();

  private slots:
    void onSymbologyChanged( int currentIndex );
    void onStreamLineSeedingMethodChanged( int currentIndex );
    void onColoringMethodChanged();
    void onColorRampMinMaxChanged();
    void loadColorRampShader();

  private:

    /**
     * Returns the value of the spin box, returns err_val if the
     * value is equal to the clear value.
     */
    double filterValue( const QgsDoubleSpinBox *spinBox, double err_val ) const;

    QgsMeshLayer *mMeshLayer = nullptr; //not owned
    int mActiveDatasetGroup = -1;
};

#endif // QGSMESHRENDERERVECTORSETTINGSWIDGET_H
