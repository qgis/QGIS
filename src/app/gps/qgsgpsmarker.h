/***************************************************************************
    qgsgpsmarker.h  - canvas item which shows a gps marker
    ---------------------
    begin                : 18 December 2009
    copyright            : (C) 2009 Tim Sutton
    email                : tim at linfiniti com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSGPSMARKER_H
#define QGSGPSMARKER_H

#include "qgsmapcanvasitem.h"
#include "qgscoordinatereferencesystem.h"
#include "qgspointxy.h"
#include "qgspointmarkeritem.h"

#include <QObject>


class QPainter;

class QgsSettingsEntryBool;
class QgsSettingsEntryString;


/**
 * \ingroup app
 * \brief A class for marking the position of a gps pointer.
 */
class QgsGpsMarker : public QObject, public QgsMapCanvasMarkerSymbolItem
{
    Q_OBJECT

  public:
    static const QgsSettingsEntryString *settingLocationMarkerSymbol;
    static const QgsSettingsEntryBool *settingShowLocationMarker;
    static const QgsSettingsEntryBool *settingRotateLocationMarker;

    explicit QgsGpsMarker( QgsMapCanvas *mapCanvas );
    ~QgsGpsMarker() override;

    /**
     * Sets the current GPS \a position (in WGS84 coordinate reference system).
     */
    void setGpsPosition( const QgsPointXY &position );

    /**
     * Sets the marker rotation for the GPS bearing.
     */
    void setMarkerRotation( double rotation );

  protected:
    //! Coordinates of the point in the center, in map CRS
    QgsPointXY mCenter;

  private slots:

    void updateMarkerSymbol();

  private:
    QgsCoordinateReferenceSystem mWgs84CRS;

    std::unique_ptr<QgsMarkerSymbol> mMarkerSymbol;
};

#endif
