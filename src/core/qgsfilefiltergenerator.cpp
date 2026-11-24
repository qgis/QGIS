/***************************************************************************
                         qgsfilefiltergenerator.h
                         ------------------------
    begin                : March 2020
    copyright            : (C) 2020 by Nyall Dawson
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

#include "qgsfilefiltergenerator.h"

#include <QString>

using namespace Qt::StringLiterals;

QString QgsFileFilterGenerator::fileFilter( StandardFileFormat format )
{
  switch ( format )
  {
    case StandardFileFormat::QML:
      return u"%1 (*.qml *.QML)"_s.arg( tr( "QGIS Layer Style Files" ) );
    case StandardFileFormat::HTML:
      return u"%1 (*.html *.HTML)"_s.arg( tr( "HTML Files" ) );
    case StandardFileFormat::TXT:
      return u"%1 (*.txt *.TXT)"_s.arg( tr( "Text Files" ) );
    case StandardFileFormat::SLD:
      return u"%1 (*.sld *.SLD)"_s.arg( tr( "SLD Files" ) );
    case StandardFileFormat::AllFiles:
      return u"%1 (*.*)"_s.arg( tr( "All Files" ) );
  }
  BUILTIN_UNREACHABLE
}

QString QgsFileFilterGenerator::filterString( const QList<StandardFileFormat> &formats, bool includeAllFiles )
{
  QStringList parts;
  parts.reserve( formats.size() + ( includeAllFiles ? 1 : 0 ) );
  for ( StandardFileFormat format : formats )
  {
    if ( format == StandardFileFormat::AllFiles )
    {
      // make sure all files option is always last
      includeAllFiles = true;
      continue;
    }
    parts.append( fileFilter( format ) );
  }

  if ( includeAllFiles )
  {
    parts.append( fileFilter( StandardFileFormat::AllFiles ) );
  }

  return parts.join( ";;" );
}
