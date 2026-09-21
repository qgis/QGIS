/***************************************************************************
                         qgsprocessingdefaultstyleregistry.cpp
                         ------------------------
    begin                : Septeber 2026
    copyright            : (C) 2026 by Nyall Dawson
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

#include "qgsprocessingdefaultstyleregistry.h"

#include "qgsprocessingutils.h"

#include <QDir>
#include <QFile>
#include <QString>

#include "moc_qgsprocessingdefaultstyleregistry.cpp"

using namespace Qt::StringLiterals;

QgsProcessingDefaultStyleRegistry::QgsProcessingDefaultStyleRegistry( QObject *parent )
  : QObject( parent )
{}

void QgsProcessingDefaultStyleRegistry::setDefaultStyleForOutput( const QString &algorithmId, const QString &outputName, const QString &qmlPath )
{
  mStyles[algorithmId][outputName] = qmlPath;
}

QString QgsProcessingDefaultStyleRegistry::defaultStyleForOutput( const QString &algorithmId, const QString &outputName ) const
{
  return mStyles.value( algorithmId ).value( outputName );
}

QString QgsProcessingDefaultStyleRegistry::configFilePath()
{
  const QDir dir( QgsProcessingUtils::userFolder() );
  return dir.filePath( u"processing_qgis_styles.conf"_s );
}

void QgsProcessingDefaultStyleRegistry::loadStyles()
{
  const QString filePath = configFilePath();
  if ( !QFile::exists( filePath ) )
    return;

  QFile file( filePath );
  if ( !file.open( QIODevice::ReadOnly ) )
    return;

  QTextStream in( &file );
  const QString originalFileContent = in.readAll();
  file.close();

  const QStringList lines = originalFileContent.split( '\n' );
  for ( const QString &line : lines )
  {
    const QStringList lineParts = line.split( '|' );
    if ( lineParts.size() < 3 )
      continue;

    const QString algorithmId = lineParts.at( 0 );
    const QString outputName = lineParts.at( 1 );
    if ( algorithmId.isEmpty() || outputName.isEmpty() )
      continue;

    const QString qmlFilePath = lineParts.mid( 2 ).join( '|' );

    mStyles[algorithmId][outputName] = qmlFilePath;
  }
}

void QgsProcessingDefaultStyleRegistry::saveStyles()
{
  const QString filePath = configFilePath();

  QFile file( filePath );
  if ( !file.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
  {
    file.close();
    return;
  }

  QTextStream out( &file );
  for ( auto algIt = mStyles.constBegin(); algIt != mStyles.constEnd(); ++algIt )
  {
    for ( auto outputIt = algIt->constBegin(); outputIt != algIt->constEnd(); ++outputIt )
    {
      out << algIt.key() << '|' << outputIt.key() << '|' << outputIt.value() << "\n";
    }
  }

  file.close();
}
