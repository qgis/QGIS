/***************************************************************************
    qgsannotationmarkeritem.h
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

#ifndef QGSANNOTATIONMARKERITEM_H
#define QGSANNOTATIONMARKERITEM_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsannotationitem.h"


/**
 * \ingroup core
 * An annotation item which renders a marker symbol at a point location.
 *
 * \since QGIS 3.16
 */
class CORE_EXPORT QgsAnnotationMarkerItem : public QgsAnnotationItem
{
  public:

    /**
     * Constructor for QgsAnnotationMarkerItem, at the specified \a point in the given \a crs.
     */
    QgsAnnotationMarkerItem( QgsPointXY point, const QgsCoordinateReferenceSystem &crs );
    ~QgsAnnotationMarkerItem() override;

    QString type() const override;
    void render( QgsRenderContext &context, QgsFeedback *feedback ) override;
    bool writeXml( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context ) const override;
    static QgsAnnotationMarkerItem *create() SIP_FACTORY;
    bool readXml( const QDomElement &element, const QgsReadWriteContext &context ) override;
    QgsAnnotationMarkerItem *clone() override SIP_FACTORY;

    /**
     * Returns the symbol used to render the marker item.
     *
     * \see setSymbol()
     */
    const QgsMarkerSymbol *symbol() const;

    /**
     * Sets the \a symbol used to render the marker item.
     *
     * The item takes ownership of the symbol.
     *
     * \see symbol()
     */
    void setSymbol( QgsMarkerSymbol *symbol SIP_TRANSFER );

  private:

    QgsPointXY mPoint;
    std::unique_ptr< QgsMarkerSymbol > mSymbol;

#ifdef SIP_RUN
    QgsAnnotationMarkerItem( const QgsAnnotationMarkerItem &other );
#endif

};

#endif // QGSANNOTATIONMARKERITEM_H
