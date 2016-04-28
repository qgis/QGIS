/***************************************************************************
    qgspointmarkeritem.h
    --------------------
    begin                : April 2016
    copyright            : (C) 2016 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPOINTMARKERITEM_H
#define QGSPOINTMARKERITEM_H

#include "qgsmapcanvasitem.h"
#include "qgsfeature.h"
#include "effects/qgspainteffect.h"
#include <QFontMetricsF>
#include <QPixmap>

class QgsMarkerSymbolV2;

/** \ingroup app
 * \class QgsPointMarkerItem
 * \brief An item that shows a point marker symbol centered on a map location.
 */

class APP_EXPORT QgsPointMarkerItem: public QgsMapCanvasItem
{
  public:

    QgsPointMarkerItem( QgsMapCanvas* canvas = nullptr );

    void paint( QPainter * painter ) override;

    /** Sets the center point of the marker symbol (in map coordinates)
     * @param p center point
    */
    void setPointLocation( const QgsPoint& p );

    /** Sets the marker symbol to use for rendering the point. Note - you may need to call
     * updateSize() after setting the symbol.
     * @param symbol marker symbol. Ownership is transferred to item.
     * @see symbol()
     * @see updateSize()
     */
    void setSymbol( QgsMarkerSymbolV2* symbol );

    /** Returns the marker symbol used for rendering the point.
     * @see setSymbol()
     */
    QgsMarkerSymbolV2* symbol();

    /** Sets the feature used for rendering the marker symbol. The feature's attributes
     * may affect the rendered symbol if data defined overrides are in place.
     * @param feature feature for symbol
     * @see feature()
     * @see updateSize()
     */
    void setFeature( const QgsFeature& feature );

    /** Returns the feature used for rendering the marker symbol.
     * @see setFeature()
     */
    QgsFeature feature() const { return mFeature; }

    /** Must be called after setting the symbol or feature and when the symbol's size may
     * have changed.
     */
    void updateSize();

    /** Sets the transparency for the marker.
     * @param transparency double between 0 and 1 inclusive, where 0 is fully opaque
     * and 1 is fully transparent
     * @see transparency()
     */
    void setTransparency( double transparency );

    /** Returns the transparency for the marker.
     * @returns transparency value between 0 and 1 inclusive, where 0 is fully opaque
     * and 1 is fully transparent
     * @see setTransparency()
     */
    double transparency() const;

  private:

    QgsFeature mFeature;
    QScopedPointer< QgsMarkerSymbolV2 > mMarkerSymbol;
    QPointF mLocation;
    QScopedPointer< QgsDrawSourceEffect > mOpacityEffect;

    QgsRenderContext renderContext( QPainter* painter );
};

#endif // QGSPOINTMARKERITEM_H
