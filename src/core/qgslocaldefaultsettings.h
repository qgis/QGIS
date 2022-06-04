/***************************************************************************
    qgslocaldefaultsettings.h
    ---------------------------
    begin                : January 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSLOCALDEFAULTSETTINGS_H
#define QGSLOCALDEFAULTSETTINGS_H

#include "qgis_core.h"
#include "qgis_sip.h"

class QgsBearingNumericFormat;
class QgsGeographicCoordinateNumericFormat;

/**
 * \brief Contains local default settings which should be respected when creating new objects
 * such as QgsProjects.
 *
 * This class contains a variety of default setting values. These values are local, profile
 * specific settings which may have been configured or tweaked by the user (as opposed to
 * global, fixed default settings).
 *
 * The values encapsulated here should be inherited when creating new objects such as new
 * QGIS projects.
 *
 * Typically, the QgsSettings backend is used to store and retrieve these local settings.
 *
 * \ingroup core
 * \since QGIS 3.12
 */
class CORE_EXPORT QgsLocalDefaultSettings
{

  public:

    /**
     * Sets the default bearing \a format, which controls how angular bearings are displayed.
     *
     * \see bearingFormat()
     */
    static void setBearingFormat( const QgsBearingNumericFormat *format );

    /**
     * Returns the default bearing format, which controls how angular bearings are displayed.
     *
     * This method returns a new object and the caller takes ownership of the returned value.
     *
     * \see setBearingFormat()
     */
    static QgsBearingNumericFormat *bearingFormat() SIP_FACTORY;

    /**
     * Sets the default geographic coordinate \a format, which controls how geographic coordinates are displayed.
     *
     * \see geographicCoordinateFormat()
     *
     * \since QGIS 3.26
     */
    static void setGeographicCoordinateFormat( const QgsGeographicCoordinateNumericFormat *format );

    /**
     * Returns the default geographic coordinate format, which controls how geographic coordinates are displayed.
     *
     * This method returns a new object and the caller takes ownership of the returned value.
     *
     * \see setGeographicCoordinateFormat()
     *
     * \since QGIS 3.26
     */
    static QgsGeographicCoordinateNumericFormat *geographicCoordinateFormat() SIP_FACTORY;

};

#endif // QGSLOCALDEFAULTSETTINGS_H
