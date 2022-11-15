/***************************************************************************
  qgsvectorlayergpslogger.h
   -------------------
  begin                : November 2022
  copyright            : (C) 2022 by Nyall Dawson
  email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVECTORLAYERGPSLOGGER_H
#define QGSVECTORLAYERGPSLOGGER_H

#include "qgis_core.h"
#include "qgis.h"
#include "qgis_sip.h"
#include "qgsgpslogger.h"

#include <QDateTime>

class QgsVectorLayer;

/**
 * \ingroup core
 * \class QgsVectorLayerGpsLogger
 * \brief Handles logging of incoming GPS data to a vector layer.
 *
 * \since QGIS 3.30
*/
class CORE_EXPORT QgsVectorLayerGpsLogger : public QgsGpsLogger
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsVectorLayerGpsLogger with the specified \a parent object.
     *
     * The logger will automatically record GPS information from the specified \a connection.
     */
    QgsVectorLayerGpsLogger( QgsGpsConnection *connection, QObject *parent SIP_TRANSFERTHIS = nullptr );

    ~QgsVectorLayerGpsLogger() override;

    /**
     * Returns TRUE if the logger will use the vector layer edit buffer for the destination layers.
     *
     * If FALSE then the features will be written directly to the destination layer's data providers.
     *
     * The default behavior is to use the edit buffer.
     *
     * \see setWriteToEditBuffer()
     */
    bool writeToEditBuffer() const { return mUseEditBuffer; }

    /**
     * Sets whether the logger will use the vector layer edit buffer for the destination layers.
     *
     * If \a buffer is FALSE then the features will be written directly to the destination layer's data providers.
     *
     * The default behavior is to use the edit buffer.
     *
     * \see writeToEditBuffer()
     */
    void setWriteToEditBuffer( bool buffer ) { mUseEditBuffer = buffer; }

    /**
     * Sets the \a layer in which recorded GPS points should be stored.
     *
     * \see setTracksLayer()
     * \see pointsLayer()
     */
    void setPointsLayer( QgsVectorLayer *layer );

    /**
     * Sets the \a layer in which recorded GPS tracks should be stored.
     *
     * \see setPointsLayer()
     * \see tracksLayer()
     */
    void setTracksLayer( QgsVectorLayer *layer );

    /**
     * Returns the layer in which recorded GPS points will be stored.
     *
     * May be NULLPTR if points are not being stored.
     *
     * \see setPointsLayer()
     * \see tracksLayer()
     */
    QgsVectorLayer *pointsLayer();

    /**
     * Returns the layer in which recorded GPS tracks will be stored.
     *
     * May be NULLPTR if tracks are not being stored.
     *
     * \see setTracksLayer()
     * \see pointsLayer()
     */
    QgsVectorLayer *tracksLayer();

    /**
     * Sets a destination \a field name for a specific GPS information \a component.
     *
     * Depending on the \a component, the field will either refer to the pointsLayer() or tracksLayer().
     *
     * Fields stored in the pointsLayer() are:
     *
     * - Qgis::GpsInformationComponent::Location:
     * - Qgis::GpsInformationComponent::Altitude:
     * - Qgis::GpsInformationComponent::GroundSpeed:
     * - Qgis::GpsInformationComponent::Bearing:
     * - Qgis::GpsInformationComponent::Pdop:
     * - Qgis::GpsInformationComponent::Hdop:
     * - Qgis::GpsInformationComponent::Vdop:
     * - Qgis::GpsInformationComponent::HorizontalAccuracy:
     * - Qgis::GpsInformationComponent::VerticalAccuracy:
     * - Qgis::GpsInformationComponent::HvAccuracy:
     * - Qgis::GpsInformationComponent::SatellitesUsed:
     * - Qgis::GpsInformationComponent::Timestamp:
     * - Qgis::GpsInformationComponent::TrackDistanceSinceLastPoint:
     * - Qgis::GpsInformationComponent::TrackTimeSinceLastPoint:
     *
     * Fields stored in the tracksLayer() are:
     *
     * - Qgis::GpsInformationComponent::TrackStartTime:
     * - Qgis::GpsInformationComponent::TrackEndTime:
     * - Qgis::GpsInformationComponent::TotalTrackLength:
     * - Qgis::GpsInformationComponent::TrackDistanceFromStart:
     *
     * \see destinationField()
     */
    void setDestinationField( Qgis::GpsInformationComponent component, const QString &field );

    /**
     * Returns the destination field name for a specific GPS information \a component.
     *
     * \see setDestinationField()
     */
    QString destinationField( Qgis::GpsInformationComponent component ) const;

    void setTransformContext( const QgsCoordinateTransformContext &context ) override;

  public slots:

    /**
     * Ends the current track, storing it in the tracksLayer() if appropriate.
     */
    void endCurrentTrack();

  private slots:

    void gpsStateChanged( const QgsGpsInformation &information );

  private:

    bool mUseEditBuffer = true;

    QPointer< QgsVectorLayer > mPointsLayer;
    QPointer< QgsVectorLayer > mTracksLayer;

    QgsCoordinateTransform mWgs84toPointLayerTransform;
    QgsCoordinateTransform mWgs84toTrackLayerTransform;

    QMap< Qgis::GpsInformationComponent, QString > mDestinationFields;

    QVariant timestamp( QgsVectorLayer *vlayer, int idx, const QDateTime &time );

};


#endif // QGSVECTORLAYERGPSLOGGER_H
