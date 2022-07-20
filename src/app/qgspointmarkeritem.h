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

#include <QFontMetricsF>
#include <QPixmap>
#include <memory>

#include "qgis_app.h"
#include "qgsmapcanvasitem.h"
#include "qgsfeature.h"
#include "qgspainteffect.h"

class QgsMarkerSymbol;
class QgsLineSymbol;

/**
 * \ingroup app
 * \class QgsMapCanvasSymbolItem
 * \brief Base class for map canvas items which are rendered using a QgsSymbol.
 */
class APP_EXPORT QgsMapCanvasSymbolItem: public QgsMapCanvasItem
{
  public:

    QgsMapCanvasSymbolItem( QgsMapCanvas *canvas = nullptr );
    ~QgsMapCanvasSymbolItem() override;

    void paint( QPainter *painter ) override;

    /**
     * Sets the symbol to use for rendering the item.
     * \see symbol()
     */
    void setSymbol( std::unique_ptr< QgsSymbol > symbol );

    /**
     * Returns the symbol used for rendering the item.
     * \see setSymbol()
     */
    QgsSymbol *symbol();

    /**
     * Sets the feature used for rendering the symbol. The feature's attributes
     * may affect the rendered symbol if data defined overrides are in place.
     * \param feature feature for symbol
     * \see feature()
     */
    void setFeature( const QgsFeature &feature );

    /**
     * Returns the feature used for rendering the symbol.
     * \see setFeature()
     */
    QgsFeature feature() const { return mFeature; }

    /**
     * Sets the \a opacity for the item.
     * \param opacity double between 0 and 1 inclusive, where 0 is fully transparent
     * and 1 is fully opaque
     * \see opacity()
     */
    void setOpacity( double opacity );

    /**
     * Returns the opacity for the item.
     * \returns opacity value between 0 and 1 inclusive, where 0 is fully transparent
     * and 1 is fully opaque
     * \see setOpacity()
     */
    double opacity() const;

  protected:

    virtual void renderSymbol( QgsRenderContext &context, const QgsFeature &feature ) = 0;

    QgsRenderContext renderContext( QPainter *painter );
    std::unique_ptr< QgsSymbol > mSymbol;
    QgsFeature mFeature;

  private:

    std::unique_ptr< QgsDrawSourceEffect > mOpacityEffect;

};

/**
 * \ingroup app
 * \class QgsMapCanvasMarkerSymbolItem
 * \brief An item that shows a point marker symbol centered on a map location.
 */
class APP_EXPORT QgsMapCanvasMarkerSymbolItem: public QgsMapCanvasSymbolItem
{
  public:

    QgsMapCanvasMarkerSymbolItem( QgsMapCanvas *canvas = nullptr );

    /**
     * Sets the center point of the marker symbol (in map coordinates)
     * \param p center point
    */
    void setPointLocation( const QgsPointXY &p );

    /**
     * Must be called after setting the symbol or feature and when the symbol's size may
     * have changed.
     */
    void updateSize();

    void renderSymbol( QgsRenderContext &context, const QgsFeature &feature ) override;
    QRectF boundingRect() const override;
    void updatePosition() override;

  private:

    QgsPointXY mMapLocation;
    QPointF mLocation;
    QRectF mCanvasBounds;

    QgsMarkerSymbol *markerSymbol();
};

/**
 * \ingroup app
 * \class QgsMapCanvasLineSymbolItem
 * \brief An item that shows a line symbol over the map.
 */
class APP_EXPORT QgsMapCanvasLineSymbolItem: public QgsMapCanvasSymbolItem
{
  public:

    QgsMapCanvasLineSymbolItem( QgsMapCanvas *canvas = nullptr );

    /**
     * Sets the line to draw (in map coordinates)
     */
    void setLine( const QPolygonF &line );

    /**
     * Sets the line to draw (in map coordinates)
    */
    void setLine( const QLineF &line );

    QRectF boundingRect() const override;

    void renderSymbol( QgsRenderContext &context, const QgsFeature &feature ) override;

  private:

    QPolygonF mLine;

    QgsLineSymbol *lineSymbol();
};

#endif // QGSPOINTMARKERITEM_H
