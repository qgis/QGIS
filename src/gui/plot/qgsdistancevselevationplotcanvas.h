/***************************************************************************
                          qgsdistancevselevationplotcanvas.h
                          ---------------
    begin                : March 2022
    copyright            : (C) 2022 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSDISTANCEVSELEVATIONPLOTCANVAS_H
#define QGSDISTANCEVSELEVATIONPLOTCANVAS_H

#include "qgsconfig.h"
#include "qgis_sip.h"
#include "qgis_gui.h"
#include "qgsplotcanvas.h"
#include "qgscoordinatereferencesystem.h"

class QgsCurve;

/**
 * \ingroup gui
 * \brief A canvas for display of distance vs elevation plots.
 *
 * \since QGIS 3.26
 */
class GUI_EXPORT QgsDistanceVsElevationPlotCanvas : public QgsPlotCanvas
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsDistanceVsElevationPlotCanvas, with the specified \a parent widget.
     */
    QgsDistanceVsElevationPlotCanvas( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    ~QgsDistanceVsElevationPlotCanvas() override;
    QgsCoordinateReferenceSystem crs() const override;
    QgsPoint toMapCoordinates( const QgsPointXY &point ) const override;
    QgsPointXY toCanvasCoordinates( const QgsPoint &point ) const override;
    void resizeEvent( QResizeEvent *event ) override;

    /**
     * Sets the \a crs associated with the canvas' map coordinates.
     *
     * \see crs()
     */
    void setCrs( const QgsCoordinateReferenceSystem &crs );

    /**
     * Sets the profile \a curve.
     *
     * The CRS associated with \a curve is set via setCrs().
     *
     * Ownership is transferred to the plot canvas.
     *
     * \see profileCurve()
     */
    void setProfileCurve( QgsCurve *curve SIP_TRANSFER );

    /**
     * Returns the profile curve.
     *
     * The CRS associated with the curve is retrieved via crs().
     *
     * \see setProfileCurve()
     */
    QgsCurve *profileCurve() const;

  protected:

    QgsCoordinateReferenceSystem mCrs;

    /**
     * Converts a map z value to a canvas y coordinate.
     */
    double zToCanvasY( double z ) const;

    //! Range of elevation values included in the plot contents rect
    double mZMin = 0;
    double mZMax = 100;

    //! Area in canvas coordinates containing the contents of the plot (excluding titles/axis/exterior labels etc)
    QRectF mPlotContentsRect;

  private:

    std::unique_ptr< QgsCurve > mProfileCurve;




};

#endif // QGSDISTANCEVSELEVATIONPLOTCANVAS_H
