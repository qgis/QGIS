/***************************************************************************
  Qgs3DExportObject.cpp
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

#include "qgs3dexportobject.h"

#include <QVector3D>
#include <QDebug>
#include <QDir>
#include <QImage>

Qgs3DExportObject::Qgs3DExportObject( const QString &name, const QString &parentName, QObject *parent )
  : QObject( parent )
  , mName( name )
  , mParentName( parentName )
  , mSmoothEdges( false )
{
}

void Qgs3DExportObject::setupPositionCoordinates( const QVector<float> &positionsBuffer, float scale, const QVector3D translation )
{
  for ( int i = 0; i < positionsBuffer.size(); i += 3 )
  {
    for ( int j = 0; j < 3; ++j )
    {
      mVertexPosition << positionsBuffer[i + j] * scale + translation[j];
    }
  }

  for ( int i = 0; i < positionsBuffer.size() / 3; ++i )
  {
    mIndexes << i + 1;
  }
}

void Qgs3DExportObject::setupPositionCoordinates( const QVector<float> &positionsBuffer, const QVector<unsigned int> &faceIndex, float scale, const QVector3D translation )
{
  // TODO: delete vertices that are not used
  for ( int i = 0; i < positionsBuffer.size(); i += 3 )
  {
    for ( int j = 0; j < 3; ++j )
    {
      mVertexPosition << positionsBuffer[i + j] * scale + translation[j];
    }
  }

  for ( int i = 0; i < faceIndex.size(); i += 3 )
  {
    // skip invalid triangles
    if ( faceIndex[i] == faceIndex[i + 1] && faceIndex[i + 1] == faceIndex[i + 2] )
      continue;
    for ( int j = 0; j < 3; ++j )
      mIndexes << faceIndex[i + j] + 1;
  }
}

void Qgs3DExportObject::setupNormalCoordinates( const QVector<float> &normalsBuffer )
{
  mNormals << normalsBuffer;
}

void Qgs3DExportObject::setupTextureCoordinates( const QVector<float> &texturesBuffer )
{
  mTexturesUV << texturesBuffer;
}

void Qgs3DExportObject::objectBounds( float &minX, float &minY, float &minZ, float &maxX, float &maxY, float &maxZ )
{
  for ( unsigned int vertice : mIndexes )
  {
    int heightIndex = ( vertice - 1 ) * 3 + 1;
    minX = std::min( minX, mVertexPosition[heightIndex - 1] );
    maxX = std::max( maxX, mVertexPosition[heightIndex - 1] );
    minY = std::min( minY, mVertexPosition[heightIndex] );
    maxY = std::max( maxY, mVertexPosition[heightIndex] );
    minZ = std::min( minZ, mVertexPosition[heightIndex + 1] );
    maxZ = std::max( maxZ, mVertexPosition[heightIndex + 1] );
  }
}

void Qgs3DExportObject::saveTo( QTextStream &out, float scale, const QVector3D &center )
{
  // Set groups
  // turns out grouping doest work as expected in blender

  // smoothen edges
  if ( mSmoothEdges )
    out << "s on\n";
  else
    out << "s off\n";

  // Construct vertices
  for ( int i = 0; i < mVertexPosition.size(); i += 3 )
  {
    // for now just ignore wrong vertex positions
    out << "v ";
    out << ( mVertexPosition[i] - center.x() ) / scale << " ";
    out << ( mVertexPosition[i + 1] - center.y() ) / scale << " ";
    out << ( mVertexPosition[i + 2] - center.z() ) / scale << "\n";
    if ( i + 3 <= mNormals.size() )
    {
      out << "vn " << mNormals[i] << " " << mNormals[i + 1] << " " << mNormals[i + 2] << "\n";
    }
    int u_index = i / 3 * 2;
    if ( u_index + 1 < mTexturesUV.size() )
    {
      // TODO: flip texture in a more appropriate way (for repeated textures)
      out << "vt " << mTexturesUV[u_index] << " " << 1.0f - mTexturesUV[u_index + 1] << "\n";
    }
  }

  bool hasTextures = mTexturesUV.size() == mVertexPosition.size() / 3 * 2;
  // if the object has normals then the normals and positions buffers should be the same size
  bool hasNormals = mNormals.size() == mVertexPosition.size();

  if ( !hasNormals && !mNormals.empty() )
  {
    qDebug() << "WARNING: vertex normals count and vertex positions count are different";
  }
  int verticesCount = mVertexPosition.size() / 3;

  auto getVertexIndex = [&]( int i ) -> QString
  {
    int negativeIndex = -1 - ( verticesCount - i );
    if ( hasNormals && !hasTextures )
      return QString( "%1//%2" ).arg( negativeIndex ).arg( negativeIndex );
    if ( !hasNormals && hasTextures )
      return QString( "%1/%2" ).arg( negativeIndex ).arg( negativeIndex );
    if ( hasNormals && hasTextures )
      return QString( "%1/%2/%3" ).arg( negativeIndex ).arg( negativeIndex ).arg( negativeIndex );
    return QString( "%1" ).arg( negativeIndex );
  };

  // Construct faces
  for ( int i = 0; i < mIndexes.size(); i += 3 )
  {
    if ( mIndexes[i] == mIndexes[i + 1] && mIndexes[i + 1] == mIndexes[i + 2] )
      continue;
    out << "f " << getVertexIndex( mIndexes[i] );
    out << " " << getVertexIndex( mIndexes[i + 1] );
    out << " " << getVertexIndex( mIndexes[i + 2] );
    out << "\n";
  }
}

QString Qgs3DExportObject::saveMaterial( QTextStream &mtlOut, const QString &folderPath )
{
  QString textureName = mName + "_material";
  if ( mTexturesUV.size() == 0 )
    return QString();
  QString filePath = QDir( folderPath ).filePath( textureName + ".jpg" );
  mTextureImage.save( filePath, "JPG" );

  mtlOut << "newmtl " << textureName << "\n";
  mtlOut << "\tmap_Kd " << textureName << ".jpg" << "\n";
  return textureName;
}
