/***************************************************************************
    qgsappgpslogging.h
    -------------------
    begin                : October 2022
    copyright            : (C) 2022 Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSAPPGPSLOGGING_H
#define QGSAPPGPSLOGGING_H

#include <QObject>
#include <QTextStream>

#include "qgis.h"
#include "qgis_app.h"


class QFile;


class QgsAppGpsConnection;
class QgsVectorLayerGpsLogger;
class QgsVectorLayer;
class QgsSettingsEntryString;

class APP_EXPORT QgsAppGpsLogging: public QObject
{
    Q_OBJECT

  public:

    static const QgsSettingsEntryString *settingLastLogFolder;
    static const QgsSettingsEntryString *settingLastGpkgLog;

    QgsAppGpsLogging( QgsAppGpsConnection *connection, QObject *parent = nullptr );
    ~QgsAppGpsLogging() override;

  public slots:
    void setNmeaLogFile( const QString &filename );
    void setNmeaLoggingEnabled( bool enabled );
    void setGpkgLogFile( const QString &filename );

  signals:

    void gpkgLoggingFailed();

  private slots:

    void gpsConnected();
    void gpsDisconnected();

    void logNmeaSentence( const QString &nmeaString ); // added to handle 'raw' data

    void startNmeaLogging();
    void stopNmeaLogging();

  private:

    void createGpkgLogger();
    bool createOrUpdateLogDatabase();
    void createGpkgLogDatabase();
    void createSpatialiteLogDatabase();

    QgsAppGpsConnection *mConnection = nullptr;

    QString mNmeaLogFile;
    bool mEnableNmeaLogging = false;

    std::unique_ptr< QFile > mLogFile;
    QTextStream mLogFileTextStream;

    QString mGpkgLogFile;
    std::unique_ptr< QgsVectorLayerGpsLogger > mGpkgLogger;
    std::unique_ptr< QgsVectorLayer > mGpkgPointsLayer;
    std::unique_ptr< QgsVectorLayer > mGpkgTracksLayer;

    static const std::vector< std::tuple< Qgis::GpsInformationComponent, std::tuple< QMetaType::Type, QString >>> sPointFields;
    static const std::vector< std::tuple< Qgis::GpsInformationComponent, std::tuple< QMetaType::Type, QString >>> sTrackFields;

};

#endif // QGSAPPGPSLOGGING_H
