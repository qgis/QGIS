/***************************************************************************
                         qgssettingsregistryapp.h
                         ----------------------
    begin                : January 2022
    copyright            : (C) 2022 by Denis Rouzaud
    email                : denis@opengis.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgssettingsregistryapp.h"
#include "qgsidentifyresultsdialog.h"
#include "qgspluginmanager.h"


QgsSettingsRegistryApp::QgsSettingsRegistryApp()
  : QgsSettingsRegistry()
{
  // copy values from old keys to new keys and delete the old ones
  // for backward compatibility, old keys are recreated when the registry gets deleted

  // single settings - added in 3.30
  QgsIdentifyResultsDialog::settingHideNullValues->copyValueFromKey( QStringLiteral( "Map/hideNullValues" ), true );
  QgsPluginManager::settingsAutomaticallyCheckForPluginUpdates->copyValueFromKey( QStringLiteral( "plugins/automatically-check-for-updates" ), true );
  QgsPluginManager::settingsAllowExperimental->copyValueFromKey( QStringLiteral( "app/plugin_installer/allowExperimental" ), true );
  QgsPluginManager::settingsAllowDeprecated->copyValueFromKey( QStringLiteral( "app/plugin_installer/allowDeprecated" ), true );
  QgsPluginManager::settingsCheckOnStartLastDate->copyValueFromKey( QStringLiteral( "app/plugin_installer/checkOnStartLastDate" ), true );
  QgsPluginManager::settingsSeenPlugins->copyValueFromKey( QStringLiteral( "app/plugin_installer/seen_plugins" ), true );
  QgsPluginManager::settingsLastZipDirectory->copyValueFromKey( QStringLiteral( "app/plugin_installer/lastZipDirectory" ), true );
  QgsPluginManager::settingsShowInstallFromZipWarning->copyValueFromKey( QStringLiteral( "app/plugin_installer/showInstallFromZipWarning" ), true );
}

QgsSettingsRegistryApp::~QgsSettingsRegistryApp()
{
  // TODO QGIS 4.0: Remove
  // backward compatibility for settings
  QgsIdentifyResultsDialog::settingHideNullValues->copyValueToKey( QStringLiteral( "Map/hideNullValues" ) );
  QgsPluginManager::settingsAutomaticallyCheckForPluginUpdates->copyValueToKey( QStringLiteral( "plugins/automatically-check-for-updates" ) );
  QgsPluginManager::settingsAllowExperimental->copyValueToKey( QStringLiteral( "app/plugin_installer/allowExperimental" ) );
  QgsPluginManager::settingsAllowDeprecated->copyValueToKey( QStringLiteral( "app/plugin_installer/allowDeprecated" ) );
  QgsPluginManager::settingsCheckOnStartLastDate->copyValueFromKey( QStringLiteral( "app/plugin_installer/checkOnStartLastDate" ), true );
  QgsPluginManager::settingsSeenPlugins->copyValueFromKey( QStringLiteral( "app/plugin_installer/seen_plugins" ), true );
  QgsPluginManager::settingsLastZipDirectory->copyValueFromKey( QStringLiteral( "app/plugin_installer/lastZipDirectory" ), true );
  QgsPluginManager::settingsShowInstallFromZipWarning->copyValueFromKey( QStringLiteral( "app/plugin_installer/showInstallFromZipWarning" ), true );
}
