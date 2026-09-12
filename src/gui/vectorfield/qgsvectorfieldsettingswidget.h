/***************************************************************************
    qgsvectorfieldsettingswidget.h
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

#ifndef QGSVECTORFIELDSETTINGSWIDGET_H
#define QGSVECTORFIELDSETTINGSWIDGET_H

#include "ui_qgsvectorfieldsettingswidgetbase.h"

#include "qgsvectorfieldsettings.h"

#include <QWidget>

SIP_NO_FILE

class QgsMeshLayer;
class QgsDoubleSpinBox;

/**
 * \ingroup gui
 * \class QgsVectorFieldSettingsWidget
 *
 * \brief A widget for setup of the vector dataset renderer settings of
 * a mesh layer. The layer must be connected and an active dataset
 * must be selected.
 *
 * \since QGIS 4.4
 */
class QgsVectorFieldSettingsWidget : public QWidget, private Ui::QgsVectorFieldSettingsWidgetBase
{
    Q_OBJECT

  public:
    /**
     * A widget to hold the renderer Vector settings for a mesh layer.
     * \param parent Parent object
     */
    QgsVectorFieldSettingsWidget( QWidget *parent = nullptr );

    //! Associates mesh layer with the widget
    void setLayer( QgsMeshLayer *layer );

    //! Associates a dataset group with the widget (should be set before syncToLayer())
    void setActiveDatasetGroup( int groupIndex ) { mActiveDatasetGroup = groupIndex; }

    //! Returns vector settings
    QgsVectorFieldSettings settings() const;

    //! Synchronizes widgets state with associated mesh layer
    void syncToLayer();

  signals:
    //! Mesh rendering settings changed
    void widgetChanged();

  private slots:
    void onSymbologyChanged( int currentIndex );
    void onStreamLineSeedingMethodChanged( int currentIndex );
    void onWindBarbUnitsChanged( int currentIndex );
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

#endif // QGSVECTORFIELDSETTINGSWIDGET_H
