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

#include "qgsapplication.h"
#include "qgssettingsregistrycore.h"
#include "qgsstylemanagerdialog.h"
#include "qgsabstractdbsourceselect.h"

const QgsSettingsEntryBool *QgsSettingsRegistryGui::settingsRespectScreenDPI = new QgsSettingsEntryBool( QStringLiteral( "respect-screen-dpi" ), QgsSettingsTree::sTreeGui, false );

QgsSettingsRegistryGui::QgsSettingsRegistryGui()
  : QgsSettingsRegistry()
{
  // copy values from old keys to new keys and delete the old ones
  // for backward compatibility, old keys are recreated when the registry gets deleted

  // single settings - added in 3.30
  settingsRespectScreenDPI->copyValueFromKey( QStringLiteral( "gui/qgis/respect_screen_dpi" ), {}, true );

  QgsAbstractDbSourceSelect::settingHoldDialogOpen->copyValueFromKey( QStringLiteral( "ogr/GPKGSourceSelect/HoldDialogOpen" ), { QStringLiteral( "ogr/GPKGSourceSelect" ) }, true );
  QgsAbstractDbSourceSelect::settingHoldDialogOpen->copyValueFromKey( QStringLiteral( "ogr/SQLiteSourceSelect/HoldDialogOpen" ), { QStringLiteral( "ogr/SQLiteSourceSelect" ) }, true );
  QgsAbstractDbSourceSelect::settingHoldDialogOpen->copyValueFromKey( QStringLiteral( "Windows/MSSQLSourceSelect/HoldDialogOpen" ), { QStringLiteral( "MSSQLSourceSelect" ) }, true );
  QgsAbstractDbSourceSelect::settingHoldDialogOpen->copyValueFromKey( QStringLiteral( "Windows/PgSourceSelect/HoldDialogOpen" ), { QStringLiteral( "PgSourceSelect" ) }, true );
  QgsAbstractDbSourceSelect::settingHoldDialogOpen->copyValueFromKey( QStringLiteral( "Windows/SpatiaLiteSourceSelect/HoldDialogOpen" ), { QStringLiteral( "SpatiaLiteSourceSelect" ) }, true );
}

QgsSettingsRegistryGui::~QgsSettingsRegistryGui()
{
  // TODO QGIS 4.0: Remove
  // backward compatibility for settings
  settingsRespectScreenDPI->copyValueToKeyIfChanged( QStringLiteral( "gui/qgis/respect_screen_dpi" ) );
}
