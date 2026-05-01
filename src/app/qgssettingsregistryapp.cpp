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

#include "qgisapp.h"
#include "qgsattributetabledialog.h"
#include "qgsgui.h"
#include "qgsidentifyresultsdialog.h"
#include "qgsimagewarper.h"
#include "qgspluginmanager.h"
#include "qgssettings.h"
#include "qgssettingseditorwidgetregistry.h"
#include "qgssettingsentryenumflag.h"
#include "qgssettingsentryimpl.h"
#include "qgssettingsenumflageditorwidgetwrapper.h"
#include "qgssettingsproxy.h"

#include <QString>

using namespace Qt::StringLiterals;

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

  // TODO: remove in QGIS 4.4 (after LTR 4.2)

  // single settings - added in 3.30
  QgsIdentifyResultsDialog::settingHideNullValues->copyValueFromKey( u"Map/hideNullValues"_s, true );
  QgsIdentifyResultsDialog::settingIdentifyExpand->copyValueFromKey( u"Map/identifyExpand"_s, true );
  QgsIdentifyResultsDialog::settingIdentifyAutoFeatureForm->copyValueFromKey( u"Map/identifyAutoFeatureForm"_s, true );
  QgsIdentifyResultsDialog::settingIdentifyAutoFeatureForm->copyValueFromKey( u"/Map/identifyAutoFeatureForm"_s, true );
  QgsIdentifyResultsDialog::settingHideDerivedAttributes->copyValueFromKey( u"Map/hideDerivedAttributes"_s, true );
  QgsIdentifyResultsDialog::settingHideDerivedAttributes->copyValueFromKey( u"/Map/hideDerivedAttributes"_s, true );
  QgsIdentifyResultsDialog::settingColumnWidth->copyValueFromKey( u"Windows/Identify/columnWidth"_s, true );
  QgsIdentifyResultsDialog::settingColumnWidthTable->copyValueFromKey( u"Windows/Identify/columnWidthTable"_s, true );
  QgisApp::settingsAskToDeleteFeatures->copyValueFromKey( u"app/askToDeleteFeatures"_s, true );
  QgsPluginManager::settingsAutomaticallyCheckForPluginUpdates->copyValueFromKey( u"plugins/automatically-check-for-updates"_s, true );
  QgsPluginManager::settingsAllowExperimental->copyValueFromKey( u"app/plugin_installer/allowExperimental"_s, true );
  QgsPluginManager::settingsAllowDeprecated->copyValueFromKey( u"app/plugin_installer/allowDeprecated"_s, true );
  QgsPluginManager::settingsCheckOnStartLastDate->copyValueFromKey( u"app/plugin_installer/checkOnStartLastDate"_s, true );
  QgsPluginManager::settingsSeenPlugins->copyValueFromKey( u"app/plugin_installer/seen_plugins"_s, true );
  QgsPluginManager::settingsLastZipDirectory->copyValueFromKey( u"app/plugin_installer/lastZipDirectory"_s, true );
  QgsPluginManager::settingsShowInstallFromZipWarning->copyValueFromKey( u"app/plugin_installer/showInstallFromZipWarning"_s, true );

  // attribute-table default add feature method
  // old key stored a string "attributeForm" / "attributeTable", new key stores the enum name ("Form" / "Table")
  {
    auto settings = QgsSettings::get();
    const QString oldKey = u"/qgis/attributeTableLastAddFeatureMethod"_s;
    if ( settings->contains( oldKey ) )
    {
      const QString lastMethod = settings->value( oldKey ).toString();
      const QgsAttributeTableConfig::AddFeatureMethod method = ( lastMethod == "attributeForm"_L1 ) ? QgsAttributeTableConfig::AddFeatureMethod::Form : QgsAttributeTableConfig::AddFeatureMethod::Table;
      QgsAttributeTableDialog::settingsDefaultAddFeatureMethod->setValue( method );
      settings->remove( oldKey );
    }
  }
}

QgsSettingsRegistryApp::~QgsSettingsRegistryApp()
{}
