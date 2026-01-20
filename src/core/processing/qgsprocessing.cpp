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

#include "moc_qgsprocessing.cpp"

const QgsSettingsEntryBool *QgsProcessing::settingsPreferFilenameAsLayerName = new QgsSettingsEntryBool( u"prefer-filename-as-layer-name"_s, sTreeConfiguration, true, QObject::tr( "Prefer filename as layer name" ) );

const QgsSettingsEntryString *QgsProcessing::settingsTempPath = new QgsSettingsEntryString( u"temp-path"_s, sTreeConfiguration, QString(), QObject::tr( "Override temporary output folder path" ) );

const QgsSettingsEntryString *QgsProcessing::settingsDefaultOutputVectorLayerExt = new QgsSettingsEntryString( u"default-output-vector-ext"_s, sTreeConfiguration, QString(), QObject::tr( "Default output vector layer extension" ) );

const QgsSettingsEntryString *QgsProcessing::settingsDefaultOutputRasterLayerFormat = new QgsSettingsEntryString( u"default-output-raster-format"_s, sTreeConfiguration, QString(), QObject::tr( "Default output raster layer format" ) );

const QString QgsProcessing::TEMPORARY_OUTPUT = u"TEMPORARY_OUTPUT"_s;

QString QgsProcessing::documentationFlagToString( Qgis::ProcessingAlgorithmDocumentationFlag flag )
{
  switch ( flag )
  {
    case Qgis::ProcessingAlgorithmDocumentationFlag::RegeneratesPrimaryKey:
      return QObject::tr( "This algorithm drops existing primary keys or FID values and regenerates them in output layers." );
    case Qgis::ProcessingAlgorithmDocumentationFlag::RegeneratesPrimaryKeyInSomeScenarios:
      return QObject::tr( "This algorithm may drop existing primary keys or FID values and regenerate them in output layers, depending on the input parameters." );
    case Qgis::ProcessingAlgorithmDocumentationFlag::RespectsEllipsoid:
      return QObject::tr( "This algorithm uses ellipsoid based measurements and respects the current ellipsoid settings." );
  }
  BUILTIN_UNREACHABLE
}
