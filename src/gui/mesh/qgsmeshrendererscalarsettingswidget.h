/***************************************************************************
    qgsmeshrendererscalarsettingswidget.h
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

#ifndef QGSMESHRENDERERSCALARSETTINGSWIDGET_H
#define QGSMESHRENDERERSCALARSETTINGSWIDGET_H

#include "ui_qgsmeshrendererscalarsettingswidgetbase.h"
#include "qgis_gui.h"
#include "qgsmeshrenderersettings.h"
#include "qgsmeshdataprovider.h"

#include <QWidget>

SIP_NO_FILE

class QgsMeshLayer;

/**
 * \ingroup gui
 * \class QgsMeshRendererScalarSettingsWidget
 *
 * \brief A widget for setup of the scalar dataset renderer settings of
 * a mesh layer. The layer must be connected and an active dataset
 * must be selected.
 */
class GUI_EXPORT QgsMeshRendererScalarSettingsWidget : public QWidget, private Ui::QgsMeshRendererScalarSettingsWidgetBase
{
    Q_OBJECT

  public:

    /**
     * A widget to hold the renderer scalar settings for a mesh layer.
     * \param parent Parent object
     */
    QgsMeshRendererScalarSettingsWidget( QWidget *parent = nullptr );

    //! Associates mesh layer with the widget
    void setLayer( QgsMeshLayer *layer );

    //! Associates a dataset group with the widget (should be set before syncToLayer())
    void setActiveDatasetGroup( int groupIndex );

    //! Returns scalar settings
    QgsMeshRendererScalarSettings settings() const;

    //! Synchronizes widgets state with associated mesh layer
    void syncToLayer();

  signals:
    //! Mesh rendering settings changed
    void widgetChanged();

  private slots:
    void minMaxChanged();
    void recalculateMinMaxButtonClicked();
    void onEdgeStrokeWidthMethodChanged();

  private:
    double spinBoxValue( const QgsDoubleSpinBox *spinBox ) const;
    QgsMeshRendererScalarSettings::DataResamplingMethod dataIntepolationMethod() const;

    bool dataIsDefinedOnFaces() const;
    bool dataIsDefinedOnEdges() const;

    QgsMeshLayer *mMeshLayer = nullptr; // not owned
    int mActiveDatasetGroup = -1;
};

#endif // QGSMESHRENDERERSCALARSETTINGSWIDGET_H
