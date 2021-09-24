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
#include "qgis_sip.h"
#include <QString>

class QgsMapSettings;

/**
 * \ingroup core
 * \brief Utilities for map settings.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsMapSettingsUtils
{

  public:

    /**
     * Flags for controlling the behavior of containsAdvancedEffects()
     * \since QGIS 3.14
     */
    enum class EffectsCheckFlag
    {
      IgnoreGeoPdfSupportedEffects = 1 << 0, //!< Ignore advanced effects which are supported in GeoPDF exports
    };
    Q_DECLARE_FLAGS( EffectsCheckFlags, EffectsCheckFlag )

    /**
     * Checks whether any of the layers attached to a map settings object contain advanced effects.
     *
     * The optional \a flags argument can be used to fine-tune the check behavior.
     */
    static QStringList containsAdvancedEffects( const QgsMapSettings &mapSettings, EffectsCheckFlags flags = QgsMapSettingsUtils::EffectsCheckFlags() );

    /**
     * Computes the six parameters of a world file.
     * \param mapSettings map settings
     * \param a the a parameter
     * \param b the b parameter
     * \param c the c parameter
     * \param d the d parameter
     * \param e the e parameter
     * \param f the f parameter
     * \since QGIS 3.10
     */
    static void worldFileParameters( const QgsMapSettings &mapSettings, double &a SIP_OUT, double &b SIP_OUT, double &c SIP_OUT, double &d SIP_OUT, double &e SIP_OUT, double &f SIP_OUT );

    /**
     * Creates the content of a world file.
     * \param mapSettings map settings
     * \note Uses 17 places of precision for all numbers output
     */
    static QString worldFileContent( const QgsMapSettings &mapSettings );

};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsMapSettingsUtils::EffectsCheckFlags )

#endif
