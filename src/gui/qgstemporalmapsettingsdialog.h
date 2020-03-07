/***************************************************************************
                         qgstemporalmapsettingsdialog.h
                         ---------------
    begin                : March 2020
    copyright            : (C) 2020 by Samweli Mwakisambwe
    email                : samweli at kartoza dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSTEMPORALMAPSETTINGSDIALOG_H
#define QGSTEMPORALMAPSETTINGSDIALOG_H

#include "ui_qgstemporalmapsettingsdialogbase.h"
#include "qgis_gui.h"

class QgsTemporalMapSettingsWidget;

/**
 * \ingroup gui
 * The QgsTemporalMapSettingsDialog class
 *
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsTemporalMapSettingsDialog : public QDialog, private Ui::QgsTemporalMapSettingsDialogBase
{
    Q_OBJECT
  public:

    /**
      * Constructor for QgsTemporalMapSettingsDialog
      *
      */
    QgsTemporalMapSettingsDialog( QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags() );

    ~QgsTemporalMapSettingsDialog() override;

    /**
     * Returns the value of frame rate from widget.
     */
    double frameRateValue();

  private slots:

    //! \brief Applies the settings made in the dialog.
    void apply();
    //! \brief Called when cancel button is pressed
    void onCancel();

  private:

    //! Widget for handling temporal map settings
    QgsTemporalMapSettingsWidget *mTemporalMapSettingsWidget = nullptr;
};

#endif // QGSTEMPORALMAPSETTINGSDIALOG_H
