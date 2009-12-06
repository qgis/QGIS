/***************************************************************************
     qgsgpsdevice.cpp
     --------------------------------------
    Date                 : Sun Sep 16 12:04:15 AKDT 2007
    Copyright            : (C) 2007 by Gary E. Sherman
    Email                : sherman at mrcc dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <QRegExp>

#include "qgsgpsdevice.h"


QgsGPSDevice::QgsGPSDevice( const QString& wptDlCmd, const QString& wptUlCmd,
                            const QString& rteDlCmd, const QString& rteUlCmd,
                            const QString& trkDlCmd, const QString& trkUlCmd )
{
  if ( !wptDlCmd.isEmpty() )
    mWptDlCmd = wptDlCmd.split( QRegExp( "\\s" ), QString::SkipEmptyParts );
  if ( !wptUlCmd.isEmpty() )
    mWptUlCmd = wptUlCmd.split( QRegExp( "\\s" ), QString::SkipEmptyParts );
  if ( !rteDlCmd.isEmpty() )
    mRteDlCmd = rteDlCmd.split( QRegExp( "\\s" ), QString::SkipEmptyParts );
  if ( !rteUlCmd.isEmpty() )
    mRteUlCmd = rteUlCmd.split( QRegExp( "\\s" ), QString::SkipEmptyParts );
  if ( !trkDlCmd.isEmpty() )
    mTrkDlCmd = trkDlCmd.split( QRegExp( "\\s" ), QString::SkipEmptyParts );
  if ( !trkUlCmd.isEmpty() )
    mTrkUlCmd = trkUlCmd.split( QRegExp( "\\s" ), QString::SkipEmptyParts );
}


QStringList QgsGPSDevice::importCommand( const QString& babel,
    const QString& type,
    const QString& in,
    const QString& out ) const
{
  const QStringList* original;
  if ( type == "-w" )
    original = &mWptDlCmd;
  else if ( type == "-r" )
    original = &mRteDlCmd;
  else if ( type == "-t" )
    original = &mTrkDlCmd;
  else throw "Bad error!";
  QStringList copy;
  QStringList::const_iterator iter;
  for ( iter = original->begin(); iter != original->end(); ++iter )
  {
    if ( *iter == "%babel" )
      copy.append( babel );
    else if ( *iter == "%type" )
      copy.append( type );
    else if ( *iter == "%in" )
      copy.append( QString( "\"%1\"").arg( in ) );
    else if ( *iter == "%out" )
      copy.append( QString( "\"%1\"").arg( out ) );
    else
      copy.append( *iter );
  }
  return copy;
}


QStringList QgsGPSDevice::exportCommand( const QString& babel,
    const QString& type,
    const QString& in,
    const QString& out ) const
{
  const QStringList* original;
  if ( type == "-w" )
    original = &mWptUlCmd;
  else if ( type == "-r" )
    original = &mRteUlCmd;
  else if ( type == "-t" )
    original = &mTrkUlCmd;
  else throw "Bad error!";
  QStringList copy;
  QStringList::const_iterator iter;
  for ( iter = original->begin(); iter != original->end(); ++iter )
  {
    if ( *iter == "%babel" )
      copy.append( babel );
    else if ( *iter == "%type" )
      copy.append( type );
    else if ( *iter == "%in" )
      copy.append( QString("\"%1\"").arg( in ) );
    else if ( *iter == "%out" )
      copy.append( QString("\"%1\"").arg( out ) );
    else
      copy.append( *iter );
  }
  return copy;
}



