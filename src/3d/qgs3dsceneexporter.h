/***************************************************************************
  qgs3dsceneexporter.h
  --------------------------------------
  Date                 : June 2020
  Copyright            : (C) 2020 by Belgacem Nedjima
  Email                : gb underscore nedjima at esi dot dz
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGS3DSCENEEXPORTER_H
#define QGS3DSCENEEXPORTER_H

#include <Qt3DCore/QEntity>
#include <Qt3DExtras/QPlaneGeometry>
#include <Qt3DRender/QSceneLoader>
#include <Qt3DRender/QMesh>
#include <QMap>
#include <QFile>
#include <QVector3D>

#include "qgs3dexportobject.h"

class QgsTessellatedPolygonGeometry;
class QgsTerrainTileEntity;
class QgsTerrainEntity;
class Qgs3DMapSettings;
class QgsFlatTerrainGenerator;
class QgsDemTerrainGenerator;
class QgsChunkNode;
class Qgs3DExportObject;
class QgsTerrainTextureGenerator;
class QgsVectorLayer;
class QgsPolygon3DSymbol;
class QgsLine3DSymbol;
class QgsPoint3DSymbol;
class QgsMeshEntity;

#define SIP_NO_FILE

/**
 * \brief Entity that handles the exporting of 3D scene
 *
 * \note Not available in Python bindings
 *
 * \ingroup 3d
 * \since QGIS 3.16
 */
class _3D_EXPORT Qgs3DSceneExporter : public Qt3DCore::QEntity
{
    Q_OBJECT

  public:
    //! Constructor
    Qgs3DSceneExporter() { }
    //! Destructor
    ~Qgs3DSceneExporter()
    {
      for ( Qgs3DExportObject *obj : mObjects )
        delete obj;
    }

    /**
     * Creates necessary export objects from entity if it represents valid vector layer entity
     * Returns FALSE if the no 3D object was extracted from the vector layer
     */
    bool parseVectorLayerEntity( Qt3DCore::QEntity *entity, QgsVectorLayer *layer );

    //! Creates terrain export objects from the terrain entity
    void parseTerrain( QgsTerrainEntity *terrain, const  QString &layer );

    //! Saves the scene to a .obj file
    void save( const QString &sceneName, const QString &sceneFolderPath );

    //! Sets whether the triangles will look smooth
    void setSmoothEdges( bool smoothEdges ) { mSmoothEdges = smoothEdges; }
    //! Returns whether the triangles will look smooth
    bool smoothEdges() const { return mSmoothEdges; }

    //! Sets whether the normals will be exported
    void setExportNormals( bool exportNormals ) { mExportNormals = exportNormals; }
    //! Returns whether the normals will be exported
    bool exportNormals() const { return mExportNormals; }

    //! Sets whether the textures will be exported
    void setExportTextures( bool exportTextures ) { mExportTextures = exportTextures; }
    //! Returns whether the textures will be exported
    bool exportTextures() const { return mExportTextures; }

    //! Sets the terrain resolution
    void setTerrainResolution( int resolution ) { mTerrainResolution = resolution; }
    //! Returns the terrain resolution
    int terrainResolution() const { return mTerrainResolution; }

    //! Sets the terrain texture resolution
    void setTerrainTextureResolution( int resolution ) { mTerrainTextureResolution = resolution; }
    //! Returns the terrain resolution
    int terrainTextureResolution() const { return mTerrainTextureResolution; }
    //! Sets the scale of the exported 3D model
    void setScale( float scale ) { mScale = scale; }
    //! Returns the scale of the exported 3D model
    float scale() const { return mScale; }

  private:
    //! Constructs Qgs3DExportObject from instanced point geometry
    QVector<Qgs3DExportObject *> processInstancedPointGeometry( Qt3DCore::QEntity *entity, const QString &objectNamePrefix );
    //! Constructs Qgs3DExportObject from 3D models loaded using a scene loader
    QVector<Qgs3DExportObject *> processSceneLoaderGeometries( Qt3DRender::QSceneLoader *sceneLoader, const QString &objectNamePrefix );
    //! Constructs Qgs3DExportObject from geometry renderer
    Qgs3DExportObject *processGeometryRenderer( Qt3DRender::QGeometryRenderer *mesh, const QString &objectNamePrefix, float sceneScale = 1.0f, QVector3D sceneTranslation = QVector3D( 0.0f, 0.0f, 0.0f ) );
    //! Extracts material information from geometry renderer and inserts it into the export object
    void processEntityMaterial( Qt3DCore::QEntity *entity, Qgs3DExportObject *object );
    //! Constricts Qgs3DExportObject from line entity
    QVector<Qgs3DExportObject *> processLines( Qt3DCore::QEntity *entity, const QString &objectNamePrefix );
    //! Constricts Qgs3DExportObject from billboard point entity
    Qgs3DExportObject *processPoints( Qt3DCore::QEntity *entity, const QString &objectNamePrefix );

    //! Returns a tile entity that contains the geometry to be exported and necessary scaling parameters
    QgsTerrainTileEntity *getFlatTerrainEntity( QgsTerrainEntity *terrain, QgsChunkNode *node );
    //! Returns a tile entity that contains the geometry to be exported and necessary scaling parameters
    QgsTerrainTileEntity *getDemTerrainEntity( QgsTerrainEntity *terrain, QgsChunkNode *node );
    //! Returns a tile entity that contains the geometry to be exported and necessary scaling parameters
    QgsTerrainTileEntity *getMeshTerrainEntity( QgsTerrainEntity *terrain, QgsChunkNode *node );

    //! Constructs a Qgs3DExportObject from the DEM tile entity
    void parseDemTile( QgsTerrainTileEntity *tileEntity, const QString &layerName );
    //! Constructs a Qgs3DExportObject from the flat tile entity
    void parseFlatTile( QgsTerrainTileEntity *tileEntity, const QString &layerName );
    //! Constructs a Qgs3DExportObject from the mesh terrain entity
    void parseMeshTile( QgsTerrainTileEntity *meshEntity, const QString &layerName );

    QString getObjectName( const QString &name );
  private:
    QMap<QString, int> usedObjectNamesCounter;
    QVector<Qgs3DExportObject *> mObjects;

    bool mSmoothEdges = false;
    int mTerrainResolution = 128;
    bool mExportNormals = true;
    bool mExportTextures = false;
    int mTerrainTextureResolution = 512;
    float mScale = 1.0f;

    friend QgsPolygon3DSymbol;
    friend QgsLine3DSymbol;
    friend QgsPoint3DSymbol;
};

#endif // QGS3DSCENEEXPORTER_H
