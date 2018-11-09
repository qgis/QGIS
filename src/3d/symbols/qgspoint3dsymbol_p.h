/***************************************************************************
  qgspoint3dsymbol_p.h
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

#ifndef QGSPOINT3DSYMBOL_P_H
#define QGSPOINT3DSYMBOL_P_H

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
#include <Qt3DRender/QMaterial>
#include <Qt3DRender/QGeometryRenderer>
#include <Qt3DCore/QTransform>

#include "qgspoint3dsymbol.h"

class Qgs3DMapSettings;
class QgsPoint3DSymbol;

class QgsVectorLayer;
class QgsFeatureRequest;


//! Entity that handles rendering of points as 3D objects
class QgsPoint3DSymbolEntity : public Qt3DCore::QEntity
{
  public:
    QgsPoint3DSymbolEntity( const Qgs3DMapSettings &map, QgsVectorLayer *layer, const QgsPoint3DSymbol &symbol, Qt3DCore::QNode *parent = nullptr );
};

class QgsPoint3DSymbolInstancedEntityFactory
{
  public:
    static void addEntityForSelectedPoints( const Qgs3DMapSettings &map, QgsVectorLayer *layer, const QgsPoint3DSymbol &symbol, QgsPoint3DSymbolEntity *parent );
    static void addEntityForNotSelectedPoints( const Qgs3DMapSettings &map, QgsVectorLayer *layer, const QgsPoint3DSymbol &symbol, QgsPoint3DSymbolEntity *parent );

  private:
    static Qt3DRender::QMaterial *material( const QgsPoint3DSymbol &symbol );
};

class QgsPoint3DSymbolInstancedEntityNode : public Qt3DCore::QEntity
{
  public:
    QgsPoint3DSymbolInstancedEntityNode( const Qgs3DMapSettings &map, QgsVectorLayer *layer, const QgsPoint3DSymbol &symbol, const QgsFeatureRequest &req, Qt3DCore::QNode *parent = nullptr );

  private:
    Qt3DRender::QGeometryRenderer *renderer( const QgsPoint3DSymbol &symbol, const QList<QVector3D> &positions ) const;
    Qt3DRender::QGeometry *symbolGeometry( QgsPoint3DSymbol::Shape shape, const QVariantMap &shapeProperties ) const;
};

class QgsPoint3DSymbolModelEntityFactory
{
  public:
    static void addEntitiesForSelectedPoints( const Qgs3DMapSettings &map, QgsVectorLayer *layer, const QgsPoint3DSymbol &symbol, QgsPoint3DSymbolEntity *parent );
    static void addEntitiesForNotSelectedPoints( const Qgs3DMapSettings &map, QgsVectorLayer *layer, const QgsPoint3DSymbol &symbol, QgsPoint3DSymbolEntity *parent );

  private:
    static void addSceneEntities( const Qgs3DMapSettings &map, QgsVectorLayer *layer, const QgsFeatureRequest &req, const QgsPoint3DSymbol &symbol, QgsPoint3DSymbolEntity *parent );
    static void addMeshEntities( const Qgs3DMapSettings &map, QgsVectorLayer *layer, const QgsFeatureRequest &req, const QgsPoint3DSymbol &symbol, QgsPoint3DSymbolEntity *parent, bool are_selected );

    static Qt3DCore::QTransform *transform( QVector3D position, const QgsPoint3DSymbol &symbol );
};

/// @endcond

#endif // QGSPOINT3DSYMBOL_P_H
