/***************************************************************************
  qgssettingsregistrygui.cpp
  --------------------------------------
  Date                 : July 2021
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

#include "qgssettingsregistrygui.h"

#include "qgsabstractdbsourceselect.h"
#include "qgsapplication.h"
#include "qgssettingsregistrycore.h"
#include "qgsstylemanagerdialog.h"

const QgsSettingsEntryBool *QgsSettingsRegistryGui::settingsRespectScreenDPI = new QgsSettingsEntryBool( u"respect-screen-dpi"_s, QgsSettingsTree::sTreeGui, false );

QgsSettingsRegistryGui::QgsSettingsRegistryGui()
  : QgsSettingsRegistry()
{
  // copy values from old keys to new keys and delete the old ones
  // for backward compatibility, old keys are recreated when the registry gets deleted

  // single settings - added in 3.30
  settingsRespectScreenDPI->copyValueFromKey( u"gui/qgis/respect_screen_dpi"_s, {}, true );

  QgsAbstractDbSourceSelect::settingHoldDialogOpen->copyValueFromKey( u"ogr/GPKGSourceSelect/HoldDialogOpen"_s, { u"ogr/GPKGSourceSelect"_s }, true );
  QgsAbstractDbSourceSelect::settingHoldDialogOpen->copyValueFromKey( u"ogr/SQLiteSourceSelect/HoldDialogOpen"_s, { u"ogr/SQLiteSourceSelect"_s }, true );
  QgsAbstractDbSourceSelect::settingHoldDialogOpen->copyValueFromKey( u"Windows/MSSQLSourceSelect/HoldDialogOpen"_s, { u"MSSQLSourceSelect"_s }, true );
  QgsAbstractDbSourceSelect::settingHoldDialogOpen->copyValueFromKey( u"Windows/PgSourceSelect/HoldDialogOpen"_s, { u"PgSourceSelect"_s }, true );
  QgsAbstractDbSourceSelect::settingHoldDialogOpen->copyValueFromKey( u"Windows/SpatiaLiteSourceSelect/HoldDialogOpen"_s, { u"SpatiaLiteSourceSelect"_s }, true );
}

QgsSettingsRegistryGui::~QgsSettingsRegistryGui()
{
  // TODO QGIS 5.0: Remove
  // backward compatibility for settings
  settingsRespectScreenDPI->copyValueToKeyIfChanged( u"gui/qgis/respect_screen_dpi"_s );
}
