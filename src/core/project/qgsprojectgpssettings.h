/***************************************************************************
    qgsprojectgpssettings.h
    ---------------------------
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
#ifndef QGSPROJECTGPSSETTINGS_H
#define QGSPROJECTGPSSETTINGS_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgis.h"
#include "qgsvectorlayerref.h"

#include <QObject>

class QDomElement;
class QgsReadWriteContext;
class QDomDocument;

/**
 * \brief Contains settings and properties relating to how a QgsProject should interact
 * with a GPS device.
 *
 * \ingroup core
 * \since QGIS 3.30
 */
class CORE_EXPORT QgsProjectGpsSettings : public QObject
{
    Q_OBJECT

  public:

    Q_PROPERTY( bool automaticallyAddTrackVertices READ automaticallyAddTrackVertices WRITE setAutomaticallyAddTrackVertices NOTIFY automaticallyAddTrackVerticesChanged )
    Q_PROPERTY( bool automaticallyCommitFeatures READ automaticallyCommitFeatures WRITE setAutomaticallyCommitFeatures NOTIFY automaticallyCommitFeaturesChanged )
    Q_PROPERTY( QgsVectorLayer *destinationLayer READ destinationLayer WRITE setDestinationLayer NOTIFY destinationLayerChanged )

    /**
     * Constructor for QgsProjectGpsSettings with the specified \a parent object.
     */
    QgsProjectGpsSettings( QObject *parent = nullptr );

    ~QgsProjectGpsSettings() override;

    /**
     * Resolves reference to layers from stored layer ID (if it has not been resolved already)
     */
    void resolveReferences( const QgsProject *project );

    /**
     * Resets the settings to a default state.
     */
    void reset();

    /**
     * Reads the settings's state from a DOM element.
     * \see writeXml()
     */
    bool readXml( const QDomElement &element, const QgsReadWriteContext &context );

    /**
     * Returns a DOM element representing the settings.
     * \see readXml()
     */
    QDomElement writeXml( QDomDocument &doc, const QgsReadWriteContext &context ) const;

    /**
     * Returns TRUE if track vertices should be automatically created whenever
     * new locations are received from the GPS device.
     *
     * \see setAutomaticallyAddTrackVertices()
     * \see automaticallyAddTrackVerticesChanged()
     */
    bool automaticallyAddTrackVertices() const;

    /**
     * Returns TRUE if features created from GPS locations should be
     * immediately committed to their target layers (skipping the usual
     * layer edit buffer).
     *
     * \see setAutomaticallyCommitFeatures()
     * \see automaticallyCommitFeaturesChanged()
     */
    bool automaticallyCommitFeatures() const;

    /**
     * Returns TRUE if the destination layer for storing features digitized from GPS
     * should follow the current active layer automatically.
     *
     * \see setDestinationFollowsActiveLayer()
     * \see destinationFollowsActiveLayerChanged()
     */
    bool destinationFollowsActiveLayer() const;

    /**
     * Returns the destination layer to be used for storing features digitized from GPS.
     *
     * \note If destinationFollowsActiveLayer() is TRUE then this layer will be changed
     * whenever the user changes the active layer in the QGIS interface.
     *
     * \see setDestinationLayer()
     * \see destinationLayerChanged()
     */
    QgsVectorLayer *destinationLayer() const;

    /**
     * Returns the map of destination layer ID to target time stamp field name.
     *
     * \see destinationTimeStampField()
     * \see setDestinationTimeStampField()
     * \see setDestinationTimeStampField()
     */
    QMap< QString, QString > destinationTimeStampFields() const;

    /**
     * Returns the destination time stamp field name for the current destinationLayer(),
     * or an empty string if time stamps should not be automatically saved.
     *
     * \see destinationTimeStampFields()
     * \see setDestinationTimeStampField()
     */
    QString destinationTimeStampField() const;

  public slots:

    /**
     * Sets whether track vertices should be automatically created whenever
     * new locations are received from the GPS device.
     *
     * \see automaticallyAddTrackVertices()
     * \see automaticallyAddTrackVerticesChanged()
     */
    void setAutomaticallyAddTrackVertices( bool enabled );

    /**
     * Sets whether features created from GPS locations should be
     * immediately committed to their target layers (skipping the usual
     * layer edit buffer).
     *
     * \see automaticallyCommitFeatures()
     * \see automaticallyCommitFeaturesChanged()
     */
    void setAutomaticallyCommitFeatures( bool enabled );

    /**
     * Sets whether the destination layer for storing features digitized from GPS
     * should follow the current active layer automatically.
     *
     * \see destinationFollowsActiveLayer()
     * \see destinationFollowsActiveLayerChanged()
     */
    void setDestinationFollowsActiveLayer( bool follow );

    /**
     * Sets the destination \a layer to be used for storing features digitized from GPS.
     *
     * \note If destinationFollowsActiveLayer() is TRUE then this layer will be changed
     * whenever the user changes the active layer in the QGIS interface.
     *
     * \see destinationLayer()
     * \see destinationLayerChanged()
     */
    void setDestinationLayer( QgsVectorLayer *layer );

    /**
     * Sets the destination field name for automatically storing timestamps in the
     * specified destination \a layer.
     *
     * Set \a field argument to an empty string if time stamps should
     * not be automatically saved.
     *
     * \see destinationTimeStampFields()
     * \see destinationTimeStampField()
     */
    void setDestinationTimeStampField( QgsVectorLayer *layer, const QString &field );

  signals:

    /**
     * Emitted whenever the automaticallyAddTrackVertices() setting
     * is changed.
     *
     * \see automaticallyAddTrackVertices()
     * \see setAutomaticallyAddTrackVertices()
     */
    void automaticallyAddTrackVerticesChanged( bool enabled );

    /**
     * Emitted whenever the automaticallyCommitFeatures() setting
     * is changed.
     *
     * \see automaticallyCommitFeatures()
     * \see setAutomaticallyCommitFeatures()
     */
    void automaticallyCommitFeaturesChanged( bool enabled );

    /**
     * Emitted whenever the destinationFollowsActiveLayer() setting
     * is changed.
     *
     * \see destinationFollowsActiveLayer()
     * \see setDestinationFollowsActiveLayer()
     */
    void destinationFollowsActiveLayerChanged( bool follows );

    /**
     * Emitted whenever the destination layer for features digitized from GPS
     * is changed.
     *
     * \see destinationLayer()
     * \see setDestinationLayer()
     */
    void destinationLayerChanged( QgsVectorLayer *layer );

    /**
     * Emitted whenever the destination field for automatic time stamps is
     * changed.
     *
     * The \a field argument will be an empty string if time stamps should
     * not be automatically saved.
     *
     * \see destinationTimeStampFields()
     * \see destinationTimeStampField()
     * \see setDestinationTimeStampField()
     */
    void destinationTimeStampFieldChanged( const QString &field );

  private:

    bool mAutoAddTrackVertices = false;
    bool mAutoCommitFeatures = false;

    bool mDestinationFollowsActiveLayer = true;
    QgsVectorLayerRef mDestinationLayer;

    QMap<QString, QString> mDestinationTimestampFields;

};

#endif // QGSPROJECTGPSSETTINGS_H
