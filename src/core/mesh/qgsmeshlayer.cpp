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
#include <limits>

#include <QUuid>

#include "qgscolorramp.h"
#include "qgslogger.h"
#include "qgsmaplayerlegend.h"
#include "qgsmeshdataprovider.h"
#include "qgsmeshlayer.h"
#include "qgsmeshlayerrenderer.h"
#include "qgsmeshlayerutils.h"
#include "qgsproviderregistry.h"
#include "qgsreadwritecontext.h"
#include "qgstriangularmesh.h"
#include "qgsmeshlayerinterpolator.h"


QgsMeshLayer::QgsMeshLayer( const QString &meshLayerPath,
                            const QString &baseName,
                            const QString &providerKey,
                            const LayerOptions & )
  : QgsMapLayer( MeshLayer, baseName, meshLayerPath )
  , mProviderKey( providerKey )
{
  // load data
  QgsDataProvider::ProviderOptions providerOptions;
  setDataProvider( providerKey, providerOptions );

  setLegend( QgsMapLayerLegend::defaultMeshLegend( this ) );

  // show at least the mesh by default so we render something
  QgsMeshRendererMeshSettings meshSettings;
  meshSettings.setEnabled( true );
  mRendererSettings.setNativeMeshSettings( meshSettings );

} // QgsMeshLayer ctor



QgsMeshLayer::~QgsMeshLayer()
{
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

QgsMeshRendererSettings QgsMeshLayer::rendererSettings() const
{
  return mRendererSettings;
}

void QgsMeshLayer::setRendererSettings( const QgsMeshRendererSettings &settings )
{
  mRendererSettings = settings;
  emit rendererChanged();
  triggerRepaint();
}

QgsMeshDatasetValue QgsMeshLayer::datasetValue( const QgsMeshDatasetIndex &index, const QgsPointXY &point ) const
{
  QgsMeshDatasetValue value;

  if ( mTriangularMesh && dataProvider() && dataProvider()->isValid() && index.isValid() )
  {
    int faceIndex = mTriangularMesh->faceIndexForPoint( point ) ;
    if ( faceIndex >= 0 )
    {
      if ( dataProvider()->datasetGroupMetadata( index ).dataType() == QgsMeshDatasetGroupMetadata::DataOnFaces )
      {
        int nativeFaceIndex = mTriangularMesh->trianglesToNativeFaces().at( faceIndex );
        return dataProvider()->datasetValue( index, nativeFaceIndex );
      }
      else
      {
        const QgsMeshFace &face = mTriangularMesh->triangles()[faceIndex];
        const int v1 = face[0], v2 = face[1], v3 = face[2];
        const QgsPoint p1 = mTriangularMesh->vertices()[v1], p2 = mTriangularMesh->vertices()[v2], p3 = mTriangularMesh->vertices()[v3];
        const QgsMeshDatasetValue val1 = dataProvider()->datasetValue( index, v1 );
        const QgsMeshDatasetValue val2 = dataProvider()->datasetValue( index, v2 );
        const QgsMeshDatasetValue val3 = dataProvider()->datasetValue( index, v3 );
        const double x = QgsMeshLayerInterpolator::interpolateFromVerticesData( p1, p2, p3, val1.x(), val2.x(), val3.x(), point );
        double y = std::numeric_limits<double>::quiet_NaN();
        bool isVector = dataProvider()->datasetGroupMetadata( index ).isVector();
        if ( isVector )
          y = QgsMeshLayerInterpolator::interpolateFromVerticesData( p1, p2, p3, val1.y(), val2.y(), val3.y(), point );

        return QgsMeshDatasetValue( x, y );
      }
    }
  }
  return value;
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

void QgsMeshLayer::onDatasetGroupsAdded( int count )
{
  // assign default style to new dataset groups
  int newDatasetGroupCount = mDataProvider->datasetGroupCount();
  for ( int i = newDatasetGroupCount - count; i < newDatasetGroupCount; ++i )
    assignDefaultStyleToDatasetGroup( i );
}

static QgsGradientColorRamp *_createDefaultColorRamp()
{
  QgsGradientStopsList stops;
  stops << QgsGradientStop( 0.25, QColor( 0, 255, 255 ) );
  stops << QgsGradientStop( 0.5, QColor( 0, 255, 0 ) );
  stops << QgsGradientStop( 0.75, QColor( 255, 255, 0 ) );
  return new QgsGradientColorRamp( QColor( 0, 0, 255 ), QColor( 255, 0, 0 ), false, stops );
}

void QgsMeshLayer::assignDefaultStyleToDatasetGroup( int groupIndex )
{
  double groupMin, groupMax;
  QgsMeshLayerUtils::calculateMinMaxForDatasetGroup( groupMin, groupMax, mDataProvider, groupIndex );

  QgsColorRampShader fcn( groupMin, groupMax, _createDefaultColorRamp() );
  fcn.classifyColorRamp( 5, -1, QgsRectangle(), nullptr );

  QgsMeshRendererScalarSettings scalarSettings;
  scalarSettings.setColorRampShader( fcn );
  mRendererSettings.setScalarSettings( groupIndex, scalarSettings );
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
  Q_UNUSED( errorMessage );
  Q_UNUSED( context );

  QDomElement elem = node.toElement();
  QDomElement elemRendererSettings = elem.firstChildElement( "mesh-renderer-settings" );
  if ( !elemRendererSettings.isNull() )
    mRendererSettings.readXml( elemRendererSettings );

  return true;
}

bool QgsMeshLayer::writeSymbology( QDomNode &node, QDomDocument &doc, QString &errorMessage, const QgsReadWriteContext &context ) const
{
  Q_UNUSED( errorMessage );
  Q_UNUSED( context );

  QDomElement elem = node.toElement();
  QDomElement elemRendererSettings = mRendererSettings.writeXml( doc );
  elem.appendChild( elemRendererSettings );

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

  QgsDataProvider::ProviderOptions providerOptions;
  if ( !setDataProvider( mProviderKey, providerOptions ) )
  {
    return false;
  }

  QDomElement elemExtraDatasets = layer_node.firstChildElement( QStringLiteral( "extra-datasets" ) );
  if ( !elemExtraDatasets.isNull() )
  {
    QDomElement elemUri = elemExtraDatasets.firstChildElement( QStringLiteral( "uri" ) );
    while ( !elemUri.isNull() )
    {
      QString uri = context.pathResolver().readPath( elemUri.text() );

      bool res = mDataProvider->addDataset( uri );
      QgsDebugMsg( QStringLiteral( "extra dataset (res %1): %2" ).arg( res ).arg( uri ) );

      elemUri = elemUri.nextSiblingElement( QStringLiteral( "uri" ) );
    }
  }

  QString errorMsg;
  readSymbology( layer_node, errorMsg, context );

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

    const QStringList extraDatasetUris = mDataProvider->extraDatasets();
    QDomElement elemExtraDatasets = document.createElement( QStringLiteral( "extra-datasets" ) );
    for ( const QString &uri : extraDatasetUris )
    {
      QString path = context.pathResolver().writePath( uri );
      QDomElement elemUri = document.createElement( QStringLiteral( "uri" ) );
      elemUri.appendChild( document.createTextNode( path ) );
      elemExtraDatasets.appendChild( elemUri );
    }
    layer_node.appendChild( elemExtraDatasets );
  }

  // renderer specific settings
  QString errorMsg;
  return writeSymbology( layer_node, document, errorMsg, context );
}

bool QgsMeshLayer::setDataProvider( QString const &provider, const QgsDataProvider::ProviderOptions &options )
{
  delete mDataProvider;

  mProviderKey = provider;
  QString dataSource = mDataSource;

  mDataProvider = qobject_cast<QgsMeshDataProvider *>( QgsProviderRegistry::instance()->createProvider( provider, dataSource, options ) );
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

  for ( int i = 0; i < mDataProvider->datasetGroupCount(); ++i )
    assignDefaultStyleToDatasetGroup( i );

  connect( mDataProvider, &QgsMeshDataProvider::dataChanged, this, &QgsMeshLayer::dataChanged );
  connect( mDataProvider, &QgsMeshDataProvider::datasetGroupsAdded, this, &QgsMeshLayer::onDatasetGroupsAdded );

  return true;
}
