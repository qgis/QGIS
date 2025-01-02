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

#ifndef QGSMESH3DENTITY_H
#define QGSMESH3DENTITY_H

#include <Qt3DCore/QEntity>

#include "mesh/qgsmesh3dgeometry_p.h"
#include "qgs3drendercontext.h"
#include "qgsmesh3dsymbol.h"
#include "qgsterraintileentity_p.h"

///@cond PRIVATE

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QGIS API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//

#define SIP_NO_FILE

class Qgs3DMapSettings;
class QgsMesh3DSymbol;

class QgsMeshLayer;
class QgsMesh3DMaterial;

//! Abstract class that handles rendering of mesh
class QgsMesh3DEntity
{
  public:
    //! Builds the geometry and the material
    void build();

  protected:
    //! Constructor
    QgsMesh3DEntity( const Qgs3DRenderContext &context, const QgsTriangularMesh &triangularMesh, const QgsMesh3DSymbol *symbol );

    virtual ~QgsMesh3DEntity() = default;

    Qgs3DRenderContext mRenderContext;
    QgsTriangularMesh mTriangularMesh;
    std::unique_ptr<QgsMesh3DSymbol> mSymbol;

  private:
    virtual void buildGeometry() = 0;
    virtual void applyMaterial() = 0;
};

//! Entity that handles rendering of dataset
class QgsMeshDataset3DEntity : public Qt3DCore::QEntity, public QgsMesh3DEntity
{
    Q_OBJECT

  public:
    //! Constructor
    QgsMeshDataset3DEntity( const Qgs3DRenderContext &context, const QgsTriangularMesh &triangularMesh, QgsMeshLayer *meshLayer, const QgsMesh3DSymbol *symbol );

  private:
    void buildGeometry() override;
    void applyMaterial() override;

    QgsMeshLayer *layer() const;
    QgsMapLayerRef mLayerRef;
};

//! Entity that handles rendering of terrain mesh
class QgsMesh3DTerrainTileEntity : public QgsTerrainTileEntity, public QgsMesh3DEntity
{
    Q_OBJECT

  public:
    QgsMesh3DTerrainTileEntity( const Qgs3DRenderContext &context, const QgsTriangularMesh &triangularMesh, const QgsMesh3DSymbol *symbol, QgsChunkNodeId nodeId, Qt3DCore::QNode *parent = nullptr );

  private:
    void buildGeometry() override;
    void applyMaterial() override;
};

///@endcond

#endif // QGSMESH3DENTITY_H
