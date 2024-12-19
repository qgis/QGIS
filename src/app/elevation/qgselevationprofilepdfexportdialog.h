/***************************************************************************
                          qgselevationprofilepdfexportdialog.h
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
#ifndef QGSELEVATIONPROFILEPDFEXPORTDIALOG_H
#define QGSELEVATIONPROFILEPDFEXPORTDIALOG_H

#include "ui_qgselevationprofilepdfexportoptionsdialog.h"
#include "qgslayoutmeasurementconverter.h"

#include <QDialog>

class Qgs2DPlot;
class QgsElevationProfileExportSettingsWidget;

/**
 * \brief A dialog for exporting an elevation profile to PDF
 */
class QgsElevationProfilePdfExportDialog : public QDialog, private Ui::QgsElevationProfilePdfExportOptionsDialogBase
{
    Q_OBJECT
  public:

    QgsElevationProfilePdfExportDialog( QWidget *parent = nullptr );

    /**
     * Sets the initial plot settings to show in the widget.
     */
    void setPlotSettings( const Qgs2DPlot &plot );

    /**
     * Updates plot settings based on the widget's state.
     */
    void updatePlotSettings( Qgs2DPlot &plot );

    QgsLayoutSize pageSizeMM() const;

  private slots:

    void pageSizeChanged( int index );
    void orientationChanged( int index );
    void setToCustomSize();

  private:

    QgsElevationProfileExportSettingsWidget *mProfileSettingsWidget = nullptr;
    QgsLayoutMeasurementConverter mConverter;
    bool mSettingPresetSize = false;
};

#endif // QGSELEVATIONPROFILEPDFEXPORTDIALOG_H
