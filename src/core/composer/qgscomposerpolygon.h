/***************************************************************************
                         qgscomposerpolygon.h
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

#ifndef QGSCOMPOSERPOLYGON_H
#define QGSCOMPOSERPOLYGON_H

#include "qgscomposernodesitem.h"
#include <QBrush>
#include <QPen>

class QgsFillSymbolV2;

/** \ingroup core
 * Composer item for polygons.
 * @note added in QGIS 2.16
 */

class CORE_EXPORT QgsComposerPolygon: public QgsComposerNodesItem
{
    Q_OBJECT

  public:

    /** Constructor
     * @param c parent composition
     */
    QgsComposerPolygon( QgsComposition* c );

    /** Constructor
     * @param polygon nodes of the shape
     * @param c parent composition
     */
    QgsComposerPolygon( QPolygonF polygon, QgsComposition* c );

    /** Destructor */
    ~QgsComposerPolygon();

    /** Overridden to return shape name */
    virtual QString displayName() const override;

    /** Returns the QgsSymbolV2 used to draw the shape. */
    QgsFillSymbolV2* polygonStyleSymbol() { return mPolygonStyleSymbol.data(); }

    /** Set the QgsSymbolV2 used to draw the shape. */
    void setPolygonStyleSymbol( QgsFillSymbolV2* symbol );

    /** Return correct graphics item type. */
    virtual int type() const override { return ComposerPolygon; }

  protected:

    /** QgsSymbolV2 use to draw the shape. */
    QScopedPointer<QgsFillSymbolV2> mPolygonStyleSymbol;

    /** Add the node newPoint at the given position according to some
     * criteres. */
    bool _addNode( const int indexPoint, const QPointF &newPoint, const double radius ) override;

    bool _removeNode( const int nodeIndex ) override;

    /** Draw nodes for the current shape. */
    void _draw( QPainter *painter ) override;

    /** Read symbol in XML. */
    void _readXMLStyle( const QDomElement &elmt ) override;

    /** Write the symbol in an XML document. */
    void _writeXMLStyle( QDomDocument &doc, QDomElement &elmt ) const override;

    /** Create a default symbol. */
    void createDefaultPolygonStyleSymbol();
};

#endif // QGSCOMPOSERPOLYGON_H
