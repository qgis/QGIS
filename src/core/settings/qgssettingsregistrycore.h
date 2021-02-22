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
#include "qgssettingsgroupmap.h"

class QgsSettingsEntryStringList;

/**
 * \ingroup core
 * \class QgsSettingsRegistryCore
 *
 * \since QGIS 3.18
 */
class CORE_EXPORT QgsSettingsRegistryCore
{
  public:

    /**
     * Constructor for QgsSettingsRegistryCore.
     */
    QgsSettingsRegistryCore();
    ~QgsSettingsRegistryCore();

    struct Layout : public QgsSettingsGroup
    {
        Layout()
          : QgsSettingsGroup( "layout", QgsSettings::Core, QObject::tr( "Layout group description" ) )
          , searchPathForTemplates( "searchPathsForTemplates", this, QStringList(), QObject::tr( "Search path for templates" ) )
          , anotherNumericSettings( "anotherNumericSettings", this, 1234, "Example settings", 100, 9999 )
          , subLayout( this )
        {}

        QgsSettingsEntryStringList searchPathForTemplates;
        QgsSettingsEntryInteger anotherNumericSettings;

        struct SubLayout : public QgsSettingsGroup
        {
          SubLayout( QgsSettingsGroup *parentGroup )
            : QgsSettingsGroup( "sub_layout", parentGroup, "Description..." )
            , searchPathForTemplatesInSub( "anotherValue", this, QStringList() )
          {}

          QgsSettingsEntryStringList searchPathForTemplatesInSub;
        };
        SubLayout subLayout;
    };
    Layout layout;

    struct Measure : public QgsSettingsGroup
    {
      Measure()
        : QgsSettingsGroup( "measure", QgsSettings::Core, QObject::tr( "Measure group description" ) )
        , planimetric( "planimetric", this, false, QObject::tr( "Planimetric description" ) )
      {}

      QgsSettingsEntryBool planimetric;
    };
    Measure measure;

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

#ifndef SIP_RUN
    struct LocatorFilters : public QgsSettingsGroupMap<LocatorFilter>
    {
      LocatorFilters()
        : QgsSettingsGroupMap( "locator_filters", QgsSettings::Core )
      {}
    };
    LocatorFilters locatorFilters;
#endif
};

#endif // QGSSETTINGSREGISTRYCORE_H
