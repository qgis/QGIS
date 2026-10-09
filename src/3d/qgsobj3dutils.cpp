/***************************************************************************
  qgsobj3dutils.cpp
  --------------------------------------
  Date                 : April 2026
  Copyright            : (C) 2026 by Dominik Cindrić
  Email                : viper dot miniq at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#define TINYOBJLOADER_IMPLEMENTATION
#define TINYOBJLOADER_USE_MAPBOX_EARCUT
#include "qgsobj3dutils.h"

#include <tiny_obj_loader.h>
#include <unordered_map>

#include "qgs3dutils.h"
#include "qgsimagetexture.h"
#include "qgslogger.h"
#include "qgsmaterial3dhandler.h"
#include "qgsphongmaterial.h"
#include "qgsphongtexturedmaterial.h"

#include <QDir>
#include <QFileInfo>
#include <QImage>
#include <QString>
#include <Qt3DCore/QAttribute>
#include <Qt3DCore/QBuffer>
#include <Qt3DCore/QGeometry>
#include <Qt3DRender/QTexture>

using namespace Qt::StringLiterals;

///@cond PRIVATE

std::vector<QgsMeshNodeData> QgsObj3DUtils::buildObjGeometries( const QString &filePath, const QgsMaterialContext &materialContext )
{
  tinyobj::ObjReaderConfig config;
  config.triangulate = true;
  config.triangulation_method = "earcut";
  config.mtl_search_path = QFileInfo( filePath ).absolutePath().toStdString();

  tinyobj::ObjReader reader;
  if ( !reader.ParseFromFile( filePath.toStdString(), config ) )
  {
    if ( !reader.Error().empty() )
      QgsDebugMsgLevel( u"tinyobj error loading '%1': %2"_s.arg( filePath, QString::fromStdString( reader.Error() ) ), 1 );
    return {};
  }
  if ( !reader.Warning().empty() )
    QgsDebugMsgLevel( u"tinyobj warning loading '%1': %2"_s.arg( filePath, QString::fromStdString( reader.Warning() ) ), 2 );

  const tinyobj::attrib_t &attrib = reader.GetAttrib();
  const std::vector<tinyobj::shape_t> &shapes = reader.GetShapes();
  const std::vector<tinyobj::material_t> &materials = reader.GetMaterials();

  const QString objDir = QFileInfo( filePath ).absolutePath();

  const bool hasNormals = !attrib.normals.empty();
  const bool hasTexCoords = !attrib.texcoords.empty();

  const size_t floatsPerVertex = 3 + ( hasNormals ? 3 : 0 ) + ( hasTexCoords ? 2 : 0 );

  std::unordered_map<int, std::vector<float>> matDataMap;

  // see https://github.com/tinyobjloader/tinyobjloader
  for ( size_t shapeIdx = 0; shapeIdx < shapes.size(); shapeIdx++ )
  {
    size_t index_offset = 0;
    for ( size_t faceIdx = 0; faceIdx < shapes[shapeIdx].mesh.num_face_vertices.size(); faceIdx++ )
    {
      const size_t faceVertexCount = size_t( shapes[shapeIdx].mesh.num_face_vertices[faceIdx] );
      const int matId = shapes[shapeIdx].mesh.material_ids[faceIdx];
      std::vector<float> &data = matDataMap[matId];

      data.reserve( data.size() + faceVertexCount * floatsPerVertex );

      for ( size_t v = 0; v < faceVertexCount; v++ )
      {
        const tinyobj::index_t idx = shapes[shapeIdx].mesh.indices[index_offset + v];

        data.push_back( attrib.vertices[3 * size_t( idx.vertex_index ) + 0] );
        data.push_back( attrib.vertices[3 * size_t( idx.vertex_index ) + 1] );
        data.push_back( attrib.vertices[3 * size_t( idx.vertex_index ) + 2] );

        if ( hasNormals )
        {
          if ( idx.normal_index >= 0 )
          {
            data.push_back( attrib.normals[3 * size_t( idx.normal_index ) + 0] );
            data.push_back( attrib.normals[3 * size_t( idx.normal_index ) + 1] );
            data.push_back( attrib.normals[3 * size_t( idx.normal_index ) + 2] );
          }
          else
          {
            data.push_back( 0.0f );
            data.push_back( 0.0f );
            data.push_back( 0.0f );
          }
        }

        if ( hasTexCoords )
        {
          if ( idx.texcoord_index >= 0 )
          {
            data.push_back( attrib.texcoords[2 * size_t( idx.texcoord_index ) + 0] );
            data.push_back( 1.0f - attrib.texcoords[2 * size_t( idx.texcoord_index ) + 1] ); // flip v
          }
          else
          {
            data.push_back( 0.0f );
            data.push_back( 0.0f );
          }
        }
      }
      index_offset += faceVertexCount;
    }
  }

  std::vector<QgsMeshNodeData> result;
  result.reserve( matDataMap.size() );

  for ( auto &[matId, vertices] : matDataMap )
  {
    if ( vertices.empty() )
      continue;

    Qt3DCore::QGeometry *geom = new Qt3DCore::QGeometry;

    QByteArray vertexBufferData( reinterpret_cast<const char *>( vertices.data() ), static_cast<qsizetype>( vertices.size() * sizeof( float ) ) );

    Qt3DCore::QBuffer *vertexBuffer = new Qt3DCore::QBuffer( geom );
    vertexBuffer->setData( vertexBufferData );

    const int vertexFloats = 3 + ( hasNormals ? 3 : 0 ) + ( hasTexCoords ? 2 : 0 );
    const int stride = vertexFloats * static_cast<int>( sizeof( float ) );
    const uint vertCount = static_cast<uint>( vertices.size() / vertexFloats );

    Qt3DRender::QAbstractTexture *diffuseTex = nullptr;
    if ( matId >= 0 && matId < static_cast<int>( materials.size() ) )
    {
      const std::string &texName = materials[matId].diffuse_texname;
      if ( !texName.empty() )
      {
        const QString texPath = QDir( objDir ).filePath( QString::fromStdString( texName ) );
        const QImage img( texPath );
        if ( !img.isNull() )
        {
          Qt3DRender::QTexture2D *texture = new Qt3DRender::QTexture2D;
          texture->wrapMode()->setX( Qt3DRender::QTextureWrapMode::Repeat );
          texture->wrapMode()->setY( Qt3DRender::QTextureWrapMode::Repeat );
          texture->setFormat( Qt3DRender::QAbstractTexture::SRGB8_Alpha8 );
          texture->addTextureImage( new QgsImageTexture( img ) );
          Qgs3DUtils::setTextureFiltering( texture, materialContext );
          diffuseTex = texture;
        }
        else
        {
          QgsDebugMsgLevel( u"OBJ texture not found or unreadable: '%1'"_s.arg( texPath ), 2 );
        }
      }
    }

    int byteOffset = 0;

    Qt3DCore::QAttribute *posAttr = new Qt3DCore::QAttribute;
    posAttr->setName( Qt3DCore::QAttribute::defaultPositionAttributeName() );
    posAttr->setVertexBaseType( Qt3DCore::QAttribute::Float );
    posAttr->setVertexSize( 3 );
    posAttr->setAttributeType( Qt3DCore::QAttribute::VertexAttribute );
    posAttr->setBuffer( vertexBuffer );
    posAttr->setByteStride( stride );
    posAttr->setByteOffset( byteOffset );
    posAttr->setCount( vertCount );
    geom->addAttribute( posAttr );
    byteOffset += 3 * static_cast<int>( sizeof( float ) );

    if ( hasNormals )
    {
      Qt3DCore::QAttribute *normalAttr = new Qt3DCore::QAttribute;
      normalAttr->setName( Qt3DCore::QAttribute::defaultNormalAttributeName() );
      normalAttr->setVertexBaseType( Qt3DCore::QAttribute::Float );
      normalAttr->setVertexSize( 3 );
      normalAttr->setAttributeType( Qt3DCore::QAttribute::VertexAttribute );
      normalAttr->setBuffer( vertexBuffer );
      normalAttr->setByteStride( stride );
      normalAttr->setByteOffset( byteOffset );
      normalAttr->setCount( vertCount );
      geom->addAttribute( normalAttr );
      byteOffset += 3 * static_cast<int>( sizeof( float ) );
    }

    if ( hasTexCoords && diffuseTex )
    {
      Qt3DCore::QAttribute *texAttr = new Qt3DCore::QAttribute;
      texAttr->setName( Qt3DCore::QAttribute::defaultTextureCoordinateAttributeName() );
      texAttr->setVertexBaseType( Qt3DCore::QAttribute::Float );
      texAttr->setVertexSize( 2 );
      texAttr->setAttributeType( Qt3DCore::QAttribute::VertexAttribute );
      texAttr->setBuffer( vertexBuffer );
      texAttr->setByteStride( stride );
      texAttr->setByteOffset( byteOffset );
      texAttr->setCount( vertCount );
      geom->addAttribute( texAttr );
    }

    QgsMaterial *mat = nullptr;
    if ( diffuseTex )
    {
      QgsPhongTexturedMaterial *texMat = new QgsPhongTexturedMaterial();
      texMat->setDiffuseTexture( diffuseTex );
      mat = texMat;
    }
    else if ( matId >= 0 && matId < static_cast<int>( materials.size() ) )
    {
      const tinyobj::material_t &m = materials[matId];
      QgsPhongMaterial *phong = new QgsPhongMaterial();
      // we explicitly avoid setting the ambient color, it is defined as white in most MTL files
      // ideally, we should be using PBR material, rather than Phong
      phong->setDiffuse( QColor::fromRgbF( m.diffuse[0], m.diffuse[1], m.diffuse[2] ) );
      phong->setSpecular( QColor::fromRgbF( m.specular[0], m.specular[1], m.specular[2] ) );
      phong->setShininess( m.shininess );
      phong->setOpacity( m.dissolve );
      mat = phong;
    }

    result.push_back( QgsMeshNodeData { std::unique_ptr<Qt3DCore::QGeometry>( geom ), std::unique_ptr<QgsMaterial>( mat ), QMatrix4x4() } );
  }

  return result;
}

///@endcond
