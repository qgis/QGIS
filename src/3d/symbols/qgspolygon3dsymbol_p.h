/***************************************************************************
  qgspolygon3dsymbol_p.h
  --------------------------------------
  Date                 : July 2017
  Copyright            : (C) 2017 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPOLYGON3DSYMBOL_P_H
#define QGSPOLYGON3DSYMBOL_P_H

/// @cond PRIVATE

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QGIS API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//

#include <Qt3DCore/QEntity>
#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DRender/QGeometryRenderer>

class Qgs3DMapSettings;
class QgsTessellatedPolygonGeometry;
class QgsPolygon3DSymbol;

class QgsPointXY;
class QgsVectorLayer;
class QgsFeatureRequest;


//! Entity that handles rendering of polygons
class QgsPolygon3DSymbolEntity : public Qt3DCore::QEntity
{
  public:
    QgsPolygon3DSymbolEntity( const Qgs3DMapSettings &map, QgsVectorLayer *layer, const QgsPolygon3DSymbol &symbol, Qt3DCore::QNode *parent = nullptr );

  private:
    void addEntityForSelectedPolygons( const Qgs3DMapSettings &map, QgsVectorLayer *layer, const QgsPolygon3DSymbol &symbol );
    void addEntityForNotSelectedPolygons( const Qgs3DMapSettings &map, QgsVectorLayer *layer, const QgsPolygon3DSymbol &symbol );

    Qt3DExtras::QPhongMaterial *material( const QgsPolygon3DSymbol &symbol ) const;
};

class QgsPolygon3DSymbolEntityNode : public Qt3DCore::QEntity
{
  public:
    QgsPolygon3DSymbolEntityNode( const Qgs3DMapSettings &map, QgsVectorLayer *layer, const QgsPolygon3DSymbol &symbol, const QgsFeatureRequest &req, Qt3DCore::QNode *parent = nullptr );

  private:
    Qt3DRender::QGeometryRenderer *renderer( const Qgs3DMapSettings &map, const QgsPolygon3DSymbol &symbol, const QgsVectorLayer *layer, const QgsFeatureRequest &request );

    QgsTessellatedPolygonGeometry *mGeometry = nullptr;
};

/// @endcond

#endif // QGSPOLYGON3DSYMBOL_P_H
