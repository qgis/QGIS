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
     * Returns the destination time field name from the pointsLayer().
     *
     * If specified, timestamps for recorded points will be stored in this field.
     *
     * \see setPointTimeField()
     */
    QString pointTimeField() const;

    /**
     * Sets the destination time \a field name from the pointsLayer().
     *
     * If specified, timestamps for recorded points will be stored in this field.
     *
     * \see pointTimeField()
     */
    void setPointTimeField( const QString &field );

    /**
     * Returns the destination distance from previous point field name from the pointsLayer().
     *
     * If specified, the distance from the previous recorded point will be stored in this field.
     *
     * \see setPointDistanceFromPreviousField()
     */
    QString pointDistanceFromPreviousField() const;

    /**
     * Sets the destination distance from previous point \a field name from the pointsLayer().
     *
     * If specified, the distance from the previous recorded point will be stored in this field.
     *
     * \see pointDistanceFromPreviousField()
     */
    void setPointDistanceFromPreviousField( const QString &field );

    /**
     * Returns the destination time delta from previous point field name from the pointsLayer().
     *
     * If specified, the time difference from the previous recorded point will be stored in this field.
     *
     * \see setPointTimeDeltaFromPreviousField()
     */
    QString pointTimeDeltaFromPreviousField() const;

    /**
     * Sets the destination time delta from previous point \a field name from the pointsLayer().
     *
     * If specified, the time difference from the previous recorded point will be stored in this field.
     *
     * \see pointTimeDeltaFromPreviousField()
     */
    void setPointTimeDeltaFromPreviousField( const QString &field );

    /**
     * Returns the destination start time field name from the tracksLayer().
     *
     * If specified, the start timestamps for recorded tracks will be stored in this field.
     *
     * \see setTrackStartTimeField()
     */
    QString trackStartTimeField() const;

    /**
     * Sets the destination start time \a field name from the tracksLayer().
     *
     * If specified, the start timestamps for recorded tracks will be stored in this field.
     *
     * \see trackStartTimeField()
     */
    void setTrackStartTimeField( const QString &field );

    /**
     * Returns the destination end time field name from the tracksLayer().
     *
     * If specified, the end timestamps for recorded tracks will be stored in this field.
     *
     * \see setTrackEndTimeField()
     */
    QString trackEndTimeField() const;

    /**
     * Sets the destination end time \a field name from the tracksLayer().
     *
     * If specified, the end timestamps for recorded tracks will be stored in this field.
     *
     * \see trackEndTimeField()
     */
    void setTrackEndTimeField( const QString &field );

    /**
     * Returns the destination track length field name from the tracksLayer().
     *
     * If specified, the total track length recorded tracks will be stored in this field.
     *
     * \see setTrackLengthField()
     */
    QString trackLengthField() const;

    /**
     * Sets the destination track length \a field name from the tracksLayer().
     *
     * If specified, the total track length recorded tracks will be stored in this field.
     *
     * \see trackLengthField()
     */
    void setTrackLengthField( const QString &field );

    void setTransformContext( const QgsCoordinateTransformContext &context ) override;

  private:

    QPointer< QgsVectorLayer > mPointsLayer;
    QPointer< QgsVectorLayer > mTracksLayer;

    QString mPointTimeField;
    QString mPointDistanceFromPreviousField;
    QString mPointTimeDeltaFromPreviousField;

    QString mTrackStartTimeField;
    QString mTrackEndTimeField;
    QString mTrackLengthField;

    QgsCoordinateTransform mWgs84toPointLayerTransform;
    QgsCoordinateTransform mWgs84toTrackLayerTransform;

};


#endif // QGSVECTORLAYERGPSLOGGER_H
