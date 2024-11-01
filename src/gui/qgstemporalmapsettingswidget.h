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

#define SIP_NO_FILE

///@cond PRIVATE
class GUI_EXPORT QgsTemporalMapSettingsWidget : public QgsPanelWidget, private Ui::QgsTemporalMapSettingsWidgetBase
{
    Q_OBJECT
  public:
    /**
      * Constructor for QgsTemporalMapSettingsWidget, with the specified \a parent widget.
      */
    QgsTemporalMapSettingsWidget( QWidget *parent = nullptr );

    ~QgsTemporalMapSettingsWidget() = default;

    /**
     * Returns the value of frame rate from widget input
     */
    double frameRateValue();

    /**
     * Sets the value of frame rate from the vcr widget.
     */
    void setFrameRateValue( double value );

    /**
     * Returns the cumulative range option state from vcr widget.
     *
     * \see setIsTemporalRangeCumulative()
     */
    bool isTemporalRangeCumulative();

    /**
     * Sets the cumulative range option state from vcr widget.
     *
     * \see isTemporalRangeCumulative(
     */
    void setIsTemporalRangeCumulative( bool state );

  signals:

    /**
     * Emitted when frame \a rate value on the spin box has changed.
     */
    void frameRateChanged( double rate );

    void temporalRangeCumulativeChanged( bool state );
};
///@endcond

#endif // QGSTEMPORALMAPSETTINGSWIDGET_H
