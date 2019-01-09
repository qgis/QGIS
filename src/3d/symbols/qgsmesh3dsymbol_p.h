/***************************************************************************
  qgsmesh3dsymbol_p.h
  -------------------
  Date                 : January 2019
  Copyright            : (C) 2019 by Peter Petrik
  Email                : zilolv at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMESH3DSYMBOL_P_H
#define QGSMESH3DSYMBOL_P_H

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
class QgsMesh3DSymbol;

class QgsPointXY;
class QgsMeshLayer;

//! Entity that handles rendering of polygons
class QgsMesh3DSymbolEntity : public Qt3DCore::QEntity
{
  public:
    QgsMesh3DSymbolEntity( const Qgs3DMapSettings &map, QgsMeshLayer *layer, const QgsMesh3DSymbol &symbol, Qt3DCore::QNode *parent = nullptr );

  private:
    Qt3DExtras::QPhongMaterial *material( const QgsMesh3DSymbol &symbol ) const;
};

class QgsMesh3DSymbolEntityNode : public Qt3DCore::QEntity
{
  public:
    QgsMesh3DSymbolEntityNode( const Qgs3DMapSettings &map, QgsMeshLayer *layer, const QgsMesh3DSymbol &symbol, Qt3DCore::QNode *parent = nullptr );

  private:
    Qt3DRender::QGeometryRenderer *renderer( const Qgs3DMapSettings &map, const QgsMesh3DSymbol &symbol, const QgsMeshLayer *layer );

    QgsTessellatedPolygonGeometry *mGeometry = nullptr;
};

/// @endcond

#endif // QGSMESH3DSYMBOL_P_H
