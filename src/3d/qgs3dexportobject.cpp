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

#include "qgsabstractmaterialsettings.h"
#include "qgslogger.h"

#include <QDir>
#include <QImage>
#include <QMatrix4x4>
#include <QVector3D>
#include <Qt3DCore/QAttribute>
#include <Qt3DCore/QBuffer>

void Qgs3DExportObject::setupPositionCoordinates( const QVector<float> &positionsBuffer, const QMatrix4x4 &transform )
{
  mVertexPosition.clear();
  for ( int i = 0; i < positionsBuffer.size(); i += 3 )
  {
    const QVector3D position( positionsBuffer[i], positionsBuffer[i + 1], positionsBuffer[i + 2] );
    const QVector3D positionFinal = transform.map( position );
    mVertexPosition << positionFinal.x() << positionFinal.y() << positionFinal.z();
  }
}

void Qgs3DExportObject::setupTriangle( const QVector<float> &positionsBuffer, const QVector<uint> &facesIndexes, const QMatrix4x4 &transform )
{
  mType = Qgs3DExportObject::TriangularFaces;
  setupPositionCoordinates( positionsBuffer, transform );

  // setup faces
  mIndexes.clear();
  for ( int i = 0; i < facesIndexes.size(); i += 3 )
  {
    if ( i + 2 >= facesIndexes.size() )
      continue;
    // skip invalid triangles
    if ( facesIndexes[i] == facesIndexes[i + 1] || facesIndexes[i + 1] == facesIndexes[i + 2] || facesIndexes[i] == facesIndexes[i + 2] )
      continue;
    for ( int j = 0; j < 3; ++j )
      mIndexes << facesIndexes[i + j];
  }
}

void Qgs3DExportObject::setupLine( const QVector<float> &positionsBuffer )
{
  mType = Qgs3DExportObject::LineStrip;
  setupPositionCoordinates( positionsBuffer );

  // setup indexes
  mIndexes.clear();
  for ( int i = 0; i < mVertexPosition.size(); i += 3 )
    mIndexes << i / 3 + 1;
}

void Qgs3DExportObject::setupPoint( const QVector<float> &positionsBuffer )
{
  mType = Qgs3DExportObject::Points;
  setupPositionCoordinates( positionsBuffer );
}

void Qgs3DExportObject::setupNormalCoordinates( const QVector<float> &normalsBuffer, const QMatrix4x4 &transform )
{
  mNormals.clear();

  // Qt does not provide QMatrix3x3 * QVector3D multiplication so we use QMatrix4x4
  QMatrix3x3 normal3x3 = transform.normalMatrix();
  QMatrix4x4 normal4x4( normal3x3( 0, 0 ), normal3x3( 0, 1 ), normal3x3( 0, 2 ), 0, normal3x3( 1, 0 ), normal3x3( 1, 1 ), normal3x3( 1, 2 ), 0, normal3x3( 2, 0 ), normal3x3( 2, 1 ), normal3x3( 2, 2 ), 0, 0, 0, 0, 1 );

  for ( int i = 0; i < normalsBuffer.size(); i += 3 )
  {
    const QVector3D normalVector( normalsBuffer[i], normalsBuffer[i + 1], normalsBuffer[i + 2] );
    QVector3D v = normal4x4.mapVector( normalVector );
    // round numbers very close to zero to avoid tiny numbers like 6e-8 in export
    if ( qgsFloatNear( v.x(), 0 ) )
      v.setX( 0 );
    if ( qgsFloatNear( v.y(), 0 ) )
      v.setY( 0 );
    if ( qgsFloatNear( v.z(), 0 ) )
      v.setZ( 0 );
    mNormals << v.x() << v.y() << v.z();
  }
}

void Qgs3DExportObject::setupTextureCoordinates( const QVector<float> &texturesBuffer )
{
  mTexturesUV.clear();
  mTexturesUV << texturesBuffer;
}

void Qgs3DExportObject::setupMaterial( QgsAbstractMaterialSettings *material )
{
  mMaterialParameters.clear();
  QMap<QString, QString> parameters = material->toExportParameters();
  for ( auto it = parameters.begin(); it != parameters.end(); ++it )
  {
    mMaterialParameters[it.key()] = it.value();
  }
}

void Qgs3DExportObject::objectBounds( float &minX, float &minY, float &minZ, float &maxX, float &maxY, float &maxZ ) const
{
  if ( mType != TriangularFaces )
    return;
  for ( const unsigned int vertice : qAsConst( mIndexes ) )
  {
    const int heightIndex = static_cast<int>( vertice ) * 3 + 1;
    minX = std::min( minX, mVertexPosition[heightIndex - 1] );
    maxX = std::max( maxX, mVertexPosition[heightIndex - 1] );
    minY = std::min( minY, mVertexPosition[heightIndex] );
    maxY = std::max( maxY, mVertexPosition[heightIndex] );
    minZ = std::min( minZ, mVertexPosition[heightIndex + 1] );
    maxZ = std::max( maxZ, mVertexPosition[heightIndex + 1] );
  }
}

void Qgs3DExportObject::saveTo( QTextStream &out, float scale, const QVector3D &center, int precision ) const
{
  // Set groups
  // turns out grouping doest work as expected in blender
  out << qSetRealNumberPrecision( precision );

  // smoothen edges
  if ( mSmoothEdges )
    out << "s on\n";
  else
    out << "s off\n";

  // Construct vertices
  // As we can have holes in the face list and we only write vertices from these faces
  // then the vertex list in the obj is not the whole from mVertexPosition!
  for ( const unsigned int vertice : qAsConst( mIndexes ) )
  {
    const int i = static_cast<int>( vertice * 3 );
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
    QgsDebugError( "Vertex normals count and vertex positions count are different" );
  }
  const int verticesCount = mIndexes.size();

  // we use negative indexes as this is the way to use relative values to reference vertex positions
  // Positive values are absolute vertex position from the beginning of the file.
  auto getVertexIndex = [&]( unsigned int i ) -> QString {
    const int negativeIndex = static_cast<int>( i - verticesCount );
    if ( hasNormals && !hasTextures )
      return u"%1//%2"_s.arg( negativeIndex ).arg( negativeIndex );
    if ( !hasNormals && hasTextures )
      return u"%1/%2"_s.arg( negativeIndex ).arg( negativeIndex );
    if ( hasNormals && hasTextures )
      return u"%1/%2/%3"_s.arg( negativeIndex ).arg( negativeIndex ).arg( negativeIndex );
    return QString::number( negativeIndex );
  };

  if ( mType == TriangularFaces )
  {
    // Construct triangular faces
    // As we have "compressed" the vertex/normal section above by using only the vertices referenced by the faces
    // we do not need to the 'mIndexes[i]' value but only the 'i' value.
    for ( int i = 0; i < mIndexes.size(); i += 3 )
    {
      out << "f " << getVertexIndex( i );
      out << " " << getVertexIndex( i + 1 );
      out << " " << getVertexIndex( i + 2 );
      out << "\n";
    }
  }
  else if ( mType == LineStrip )
  {
    out << "l";
    for ( const unsigned int i : qAsConst( mIndexes ) )
      out << " " << getVertexIndex( i );
    out << "\n";
  }
  else if ( mType == Points )
  {
    out << "p";
    for ( const unsigned int i : qAsConst( mIndexes ) )
      out << " " << getVertexIndex( i );
    out << "\n";
  }
}

QString Qgs3DExportObject::saveMaterial( QTextStream &mtlOut, const QString &folderPath ) const
{
  QString materialName = mName + "_material";
  if ( mMaterialParameters.size() == 0 && ( mTexturesUV.size() == 0 || mTextureImage.isNull() ) )
    return QString();
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
