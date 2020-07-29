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
 * An annotation item which renders a fill symbol for a polygon geometry.
 *
 * \since QGIS 3.16
 */
class CORE_EXPORT QgsAnnotationPolygonItem : public QgsAnnotationItem
{
  public:

    /**
     * Constructor for QgsAnnotationPolygonItem, with the specified \a polygon in the given \a crs.
     */
    QgsAnnotationPolygonItem( const QgsPolygon &polygon, const QgsCoordinateReferenceSystem &crs );
    ~QgsAnnotationPolygonItem() override;

    QString type() const override;
    void render( QgsRenderContext &context, QgsFeedback *feedback ) override;
    bool writeXml( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context ) const override;
    static QgsAnnotationPolygonItem *create() SIP_FACTORY;
    bool readXml( const QDomElement &element, const QgsReadWriteContext &context ) override;
    QgsAnnotationPolygonItem *clone() override SIP_FACTORY;

    /**
     * Returns the polygon geometry of the item.
     *
     * The coordinate reference system for the polygon can be retrieved through crs().
     *
     * \see setPolygon()
     */
    QgsPolygon polygon() const { return mPolygon; }

    /**
     * Sets the \a polygon geometry of the item.
     *
     * The coordinate reference system for the polygon can be set through setCrs().
     *
     * \see polygon()
     */
    void setPolygon( const QgsPolygon &polygon ) { mPolygon = polygon; }

    /**
     * Returns the symbol used to render the item.
     *
     * \see setSymbol()
     */
    const QgsFillSymbol *symbol() const;

    /**
     * Sets the \a symbol used to render the marker item.
     *
     * The item takes ownership of the symbol.
     *
     * \see symbol()
     */
    void setSymbol( QgsFillSymbol *symbol SIP_TRANSFER );

  private:

    QgsPolygon mPolygon;
    std::unique_ptr< QgsFillSymbol > mSymbol;

#ifdef SIP_RUN
    QgsAnnotationPolygonItem( const QgsAnnotationPolygonItem &other );
#endif

};
#endif // QGSANNOTATIONPOLYGONITEM_H
