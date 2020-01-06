/***************************************************************************
                         qgsmesh3dentity.h
                         -------------------------
    begin                : january 2020
    copyright            : (C) 2020 by Vincent Cloarec
    email                : vcloarec at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMESHENTITY_H
#define QGSMESHENTITY_H

#include <Qt3DCore/QEntity>
#include <Qt3DRender/QGeometryRenderer>
#include <Qt3DRender/QTexture>

#include "mesh/qgsmesh3dgeometry_p.h"
#include "qgs3dmapsettings.h"
#include "qgsmesh3dsymbol.h"

class Qgs3DMapSettings;
class QgsTessellatedPolygonGeometry;
class QgsMesh3DSymbol;

class QgsMeshLayer;
class QgsMesh3dMaterial;

//! Entity that handles rendering of mesh
class QgsMesh3dEntity: public Qt3DCore::QEntity
{
  public:
    QgsMesh3dEntity( const Qgs3DMapSettings &map, QgsMeshLayer *layer, const QgsMesh3DSymbol &symbol );
    void build();

  private:
    virtual void buildGeometry();
    virtual void applyMaterial();

    QgsMesh3DSymbol mSymbol;
    Qgs3DMapSettings mMapSettings;
    QgsMeshLayer *mLayer;
    QgsMesh3dMaterial *mMaterial = nullptr;

};

#endif // QGSMESHENTITY_H
