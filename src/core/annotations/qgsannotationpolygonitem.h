/***************************************************************************
    qgsannotationpolygonitem.h
    ----------------
    begin                : July 2020
    copyright            : (C) 2020 by Nyall Dawson
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

#ifndef QGSANNOTATIONPOLYGONITEM_H
#define QGSANNOTATIONPOLYGONITEM_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsannotationitem.h"


/**
 * \ingroup core
 * \brief An annotation item which renders a fill symbol for a polygon geometry.
 *
 * \since QGIS 3.16
 */
class CORE_EXPORT QgsAnnotationPolygonItem : public QgsAnnotationItem
{
  public:

    /**
     * Constructor for QgsAnnotationPolygonItem, with the specified \a polygon geometry.
     */
    QgsAnnotationPolygonItem( QgsCurvePolygon *polygon SIP_TRANSFER );
    ~QgsAnnotationPolygonItem() override;

    QString type() const override;
    void render( QgsRenderContext &context, QgsFeedback *feedback ) override;
    bool writeXml( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context ) const override;
    QList< QgsAnnotationItemNode > nodes() const override;
    Qgis::AnnotationItemEditOperationResult applyEdit( QgsAbstractAnnotationItemEditOperation *operation ) override;
    QgsAnnotationItemEditOperationTransientResults *transientEditResults( QgsAbstractAnnotationItemEditOperation *operation ) override SIP_FACTORY;

    /**
     * Creates a new polygon annotation item.
     */
    static QgsAnnotationPolygonItem *create() SIP_FACTORY;

    bool readXml( const QDomElement &element, const QgsReadWriteContext &context ) override;
    QgsAnnotationPolygonItem *clone() override SIP_FACTORY;
    QgsRectangle boundingBox() const override;

    /**
     * Returns the geometry of the item.
     *
     * The coordinate reference system for the polygon will be the parent layer's QgsAnnotationLayer::crs().
     *
     * \see setGeometry()
     */
    const QgsCurvePolygon *geometry() const { return mPolygon.get(); }

    /**
     * Sets the \a geometry of the item.
     *
     * The coordinate reference system for the polygon will be the parent layer's QgsAnnotationLayer::crs().
     *
     * \see geometry()
     */
    void setGeometry( QgsCurvePolygon *geometry SIP_TRANSFER ) { mPolygon.reset( geometry ); }

    /**
     * Returns the symbol used to render the item.
     *
     * \see setSymbol()
     */
    const QgsFillSymbol *symbol() const;

    /**
     * Sets the \a symbol used to render the polygon item.
     *
     * The item takes ownership of the symbol.
     *
     * \see symbol()
     */
    void setSymbol( QgsFillSymbol *symbol SIP_TRANSFER );

  private:

    std::unique_ptr< QgsCurvePolygon > mPolygon;
    std::unique_ptr< QgsFillSymbol > mSymbol;

#ifdef SIP_RUN
    QgsAnnotationPolygonItem( const QgsAnnotationPolygonItem &other );
#endif

};
#endif // QGSANNOTATIONPOLYGONITEM_H
