/***************************************************************************
  qgsbloomsettings.h
  --------------------------------------
  Date                 : May 2026
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

#ifndef QGSBLOOMSETTINGS_H
#define QGSBLOOMSETTINGS_H

#include "qgis_3d.h"

#include <QMap>
#include <QString>

#define SIP_NO_FILE

class QgsReadWriteContext;
class QDomElement;


/**
 * \brief Contains the configuration of the lighting "bloom" effect.
 * \ingroup qgis_3d
 * \note Not available in Python bindings
 * \since QGIS 4.2
 */
class _3D_EXPORT QgsBloomSettings
{
  public:
    QgsBloomSettings() = default;
    QgsBloomSettings( const QgsBloomSettings &other );
    QgsBloomSettings &operator=( QgsBloomSettings const &rhs );

    /**
     * Reads settings from a DOM \a element.
     *
     * \see writeXml()
     */
    void readXml( const QDomElement &element, const QgsReadWriteContext &context );

    /**
     * Writes settings to a DOM \a element.
     *
     * \see readXml()
     */
    void writeXml( QDomElement &element, const QgsReadWriteContext &context ) const;

    /**
     * Sets whether the bloom effect is \a enabled.
     *
     * \see isEnabled()
     */
    void setEnabled( bool enabled ) { mEnabled = enabled; }

    /**
     * Returns whether the bloom effect is enabled.
     *
     * \see setEnabled()
     */
    bool isEnabled() const { return mEnabled; }

    /**
     * Sets the intensity of the bloom effect.
     *
     * \see intensity()
     */
    void setIntensity( double factor ) { mIntensity = factor; }

    /**
     * Returns the intensity of the bloom effect.
     *
     * \see setIntensity()
     */
    double intensity() const { return mIntensity; }

    /**
     * Sets the filter \a radius for the bloom.
     *
     * \see radius()
     */
    void setRadius( double radius ) { mRadius = radius; }

    /**
     * Returns the filter radius for the bloom.
     *
     * \see setRadius()
     */
    double radius() const { return mRadius; }

  private:
    bool mEnabled = false;
    double mIntensity = 0.05;
    double mRadius = 0.005;
};

#endif // QGSBLOOMSETTINGS_H
