/***************************************************************************
    qgstopocentricwidget.h
    ---------------------
    begin                : March 2026
    copyright            : (C) 2026 by Dominik Cindrić
    email                : viper dot miniq at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSTOPOCENTRICWIDGET_H
#define QGSTOPOCENTRICWIDGET_H

#include "qgis_app.h"

#include <QTimer>
#include <QWidget>

class QDoubleSpinBox;
class QSlider;

/**
 * Widget for displaying and editing the topocentric projection origin
 */
class APP_EXPORT QgsTopocentricWidget : public QWidget
{
    Q_OBJECT

  public:
    explicit QgsTopocentricWidget( QWidget *parent = nullptr );

    //! Returns the current latitude of the topocentric origin in degrees.
    double latitude() const;

    //! Returns the current longitude of the topocentric origin in degrees.
    double longitude() const;

    //! Sets the latitude of the topocentric origin in degrees.
    void setLatitude( double latitude );

    //! Sets the longitude of the topocentric origin in degrees.
    void setLongitude( double longitude );

  signals:
    //! Emitted when the origin coordinates change.
    void originChanged( double latitude, double longitude );

  private:
    QDoubleSpinBox *doubleSpinBoxX = nullptr;
    QSlider *horizontalSliderX = nullptr;
    QDoubleSpinBox *doubleSpinBoxY = nullptr;
    QSlider *horizontalSliderY = nullptr;
    QTimer *mEditTimer = nullptr;
};

#endif // QGSTOPOCENTRICWIDGET_H
