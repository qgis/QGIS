/***************************************************************************
    qgsannotationlineitem.h
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

#ifndef QGSANNOTATIONLINEITEM_H
#define QGSANNOTATIONLINEITEM_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsannotationitem.h"

class QgsCurve;

/**
 * \ingroup core
 * \brief An annotation item which renders a line symbol along a line geometry.
 *
 * \since QGIS 3.16
 */
class CORE_EXPORT QgsAnnotationLineItem : public QgsAnnotationItem
{
  public:

    /**
     * Constructor for QgsAnnotationLineItem, with the specified \a linestring.
     */
    QgsAnnotationLineItem( QgsCurve *curve SIP_TRANSFER );
    ~QgsAnnotationLineItem() override;

    QString type() const override;
    void render( QgsRenderContext &context, QgsFeedback *feedback ) override;
    bool writeXml( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context ) const override;
    QList< QgsAnnotationItemNode > nodes() const override;
    Qgis::AnnotationItemEditOperationResult applyEdit( QgsAbstractAnnotationItemEditOperation *operation ) override;
    QgsAnnotationItemEditOperationTransientResults *transientEditResults( QgsAbstractAnnotationItemEditOperation *operation ) override SIP_FACTORY;

    /**
     * Creates a new linestring annotation item.
     */
    static QgsAnnotationLineItem *create() SIP_FACTORY;

    bool readXml( const QDomElement &element, const QgsReadWriteContext &context ) override;
    QgsRectangle boundingBox() const override;

    QgsAnnotationLineItem *clone() override SIP_FACTORY;

    /**
     * Returns the geometry of the item.
     *
     * The coordinate reference system for the line will be the parent layer's QgsAnnotationLayer::crs().
     *
     * \see setGeometry()
     */
    const QgsCurve *geometry() const { return mCurve.get(); }

    /**
     * Sets the \a geometry of the item. Ownership of \a geometry is transferred.
     *
     * The coordinate reference system for the line will be the parent layer's QgsAnnotationLayer::crs().
     *
     * \see geometry()
     */
    void setGeometry( QgsCurve *geometry SIP_TRANSFER ) { mCurve.reset( geometry ); }

    /**
     * Returns the symbol used to render the item.
     *
     * \see setSymbol()
     */
    const QgsLineSymbol *symbol() const;

    /**
     * Sets the \a symbol used to render the marker item.
     *
     * The item takes ownership of the symbol.
     *
     * \see symbol()
     */
    void setSymbol( QgsLineSymbol *symbol SIP_TRANSFER );

  private:

    std::unique_ptr< QgsCurve > mCurve;
    std::unique_ptr< QgsLineSymbol > mSymbol;

#ifdef SIP_RUN
    QgsAnnotationLineItem( const QgsAnnotationLineItem &other );
#endif

};

#endif // QGSANNOTATIONLINEITEM_H
