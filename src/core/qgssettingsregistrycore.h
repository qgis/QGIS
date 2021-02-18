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
#include "qgssettingsregistry.h"
#include "qgssettingsentry.h"
#include "qgssettingsgroup.h"
#include "qgssettingsmap.h"

class QgsSettingsEntryStringList;

/**
 * \ingroup core
 * \class QgsSettingsRegistryCore
 *
 * \since QGIS 3.17
 */
class CORE_EXPORT QgsSettingsRegistryCore : public QgsSettingsRegistry
{
  public:

    /**
     * Constructor for QgsSettingsRegistryCore.
     */
    QgsSettingsRegistryCore();
    ~QgsSettingsRegistryCore() override;

    struct SettingsEntries
    {
        struct Layout : public QgsSettingsGroup
        {
            struct SubLayout : public QgsSettingsGroup
            {
              QgsSettingsEntryStringList searchPathForTemplatesInSub;
            };
            SubLayout subLayout;

            QgsSettingsEntryStringList searchPathForTemplates;
            QgsSettingsEntryInteger anotherNumericSettings;
        };
        Layout layout;

        struct Measure : public QgsSettingsGroup
        {
          QgsSettingsEntryBool planimetric;
        };
        Measure measure;

        struct LocatorFilter : public QgsSettingsGroup
        {
          LocatorFilter()
            : enabled( "enabled", this, true, "" )
            , byDefault( "default", this, false, "" )
            , prefix( "prefix", this, QString(), "" )
          {}

          QgsSettingsEntryBool enabled;
          QgsSettingsEntryBool byDefault;
          QgsSettingsEntryString prefix;
        };

        QgsSettingsMap<LocatorFilter> locatorFilters;

    };

    SettingsEntries settingsEntries() const;

  private:

    SettingsEntries mSettingsEntries;
};

#endif // QGSSETTINGSREGISTRYCORE_H
