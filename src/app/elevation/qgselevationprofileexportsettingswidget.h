/***************************************************************************
                          qgselevationprofileexportsettingswidget.h
                          ---------------
    begin                : March 2022
    copyright            : (C) 2022 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSELEVATIONPROFILEEXPORTSETTINGSWIDGET_H
#define QGSELEVATIONPROFILEEXPORTSETTINGSWIDGET_H

#include "ui_qgselevationprofileexportsettingswidgetbase.h"

#include <QWidget>

class Qgs2DPlot;

/**
 * \brief Contains generic settings for exporting elevation profiles to drawings.
 */
class QgsElevationProfileExportSettingsWidget : public QWidget, private Ui::QgsElevationProfileExportSettingsWidgetBase
{
    Q_OBJECT
  public:

    QgsElevationProfileExportSettingsWidget( QWidget *parent = nullptr );

    /**
     * Sets the initial plot settings to show in the widget.
     */
    void setPlotSettings( const Qgs2DPlot &plot );

    /**
     * Updates plot settings based on the widget's state.
     */
    void updatePlotSettings( Qgs2DPlot &plot );

};

#endif // QGSELEVATIONPROFILEEXPORTSETTINGSWIDGET_H
