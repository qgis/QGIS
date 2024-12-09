/***************************************************************************
    qgsgpsbearingitem.h
    -------------------
    begin                : December 2019
    copyright            : (C) 2019 Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSGPSBEARINGITEM_H
#define QGSGPSBEARINGITEM_H

#include "qgspointmarkeritem.h"
#include "qgscoordinatereferencesystem.h"
#include "qgspointxy.h"

class QPainter;
class QgsLineSymbol;

/**
 * \ingroup app
 * \brief A canvas item for showing the bearing of the GPS device
 */
class QgsGpsBearingItem : public QObject, public QgsMapCanvasLineSymbolItem
{
    Q_OBJECT

  public:
    explicit QgsGpsBearingItem( QgsMapCanvas *mapCanvas );

    /**
     * Point is in WGS84
     */
    void setGpsPosition( const QgsPointXY &point );
    void setGpsBearing( double bearing );

    void updatePosition() override;

  protected:
    //! coordinates of the point in the center (map units)
    QgsPointXY mCenter;

    //! coordinates of the point in the center (WGS84)
    QgsPointXY mCenterWGS84;

  private:
    void updateLine();

    QgsCoordinateReferenceSystem mWgs84CRS;
    double mBearing = 0;
};

#endif // QGSGPSBEARINGITEM_H
