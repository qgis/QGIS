/***************************************************************************
                         qgstemporalmapsettingswidget.h
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

#ifndef QGSTEMPORALMAPSETTINGSWIDGET_H
#define QGSTEMPORALMAPSETTINGSWIDGET_H

#include "ui_qgstemporalmapsettingswidgetbase.h"

#include "qgis_gui.h"


/**
 * \ingroup gui
 * The QgsTemporalMapSettingsWidget class
 *
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsTemporalMapSettingsWidget : public QWidget, private Ui::QgsTemporalMapSettingsWidgetBase
{
    Q_OBJECT
  public:

    /**
      * Constructor for QgsTemporalMapSettingsWidget
      *
      */
    QgsTemporalMapSettingsWidget( QWidget *parent = nullptr );

    ~QgsTemporalMapSettingsWidget() override;

    /**
     * Returns the value of frame rate from widget input
     */
    double frameRateValue();

    /**
     * Sets the value of frame rate from the vcr widget.
     */
    void setFrameRateValue( double value );

};

#endif // QGSTEMPORALMAPSETTINGSWIDGET_H
