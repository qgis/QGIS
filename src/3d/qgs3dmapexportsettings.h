/***************************************************************************
  qgs3dmapexportsettings.h
  --------------------------------------
  Date                 : July 2020
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

#ifndef QGS3DMAPEXPORTSETTINGS_H
#define QGS3DMAPEXPORTSETTINGS_H

#include "qgis.h"
#include "qgis_3d.h"

#include <QDir>
#include <QObject>
#include <QString>

/**
 * \brief Manages the various settings the user can choose from when exporting a 3D scene.
 * \ingroup qgis_3d
 * \since QGIS 3.16
 */
class _3D_EXPORT Qgs3DMapExportSettings
{
  public:
    //! Constructor
    Qgs3DMapExportSettings();

    //! destructor (save the export settings before deallocation)
    ~Qgs3DMapExportSettings();

    //! Returns the scene name
    QString sceneName() const { return mSceneName; }
    //! Returns the scene folder path
    QString sceneFolderPath() const { return mSceneFolderPath; }
    //! Returns the terrain resolution
    int terrrainResolution() const { return mTerrainResolution; }
    //! Returns whether triangles edges will look smooth
    bool smoothEdges() const { return mSmoothEdges; }
    //! Returns whether normals will be exported
    bool exportNormals() const { return mExportNormals; }
    //! Returns whether textures will be exported
    bool exportTextures() const { return mExportTextures; }
    //! Returns the terrain texture resolution
    int terrainTextureResolution() const { return mTerrainTextureResolution; }
    //! Returns the scale of the exported model
    float scale() const { return mScale; }
    /**
     * Returns the export format for the 3D scene.
     * \since QGIS 4.0
     */
    Qgis::Export3DSceneFormat exportFormat() const { return mExportFormat; }
    /**
     * Returns the full file uri where the 3D scene will be exported.
     * \since QGIS 4.0
     */
    QString exportFileUri() const;

    /**
     * Returns whether terrain export is enabled.
     * It terrain export is disabled, the terrain resolution and terrain texture resolution
     * parameters have no effect.
     *
     * \see setTerrainExportEnabled()
     * \since QGIS 4.0
     */
    bool terrainExportEnabled() const { return mTerrainExportEnabled; }

    //! Sets the scene name
    void setSceneName( const QString &sceneName ) { mSceneName = sceneName; }
    //! Sets the folder path where exported 3D scene files will be saved.
    void setSceneFolderPath( const QString &sceneFolderPath ) { mSceneFolderPath = sceneFolderPath; }
    //! Sets the terrain resolution
    void setTerrainResolution( int resolution ) { mTerrainResolution = resolution; }
    //! Sets whether triangles edges will look smooth
    void setSmoothEdges( bool smoothEdges ) { mSmoothEdges = smoothEdges; }
    //! Sets whether normals should be exported
    void setExportNormals( bool exportNormals ) { mExportNormals = exportNormals; }
    //! Sets whether textures will be exported
    void setExportTextures( bool exportTextures ) { mExportTextures = exportTextures; }
    //! Sets the terrain texture resolution
    void setTerrainTextureResolution( int resolution ) { mTerrainTextureResolution = resolution; }
    //! Sets the scale of exported model
    void setScale( float scale ) { mScale = scale; }
    /**
     * Sets the export format for the 3D scene.
     * \since QGIS 4.0
     */
    void setExportFormat( Qgis::Export3DSceneFormat exportFormat ) { mExportFormat = exportFormat; }

    /**
     * Sets whether terrain export is enabled.
     *
     * \see terrainExportEnabled()
     * \since QGIS 4.0
     */
    void setTerrainExportEnabled( bool enabled ) { mTerrainExportEnabled = enabled; }

  private:
    QString mSceneName = QString( "Scene" );
    QString mSceneFolderPath = QDir::homePath();
    int mTerrainResolution = 128;
    bool mSmoothEdges = false;
    bool mExportNormals = true;
    bool mExportTextures = false;
    int mTerrainTextureResolution = 512;
    float mScale = 1.0f;
    bool mTerrainExportEnabled = true;
    Qgis::Export3DSceneFormat mExportFormat = Qgis::Export3DSceneFormat::Obj;
};

#endif // QGS3DMAPEXPORTSETTINGS_H
