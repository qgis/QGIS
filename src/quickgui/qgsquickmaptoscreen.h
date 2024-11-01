/***************************************************************************
  qgsquickmaptoscreen.h
  ----------------------------------------------------
  Date                 : 22.08.2018
  Copyright            : (C) 2018 by Denis Rouzaud
  Email                : denis (at) opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSQUICKMAPTOSCREEN_H
#define QGSQUICKMAPTOSCREEN_H

#include "qgsquickmapsettings.h"

#include <QObject>
#include <QPointF>
#include <qgspoint.h>

/**
 * \ingroup quick
 *
 * \brief The QgsQuickMapToScreen class transform map points to screen coordinates as
 * well as distances from map to screen units. Screen points and/or distances will be
 * automatically updated on map extent changes.
 *
 * \since QGIS 3.32
 */
class QUICK_EXPORT QgsQuickMapToScreen : public QObject
{
    Q_OBJECT

    //! Map settings used to define the map canvas CRS and detect any extent change
    Q_PROPERTY( QgsQuickMapSettings *mapSettings READ mapSettings WRITE setMapSettings NOTIFY mapSettingsChanged )

    //! Point in map coordinates
    Q_PROPERTY( QgsPoint mapPoint READ mapPoint WRITE setMapPoint NOTIFY mapPointChanged )
    //! Point in screen coordinates (read-only)
    Q_PROPERTY( QPointF screenPoint READ screenPoint NOTIFY screenPointChanged )

    //! Distance in map unit
    Q_PROPERTY( double mapDistance READ mapDistance WRITE setMapDistance NOTIFY mapDistanceChanged )
    //! Distance in screen coordinates (read-only)
    Q_PROPERTY( double screenDistance READ screenDistance NOTIFY screenDistanceChanged )

  public:
    //! Creates a map to screen object
    explicit QgsQuickMapToScreen( QObject *parent = nullptr );

    //! \copydoc mapSettings
    void setMapSettings( QgsQuickMapSettings *mapSettings );
    //! \copydoc mapSettings
    QgsQuickMapSettings *mapSettings() const;

    //! \copydoc mapPoint
    void setMapPoint( const QgsPoint &point );
    //! \copydoc mapPoint
    QgsPoint mapPoint() const;

    //! \copydoc mapDistance
    void setMapDistance( const double distance );
    //! \copydoc mapDistance
    double mapDistance() const;

    //! \copydoc screenPoint
    QPointF screenPoint() const;

    //! \copydoc screenDistance
    double screenDistance() const;

  signals:

    //! \copydoc mapSettings
    void mapSettingsChanged();
    //! \copydoc mapPoint
    void mapPointChanged();
    //! \copydoc mapDistance
    void mapDistanceChanged();
    //! \copydoc screenPoint
    void screenPointChanged();
    //! \copydoc screenDistance
    void screenDistanceChanged();

  private slots:
    void transformPoint();
    void transformDistance();

  private:
    QgsQuickMapSettings *mMapSettings = nullptr;
    QgsPoint mMapPoint = QgsPoint();
    double mMapDistance = 0.0;
    QPointF mScreenPoint = QPointF();
    double mScreenDistance = 0.0;
};

#endif // QGSQUICKMAPTOSCREEN_H
