/***************************************************************************
  qgssettingsregistrygui.h
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


#ifndef QGSSETTINGSREGISTRYGUI_H
#define QGSSETTINGSREGISTRYGUI_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgssettingsregistry.h"
#include "qgssettingsentry.h"
#include "qgssettingsgroupmap.h"

class QgsSettingsEntryStringList;

/**
 * \ingroup core
 * \class QgsSettingsRegistryGui
 *
 * \since QGIS 3.18
 */
class CORE_EXPORT QgsSettingsRegistryGui : public QgsSettingsRegistry
{
  public:

    /**
     * Constructor for QgsSettingsRegistryGui.
     */
    QgsSettingsRegistryGui();
    ~QgsSettingsRegistryGui() override;

    struct SettingsEntries
    {
        struct LocatorFilter : public QgsSettingsGroup
        {
          LocatorFilter( QgsSettingsGroup *parentGroup = nullptr )
            : QgsSettingsGroup( "", parentGroup )
            , enabled( "enabled", this, true, QObject::tr( "Enabled" ) )
            , byDefault( "default", this, false, QObject::tr( "Default value" ) )
            , prefix( "prefix", this, QString(), QObject::tr( "Locator filter prefix" ) )
          {}

          QgsSettingsEntryBool enabled;
          QgsSettingsEntryBool byDefault;
          QgsSettingsEntryString prefix;
        };

        struct LocatorFilters : public QgsSettingsGroupMap<LocatorFilter>
        {
          LocatorFilters( QgsSettingsGroup *parentGroup = nullptr )
            : QgsSettingsGroupMap( "locator_filters", parentGroup )
          {}
        };
        LocatorFilters locatorFilters;

    };

    SettingsEntries settingsEntries() const;

  private:

    SettingsEntries mSettingsEntries;
};

#endif // QGSSETTINGSREGISTRYGUI_H
