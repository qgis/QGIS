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

#include "qgslayout.h"
#include "qgslocator.h"

class QgsSettingsEntryStringList;

/**
 * \ingroup core
 * \class QgsSettingsRegistryCore
 *
 * \since QGIS 3.18
 */
class CORE_EXPORT QgsSettingsRegistryCore : public QgsSettingsGroup
{
  public:

    QgsSettingsRegistryCore()
      : QgsSettingsGroup( QgsSettings::Core, QObject::tr( "Settings of section core" ) )
      , layout( this )
      , locatorFilters( this )
      , measure( this )
    {}

    QgsLayout::SettingsStructure::Layout layout;
    QgsLocator::SettingsStructure::LocatorFilters locatorFilters;


    // Settings that don't have a clear appartenance class can be defined here:

    struct Measure : public QgsSettingsGroup
    {
      Measure( QgsSettingsGroup *parent )
        : QgsSettingsGroup( "measure", parent, QObject::tr( "Measure group description" ) )
        , planimetric( "planimetric", this, false, QObject::tr( "Planimetric settings description" ) )
      {}

      QgsSettingsEntryBool planimetric;
    };
    Measure measure;

};

#endif // QGSSETTINGSREGISTRYCORE_H
