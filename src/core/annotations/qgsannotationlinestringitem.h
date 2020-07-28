/***************************************************************************
    qgsannotationlinestringitem.h
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

#ifndef QGSANNOTATIONLINESTRINGITEM_H
#define QGSANNOTATIONLINESTRINGITEM_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsannotationitem.h"


/**
 * \ingroup core
 * An annotation item which renders a line symbol along a linestring geometry.
 *
 * \since QGIS 3.16
 */
class CORE_EXPORT QgsAnnotationLineStringItem : public QgsAnnotationItem
{
  public:

    /**
     * Constructor for QgsAnnotationLineStringItem, with the specified \a linestring in the given \a crs.
     */
    QgsAnnotationLineStringItem( const QgsLineString &linestring, const QgsCoordinateReferenceSystem &crs );
    ~QgsAnnotationLineStringItem() override;

    QString type() const override;
    void render( QgsRenderContext &context, QgsFeedback *feedback ) override;
    bool writeXml( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context ) const override;
    static QgsAnnotationLineStringItem *create() SIP_FACTORY;
    bool readXml( const QDomElement &element, const QgsReadWriteContext &context ) override;

    QgsAnnotationLineStringItem *clone() override SIP_FACTORY;

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

    QgsLineString mLineString;
    std::unique_ptr< QgsLineSymbol > mSymbol;

#ifdef SIP_RUN
    QgsAnnotationLineStringItem( const QgsAnnotationLineStringItem &other );
#endif

};

#endif // QGSANNOTATIONLINESTRINGITEM_H
