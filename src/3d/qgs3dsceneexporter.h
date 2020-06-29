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

class Qgs3DSceneExporter
{

  public:
    Qgs3DSceneExporter( );

    void parseEntity( Qt3DCore::QEntity *entity );
    void saveToFile( const QString &filePath );
  private:
    void processAttribute( Qt3DRender::QAttribute *attribute );
  private:
    QVector<float> mVertices;
};

#endif // QGS3DSCENEEXPORTER_H
