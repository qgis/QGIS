/***************************************************************************
  qgssettingsregistry.h
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


#ifndef QGSSETTINGSREGISTRY_H
#define QGSSETTINGSREGISTRY_H

#include "qgis.h"
#include "qgis_core.h"
#include "qgis_sip.h"

#include <QMap>

class QgsSettingsEntryBase;
class QgsSettingsEntryGroup;

/**
 * \ingroup core
 * \class QgsSettingsRegistry
 * \brief QgsSettingsRegistry is used for settings introspection and collects a
 * list of child QgsSettingsRegistry and a list of child QgsSettingsRegistry
 *
 * \since QGIS 3.20
 * \deprecated QGIS 3.30. Use QgsSettings::treeRoot() instead.
 */
class CORE_DEPRECATED_EXPORT QgsSettingsRegistry
{
  public:

    QgsSettingsRegistry();
    virtual ~QgsSettingsRegistry();

    /**
     * Returns the list of registered QgsSettingsEntryBase.
     */
    QList<const QgsSettingsEntryBase *> settingEntries() const;

    /**
     * Returns the QgsSettingsEntry with the given \a key or nullptr if not found.
     *
     * The \a searchChildRegistries parameter specifies if child registries should be included in the search
     */
    const QgsSettingsEntryBase *settingsEntry( const QString &key, bool searchChildRegistries = true ) const;

    /**
     * Append a child \a settingsRegistry to the register.
     */
    void addSubRegistry( const QgsSettingsRegistry *settingsRegistry );

    /**
     * Remove a child \a settingsRegistry from the register.
     */
    void removeSubRegistry( const QgsSettingsRegistry *settingsRegistry );

    /**
     * Returns the list of registered child QgsSettingsRegistry.
     */
    QList<const QgsSettingsRegistry *> subRegistries() const;

  protected:

    /**
     * Adds \a settingsEntry to the registry.
     */
    bool addSettingsEntry( const QgsSettingsEntryBase *settingsEntry );

    /**
     * Adds a group of setting to the registry
     * \since QGIS 3.26
     * \deprecated QGIS 3.30
     */
    Q_DECL_DEPRECATED void addSettingsEntryGroup( const QgsSettingsEntryGroup *settingsGroup ) SIP_DEPRECATED;

  private:

    QMap<QString, const QgsSettingsEntryBase *> mSettingsEntriesMap;

    Q_NOWARN_DEPRECATED_PUSH
    QMap<const QgsSettingsEntryBase *, const QgsSettingsEntryGroup *> mSettingsEntriesGroupMap;
    Q_NOWARN_DEPRECATED_POP

    QList<const QgsSettingsRegistry *> mSettingsRegistryChildList;

    friend class QgsSettingsEntryBase;

};

#endif // QGSSETTINGSREGISTRY_H
