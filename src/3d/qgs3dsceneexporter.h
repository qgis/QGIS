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
#include <Qt3DRender/QAttribute>
#include <Qt3DExtras/QPlaneGeometry>
#include <QMap>
#include <QFile>

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

/**
 * \brief The Qgs3DSceneExporter class
 * Entity that handles the exporting of 3D scene
 * \ingroup 3d
 * \since QGIS 3.16
 */
class Qgs3DSceneExporter : public Qt3DCore::QEntity
{
    Q_OBJECT
  public:
    //! Constructor
    Qgs3DSceneExporter( Qt3DCore::QNode *parent = nullptr );

    /**
     * Creates necessary export objects from entity if it represents valid vector layer entity
     * Returns false if the no 3D object was extracted from the vector layer
     */
    bool parseVectorLayerEntity( Qt3DCore::QEntity *entity, QgsVectorLayer *layer );

    //! Creates terrain export objects from the terrain entity
    void parseTerrain( QgsTerrainEntity *terrain );
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
    //! Processes the attribute directly by taking a position buffer and converting it to Qgs3DExportObject
    void pocessPoistionAttributes( Qt3DRender::QGeometry *geometry );
    //! constructs Qgs3DExportObject from the polygon geometry
    void processPolygonGeometry( QgsTessellatedPolygonGeometry *geom, const QgsPolygon3DSymbol *polygonSymbol );

    void processBufferedLineGeometry( QgsTessellatedPolygonGeometry *geom, const QgsLine3DSymbol *lineSymbol );

    //! Returns a tile entity that contains the geometry to be exported and necessary scaling parameters
    QgsTerrainTileEntity *getFlatTerrainEntity( QgsTerrainEntity *terrain, QgsChunkNode *node );
    //! Returns a tile entity that contains the geometry to be exported and necessary scaling parameters
    QgsTerrainTileEntity *getDemTerrainEntity( QgsTerrainEntity *terrain, QgsChunkNode *node );

    //! Constructs a Qgs3DExportObject from the DEM tile entity
    void parseDemTile( QgsTerrainTileEntity *tileEntity );
    //! Constructs a Qgs3DExportObject from the flat tile entity
    void parseFlatTile( QgsTerrainTileEntity *tileEntity );

    QString getObjectName( const QString &name );
  private:
    QMap<QString, int> usedObjectNamesCounter;
    QVector<Qgs3DExportObject *> mObjects;

    bool mSmoothEdges;
    int mTerrainResolution;
    bool mExportNormals;
    bool mExportTextures;
    int mTerrainTextureResolution;
    float mScale;
};

#endif // QGS3DSCENEEXPORTER_H
