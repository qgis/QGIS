/***************************************************************************
                         qgsmeshlayer.cpp
                         ----------------
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

#include <cstddef>

#include <QUuid>

#include "qgsfillsymbollayer.h"
#include "qgslogger.h"
#include "qgsmeshdataprovider.h"
#include "qgsmeshlayer.h"
#include "qgsmeshlayerrenderer.h"
#include "qgsproviderregistry.h"
#include "qgsreadwritecontext.h"
#include "qgstriangularmesh.h"

QgsMeshLayer::QgsMeshLayer( const QString &meshLayerPath,
                            const QString &baseName,
                            const QString &providerKey )
  : QgsMapLayer( MeshLayer, baseName, meshLayerPath )
  , mProviderKey( providerKey )
{
  // load data
  setDataProvider( providerKey );

  QgsSymbolLayerList l1;
  l1 << new QgsSimpleFillSymbolLayer( Qt::white, Qt::NoBrush, Qt::black, Qt::SolidLine, 1.0 );
  mNativeMeshSymbol.reset( new QgsFillSymbol( l1 ) );


  toggleTriangularMeshRendering( false );

} // QgsMeshLayer ctor



QgsMeshLayer::~QgsMeshLayer()
{
  if ( mDataProvider )
    delete mDataProvider;
}

QgsMeshDataProvider *QgsMeshLayer::dataProvider()
{
  return mDataProvider;
}

const QgsMeshDataProvider *QgsMeshLayer::dataProvider() const
{
  return mDataProvider;
}

QgsMeshLayer *QgsMeshLayer::clone() const
{
  QgsMeshLayer *layer = new QgsMeshLayer( source(), name(), mProviderKey );
  QgsMapLayer::clone( layer );
  return layer;
}

QgsRectangle QgsMeshLayer::extent() const
{
  if ( mDataProvider )
    return mDataProvider->extent();
  else
  {
    QgsRectangle rec;
    rec.setMinimal();
    return rec;
  }
}

QString QgsMeshLayer::providerType() const
{
  return mProviderKey;
}

QgsMesh *QgsMeshLayer::nativeMesh() SIP_SKIP
{
  return mNativeMesh.get();
}


QgsTriangularMesh *QgsMeshLayer::triangularMesh() SIP_SKIP
{
  return mTriangularMesh.get();
}

QgsSymbol *QgsMeshLayer::nativeMeshSymbol()
{
  return mNativeMeshSymbol.get();
}

QgsSymbol *QgsMeshLayer::triangularMeshSymbol()
{
  return mTriangularMeshSymbol.get();
}

void QgsMeshLayer::toggleTriangularMeshRendering( bool toggle )
{
  if ( toggle && mTriangularMeshSymbol )
    return;

  if ( toggle )
  {
    QgsSymbolLayerList l2;
    l2 << new QgsSimpleFillSymbolLayer( Qt::white, Qt::NoBrush, Qt::red, Qt::SolidLine, 0.26 );
    mTriangularMeshSymbol.reset( new QgsFillSymbol( l2 ) );
  }
  else
  {
    mTriangularMeshSymbol.reset();
  }
  triggerRepaint();
}

void QgsMeshLayer::setActiveScalarDataset( int index )
{
  Q_ASSERT( dataProvider()->datasetCount() > index );
  if ( dataProvider()->datasetHasScalarData( index ) )
    mActiveScalarDataset = index;
}

void QgsMeshLayer::setActiveVectorDataset( int index )
{
  Q_ASSERT( dataProvider()->datasetCount() > index );
  if ( !dataProvider()->datasetHasScalarData( index ) )
    mActiveVectorDataset = index;
}

void QgsMeshLayer::fillNativeMesh()
{
  Q_ASSERT( !mNativeMesh );

  mNativeMesh.reset( new QgsMesh() );

  if ( !( dataProvider() && dataProvider()->isValid() ) )
    return;

  mNativeMesh->vertices.resize( dataProvider()->vertexCount() );
  for ( int i = 0; i < dataProvider()->vertexCount(); ++i )
  {
    mNativeMesh->vertices[i] = dataProvider()->vertex( i );
  }

  mNativeMesh->faces.resize( dataProvider()->faceCount() );
  for ( int i = 0; i < dataProvider()->faceCount(); ++i )
  {
    mNativeMesh->faces[i] = dataProvider()->face( i );
  }
}

QgsMapLayerRenderer *QgsMeshLayer::createMapRenderer( QgsRenderContext &rendererContext )
{
  if ( !mNativeMesh )
  {
    // lazy loading of mesh data
    fillNativeMesh();
  }

  if ( !mTriangularMesh )
    mTriangularMesh.reset( new QgsTriangularMesh() );

  mTriangularMesh->update( mNativeMesh.get(), &rendererContext );
  return new QgsMeshLayerRenderer( this, rendererContext );
}

bool QgsMeshLayer::readSymbology( const QDomNode &node, QString &errorMessage, QgsReadWriteContext &context )
{
  Q_UNUSED( node );
  Q_UNUSED( errorMessage );
  Q_UNUSED( context );
  return true;
}

bool QgsMeshLayer::writeSymbology( QDomNode &node, QDomDocument &doc, QString &errorMessage, const QgsReadWriteContext &context ) const
{
  Q_UNUSED( node );
  Q_UNUSED( doc );
  Q_UNUSED( errorMessage );
  Q_UNUSED( context );
  return true;
}

QString QgsMeshLayer::decodedSource( const QString &source, const QString &provider, const QgsReadWriteContext &context ) const
{
  QString src( source );
  if ( provider == QLatin1String( "mdal" ) )
  {
    src = context.pathResolver().readPath( src );
  }
  return src;
}

QString QgsMeshLayer::encodedSource( const QString &source, const QgsReadWriteContext &context ) const
{
  QString src( source );
  if ( providerType() == QLatin1String( "mdal" ) )
  {
    src = context.pathResolver().writePath( src );
  }
  return src;
}

bool QgsMeshLayer::readXml( const QDomNode &layer_node, QgsReadWriteContext &context )
{
  Q_UNUSED( context );

  QgsDebugMsgLevel( QStringLiteral( "Datasource in QgsMeshLayer::readXml: %1" ).arg( mDataSource.toLocal8Bit().data() ), 3 );

  //process provider key
  QDomNode pkeyNode = layer_node.namedItem( QStringLiteral( "provider" ) );

  if ( pkeyNode.isNull() )
  {
    mProviderKey.clear();
  }
  else
  {
    QDomElement pkeyElt = pkeyNode.toElement();
    mProviderKey = pkeyElt.text();
  }

  if ( !setDataProvider( mProviderKey ) )
  {
    return false;
  }

  return mValid; // should be true if read successfully
}

bool QgsMeshLayer::writeXml( QDomNode &layer_node, QDomDocument &document, const QgsReadWriteContext &context ) const
{
  // first get the layer element so that we can append the type attribute
  QDomElement mapLayerNode = layer_node.toElement();

  if ( mapLayerNode.isNull() || ( QLatin1String( "maplayer" ) != mapLayerNode.nodeName() ) )
  {
    QgsDebugMsgLevel( QStringLiteral( "can't find <maplayer>" ), 2 );
    return false;
  }

  mapLayerNode.setAttribute( QStringLiteral( "type" ), QStringLiteral( "mesh" ) );

  // add provider node
  if ( mDataProvider )
  {
    QDomElement provider  = document.createElement( QStringLiteral( "provider" ) );
    QDomText providerText = document.createTextNode( providerType() );
    provider.appendChild( providerText );
    layer_node.appendChild( provider );
  }

  // renderer specific settings
  QString errorMsg;
  return writeSymbology( layer_node, document, errorMsg, context );
}

bool QgsMeshLayer::setDataProvider( QString const &provider )
{
  if ( mDataProvider )
    delete mDataProvider;

  mProviderKey = provider;
  QString dataSource = mDataSource;

  mDataProvider = qobject_cast<QgsMeshDataProvider *>( QgsProviderRegistry::instance()->createProvider( provider, dataSource ) );
  if ( !mDataProvider )
  {
    QgsDebugMsgLevel( QStringLiteral( "Unable to get mesh data provider" ), 2 );
    return false;
  }

  mDataProvider->setParent( this );
  QgsDebugMsgLevel( QStringLiteral( "Instantiated the mesh data provider plugin" ), 2 );

  mValid = mDataProvider->isValid();
  if ( !mValid )
  {
    QgsDebugMsgLevel( QStringLiteral( "Invalid mesh provider plugin %1" ).arg( QString( mDataSource.toUtf8() ) ), 2 );
    return false;
  }

  if ( provider == QStringLiteral( "mesh_memory" ) )
  {
    // required so that source differs between memory layers
    mDataSource = mDataSource + QStringLiteral( "&uid=%1" ).arg( QUuid::createUuid().toString() );
  }

  return true;
} // QgsMeshLayer:: setDataProvider
