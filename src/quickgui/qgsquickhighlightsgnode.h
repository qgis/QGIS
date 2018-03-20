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
    //! Create new QT Quick scene node based on geometry
    QgsQuickHighlightSGNode( const QVector<QgsPoint> &points, QgsWkbTypes::GeometryType type, const QColor &color, qreal width );
    virtual ~QgsQuickHighlightSGNode();

  protected:
    QSGGeometryNode *createLineGeometry( const QVector<QgsPoint> &points, qreal width );
    QSGGeometryNode *createPointGeometry( const QgsPoint &point, qreal width );
    virtual QSGGeometryNode *createPolygonGeometry( const QVector<QgsPoint> &points );

    QSGFlatColorMaterial mMaterial;
};

#endif // QGSQUICKHIGHLIGHTSGNODE
