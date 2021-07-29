/***************************************************************************
  qgsbabelformat.cpp - import/export formats for GPSBabel
   -------------------
  begin                : Oct 20, 2004
  copyright            : (C) 2004 by Lars Luthman
  email                : larsl@users.sourceforge.net

***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsbabelformat.h"

#include <QRegExp>
#include <QString>


QgsAbstractBabelFormat::QgsAbstractBabelFormat( const QString &name )
  : mName( name )
{
}

QString QgsAbstractBabelFormat::featureTypeToArgument( Qgis::GpsFeatureType type )
{
  switch ( type )
  {
    case Qgis::GpsFeatureType::Waypoint:
      return QStringLiteral( "-w" );
    case Qgis::GpsFeatureType::Route:
      return QStringLiteral( "-r" );
    case Qgis::GpsFeatureType::Track:
      return QStringLiteral( "-t" );
  }
  return QString();
}

QString QgsAbstractBabelFormat::name() const
{
  return mName;
}

QStringList QgsAbstractBabelFormat::importCommand( const QString &, Qgis::GpsFeatureType, const QString &, const QString & ) const
{
  return QStringList();
}

QStringList QgsAbstractBabelFormat::exportCommand( const QString &, Qgis::GpsFeatureType, const QString &, const QString & ) const
{
  return QStringList();
}

Qgis::BabelFormatCapabilities QgsAbstractBabelFormat::capabilities() const
{
  return mCapabilities;
}


//
// QgsSimpleBabelFormat
//

QgsBabelSimpleImportFormat::QgsBabelSimpleImportFormat( const QString &format,
    Qgis::BabelFormatCapabilities capabilities )
  : mFormat( format )
{
  mCapabilities = capabilities;
  mCapabilities |= Qgis::BabelFormatCapability::Import;
  mCapabilities &= ~( static_cast< int >( Qgis::BabelFormatCapability::Export ) );
}

QStringList QgsBabelSimpleImportFormat::importCommand( const QString &babel,
    Qgis::GpsFeatureType featureType,
    const QString &input,
    const QString &output )const
{
  return { QStringLiteral( "\"%1\"" ).arg( babel ),
           featureTypeToArgument( featureType ),
           QStringLiteral( "-i" ),
           mFormat,
           QStringLiteral( "-o" ),
           QStringLiteral( "gpx" ),
           QStringLiteral( "\"%1\"" ).arg( input ),
           QStringLiteral( "\"%1\"" ).arg( output ) };
}
