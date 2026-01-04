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

#include "qgsgui.h"
#include "qgsidentifyresultsdialog.h"
#include "qgsimagewarper.h"
#include "qgspluginmanager.h"
#include "qgssettingseditorwidgetregistry.h"
#include "qgssettingsentryenumflag.h"
#include "qgssettingsentryimpl.h"
#include "qgssettingsenumflageditorwidgetwrapper.h"

#ifdef HAVE_GEOREFERENCER
#include "qgsgcptransformer.h"
#endif

#if defined( _MSC_VER )
#ifndef SIP_RUN
template class QgsSettingsEnumEditorWidgetWrapper<QgsImageWarper::ResamplingMethod>;
template class QgsSettingsEnumEditorWidgetWrapper<QgsGcpTransformerInterface::TransformMethod>;
#endif
#endif


QgsSettingsRegistryApp::QgsSettingsRegistryApp()
  : QgsSettingsRegistry()
{
#ifdef HAVE_GEOREFERENCER
  QgsGui::instance()->settingsEditorWidgetRegistry()->addWrapper( new QgsSettingsEnumEditorWidgetWrapper<QgsImageWarper::ResamplingMethod>() );
  QgsGui::instance()->settingsEditorWidgetRegistry()->addWrapper( new QgsSettingsEnumEditorWidgetWrapper<QgsGcpTransformerInterface::TransformMethod>() );
#endif

  // copy values from old keys to new keys and delete the old ones
  // for backward compatibility, old keys are recreated when the registry gets deleted

  // single settings - added in 3.30
  QgsIdentifyResultsDialog::settingHideNullValues->copyValueFromKey( u"Map/hideNullValues"_s, true );
  QgsPluginManager::settingsAutomaticallyCheckForPluginUpdates->copyValueFromKey( u"plugins/automatically-check-for-updates"_s, true );
  QgsPluginManager::settingsAllowExperimental->copyValueFromKey( u"app/plugin_installer/allowExperimental"_s, true );
  QgsPluginManager::settingsAllowDeprecated->copyValueFromKey( u"app/plugin_installer/allowDeprecated"_s, true );
  QgsPluginManager::settingsCheckOnStartLastDate->copyValueFromKey( u"app/plugin_installer/checkOnStartLastDate"_s, true );
  QgsPluginManager::settingsSeenPlugins->copyValueFromKey( u"app/plugin_installer/seen_plugins"_s, true );
  QgsPluginManager::settingsLastZipDirectory->copyValueFromKey( u"app/plugin_installer/lastZipDirectory"_s, true );
  QgsPluginManager::settingsShowInstallFromZipWarning->copyValueFromKey( u"app/plugin_installer/showInstallFromZipWarning"_s, true );
}

QgsSettingsRegistryApp::~QgsSettingsRegistryApp()
{
  // TODO QGIS 5.0: Remove
  // backward compatibility for settings
  QgsIdentifyResultsDialog::settingHideNullValues->copyValueToKeyIfChanged( u"Map/hideNullValues"_s );
  QgsPluginManager::settingsAutomaticallyCheckForPluginUpdates->copyValueToKeyIfChanged( u"plugins/automatically-check-for-updates"_s );
  QgsPluginManager::settingsAllowExperimental->copyValueToKeyIfChanged( u"app/plugin_installer/allowExperimental"_s );
  QgsPluginManager::settingsAllowDeprecated->copyValueToKeyIfChanged( u"app/plugin_installer/allowDeprecated"_s );
  QgsPluginManager::settingsCheckOnStartLastDate->copyValueFromKey( u"app/plugin_installer/checkOnStartLastDate"_s, true );
  QgsPluginManager::settingsSeenPlugins->copyValueFromKey( u"app/plugin_installer/seen_plugins"_s, true );
  QgsPluginManager::settingsLastZipDirectory->copyValueFromKey( u"app/plugin_installer/lastZipDirectory"_s, true );
  QgsPluginManager::settingsShowInstallFromZipWarning->copyValueFromKey( u"app/plugin_installer/showInstallFromZipWarning"_s, true );
}
