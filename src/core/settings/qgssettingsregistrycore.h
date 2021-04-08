/***************************************************************************
  qgssettingsregistrycore.h
  --------------------------------------
  Date                 : February 2021
  Copyright            : (C) 2021 by Damiano Lombardi
  Email                : damiano at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef QGSSETTINGSREGISTRYCORE_H
#define QGSSETTINGSREGISTRYCORE_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgssettingsentry.h"

#include <QMap>

/**
 * \ingroup core
 * \class QgsSettingsRegistryCore
 * QgsSettingsRegistryCore is used for settings introspection and collects all
 * QgsSettingsEntry instances of core.
 *
 * \since QGIS 3.20
 */
class CORE_EXPORT QgsSettingsRegistryCore
{
  public:

    /**
     * Constructor for QgsSettingsRegistryCore.
     */
    QgsSettingsRegistryCore();

    /**
     * Destructor for QgsSettingsRegistryCore.
     */
    virtual ~QgsSettingsRegistryCore();

    /**
     * Returns the QgsSettingsEntry with the given \a key or nullptr if not found.
     */
    const QgsSettingsEntryBase *getSettingsEntry( const QString &key );

    /**
     * Add \a settingsEntry to the register.
     */
    void addSettingsEntry( const QgsSettingsEntryBase *settingsEntry );

  private:

    QMap<QString, const QgsSettingsEntryBase *> mSettingsEntriesMap;
    QMap<QString, const QgsSettingsEntryBase *> mDynamicSettingsEntriesMap;

};

#endif // QGSSETTINGSREGISTRYCORE_H
