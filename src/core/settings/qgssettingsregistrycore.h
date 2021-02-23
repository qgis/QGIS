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
class CORE_EXPORT QgsSettingsRegistryCore : public QgsSettingsGroup
{
  public:

    QgsSettingsRegistryCore()
      : QgsSettingsGroup( QgsSettings::Core, QObject::tr( "Settings of section core" ) )
    {}

//    struct Layout : public QgsSettingsGroup
//    {
//        Layout(QgsSettingsGroup *parent)
//          : QgsSettingsGroup( "layout", parent, QObject::tr( "Layout group description" ) )
//          , searchPathForTemplates( "searchPathsForTemplates", this, QStringList(), QObject::tr( "Search path for templates" ) )
//          , anotherNumericSettings( "anotherNumericSettings", this, 1234, "Example settings", 100, 9999 )
//          , subLayout( this )
//        {}

//        QgsSettingsEntryStringList searchPathForTemplates;
//        QgsSettingsEntryInteger anotherNumericSettings;

//        struct SubLayout : public QgsSettingsGroup
//        {
//          SubLayout( QgsSettingsGroup *parentGroup )
//            : QgsSettingsGroup( "sub_layout", parentGroup, "Description..." )
//            , searchPathForTemplatesInSub( "anotherValue", this, QStringList() )
//          {}

//          QgsSettingsEntryStringList searchPathForTemplatesInSub;
//        };
//        SubLayout subLayout;
//    };
//    Layout layout;

//    struct Measure : public QgsSettingsGroup
//    {
//      Measure(QgsSettingsGroup *parent)
//        : QgsSettingsGroup( "measure", parent, QObject::tr( "Measure group description" ) )
//        , planimetric( "planimetric", this, false, QObject::tr( "Planimetric description" ) )
//      {}

//      QgsSettingsEntryBool planimetric;
//    };
//    Measure measure;
};

#endif // QGSSETTINGSREGISTRYCORE_H
