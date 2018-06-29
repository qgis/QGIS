/***************************************************************************
  qgsquickscalebarkit.h
  --------------------------------------
  Date                 : Nov 2017
  Copyright            : (C) 2017 by Peter Petrik
  Email                : zilolv at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSQUICKSCALEBARKIT_H
#define QGSQUICKSCALEBARKIT_H

#include <QObject>
#include <QString>

#include "qgis_quick.h"
#include "qgsquickutils.h"

class QgsQuickMapSettings;

/**
 * \ingroup quick
 *
 * The class QgsQuickScaleBarKit encapsulates the utilies to calculate
 * scale bar properties
 *
 * It requires connection to mapSettings of the active canvas to automatically
 * update text and width
 *
 * From preferred width in pixel, it calculates the width (pixel) of scalebar
 * distance in meters or kilometers (int) rounded to "nice" number (e.g. 72.4 to 100)
 * and units text (e.g. km)
 *
 * System of measurement for result could be set too, so for example the resulting scalebar
 * can show results in the imperial units.
 *
 * \note QML Type: ScaleBarKit
 *
 * \since QGIS 3.2
 */
class QUICK_EXPORT QgsQuickScaleBarKit : public QObject
{
    Q_OBJECT

    /**
     * Associated map settings. Should be initialized before the first use from mapcanvas map settings.
     */
    Q_PROPERTY( QgsQuickMapSettings *mapSettings MEMBER mMapSettings WRITE setMapSettings NOTIFY mapSettingsChanged )

    /**
     * Preferred width of scalebar in pixels. Default set to 300.
     */
    Q_PROPERTY( int preferredWidth MEMBER mPreferredWidth NOTIFY preferredWidthChanged )

    /**
     * Preferred system of measurement for the result
     */
    Q_PROPERTY( QgsUnitTypes::SystemOfMeasurement systemOfMeasurement MEMBER mSystemOfMeasurement NOTIFY systemOfMeasurementChanged )

    /**
     * Units of distance (e.g. km or m) of result in desired systemOfMeasurement Read-only (result).
     */
    Q_PROPERTY( QString units READ units NOTIFY scaleBarChanged )

    /**
     * Distance rounded to "nice" number (e.g. 100, 20) corresponding to width and system of measurement
     *
     * To be used with units property for labels. Read-only (result).
     */
    Q_PROPERTY( int distance READ distance NOTIFY scaleBarChanged )

    /**
     * Calculated width of scalebar in pixels representing distance + units. Differs minimum possible from preferredWidth to
     * Gets "nice" distance number.
     */
    Q_PROPERTY( int width READ width NOTIFY scaleBarChanged )

  public:
    //! create new scale bar kit
    explicit QgsQuickScaleBarKit( QObject *parent = nullptr );
    ~QgsQuickScaleBarKit() = default;

    //! \copydoc QgsQuickScaleBarKit::mapSettings
    void setMapSettings( QgsQuickMapSettings *mapSettings );

    //! \copydoc QgsQuickScaleBarKit::width
    int width() const;

    /**
     * \copydoc QgsQuickScaleBarKit::units
     * \see QgsQuickScaleBarKit::units()
     */
    int distance() const;

    /**
     * \copydoc QgsQuickScaleBarKit::units
     * \see QgsQuickScaleBarKit::distance()
     */
    QString units() const;

  signals:

    /**
     * width, distance and/or units changed
     */
    void scaleBarChanged();

    //! \copydoc QgsQuickScaleBarKit::mapSettings
    void mapSettingsChanged();

    //! \copydoc QgsQuickScaleBarKit::preferredWidth
    void preferredWidthChanged();

    //! \copydoc QgsQuickScaleBarKit::systemOfMeasurement
    void systemOfMeasurementChanged();

  public slots:
    //! recalculate width, distance and units.
    void updateScaleBar();

  private:
    QgsQuickMapSettings *mMapSettings = nullptr; // not owned
    int mPreferredWidth; // pixels
    int mWidth; // pixels
    int mDistance; // in meters or kilometers, rounded
    QString mUnits; // e.g. km or m
    QgsUnitTypes::SystemOfMeasurement mSystemOfMeasurement = QgsUnitTypes::MetricSystem;
};


#endif // QGSQUICKSCALEBARKIT_H
