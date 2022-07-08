/***************************************************************************
 qgsfillsymbol.h
 ---------------------
 begin                : November 2009
 copyright            : (C) 2009 by Martin Dobias
 email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSFILLSYMBOL_H
#define QGSFILLSYMBOL_H

#include "qgis_core.h"
#include "qgis.h"
#include "qgssymbol.h"

/**
 * \ingroup core
 * \class QgsFillSymbol
 *
 * \brief A fill symbol type, for rendering Polygon and MultiPolygon geometries.
 */
class CORE_EXPORT QgsFillSymbol : public QgsSymbol
{
  public:

    /**
     * Create a fill symbol with one symbol layer: SimpleFill with specified properties.
     * This is a convenience method for easier creation of fill symbols.
     */
    static QgsFillSymbol *createSimple( const QVariantMap &properties ) SIP_FACTORY;

    /**
     * Constructor for QgsFillSymbol, with the specified list of initial symbol \a layers.
     *
     * Ownership of the \a layers are transferred to the symbol.
     */
    QgsFillSymbol( const QgsSymbolLayerList &layers SIP_TRANSFER = QgsSymbolLayerList() );
    void setAngle( double angle ) const;

    /**
     * Renders the symbol using the given render \a context.
     *
     * The \a points list dictates the exterior ring for the polygon to render, and
     * interior rings are optionally specified via the \a rings argument.
     *
     * The \a f argument is used to pass the feature currently being rendered (when available).
     *
     * If only a single symbol layer from the symbol should be rendered, it should be specified
     * in the \a layer argument. A \a layer of -1 indicates that all symbol layers should be
     * rendered.
     *
     * If \a selected is TRUE then the symbol will be drawn using the "selected feature"
     * style and colors instead of the symbol's normal style.
     */
    void renderPolygon( const QPolygonF &points, const QVector<QPolygonF> *rings, const QgsFeature *f, QgsRenderContext &context, int layer = -1, bool selected = false );

    QgsFillSymbol *clone() const override SIP_FACTORY;

  private:

    void renderPolygonUsingLayer( QgsSymbolLayer *layer, const QPolygonF &points, const QVector<QPolygonF> *rings, QgsSymbolRenderContext &context ) const;
    //! Calculates the bounds of a polygon including rings
    QRectF polygonBounds( const QPolygonF &points, const QVector<QPolygonF> *rings ) const;
    //! Translates the rings in a polygon by a set distance
    QVector<QPolygonF> *translateRings( const QVector<QPolygonF> *rings, double dx, double dy ) const;
};

#endif // QGSFILLSYMBOL_H

