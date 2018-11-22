/***************************************************************************
                         qgsmapsettingsutils.h
                             -------------------
    begin                : May 2017
    copyright            : (C) 2017 by Mathieu Pellerin
    email                : nirvn dot asia at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPSETTINGSUTILS_H
#define QGSMAPSETTINGSUTILS_H

#include "qgis_core.h"
#include "qgsmapsettings.h"

#include <QString>

/**
 * \ingroup core
 * Utilities for map settings.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsMapSettingsUtils
{

  public:

    /**
     * Checks whether any of the layers attached to a map settings object contain advanced effects
     * \param mapSettings map settings
     */
    static const QStringList containsAdvancedEffects( const QgsMapSettings &mapSettings );

    /**
     * Creates the content of a world file.
     * \param mapSettings map settings
     * \note Uses 17 places of precision for all numbers output
     */
    static QString worldFileContent( const QgsMapSettings &mapSettings );

};

#endif
