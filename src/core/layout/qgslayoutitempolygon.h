/***************************************************************************
                         qgslayoutitempolygon.h
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

#ifndef QGSLAYOUTITEMPOLYGON_H
#define QGSLAYOUTITEMPOLYGON_H

#include "qgis_core.h"
#include "qgslayoutitemnodeitem.h"
#include "qgssymbol.h"

/**
 * \ingroup core
 * Layout item for node based polygon shapes.
 * \since QGIS 3.0
 */

class CORE_EXPORT QgsLayoutItemPolygon: public QgsLayoutNodesItem
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsLayoutItemPolygon for the specified \a layout.
     */
    QgsLayoutItemPolygon( QgsLayout *layout );

    /**
     * Constructor for QgsLayoutItemPolygon for the specified \a polygon
     * and \a layout.
     */
    QgsLayoutItemPolygon( const QPolygonF &polygon, QgsLayout *layout );

    /**
     * Returns a new polygon item for the specified \a layout.
     *
     * The caller takes responsibility for deleting the returned object.
     */
    static QgsLayoutItemPolygon *create( QgsLayout *layout ) SIP_FACTORY;

    int type() const override;
    QIcon icon() const override;
    QString displayName() const override;

    /**
     * Returns the fill symbol used to draw the shape.
     * \see setSymbol()
     */
    QgsFillSymbol *symbol() { return mPolygonStyleSymbol.get(); }

    /**
     * Sets the \a symbol used to draw the shape.
     * Ownership of \a symbol is not transferred.
     * \see symbol()
     */
    void setSymbol( QgsFillSymbol *symbol );

  protected:
    bool _addNode( int indexPoint, QPointF newPoint, double radius ) override;
    bool _removeNode( int nodeIndex ) override;
    void _draw( QgsLayoutItemRenderContext &context, const QStyleOptionGraphicsItem *itemStyle = nullptr ) override;
    void _readXmlStyle( const QDomElement &elmt, const QgsReadWriteContext &context ) override;
    void _writeXmlStyle( QDomDocument &doc, QDomElement &elmt, const QgsReadWriteContext &context ) const override;

  private:

    //! QgsSymbol use to draw the shape.
    std::unique_ptr<QgsFillSymbol> mPolygonStyleSymbol;
    //! Create a default symbol.
    void createDefaultPolygonStyleSymbol();

    /**
     * Should be called after the shape's symbol is changed. Redraws the shape and recalculates
     * its selection bounds.
    */
    void refreshSymbol();
};

#endif // QGSLAYOUTITEMPOLYGON_H
