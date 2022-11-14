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
#include <QDir>
#include <QImage>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <Qt3DRender/QAttribute>
#include <Qt3DRender/QBuffer>
typedef Qt3DRender::QAttribute Qt3DQAttribute;
typedef Qt3DRender::QBuffer Qt3DQBuffer;
#else
#include <Qt3DCore/QAttribute>
#include <Qt3DCore/QBuffer>
typedef Qt3DCore::QAttribute Qt3DQAttribute;
typedef Qt3DCore::QBuffer Qt3DQBuffer;
#endif

#include "qgslogger.h"
#include "qgsabstractmaterialsettings.h"


template<typename T>
void insertIndexData( QVector<uint> &vertexIndex, const QVector<T> &faceIndex )
{
  for ( int i = 0; i < faceIndex.size(); i += 3 )
  {
    if ( i + 2 >= faceIndex.size() ) continue;
    // skip invalid triangles
    if ( faceIndex[i] == faceIndex[i + 1] || faceIndex[i + 1] == faceIndex[i + 2] || faceIndex[i] == faceIndex[i + 2] )
      continue;
    for ( int j = 0; j < 3; ++j )
      vertexIndex << faceIndex[i + j] + 1;
  }
}

void Qgs3DExportObject::setupPositionCoordinates( const QVector<float> &positionsBuffer, float scale, const QVector3D &translation )
{
  for ( int i = 0; i < positionsBuffer.size(); i += 3 )
  {
    for ( int j = 0; j < 3; ++j )
    {
      mVertexPosition << positionsBuffer[i + j] * scale + translation[j];
    }
  }
}

void Qgs3DExportObject::setupFaces( const QVector<uint> &facesIndexes )
{
  insertIndexData<uint>( mIndexes, facesIndexes );
}

void Qgs3DExportObject::setupLine( const QVector<uint> &lineIndexes )
{
  Q_UNUSED( lineIndexes );
  for ( int i = 0; i < mVertexPosition.size(); i += 3 ) mIndexes << i / 3 + 1;
}

void Qgs3DExportObject::setupNormalCoordinates( const QVector<float> &normalsBuffer )
{
  mNormals << normalsBuffer;
}

void Qgs3DExportObject::setupTextureCoordinates( const QVector<float> &texturesBuffer )
{
  mTexturesUV << texturesBuffer;
}

void Qgs3DExportObject::setupMaterial( QgsAbstractMaterialSettings *material )
{
  QMap<QString, QString> parameters = material->toExportParameters();
  for ( auto it = parameters.begin(); it != parameters.end(); ++it )
  {
    setMaterialParameter( it.key(), it.value() );
  }
}

void Qgs3DExportObject::objectBounds( float &minX, float &minY, float &minZ, float &maxX, float &maxY, float &maxZ )
{
  if ( mType != TriangularFaces ) return;
  for ( const unsigned int vertice : mIndexes )
  {
    const int heightIndex = ( vertice - 1 ) * 3 + 1;
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
    const int u_index = i / 3 * 2;
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
    QgsDebugMsg( "Vertex normals count and vertex positions count are different" );
  }
  int verticesCount = mVertexPosition.size() / 3;

  auto getVertexIndex = [&]( int i ) -> QString
  {
    const int negativeIndex = -1 - ( verticesCount - i );
    if ( hasNormals && !hasTextures )
      return QStringLiteral( "%1//%2" ).arg( negativeIndex ).arg( negativeIndex );
    if ( !hasNormals && hasTextures )
      return QStringLiteral( "%1/%2" ).arg( negativeIndex ).arg( negativeIndex );
    if ( hasNormals && hasTextures )
      return QStringLiteral( "%1/%2/%3" ).arg( negativeIndex ).arg( negativeIndex ).arg( negativeIndex );
    return QString::number( negativeIndex );
  };

  if ( mType == TriangularFaces )
  {
    // Construct triangular faces
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
  else if ( mType == LineStrip )
  {
    out << "l";
    for ( const int i : mIndexes ) out << " " << getVertexIndex( i );
    out << "\n";
  }
  else if ( mType == Points )
  {
    out << "p";
    for ( int i = 0; i < mVertexPosition.size(); i += 3 )
      out << " " << getVertexIndex( i / 3 + 1 );
    out << "\n";
  }
}

QString Qgs3DExportObject::saveMaterial( QTextStream &mtlOut, const QString &folderPath )
{
  QString materialName = mName + "_material";
  if ( mMaterialParameters.size() == 0 && ( mTexturesUV.size() == 0 || mTextureImage.isNull() ) ) return QString();
  mtlOut << "newmtl " << materialName << "\n";
  if ( mTexturesUV.size() != 0 && !mTextureImage.isNull() )
  {
    const QString filePath = QDir( folderPath ).filePath( materialName + ".jpg" );
    mTextureImage.save( filePath, "JPG" );
    mtlOut << "\tmap_Kd " << materialName << ".jpg" << "\n";
  }
  for ( auto it = mMaterialParameters.constBegin(); it != mMaterialParameters.constEnd(); it++ )
  {
    mtlOut << "\t" << it.key() << " " << it.value() << "\n";
  }
  mtlOut << "\tillum 2\n";
  return materialName;
}
