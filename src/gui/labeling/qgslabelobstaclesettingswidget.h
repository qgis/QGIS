/***************************************************************************
    qgslabelobstaclesettingswidget.h
    ----------------------
    begin                : December 2019
    copyright            : (C) 2019 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLABELOBSTACLESETTINGSWIDGET_H
#define QGSLABELOBSTACLESETTINGSWIDGET_H

#include "ui_qgslabelobstaclesettingswidgetbase.h"
#include "qgslabelsettingswidgetbase.h"
#include "qgspallabeling.h"
#include "qgis_gui.h"
#include "qgis_sip.h"

/**
 * \ingroup gui
 * \class QgsLabelObstacleSettingsWidget
 * \brief A widget for customising label obstacle settings.
 * \since QGIS 3.12
 */
class GUI_EXPORT QgsLabelObstacleSettingsWidget : public QgsLabelSettingsWidgetBase, private Ui::QgsLabelObstacleSettingsWidgetBase
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsLabelObstacleSettingsWidget.
     * \param parent parent widget
     * \param vl associated vector layer
     */
    QgsLabelObstacleSettingsWidget( QWidget *parent SIP_TRANSFERTHIS = nullptr, QgsVectorLayer *vl = nullptr );

    /**
     * Sets the obstacle \a settings to show in the widget.
     *
     * \see settings()
     */
    void setSettings( const QgsLabelObstacleSettings &settings );

    /**
     * Returns the obstacle settings defined by the widget.
     *
     * \see setSettings()
     */
    QgsLabelObstacleSettings settings() const;

    void setGeometryType( Qgis::GeometryType type ) override;

    void updateDataDefinedProperties( QgsPropertyCollection &properties ) override;

  private:
    bool mBlockSignals = false;
};

#endif // QGSLABELOBSTACLESETTINGSWIDGET_H
