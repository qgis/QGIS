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

    Q_PROPERTY( bool automaticallyAddTrackPoints READ automaticallyAddTrackPoints WRITE setAutomaticallyAddTrackPoints NOTIFY automaticallyAddTrackPointsChanged )


    /**
     * Constructor for QgsProjectGpsSettings with the specified \a parent object.
     */
    QgsProjectGpsSettings( QObject *parent = nullptr );

    ~QgsProjectGpsSettings() override;

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
     * Returns TRUE if track points should be automatically created whenever
     * new locations are received from the GPS device.
     *
     * \see setAutomaticallyAddTrackPoints()
     * \see automaticallyAddTrackPointsChanged()
     */
    bool automaticallyAddTrackPoints() const;

    /**
     * Returns TRUE if features created from GPS locations should be
     * immediately commited to their target layers (skipping the usual
     * layer edit buffer).
     *
     * \see setAutomaticallyCommitFeatures()
     * \see automaticallyCommitFeaturesChanged()
     */
    bool automaticallyCommitFeatures() const;

  public slots:

    /**
     * Sets whether track points should be automatically created whenever
     * new locations are received from the GPS device.
     *
     * \see automaticallyAddTrackPoints()
     * \see automaticallyAddTrackPointsChanged()
     */
    void setAutomaticallyAddTrackPoints( bool enabled );

    /**
     * Sets whether features created from GPS locations should be
     * immediately commited to their target layers (skipping the usual
     * layer edit buffer).
     *
     * \see automaticallyCommitFeatures()
     * \see automaticallyCommitFeaturesChanged()
     */
    void setAutomaticallyCommitFeatures( bool enabled );

  signals:

    /**
     * Emitted whenever the automaticallyAddTrackPoints() setting
     * is changed.
     *
     * \see automaticallyAddTrackPoints()
     * \see setAutomaticallyAddTrackPoints()
     */
    void automaticallyAddTrackPointsChanged( bool enabled );

    /**
     * Emitted whenever the automaticallyCommitFeatures() setting
     * is changed.
     *
     * \see automaticallyCommitFeatures()
     * \see setAutomaticallyCommitFeatures()
     */
    void automaticallyCommitFeaturesChanged( bool enabled );

  private:

    bool mAutoAddTrackPoints = false;
    bool mAutoCommitFeatures = false;


};

#endif // QGSPROJECTGPSSETTINGS_H
