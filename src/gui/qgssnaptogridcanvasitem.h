/***************************************************************************
    qgssnaptogridcanvasitem.h
    ----------------------
    begin                : August 2018
    copyright            : (C) Matthias Kuhn
    email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSNAPTOGRIDCANVASITEM_H
#define QGSSNAPTOGRIDCANVASITEM_H

#include <QObject>
#include <QPen>

#include "qgscoordinatereferencesystem.h"
#include "qgsmapcanvasitem.h"
#include "qgscoordinatetransform.h"

#ifdef SIP_RUN
% ModuleHeaderCode
// For ConvertToSubClassCode.
#include <qgssnaptogridcanvasitem.h>
% End
#endif

/**
 * \ingroup gui
 *
 * \brief Shows a grid on the map canvas given a spatial resolution.
 *
 * \since QGIS 3.4
 */
#ifndef SIP_RUN
class GUI_EXPORT QgsSnapToGridCanvasItem : public QObject, public QgsMapCanvasItem
{
#else
class GUI_EXPORT QgsSnapToGridCanvasItem : public QgsMapCanvasItem
{
#endif

    Q_OBJECT

#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    if ( dynamic_cast<QgsSnapToGridCanvasItem *>( sipCpp ) )
    {
      sipType = sipType_QgsSnapToGridCanvasItem;
      // We need to tweak the pointer as sip believes it is single inheritance
      // from QgsMapCanvasItem, but the raw address of QgsSnapToGridCanvasItem (sipCpp)
      // is actually a QObject
      *sipCppRet = dynamic_cast<QgsSnapToGridCanvasItem *>( sipCpp );
    }
    else
      sipType = nullptr;
    SIP_END
#endif

  public:

    /**
     * Will automatically be added to the \a mapCanvas.
     */
    QgsSnapToGridCanvasItem( QgsMapCanvas *mapCanvas SIP_TRANSFERTHIS );

    void paint( QPainter *painter ) override;

    /**
     * A point that will be highlighted on the map canvas.
     * The point needs to be in map coordinates. The closest point on the
     * grid will be highlighted.
     */
    QgsPointXY point() const;

    /**
     * A point that will be highlighted on the map canvas.
     * The point needs to be in map coordinates. The closest point on the
     * grid will be highlighted.
     */
    void setPoint( const QgsPointXY &point );

    /**
     * The resolution of the grid in map units.
     * If a crs has been specified it will be in CRS units.
     */
    double precision() const;

    /**
     * The resolution of the grid in map units.
     * If a crs has been specified it will be in CRS units.
     */
    void setPrecision( double precision );

    /**
     * The CRS in which the grid should be calculated.
     * By default will be an invalid QgsCoordinateReferenceSystem and
     * as such equal to the CRS of the map canvas.
     */
    QgsCoordinateReferenceSystem crs() const;

    /**
     * The CRS in which the grid should be calculated.
     * By default will be an invalid QgsCoordinateReferenceSystem and
     * as such equal to the CRS of the map canvas.
     */
    void setCrs( const QgsCoordinateReferenceSystem &crs );

    /**
     * Enable this item. It will be hidden if disabled.
     */
    bool enabled() const;

    /**
     * Enable this item. It will be hidden if disabled.
     */
    void setEnabled( bool enabled );

  private slots:
    void updateMapCanvasCrs();

    void updateZoomFactor();

  private:
    QPen mGridPen = QPen( QColor( 127, 127, 127, 150 ) );
    QPen mCurrentPointPen = QPen( QColor( 200, 200, 200, 150 ) );

    bool mEnabled = true;
    bool mAvailableByZoomFactor = false;

    double mPrecision = 0.0;
    QgsCoordinateTransform mTransform;
    QgsPointXY mPoint;
};

#endif // QGSSNAPTOGRIDCANVASITEM_H
