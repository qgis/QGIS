/***************************************************************************
     qgsbabelgpsdevice.cpp
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
#include "qgsbabelgpsdevice.h"

#include <QRegularExpression>

QgsBabelGpsDeviceFormat::QgsBabelGpsDeviceFormat( const QString &waypointDownloadCommand, const QString &waypointUploadCommand,
    const QString &routeDownloadCommand, const QString &routeUploadCommand,
    const QString &trackDownloadCommand, const QString &trackUploadCommand )
{
  const thread_local QRegularExpression whiteSpaceRx( u"\\s"_s );

  if ( !waypointDownloadCommand.isEmpty() )
  {
    mWaypointDownloadCommand = waypointDownloadCommand.split( whiteSpaceRx, Qt::SkipEmptyParts );
    mCapabilities |= Qgis::BabelFormatCapability::Waypoints;
    mCapabilities |= Qgis::BabelFormatCapability::Import;
  }
  if ( !waypointUploadCommand.isEmpty() )
  {
    mWaypointUploadCommand = waypointUploadCommand.split( whiteSpaceRx, Qt::SkipEmptyParts );
    mCapabilities |= Qgis::BabelFormatCapability::Waypoints;
    mCapabilities |= Qgis::BabelFormatCapability::Export;
  }
  if ( !routeDownloadCommand.isEmpty() )
  {
    mRouteDownloadCommand = routeDownloadCommand.split( whiteSpaceRx, Qt::SkipEmptyParts );
    mCapabilities |= Qgis::BabelFormatCapability::Routes;
    mCapabilities |= Qgis::BabelFormatCapability::Import;
  }
  if ( !routeUploadCommand.isEmpty() )
  {
    mRouteUploadCommand = routeUploadCommand.split( whiteSpaceRx, Qt::SkipEmptyParts );
    mCapabilities |= Qgis::BabelFormatCapability::Routes;
    mCapabilities |= Qgis::BabelFormatCapability::Export;
  }
  if ( !trackDownloadCommand.isEmpty() )
  {
    mTrackDownloadCommand = trackDownloadCommand.split( whiteSpaceRx, Qt::SkipEmptyParts );
    mCapabilities |= Qgis::BabelFormatCapability::Tracks;
    mCapabilities |= Qgis::BabelFormatCapability::Import;
  }
  if ( !trackUploadCommand.isEmpty() )
  {
    mTrackUploadCommand = trackUploadCommand.split( whiteSpaceRx, Qt::SkipEmptyParts );
    mCapabilities |= Qgis::BabelFormatCapability::Tracks;
    mCapabilities |= Qgis::BabelFormatCapability::Export;
  }
}

QStringList QgsBabelGpsDeviceFormat::importCommand( const QString &babel,
    Qgis::GpsFeatureType type,
    const QString &in,
    const QString &out, Qgis::BabelCommandFlags flags ) const
{
  QStringList original;

  switch ( type )
  {
    case Qgis::GpsFeatureType::Waypoint:
      original = mWaypointDownloadCommand;
      break;
    case Qgis::GpsFeatureType::Route:
      original = mRouteDownloadCommand;
      break;
    case Qgis::GpsFeatureType::Track:
      original = mTrackDownloadCommand;
      break;
  }

  QStringList copy;
  copy.reserve( original.size() );
  for ( const QString &iter : std::as_const( original ) )
  {
    if ( iter == "%babel"_L1 )
      copy.append( ( flags & Qgis::BabelCommandFlag::QuoteFilePaths ) ? u"\"%1\""_s.arg( babel ) : babel );
    else if ( iter == "%type"_L1 )
      copy.append( featureTypeToArgument( type ) );
    else if ( iter == "%in"_L1 )
      copy.append( ( flags & Qgis::BabelCommandFlag::QuoteFilePaths ) ? u"\"%1\""_s.arg( in ) : in );
    else if ( iter == "%out"_L1 )
      copy.append( ( flags & Qgis::BabelCommandFlag::QuoteFilePaths ) ? u"\"%1\""_s.arg( out ) : out );
    else
      copy.append( iter );
  }
  return copy;
}

QStringList QgsBabelGpsDeviceFormat::exportCommand( const QString &babel,
    Qgis::GpsFeatureType type,
    const QString &in,
    const QString &out, Qgis::BabelCommandFlags flags ) const
{
  QStringList original;
  switch ( type )
  {
    case Qgis::GpsFeatureType::Waypoint:
      original = mWaypointUploadCommand;
      break;
    case Qgis::GpsFeatureType::Route:
      original = mRouteUploadCommand;
      break;
    case Qgis::GpsFeatureType::Track:
      original = mTrackUploadCommand;
      break;
  }

  QStringList copy;
  copy.reserve( original.size() );
  for ( const QString &iter : std::as_const( original ) )
  {
    if ( iter == "%babel"_L1 )
      copy.append( ( flags & Qgis::BabelCommandFlag::QuoteFilePaths ) ? u"\"%1\""_s.arg( babel ) : babel );
    else if ( iter == "%type"_L1 )
      copy.append( featureTypeToArgument( type ) );
    else if ( iter == "%in"_L1 )
      copy.append( ( flags & Qgis::BabelCommandFlag::QuoteFilePaths ) ? u"\"%1\""_s.arg( in ) : in );
    else if ( iter == "%out"_L1 )
      copy.append( ( flags & Qgis::BabelCommandFlag::QuoteFilePaths ) ? u"\"%1\""_s.arg( out ) : out );
    else
      copy.append( iter );
  }
  return copy;
}



