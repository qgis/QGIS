/***************************************************************************
    qgsresidualplotitem.h
     --------------------------------------
    Date                 : 10-May-2010
    Copyright            : (c) 2010 by Marco Hugentobler
    Email                : marco at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRESIDUALPLOTITEM_H
#define QGSRESIDUALPLOTITEM_H

#include "qgslayoutitem.h"
#include "qgsgcplist.h"
#include "qgsrectangle.h"

/**
 * A composer item to visualise the distribution of georeference residuals. For the visualisation,
 * the length of the residual arrows are scaled.
*/
class QgsResidualPlotItem: public QgsLayoutItem
{
    Q_OBJECT

  public:
    explicit QgsResidualPlotItem( QgsLayout *layout );
    ~QgsResidualPlotItem() override;

    QgsLayoutItem::Flags itemFlags() const override;

    //! \brief Reimplementation of QCanvasItem::paint
    void paint( QPainter *painter, const QStyleOptionGraphicsItem *itemStyle, QWidget *pWidget ) override;

    void setGCPList( const QgsGCPList &list );
    const QgsGCPList &GCPList() const { return mGCPList; }

    void setExtent( const QgsRectangle &rect ) { mExtent = rect;}
    QgsRectangle extent() const { return mExtent; }

    void setConvertScaleToMapUnits( bool convert ) { mConvertScaleToMapUnits = convert; }
    bool convertScaleToMapUnits() const { return mConvertScaleToMapUnits; }

    void draw( QgsLayoutItemRenderContext &context ) override;
  private:
    //gcp list
    QgsGCPList mGCPList;

    QgsRectangle mExtent;
    //! True if the scale bar units should be converted to map units. This can be done for transformation where the scaling in all directions is the same (helmert)
    bool mConvertScaleToMapUnits;

    //! Calculates maximal possible mm to pixel ratio such that the residual arrow is still inside the frame
    double maxMMToPixelRatioForGCP( const QgsGeorefDataPoint *p, double pixelXMM, double pixelYMM );

    //! Returns distance between two points
    double dist( QPointF p1, QPointF p2 ) const;

    /**
     * Draws an arrow head on to a QPainter.
     * \param p destination painter
     * \param x x-coordinate of arrow center
     * \param y y-coordinate of arrow center
     * \param angle angle in degrees which arrow should point toward, measured
     * clockwise from pointing vertical upward
     * \param arrowHeadWidth size of arrow head
     */
    static void drawArrowHead( QPainter *p, double x, double y, double angle, double arrowHeadWidth );

    /**
     * Calculates the angle of the line from p1 to p2 (counter clockwise,
     * starting from a line from north to south)
     * \param p1 start point of line
     * \param p2 end point of line
     * \returns angle in degrees, clockwise from south
     */
    static double angle( QPointF p1, QPointF p2 );
};

#endif // QGSRESIDUALPLOTITEM_H
