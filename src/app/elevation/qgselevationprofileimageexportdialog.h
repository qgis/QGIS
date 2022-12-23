/***************************************************************************
                          qgselevationprofileimageexportdialog.h
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
#ifndef QGSELEVATIONPROFILEIMAGEEXPORTDIALOG_H
#define QGSELEVATIONPROFILEIMAGEEXPORTDIALOG_H

#include "ui_qgselevationprofileimageexportoptionsdialog.h"
#include "qgslayoutmeasurementconverter.h"

#include <QDialog>

class Qgs2DPlot;
class QgsElevationProfileExportSettingsWidget;

/**
 * \brief A dialog for exporting an elevation profile to image
 */
class QgsElevationProfileImageExportDialog : public QDialog, private Ui::QgsElevationProfileImageExportOptionsDialogBase
{
    Q_OBJECT
  public:

    QgsElevationProfileImageExportDialog( QWidget *parent = nullptr );

    /**
     * Sets the initial plot settings to show in the widget.
     */
    void setPlotSettings( const Qgs2DPlot &plot );

    /**
     * Updates plot settings based on the widget's state.
     */
    void updatePlotSettings( Qgs2DPlot &plot );

    void setImageSize( const QSize &size );
    QSize imageSize() const;

  private:

    QgsElevationProfileExportSettingsWidget *mProfileSettingsWidget = nullptr;
};

#endif // QGSELEVATIONPROFILEIMAGEEXPORTDIALOG_H
