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

#include "qgis_3d.h"

#include <QString>
#include <QObject>
#include <QDir>

/**
 * \brief Manages the various settings the user can choose from when exporting a 3D scene
 * \ingroup 3d
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

    //! Sets the scene name
    void setSceneName( const QString &sceneName ) { mSceneName = sceneName; }
    //! Sets the scene's .obj file folder path
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

  private:
    QString mSceneName = QString( "Scene" );
    QString mSceneFolderPath = QDir::homePath();
    int mTerrainResolution = 128;
    bool mSmoothEdges = false;
    bool mExportNormals = true;
    bool mExportTextures = false;
    int mTerrainTextureResolution = 512;
    float mScale = 1.0f;
};

#endif // QGS3DMAPEXPORTSETTINGS_H
