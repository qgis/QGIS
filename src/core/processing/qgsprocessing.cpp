/***************************************************************************
                         qgsprocessing.h
                         ---------------
    begin                : July 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprocessing.h"

const QgsSettingsEntryBool *QgsProcessing::settingsPreferFilenameAsLayerName = new QgsSettingsEntryBool( QStringLiteral( "prefer-filename-as-layer-name" ), sTreeConfiguration, true, QObject::tr( "Prefer filename as layer name" ) );

const QgsSettingsEntryString *QgsProcessing::settingsTempPath = new QgsSettingsEntryString( QStringLiteral( "temp-path" ), sTreeConfiguration, QString(), QObject::tr( "Override temporary output folder path" ) );

const QgsSettingsEntryString *QgsProcessing::settingsDefaultOutputVectorLayerExt = new QgsSettingsEntryString( QStringLiteral( "default-output-vector-ext" ), sTreeConfiguration, QString(), QObject::tr( "Default output vector layer extension" ) );

const QgsSettingsEntryString *QgsProcessing::settingsDefaultOutputRasterLayerExt = new QgsSettingsEntryString( QStringLiteral( "default-output-raster-ext" ), sTreeConfiguration, QString(), QObject::tr( "Default output raster layer extension" ) );

const QString QgsProcessing::TEMPORARY_OUTPUT = QStringLiteral( "TEMPORARY_OUTPUT" );
