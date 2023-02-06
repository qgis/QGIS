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

const QgsSettingsEntryString *QgsProcessing::settingsTempPath = new QgsSettingsEntryString( QStringLiteral( "temp-path" ), sTreeConfiguration, QString() );

const QgsSettingsEntryInteger *QgsProcessing::settingsDefaultOutputVectorLayerExt = new QgsSettingsEntryInteger( QStringLiteral( "default-output-vector-layer-ext" ), sTreeConfiguration, -1 );

const QgsSettingsEntryInteger *QgsProcessing::settingsDefaultOutputRasterLayerExt = new QgsSettingsEntryInteger( QStringLiteral( "default-output-raster-layer-ext" ), sTreeConfiguration, -1 );

const QString QgsProcessing::TEMPORARY_OUTPUT = QStringLiteral( "TEMPORARY_OUTPUT" );
