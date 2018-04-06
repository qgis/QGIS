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

#include "qgsmdalprovider.h"
#include <QFile>
#include <QJsonDocument>
#include <limits>
#include "mdal.h"

static const QString TEXT_PROVIDER_KEY = QStringLiteral( "mdal" );
static const QString TEXT_PROVIDER_DESCRIPTION = QStringLiteral( "MDAL provider" );

bool QgsMdalProvider::isValid() const
{
  return mMeshH != nullptr;
}

QString QgsMdalProvider::name() const
{
  return TEXT_PROVIDER_KEY;
}

QString QgsMdalProvider::description() const
{
  return TEXT_PROVIDER_DESCRIPTION;
}

QgsCoordinateReferenceSystem QgsMdalProvider::crs() const
{
  return QgsCoordinateReferenceSystem();
}

QgsMdalProvider::QgsMdalProvider( const QString &uri )
  : QgsMeshDataProvider( uri )
{
  QByteArray curi = uri.toAscii();
  mMeshH = MDAL_LoadMesh( curi.constData() );
}

QgsMdalProvider::~QgsMdalProvider()
{
  if ( mMeshH )
    MDAL_CloseMesh( mMeshH );
}

size_t QgsMdalProvider::vertexCount() const
{
  if ( mMeshH )
    return MDAL_M_vertexCount( mMeshH );
  else
    return ( size_t ) 0;
}

size_t QgsMdalProvider::faceCount() const
{
  if ( mMeshH )
    return MDAL_M_faceCount( mMeshH );
  else
    return ( size_t ) 0;
}

QgsMeshVertex QgsMdalProvider::vertex( size_t index ) const
{
  Q_ASSERT( index < vertexCount() );
  double x = MDAL_M_vertexXCoordinatesAt( mMeshH, index );
  double y = MDAL_M_vertexYCoordinatesAt( mMeshH, index );
  QgsMeshVertex vertex( x, y );
  return vertex;
}

QgsMeshFace QgsMdalProvider::face( size_t index ) const
{
  Q_ASSERT( index < faceCount() );
  QgsMeshFace face;
  int n_face_vertices = MDAL_M_faceVerticesCountAt( mMeshH, index );
  for ( size_t j = 0; j < n_face_vertices; ++j )
  {
    int vertex_index = MDAL_M_faceVerticesIndexAt( mMeshH, index, j );
    face.push_back( vertex_index );
  }
  return face;
}

/*----------------------------------------------------------------------------------------------*/

/**
 * Class factory to return a pointer to a newly created
 * QgsGdalProvider object
 */
QGISEXTERN QgsMdalProvider *classFactory( const QString *uri )
{
  return new QgsMdalProvider( *uri );
}

/**
 * Required key function (used to map the plugin to a data store type)
*/
QGISEXTERN QString providerKey()
{
  return TEXT_PROVIDER_KEY;
}

/**
 * Required description function
 */
QGISEXTERN QString description()
{
  return TEXT_PROVIDER_DESCRIPTION;
}

/**
 * Required isProvider function. Used to determine if this shared library
 * is a data provider plugin
 */
QGISEXTERN bool isProvider()
{
  return true;
}

QGISEXTERN void cleanupProvider()
{
}

