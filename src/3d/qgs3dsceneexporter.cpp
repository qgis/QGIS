/***************************************************************************
  qgs3dsceneexporter.cpp
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

#include "qgs3dsceneexporter.h"

#include <QVector>
#include <Qt3DCore/QEntity>
#include <Qt3DCore/QComponent>
#include <Qt3DCore/QNode>
#include <Qt3DRender/QGeometry>
#include <Qt3DRender/QAttribute>
#include <Qt3DRender/QBuffer>
#include <Qt3DRender/QGeometryRenderer>
#include <QByteArray>
#include <QFile>
#include <QTextStream>

Qgs3DSceneExporter::Qgs3DSceneExporter( )
  : mVertices( QVector<float>() )
{

}


void Qgs3DSceneExporter::parseEntity( Qt3DCore::QEntity *entity )
{
  if ( entity == nullptr ) return;
  for ( Qt3DCore::QComponent *c : entity->components() )
  {
    Qt3DRender::QGeometryRenderer *comp = qobject_cast<Qt3DRender::QGeometryRenderer *>( c );
    if ( comp == nullptr ) continue;
    Qt3DRender::QGeometry *geom = comp->geometry();
    for ( Qt3DRender::QAttribute *attribute : geom->attributes() )
    {
      processAttribute( attribute );
    }
  }
  for ( QObject *child : entity->children() )
  {
    Qt3DCore::QEntity *childEntity = qobject_cast<Qt3DCore::QEntity *>( child );
    if ( childEntity != nullptr ) parseEntity( childEntity );
  }
}

void Qgs3DSceneExporter::processAttribute( Qt3DRender::QAttribute *attribute )
{
  // We only process position attributes
  if ( attribute->name() != Qt3DRender::QAttribute::defaultPositionAttributeName() ) return;

  QByteArray data = attribute->buffer()->data();
  uint bytesOffset = attribute->byteOffset();
  uint bytesStride = attribute->byteStride();

  QVector<float> floatData;
  for ( int i = 0; i < data.size(); i += sizeof( float ) )
  {
    float v = 0x0;
    // Maybe we can have a problem with endianness ?
    char *v_arr = ( char * ) &v;
    for ( int j = 0; j < sizeof( float ); ++j )
    {
      v_arr[j] = data.at( i + j );
    }
    floatData.push_back( v );
  }

  for ( int i = bytesOffset / sizeof( float ); i < floatData.size(); i += bytesStride / sizeof( float ) )
  {
    mVertices << floatData[i] << floatData[i + 1] << floatData[i + 2];
  }
}

void Qgs3DSceneExporter::saveToFile( const QString &filePath )
{
  QFile file( filePath );
  if ( !file.open( QIODevice::WriteOnly | QIODevice::Text ) )
    return;

  QTextStream out( &file );

  // Construct vertices
  for ( int i = 0; i < mVertices.size(); i += 3 )
  {
    out << "v " << mVertices[i] << " " << mVertices[i + 1] << " " << mVertices[i + 2] << "\n";
  }

  // Construct faces
  for ( int i = 0; i < mVertices.size() / 9; ++i )
  {
    out << "f " << 3 * i + 1 << " " << 3 * i + 2 << " " << 3 * i + 3 << "\n";
  }
}
