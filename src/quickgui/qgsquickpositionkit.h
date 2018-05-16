/***************************************************************************
 qgsquickpositionkit.h
  --------------------------------------
  Date                 : Dec. 2017
  Copyright            : (C) 2017 Peter Petrik
  Email                : zilolv at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSQUICKPOSITIONKIT_H
#define QGSQUICKPOSITIONKIT_H

#include <QObject>
#include <QtPositioning>

#include "qgspoint.h"

#include "qgis_quick.h"
#include "qgsquickmapsettings.h"
#include "qgsquickcoordinatetransformer.h"

/**
 * \ingroup quick
 * Convenient set of tools to read GPS position and accuracy.
 *
 * Also, if one can use use_simulated_location to specify simulated position.
 * Simulated position source generates random points in circles around the selected
 * point and radius. Real GPS position is not used in this mode.
 *
 * \note QML Type: PositionKit
 *
 * \since QGIS 3.2
 */
class QUICK_EXPORT QgsQuickPositionKit : public QObject
{
    Q_OBJECT

    /**
     * GPS position in WGS84 coords.
     */
    Q_PROPERTY( QgsPoint position READ position NOTIFY positionChanged )

    /**
     * GPS position in map coords.
     */
    Q_PROPERTY( QgsPoint projectedPosition READ projectedPosition NOTIFY positionChanged )

    /**
     * GPS position in pixels.
     */
    Q_PROPERTY( QPointF screenPosition READ screenPosition WRITE setScreenPosition NOTIFY screenPositionChanged )

    /**
     * GPS position is available (position property is a valid number).
     */
    Q_PROPERTY( bool hasPosition READ hasPosition NOTIFY hasPositionChanged )

    /**
     * GPS horizontal accuracy in accuracyUnits, -1 if not available.
     */
    Q_PROPERTY( qreal accuracy READ accuracy NOTIFY positionChanged )

    /**
     * Screen horizontal accuracy, 2 if not available or resolution is too small.
     */
    Q_PROPERTY( double screenAccuracy READ screenAccuracy NOTIFY positionChanged )

    /**
     * GPS horizontal accuracy units.
     */
    Q_PROPERTY( QString accuracyUnits READ accuracyUnits NOTIFY positionChanged )

    /**
     * GPS direction, bearing in degrees clockwise from north to direction of travel. -1 if not available
     */
    Q_PROPERTY( qreal direction READ direction NOTIFY positionChanged )

    /**
     * GPS position and accuracy is simulated (not real from GPS sensor). Default false (use real GPS)
     */
    Q_PROPERTY( bool isSimulated READ simulated NOTIFY isSimulatedChanged )

    /**
     * Associated map settings. Should be initialized before the first use from mapcanvas map settings.
     */
    Q_PROPERTY( QgsQuickMapSettings *mapSettings READ mapSettings WRITE setMapSettings NOTIFY mapSettingsChanged )

    /**
     * Vector containing longitude, latitude and radius of similated position. If empty, no simulated source will be used.
     * e.g. [-97.36, 36.93, 2]
     */
    Q_PROPERTY( QVector<double> simulatePositionLongLatRad READ simulatePositionLongLatRad WRITE setSimulatePositionLongLatRad NOTIFY simulatePositionLongLatRadChanged )

  public:
    //! Creates new position kit
    explicit QgsQuickPositionKit( QObject *parent = 0 );

    //! True if there is \copydoc QgsQuickPositionKit::position
    bool hasPosition() const;

    //! \copydoc QgsQuickPositionKit::position
    QgsPoint position() const;

    //! \copydoc QgsQuickPositionKit::projectedPosition
    QgsPoint projectedPosition() const;

    //! \copydoc QgsQuickPositionKit::screenPosition
    QPointF screenPosition() const;

    //! \copydoc QgsQuickPositionKit::screenPosition
    void setScreenPosition( const QPointF &screenPosition );

    //! \copydoc QgsQuickPositionKit::accuracy
    qreal accuracy() const;

    //! \copydoc QgsQuickPositionKit::screenAccuracy
    double screenAccuracy() const;

    //! \copydoc QgsQuickPositionKit::accuracyUnits
    QString accuracyUnits() const;

    //! \copydoc QgsQuickPositionKit::direction
    qreal direction() const;

    //! \copydoc QgsQuickPositionKit::isSimulated
    bool simulated() const;

    //! \copydoc QgsQuickPositionKit::mapSettings
    void setMapSettings( QgsQuickMapSettings *mapSettings );

    //! \copydoc QgsQuickPositionKit::mapSettings
    QgsQuickMapSettings *mapSettings() const;

    //! \copydoc QgsQuickPositionKit::simulatePositionLongLatRad
    QVector<double> simulatePositionLongLatRad() const;

    //! \copydoc QgsQuickPositionKit::simulatePositionLongLatRad
    void setSimulatePositionLongLatRad( const QVector<double> &simulatePositionLongLatRad );

    /**
     * Use simulated GPS source.
     *
     * Simulated GPS source emulates point on circle around defined point in specified radius
     *
     * We do not want to have the origin point as property
     * We basically want to set it once based on project/map cente and keep
     * it that way regardless of mapsettings change (e.g. zoom etc)
     *
     * \param longitude longitude of the centre of the emulated points
     * \param latitude latitude of the centre of the emulated points
     * \param radius distance of emulated points from the centre (in degrees WSG84)
     */
    Q_INVOKABLE void use_simulated_location( double longitude, double latitude, double radius );

    /**
     * Generates label for gps accuracy according set gps source. If there is no accuracy, return empty string.
     */
    Q_INVOKABLE QString sourceAccuracyLabel();

    /**
     * Generates label for gps position according set gps source. If has no position, return empty string.
     * \param precision Defines number of digits after comma.
     */
    Q_INVOKABLE QString sourcePositionLabel( int precision );

    /**
     * Use real GPS source (not simulated)
     */
    Q_INVOKABLE void use_gps_location();

    /**
     * Used for changing position source when simulatePositionLongLatRad is un/set.
     * \param simulatePositionLongLatRad Vector containing longitute, latitute and radius.
     */
    Q_INVOKABLE void onSimulatePositionLongLatRadChanged( QVector<double> simulatePositionLongLatRad );

    /**
     * Updates screen position according projected position.
     */
    Q_INVOKABLE void updateScreenPosition();

  signals:
    //! source position changed
    void positionChanged();

    //! screenPosition changed
    void screenPositionChanged();

    //! hasPosition changed
    void hasPositionChanged();

    //! changed if source position is simulated or not
    void isSimulatedChanged();

    //! \copydoc QgsQuickPositionKit::mapSettings
    void mapSettingsChanged();

    //! \copydoc QgsQuickPositionKit::simulatePositionLongLatRad
    void simulatePositionLongLatRadChanged( QVector<double> simulatePositionLongLatRad );

  public slots:

  private slots:
    void positionUpdated( const QGeoPositionInfo &info );
    void onUpdateTimeout();

  protected:
    //! \copydoc QgsQuickPositionKit::position
    QgsPoint mPosition;
    //! \copydoc QgsQuickPositionKit::projectedPosition
    QgsPoint mProjectedPosition;
    //! \copydoc QgsQuickPositionKit::screenPosition
    QPointF mScreenPosition;

    //! \copydoc QgsQuickPositionKit::accuracy
    qreal mAccuracy;
    //! \copydoc QgsQuickPositionKit::screenAccuracy
    double mScreenAccuracy;

    //! \copydoc QgsQuickPositionKit::accuracyUnits
    QString mAccuracyUnits;
    //! \copydoc QgsQuickPositionKit::direction
    qreal mDirection;
    //! \copydoc QgsQuickPositionKit::position
    bool mHasPosition;

    //! \copydoc QgsQuickPositionKit::isSimulated
    bool mIsSimulated;

    //! \copydoc QgsQuickPositionKit::simulatePositionLongLatRad
    QVector<double> mSimulatePositionLongLatRad;

    std::unique_ptr<QGeoPositionInfoSource> mSource;

  private:
    void replacePositionSource( QGeoPositionInfoSource *source );
    QString calculateStatusLabel();
    double calculateScreenAccuracy();

    QgsQuickMapSettings *mMapSettings = nullptr; // not owned
    QgsCoordinateTransform mCoordinateTransform;

    QGeoPositionInfoSource *gpsSource();
    QGeoPositionInfoSource *simulatedSource( double longitude, double latitude, double radius );

    void updateProjectedPosition();
};

#endif // QGSQUICKPOSITIONKIT_H
