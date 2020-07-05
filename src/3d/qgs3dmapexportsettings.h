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
 * @brief The Qgs3DMapExportSettings class
 * Manages the various settings the user can choose from when exorting a 3D scene
 * \ingroup 3d
 * \since QGIS 3.16
 */
class _3D_EXPORT Qgs3DMapExportSettings : public QObject
{
    Q_OBJECT
  public:
    //! Constructor
    Qgs3DMapExportSettings( QObject *parent = nullptr ): QObject( parent ) { }

    //! Returns the scene name
    QString sceneName() const { return mSceneName; }
    //! Returns the scene folder path
    QString sceneFolderPath() const { return mSceneFolderPath; }
    //! Returns the terrain resolution
    int terrrainResolution() const { return mTerrainResolution; }
    //! Returns whether triangles edges will look smooth
    bool smoothEdges() const { return mSmoothEdges; }

    //! Sets the scene name
    void setSceneName( const QString &sceneName ) { mSceneName = sceneName; }
    //! Sets the scene's .obj file folder path
    void setSceneFolderPath( const QString &sceneFolderPath ) { mSceneFolderPath = sceneFolderPath; }
    //! Sets the terrain resolution
    void setTerrainResolution( int resolution ) { mTerrainResolution = resolution; }
    //! Sets whether triangles edges will look smooth
    void setSmoothEdges( bool smoothEdges ) { mSmoothEdges = smoothEdges; }
    //! Returns a path to a file that with the "name.extension" in the export folder
    QString getFilePath( const QString &name, const QString &extension ) const
    {
      return QDir( mSceneFolderPath ).filePath( name + "." + extension );
    }
  private:
    QString mSceneName = QString( "Scene" );
    QString mSceneFolderPath = QDir::homePath();
    int mTerrainResolution = 128;
    bool mSmoothEdges = false;
};

#endif // QGS3DMAPEXPORTSETTINGS_H
