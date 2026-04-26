/***************************************************************************
  qgssunlightsettings.h
  --------------------------------------
  Date                 : April 2026
  Copyright            : (C) 2026 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSUNLIGHTSETTINGS_H
#define QGSSUNLIGHTSETTINGS_H

#include "qgis_3d.h"
#include "qgslightsource.h"
#include "qgsvector3d.h"

#include <QColor>

class QDomDocument;
class QDomElement;

/**
 * \ingroup qgis_3d
 * \brief Definition of a sun light in a 3D map scene.
 *
 * A light source based on the real-world sun position.
 *
 * \since QGIS 4.2
 */
class _3D_EXPORT QgsSunLightSettings : public QgsLightSource
{
  public:
    QgsSunLightSettings() = default;

    Qgis::LightSourceType type() const override;
    QgsSunLightSettings *clone() const override SIP_FACTORY;
    Qt3DCore::QEntity *createEntity( const Qgs3DMapSettings &map, Qt3DCore::QEntity *parent ) const override SIP_SKIP;
    QDomElement writeXml( QDomDocument &doc, const QgsReadWriteContext &context = QgsReadWriteContext() ) const override;
    void readXml( const QDomElement &elem, const QgsReadWriteContext &context = QgsReadWriteContext() ) override;

    /**
     * Returns the configured date and time for the sun position calculation.
     *
     * \see setSunTime()
     */
    QDateTime sunTime() const { return mSunTime; }

    /**
     * Sets the date and \a time for the sun position calculation.
     *
     * \see sunTime()
     */
    void setSunTime( const QDateTime &time ) { mSunTime = time; }

    /**
     * Returns the base color of the sunlight.
     *
     * \see setColor()
     */
    QColor color() const { return mColor; }

    /**
     * Sets the base \a color of the sunlight.
     *
     * \see color()
     */
    void setColor( const QColor &color ) { mColor = color; }

    /**
     * Returns the base intensity of the sunlight.
     *
     * \see setIntensity()
     */
    double intensity() const { return mIntensity; }

    /**
     * Sets the base \a intensity of the sunlight.
     *
     * \see intensity()
     */
    void setIntensity( double intensity ) { mIntensity = intensity; }

    /**
     * Returns the atmospheric pressure (in millibars/hPa) used for the solar refraction calculation.
     *
     * \see setAtmosphericPressure()
     */
    double atmosphericPressure() const { return mAtmosphericPressure; }

    /**
     * Sets the atmospheric \a pressure (in millibars/hPa) for the solar refraction calculation.
     *
     * \see atmosphericPressure()
     */
    void setAtmosphericPressure( double pressure ) { mAtmosphericPressure = pressure; }

    /**
     * Returns the ambient temperature (in Celsius) used for the solar refraction calculation.
     *
     * \see setTemperature()
     */
    double temperature() const { return mTemperature; }

    /**
     * Sets the ambient \a temperature (in Celsius) for the solar refraction calculation.
     *
     * \see temperature()
     */
    void setTemperature( double temperature ) { mTemperature = temperature; }

    /**
     * Returns the reference elevation (in meters above sea level) used for the solar position calculation.
     *
     * \see setReferenceElevation()
     */
    double referenceElevation() const { return mReferenceElevation; }

    /**
     * Sets the reference \a elevation (in meters above sea level) for the solar position calculation.
     *
     * \see referenceElevation()
     */
    void setReferenceElevation( double elevation ) { mReferenceElevation = elevation; }

    bool operator==( const QgsSunLightSettings &other ) const;

  private:
    QDateTime mSunTime = QDateTime::currentDateTimeUtc();

    QColor mColor = Qt::white;
    double mIntensity = 1.0;

    double mAtmosphericPressure = 1013.25;
    double mTemperature = 15.0;

    double mReferenceElevation = 0.0;
};

#endif // QGSSUNLIGHTSETTINGS_H
