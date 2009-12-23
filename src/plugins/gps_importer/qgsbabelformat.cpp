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
/*  $Id$ */

#include "qgsbabelformat.h"

#include <QRegExp>
#include <QString>


QgsBabelFormat::QgsBabelFormat( const QString& name ) :
    mName( name ),
    mSupportsImport( false ), mSupportsExport( false ),
    mSupportsWaypoints( false ), mSupportsRoutes( false ), mSupportsTracks( false )
{
}


const QString& QgsBabelFormat::name() const
{
  return mName;
}


QStringList QgsBabelFormat::importCommand( const QString& babel,
    const QString& featuretype,
    const QString& input,
    const QString& output ) const
{
  QStringList empty;
  return empty;
}


QStringList QgsBabelFormat::exportCommand( const QString& babel,
    const QString& featuretype,
    const QString& input,
    const QString& output ) const
{
  QStringList empty;
  return empty;
}


bool QgsBabelFormat::supportsImport() const
{
  return mSupportsImport;
}


bool QgsBabelFormat::supportsExport() const
{
  return mSupportsExport;
}


bool QgsBabelFormat::supportsWaypoints() const
{
  return mSupportsWaypoints;
}


bool QgsBabelFormat::supportsRoutes() const
{
  return mSupportsRoutes;
}


bool QgsBabelFormat::supportsTracks() const
{
  return mSupportsTracks;
}



QgsSimpleBabelFormat::QgsSimpleBabelFormat( const QString& format,
    bool hasWaypoints, bool hasRoutes,
    bool hasTracks ) : mFormat( format )
{
  mSupportsWaypoints = hasWaypoints;
  mSupportsRoutes = hasRoutes;
  mSupportsTracks = hasTracks;
  mSupportsImport = true;
  mSupportsExport = false;
}


QStringList QgsSimpleBabelFormat::importCommand( const QString& babel,
    const QString& featuretype,
    const QString& input,
    const QString& output )const
{
  QStringList args;
  args << babel << featuretype << "-i" << mFormat << "-o" << "gpx" << input << output;
  return args;
}



QgsBabelCommand::QgsBabelCommand( const QString& importCmd,
                                  const QString& exportCmd )
{
  mSupportsWaypoints = true;
  mSupportsRoutes = true;
  mSupportsTracks = true;
  mSupportsImport = false;
  mSupportsExport = false;
  if ( !importCmd.isEmpty() )
  {
    mImportCmd = importCmd.split( QRegExp( "\\s" ), QString::SkipEmptyParts );
    mSupportsImport = true;
  }
  if ( !exportCmd.isEmpty() )
  {
    mExportCmd = exportCmd.split( QRegExp( "\\s" ), QString::SkipEmptyParts );
    mSupportsExport = true;
  }
}


QStringList QgsBabelCommand::importCommand( const QString& babel,
    const QString& featuretype,
    const QString& input,
    const QString& output ) const
{
  QStringList copy;
  QStringList::const_iterator iter;
  for ( iter = mImportCmd.begin(); iter != mImportCmd.end(); ++iter )
  {
    if ( *iter == "%babel" )
      copy.append( babel );
    else if ( *iter == "%type" )
      copy.append( featuretype );
    else if ( *iter == "%in" )
      copy.append( QString( "\"%1\"" ).arg( input ) );
    else if ( *iter == "%out" )
      copy.append( QString( "\"%1\"" ).arg( output ) );
    else
      copy.append( *iter );
  }
  return copy;
}


QStringList QgsBabelCommand::exportCommand( const QString& babel,
    const QString& featuretype,
    const QString& input,
    const QString& output ) const
{
  QStringList copy;
  QStringList::const_iterator iter;
  for ( iter = mExportCmd.begin(); iter != mExportCmd.end(); ++iter )
  {
    if ( *iter == "%babel" )
      copy.append( babel );
    else if ( *iter == "%type" )
      copy.append( featuretype );
    else if ( *iter == "%in" )
      copy.append( QString( "\"%1\"" ).arg( input ) );
    else if ( *iter == "%out" )
      copy.append( QString( "\"%1\"" ).arg( output ) );
    else
      copy.append( *iter );
  }
  return copy;
}
