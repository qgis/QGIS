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
     * Creates necessary export objects from entity if it represents valid entity
     * If the entity doesn't define exportable object it will be ignored
     */
    void parseEntity( Qt3DCore::QEntity *entity );
    //! Creates terrain export objects from the terrain entity
    void parseEntity( QgsTerrainEntity *terrain );
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

    //! Sets the terrian resolution
    void setTerrainResolution( int resolution ) { mTerrainResolution = resolution; }
    //! Returns the terrain resolution
    int terrainResolution() const { return mTerrainResolution; }

    //! Sets the terrian texture resolution
    void setTerrainTextureResolution( int resolution ) { mTerrainTextureResolution = resolution; }
    //! Returns the terrain resolution
    int terrainTextureResolution() const { return mTerrainTextureResolution; }

  private:
    //! Processes the attribute directly by taking a position buffer and converting it to Qgs3DExportObject
    void processAttribute( Qt3DRender::QAttribute *attribute );
    //! Processes the tessellated polygons geometry and constructs Qgs3DExportObject from it
    void process( QgsTessellatedPolygonGeometry *geom );

    //! Returns a tile entity that contains the geometry to be exported and necessary scaling parameters
    QgsTerrainTileEntity *getFlatTerrainEntity( QgsTerrainEntity *terrain, QgsChunkNode *node );
    //! Returns a tile entity that contains the geometry to be exported and necessary scaling parameters
    QgsTerrainTileEntity *getDemTerrainEntity( QgsTerrainEntity *terrain, QgsChunkNode *node );

    //! Constructs a Qgs3DExportObject from the DEM tile entity
    void parseDemTile( QgsTerrainTileEntity *tileEntity, QgsTerrainTextureGenerator *textureGenerator );
    //! Constructs a Qgs3DExportObject from the flat tile entity
    void parseFlatTile( QgsTerrainTileEntity *tileEntity, QgsTerrainTextureGenerator *textureGenerator );

    QString getObjectName( const QString &name );
  private:
    QMap<QString, int> usedObjectNamesCounter;
    QVector<Qgs3DExportObject *> mObjects;

    bool mSmoothEdges;
    int mTerrainResolution;
    bool mExportNormals;
    bool mExportTextures;
    int mTerrainTextureResolution;
};

#endif // QGS3DSCENEEXPORTER_H
