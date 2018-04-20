/***************************************************************************
                         qgsmeshmemorydataprovider.cpp
                         -----------------------------
    begin                : April 2018
    copyright            : (C) 2018 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsmeshmemorydataprovider.h"

static const QString TEXT_PROVIDER_KEY = QStringLiteral( "mesh_memory" );
static const QString TEXT_PROVIDER_DESCRIPTION = QStringLiteral( "Mesh memory provider" );

bool QgsMeshMemoryDataProvider::isValid() const
{
  return true;
}

QString QgsMeshMemoryDataProvider::name() const
{
  return TEXT_PROVIDER_KEY;
}

QString QgsMeshMemoryDataProvider::description() const
{
  return TEXT_PROVIDER_DESCRIPTION;
}

QgsCoordinateReferenceSystem QgsMeshMemoryDataProvider::crs() const
{
  return QgsCoordinateReferenceSystem();
}

QgsMeshMemoryDataProvider::QgsMeshMemoryDataProvider( const QString &uri )
  : QgsMeshDataProvider( uri )
{
  mIsValid = splitSections( uri );
}

QgsMeshMemoryDataProvider::~QgsMeshMemoryDataProvider()
{
}

QString QgsMeshMemoryDataProvider::providerKey()
{
  return TEXT_PROVIDER_KEY;
}

QString QgsMeshMemoryDataProvider::providerDescription()
{
  return TEXT_PROVIDER_DESCRIPTION;
}

QgsMeshMemoryDataProvider *QgsMeshMemoryDataProvider::createProvider( const QString &uri )
{
  return new QgsMeshMemoryDataProvider( uri );
}

bool QgsMeshMemoryDataProvider::splitSections( const QString &uri )
{
  const QStringList sections = uri.split( QStringLiteral( "---" ), QString::SkipEmptyParts );
  if ( sections.size() != 2 )
  {
    setError( QgsError( QStringLiteral( "Invalid mesh definition, does not contain 2 sections" ),
                        QStringLiteral( "Mesh Memory Provider" ) ) );
    return false;
  }

  if ( addVertices( sections[0] ) )
    return addFaces( sections[1] );
  else
    return false;
}

bool QgsMeshMemoryDataProvider::addVertices( const QString &def )
{
  QVector<QgsMeshVertex> vertices;

  const QStringList verticesCoords = def.split( '\n', QString::SkipEmptyParts );
  for ( int i = 0; i < verticesCoords.size(); ++i )
  {
    const QStringList coords = verticesCoords[i].split( ',', QString::SkipEmptyParts );
    if ( coords.size() != 2 )
    {
      setError( QgsError( QStringLiteral( "Invalid mesh definition, vertex definition does not contain x, y" ),
                          QStringLiteral( "Mesh Memory Provider" ) ) );
      return false;
    }
    double x = coords.at( 0 ).toDouble();
    double y = coords.at( 1 ).toDouble();
    QgsMeshVertex vertex( x, y );
    vertices.push_back( vertex );
  }

  mVertices = vertices;
  return true;
}

bool QgsMeshMemoryDataProvider::addFaces( const QString &def )
{
  QVector<QgsMeshFace> faces;

  const QStringList facesVertices = def.split( '\n', QString::SkipEmptyParts );
  for ( int i = 0; i < facesVertices.size(); ++i )
  {
    const QStringList vertices = facesVertices[i].split( ',', QString::SkipEmptyParts );
    if ( vertices.size() < 3 )
    {
      setError( QgsError( QStringLiteral( "Invalid mesh definition, face must contain at least 3 vertices" ),
                          QStringLiteral( "Mesh Memory Provider" ) ) );
      return false;
    }
    QgsMeshFace face;
    for ( int j = 0; j < vertices.size(); ++j )
    {
      int vertex_id = vertices[j].toInt();
      if ( vertex_id < 0 )
      {
        setError( QgsError( QStringLiteral( "Invalid mesh definition, vertex index must be positive value" ),  QStringLiteral( "Mesh Memory Provider" ) ) );
        return false;
      }
      if ( mVertices.size() < vertex_id )
      {
        setError( QgsError( QStringLiteral( "Invalid mesh definition, missing vertex id defined in face" ),  QStringLiteral( "Mesh Memory Provider" ) ) );
        return false;
      }

      face.push_back( vertex_id );
    }
    faces.push_back( face );
  }

  mFaces = faces;
  return true;
}

int QgsMeshMemoryDataProvider::vertexCount() const
{
  return mVertices.size();
}

int QgsMeshMemoryDataProvider::faceCount() const
{
  return mFaces.size();
}

QgsMeshVertex QgsMeshMemoryDataProvider::vertex( int index ) const
{
  Q_ASSERT( vertexCount() > index );
  return mVertices[index];
}

QgsMeshFace QgsMeshMemoryDataProvider::face( int index ) const
{
  Q_ASSERT( faceCount() > index );
  return mFaces[index];
}


