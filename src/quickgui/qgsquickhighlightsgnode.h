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
#include "qgswkbtypes.h"

#include "qgis_quick.h"

/**
 * \ingroup quick
 *
 * This is used to transform (render) QgsGeometry to node for QtQuick scene graph.
 *
 * Note: support for multi-part geometries and polygons is not implemented
 *
 * \note QML Type: not exported
 *
 * \since QGIS 3.2
 */
class QUICK_EXPORT QgsQuickHighlightSGNode : public QSGNode
{
  public:
    //! Constructor of new QT Quick scene node based on geometry
    QgsQuickHighlightSGNode( const QVector<QgsPoint> &points, QgsWkbTypes::GeometryType type, const QColor &color, qreal width );
    //! Destructor
    ~QgsQuickHighlightSGNode() = default;

  protected:
    //! Constructs line geometry from points
    QSGGeometryNode *createLineGeometry( const QVector<QgsPoint> &points, qreal width );
    //! Constructs point geometry from qgs point
    QSGGeometryNode *createPointGeometry( const QgsPoint &point, qreal width );
    //! Constructs polygon geometry from points (not implemented)
    QSGGeometryNode *createPolygonGeometry( const QVector<QgsPoint> &points );

    QSGFlatColorMaterial mMaterial;
};

#endif // QGSQUICKHIGHLIGHTSGNODE
