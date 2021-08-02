/***************************************************************************
                          qgsbabelgpsdevice.h
 Functions:
                             -------------------
    begin                : Oct 05, 2004
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

#ifndef QGSBABELGPSDEVICE_H
#define QGSBABELGPSDEVICE_H

#include <QString>
#include <QStringList>

#include "qgsbabelformat.h"

/**
 * \ingroup core
 * \class QgsBabelGpsDeviceFormat
 * \brief A babel format capable of interacting directly with a GPS device.
 *
 * \since QGIS 3.22
*/
class CORE_EXPORT QgsBabelGpsDeviceFormat : public QgsAbstractBabelFormat
{
  public:

    //! Default constructor
    QgsBabelGpsDeviceFormat() = default;

    /**
     * Constructor for QgsBabelGpsDeviceFormat.
     *
     * \param waypointDownloadCommand command for downloading waypoints from device
     * \param waypointUploadCommand command for uploading waypoints to device
     * \param routeDownloadCommand command for downloading routes from device
     * \param routeUploadCommand command for uploading routes to device
     * \param trackDownloadCommand command for downloading tracks from device
     * \param trackUploadCommand command for uploading tracks to device
     */
    QgsBabelGpsDeviceFormat( const QString &waypointDownloadCommand,
                             const QString &waypointUploadCommand,
                             const QString &routeDownloadCommand,
                             const QString &routeUploadCommand,
                             const QString &trackDownloadCommand,
                             const QString &trackUploadCommand );

    QStringList importCommand( const QString &babel, Qgis::GpsFeatureType type, const QString &in, const QString &out,
                               Qgis::BabelCommandFlags flags = Qgis::BabelCommandFlags() ) const override;
    QStringList exportCommand( const QString &babel, Qgis::GpsFeatureType type, const QString &in, const QString &out,
                               Qgis::BabelCommandFlags flags = Qgis::BabelCommandFlags() ) const override;

  private:

    QStringList mWaypointDownloadCommand;
    QStringList mWaypointUploadCommand;
    QStringList mRouteDownloadCommand;
    QStringList mRouteUploadCommand;
    QStringList mTrackDownloadCommand;
    QStringList mTrackUploadCommand;
};

#endif // QGSBABELGPSDEVICE_H
