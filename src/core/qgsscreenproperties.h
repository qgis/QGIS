/***************************************************************************
     qgsscreenproperties.h
     ---------------
    Date                 : June 2023
    Copyright            : (C) 2023 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSSCREENPROPERTIES_H
#define QGSSCREENPROPERTIES_H

#include "qgis_core.h"

class QScreen;
class QgsRenderContext;

/**
 * \ingroup core
 * \class QgsScreenProperties
 * \brief Stores properties relating to a screen.
 *
 * This class is designed to be a publicly constructible, copyable store
 * for the properties available from a QScreen object.
 *
 * \since QGIS 3.32
 */
class CORE_EXPORT QgsScreenProperties
{

  public:

    /**
     * Constructor for invalid properties.
     */
    QgsScreenProperties();

    /**
     * Constructor for QgsScreenProperties, copying
     * properties from the specified \a screen.
     */
    explicit QgsScreenProperties( const QScreen *screen );

    /**
     * Returns TRUE if the properties are valid.
     */
    bool isValid() const { return mValid; }

    /**
     * This property holds the screen's ratio between physical pixels and device-independent pixels.
     *
     * Returns the ratio between physical pixels and device-independent pixels for the screen.
     *
     * Common values are 1.0 on normal displays and 2.0 on "retina" displays. Higher values are also possible.
     */
    double devicePixelRatio() const { return mDevicePixelRatio; }

    /**
     * This property holds the number of physical dots or pixels per inch.
     *
     * This value represents the pixel density on the screen's display.
     * Depending on what information the underlying system provides the value might not be entirely accurate.
     *
     * \note Physical DPI is expressed in device-independent dots. Multiply by devicePixelRatio() to get
     * device-dependent density.
     */
    double physicalDpi() const { return mPhysicalDpi; }

    /**
     * Updates the settings in a render \a context
     * to match the screen settings.
     */
    void updateRenderContextForScreen( QgsRenderContext &context ) const;

  private:

    bool mValid = false;
    double mDevicePixelRatio = 1;
    double mPhysicalDpi = 96;
};

#endif // QGSSCREENPROPERTIES_H
