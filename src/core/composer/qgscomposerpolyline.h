/***************************************************************************
                         qgscomposerpolyline.h
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

#ifndef QGSCOMPOSERPOLYLINE_H
#define QGSCOMPOSERPOLYLINE_H

#include "qgis_core.h"
#include "qgscomposernodesitem.h"
#include "qgssymbol.h"
#include <QBrush>
#include <QPen>

class QgsLineSymbol;

/**
 * \ingroup core
 * Composer item for polylines.
 * \since QGIS 2.16
 */
class CORE_EXPORT QgsComposerPolyline: public QgsComposerNodesItem
{
    Q_OBJECT

  public:

    /**
     * Constructor
     * \param c parent composition
     */
    QgsComposerPolyline( QgsComposition *c );

    /**
     * Constructor
     * \param polyline nodes of the shape
     * \param c parent composition
     */
    QgsComposerPolyline( const QPolygonF &polyline, QgsComposition *c );

    //! Overridden to return shape name
    virtual QString displayName() const override;

    //! Returns the QgsSymbol used to draw the shape.
    QgsLineSymbol *polylineStyleSymbol() { return mPolylineStyleSymbol.get(); }

    //! Set the QgsSymbol used to draw the shape.
    void setPolylineStyleSymbol( QgsLineSymbol *symbol );

    //! Overridden to return shape type
    virtual int type() const override { return ComposerPolyline; }

  protected:

    //! QgsSymbol use to draw the shape.
    std::unique_ptr<QgsLineSymbol> mPolylineStyleSymbol;

    /**
     * Add the node newPoint at the given position according to some
     * criteres. */
    bool _addNode( const int indexPoint, QPointF newPoint, const double radius ) override;

    bool _removeNode( const int nodeIndex ) override;

    //! Draw nodes for the current shape.
    void _draw( QPainter *painter ) override;

    //! Read symbol in XML.
    void _readXmlStyle( const QDomElement &elmt ) override;

    //! Write the symbol in an XML document.
    void _writeXmlStyle( QDomDocument &doc, QDomElement &elmt ) const override;

    //! Create a default symbol.
    void createDefaultPolylineStyleSymbol();
};

#endif // QGSCOMPOSERPOLYLINE_H

