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

class _3D_EXPORT Qgs3DMapExportSettings : public QObject
{
    Q_OBJECT
  public:
    Qgs3DMapExportSettings(QObject* parent = nullptr);
    QString sceneName() const { return mSceneName; }
    QString sceneFolderPath() const { return mSceneFolderPath; }
    int levelOfDetails() const { return mLevelOfDetails; }
    bool smoothEdges() const { return mSmoothEdges; }

    void setSceneName( const QString& sceneName ) { mSceneName = sceneName; }
    void setSceneFolderPath( const QString& sceneFolderPath ) { mSceneFolderPath = sceneFolderPath; }
    void setLevelOfDetails( int levelOfDetails ) { mLevelOfDetails = levelOfDetails; }
    void setSmoothEdges( bool smoothEdges ) { mSmoothEdges = smoothEdges; }
  private:
    QString mSceneName;
    QString mSceneFolderPath;
    int mLevelOfDetails;
    bool mSmoothEdges;
};

#endif // QGS3DMAPEXPORTSETTINGS_H
