/***************************************************************************
                         qgslayoutitempolyline.h
    begin                : March 2016
    copyright            : (C) 2016 Paul Blottiere, Oslandia
     email                : paul dot blottiere at oslandia dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAYOUTITEMPOLYLINE_H
#define QGSLAYOUTITEMPOLYLINE_H

#include "qgis_core.h"
#include "qgslayoutitemnodeitem.h"
#include "qgssymbol.h"

/**
 * \ingroup core
 * Layout item for node based polyline shapes.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsLayoutItemPolyline: public QgsLayoutNodesItem
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsLayoutItemPolyline for the specified \a layout.
     */
    QgsLayoutItemPolyline( QgsLayout *layout );

    /**
     * Constructor for QgsLayoutItemPolyline for the specified \a polyline
     * and \a layout.
     */
    QgsLayoutItemPolyline( const QPolygonF &polyline, QgsLayout *layout );

    /**
     * Returns a new polyline item for the specified \a layout.
     *
     * The caller takes responsibility for deleting the returned object.
     */
    static QgsLayoutItemPolyline *create( QgsLayout *layout ) SIP_FACTORY;

    int type() const override;
    QString stringType() const override;
    QString displayName() const override;

    /**
     * Returns the line symbol used to draw the shape.
     * \see setSymbol()
     */
    QgsLineSymbol *symbol() { return mPolylineStyleSymbol.get(); }

    /**
     * Sets the \a symbol used to draw the shape.
     * Ownership of \a symbol is not transferred.
     * \see symbol()
     */
    void setSymbol( QgsLineSymbol *symbol );

  protected:

    bool _addNode( const int indexPoint, QPointF newPoint, const double radius ) override;
    bool _removeNode( const int nodeIndex ) override;
    void _draw( QgsRenderContext &context, const QStyleOptionGraphicsItem *itemStyle = nullptr ) override;
    void _readXmlStyle( const QDomElement &elmt, const QgsReadWriteContext &context ) override;
    void _writeXmlStyle( QDomDocument &doc, QDomElement &elmt, const QgsReadWriteContext &context ) const override;

  private:

    //! QgsSymbol use to draw the shape.
    std::unique_ptr<QgsLineSymbol> mPolylineStyleSymbol;

    //! Create a default symbol.
    void createDefaultPolylineStyleSymbol();

    /**
     * Should be called after the shape's symbol is changed. Redraws the shape and recalculates
     * its selection bounds.
    */
    void refreshSymbol();
};

#endif // QGSLAYOUTITEMPOLYLINE_H

