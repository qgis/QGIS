/***************************************************************************
 qgsquickhighlightsgnode.h
  --------------------------------------
  Date                 : Nov 2017
  Copyright            : (C) 2017 by Matthias Kuhn
  Email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSQUICKHIGHLIGHTSGNODE_H
#define QGSQUICKHIGHLIGHTSGNODE_H

#include <QtQuick/QSGNode>
#include <QtQuick/QSGFlatColorMaterial>

#include "qgspoint.h"
#include "qgsgeometry.h"

#include "qgis_quick.h"

/**
 * \ingroup quick
 *
 * This is used to transform (render) QgsGeometry to node for QtQuick scene graph.
 *
 * \note QML Type: not exported
 *
 * \since QGIS 3.2
 */
class QUICK_EXPORT QgsQuickHighlightSGNode : public QSGNode
{
  public:

    /**
     * Constructor of new QT Quick scene node based on geometry
     * \param geom Geometry to render in the map coordinates
     * \param color color used to render geom
     * \param width width of pen, see QSGGeometry::setLineWidth()
     */
    QgsQuickHighlightSGNode( const QgsGeometry &geom, const QColor &color, float width );
    //! Destructor
    ~QgsQuickHighlightSGNode() = default;

  private:
    void handleGeometryCollection( const QgsGeometry &geom );
    void handleSingleGeometry( const QgsGeometry &geom );

    QSGGeometryNode *createLineGeometry( const QVector<QgsPoint> &points );
    QSGGeometryNode *createPointGeometry( const QgsPoint &point );
    QSGGeometryNode *createPolygonGeometry( const QgsPolygon &polygon );

    QSGFlatColorMaterial mMaterial;
    float mWidth;
};

#endif // QGSQUICKHIGHLIGHTSGNODE
