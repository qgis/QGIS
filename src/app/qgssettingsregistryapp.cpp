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

QgsSettingsRegistryApp::QgsSettingsRegistryApp()
  : QgsSettingsRegistry()
{
  // copy values from old keys to new keys and delete the old ones
  // for backward compatibility, old keys are recreated when the registry gets deleted

  // single settings - added in 3.30
  QgsIdentifyResultsDialog::settingHideNullValues->copyValueFromKey( QStringLiteral( "Map/hideNullValues" ), {}, true );

}

QgsSettingsRegistryApp::~QgsSettingsRegistryApp()
{
  // TODO QGIS 4.0: Remove
  // backward compatibility for settings
  QgsIdentifyResultsDialog::settingHideNullValues->copyValueToKey( QStringLiteral( "Map/hideNullValues" ) );
}
