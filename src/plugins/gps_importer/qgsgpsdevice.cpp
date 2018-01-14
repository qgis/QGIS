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


QgsGpsDevice::QgsGpsDevice( const QString &wptDlCmd, const QString &wptUlCmd,
                            const QString &rteDlCmd, const QString &rteUlCmd,
                            const QString &trkDlCmd, const QString &trkUlCmd )
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


QStringList QgsGpsDevice::importCommand( const QString &babel,
    const QString &type,
    const QString &in,
    const QString &out ) const
{
  const QStringList *original = nullptr;
  if ( type == QLatin1String( "-w" ) )
    original = &mWptDlCmd;
  else if ( type == QLatin1String( "-r" ) )
    original = &mRteDlCmd;
  else if ( type == QLatin1String( "-t" ) )
    original = &mTrkDlCmd;
  else throw "Bad error!";
  QStringList copy;
  QStringList::const_iterator iter;
  for ( iter = original->begin(); iter != original->end(); ++iter )
  {
    if ( *iter == QLatin1String( "%babel" ) )
      copy.append( babel );
    else if ( *iter == QLatin1String( "%type" ) )
      copy.append( type );
    else if ( *iter == QLatin1String( "%in" ) )
      copy.append( QStringLiteral( "\"%1\"" ).arg( in ) );
    else if ( *iter == QLatin1String( "%out" ) )
      copy.append( QStringLiteral( "\"%1\"" ).arg( out ) );
    else
      copy.append( *iter );
  }
  return copy;
}


QStringList QgsGpsDevice::exportCommand( const QString &babel,
    const QString &type,
    const QString &in,
    const QString &out ) const
{
  const QStringList *original = nullptr;
  if ( type == QLatin1String( "-w" ) )
    original = &mWptUlCmd;
  else if ( type == QLatin1String( "-r" ) )
    original = &mRteUlCmd;
  else if ( type == QLatin1String( "-t" ) )
    original = &mTrkUlCmd;
  else throw "Bad error!";
  QStringList copy;
  QStringList::const_iterator iter;
  for ( iter = original->begin(); iter != original->end(); ++iter )
  {
    if ( *iter == QLatin1String( "%babel" ) )
      copy.append( babel );
    else if ( *iter == QLatin1String( "%type" ) )
      copy.append( type );
    else if ( *iter == QLatin1String( "%in" ) )
      copy.append( QStringLiteral( "\"%1\"" ).arg( in ) );
    else if ( *iter == QLatin1String( "%out" ) )
      copy.append( QStringLiteral( "\"%1\"" ).arg( out ) );
    else
      copy.append( *iter );
  }
  return copy;
}



