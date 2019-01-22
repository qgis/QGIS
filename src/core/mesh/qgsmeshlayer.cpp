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
#include "qgspainting.h"
#include "qgsproviderregistry.h"
#include "qgsreadwritecontext.h"
#include "qgsstyle.h"
#include "qgstriangularmesh.h"


QgsMeshLayer::QgsMeshLayer( const QString &meshLayerPath,
                            const QString &baseName,
                            const QString &providerKey,
                            const LayerOptions & )
  : QgsMapLayer( MeshLayer, baseName, meshLayerPath )
{
  setProviderType( providerKey );
  // if we’re given a provider type, try to create and bind one to this layer
  if ( !meshLayerPath.isEmpty() && !providerKey.isEmpty() )
  {
    QgsDataProvider::ProviderOptions providerOptions;
    setDataProvider( providerKey, providerOptions );
  }

  setLegend( QgsMapLayerLegend::defaultMeshLegend( this ) );
  setDefaultRendererSettings();
} // QgsMeshLayer ctor

void QgsMeshLayer::setDefaultRendererSettings()
{
  if ( mDataProvider && mDataProvider->datasetGroupCount() > 0 )
  {
    // show data from the first dataset group
    mRendererSettings.setActiveScalarDataset( QgsMeshDatasetIndex( 0, 0 ) );
  }
  else
  {
    // show at least the mesh by default
    QgsMeshRendererMeshSettings meshSettings;
    meshSettings.setEnabled( true );
    mRendererSettings.setNativeMeshSettings( meshSettings );
  }
}

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

QgsMesh *QgsMeshLayer::nativeMesh()
{
  return mNativeMesh.get();
}

const QgsMesh *QgsMeshLayer::nativeMesh() const
{
  return mNativeMesh.get();
}

QgsTriangularMesh *QgsMeshLayer::triangularMesh()
{
  return mTriangularMesh.get();
}

const QgsTriangularMesh *QgsMeshLayer::triangularMesh() const
{
  return mTriangularMesh.get();
}

QgsMeshLayerRendererCache *QgsMeshLayer::rendererCache()
{
  return mRendererCache.get();
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
      int nativeFaceIndex = mTriangularMesh->trianglesToNativeFaces().at( faceIndex );
      if ( dataProvider()->isFaceActive( index, nativeFaceIndex ) )
      {

        if ( dataProvider()->datasetGroupMetadata( index ).dataType() == QgsMeshDatasetGroupMetadata::DataOnFaces )
        {
          int nativeFaceIndex = mTriangularMesh->trianglesToNativeFaces().at( faceIndex );
          value = dataProvider()->datasetValue( index, nativeFaceIndex );
        }
        else
        {
          const QgsMeshFace &face = mTriangularMesh->triangles()[faceIndex];
          const int v1 = face[0], v2 = face[1], v3 = face[2];
          const QgsPoint p1 = mTriangularMesh->vertices()[v1], p2 = mTriangularMesh->vertices()[v2], p3 = mTriangularMesh->vertices()[v3];
          const QgsMeshDatasetValue val1 = dataProvider()->datasetValue( index, v1 );
          const QgsMeshDatasetValue val2 = dataProvider()->datasetValue( index, v2 );
          const QgsMeshDatasetValue val3 = dataProvider()->datasetValue( index, v3 );
          const double x = QgsMeshLayerUtils::interpolateFromVerticesData( p1, p2, p3, val1.x(), val2.x(), val3.x(), point );
          double y = std::numeric_limits<double>::quiet_NaN();
          bool isVector = dataProvider()->datasetGroupMetadata( index ).isVector();
          if ( isVector )
            y = QgsMeshLayerUtils::interpolateFromVerticesData( p1, p2, p3, val1.y(), val2.y(), val3.y(), point );

          value = QgsMeshDatasetValue( x, y );
        }

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

  dataProvider()->populateMesh( mNativeMesh.get() );
}

void QgsMeshLayer::onDatasetGroupsAdded( int count )
{
  // assign default style to new dataset groups
  int newDatasetGroupCount = mDataProvider->datasetGroupCount();
  for ( int i = newDatasetGroupCount - count; i < newDatasetGroupCount; ++i )
    assignDefaultStyleToDatasetGroup( i );
}

static QgsColorRamp *_createDefaultColorRamp()
{
  QgsColorRamp *ramp = QgsStyle::defaultStyle()->colorRamp( QStringLiteral( "Plasma" ) );
  if ( ramp )
    return ramp;

  // definition of "Plasma" color ramp (in case it is not available in the style for some reason)
  QgsStringMap props;
  props["color1"] = "13,8,135,255";
  props["color2"] = "240,249,33,255";
  props["stops"] =
    "0.0196078;27,6,141,255:0.0392157;38,5,145,255:0.0588235;47,5,150,255:0.0784314;56,4,154,255:0.0980392;65,4,157,255:"
    "0.117647;73,3,160,255:0.137255;81,2,163,255:0.156863;89,1,165,255:0.176471;97,0,167,255:0.196078;105,0,168,255:"
    "0.215686;113,0,168,255:0.235294;120,1,168,255:0.254902;128,4,168,255:0.27451;135,7,166,255:0.294118;142,12,164,255:"
    "0.313725;149,17,161,255:0.333333;156,23,158,255:0.352941;162,29,154,255:0.372549;168,34,150,255:0.392157;174,40,146,255:"
    "0.411765;180,46,141,255:0.431373;186,51,136,255:0.45098;191,57,132,255:0.470588;196,62,127,255:0.490196;201,68,122,255:"
    "0.509804;205,74,118,255:0.529412;210,79,113,255:0.54902;214,85,109,255:0.568627;218,91,105,255:0.588235;222,97,100,255:"
    "0.607843;226,102,96,255:0.627451;230,108,92,255:0.647059;233,114,87,255:0.666667;237,121,83,255:0.686275;240,127,79,255:"
    "0.705882;243,133,75,255:0.72549;245,140,70,255:0.745098;247,147,66,255:0.764706;249,154,62,255:0.784314;251,161,57,255:"
    "0.803922;252,168,53,255:0.823529;253,175,49,255:0.843137;254,183,45,255:0.862745;254,190,42,255:0.882353;253,198,39,255:"
    "0.901961;252,206,37,255:0.921569;251,215,36,255:0.941176;248,223,37,255:0.960784;246,232,38,255:0.980392;243,240,39,255";
  return QgsGradientColorRamp::create( props );
}

void QgsMeshLayer::assignDefaultStyleToDatasetGroup( int groupIndex )
{
  const QgsMeshDatasetGroupMetadata metadata = mDataProvider->datasetGroupMetadata( groupIndex );
  double groupMin = metadata.minimum();
  double groupMax = metadata.maximum();

  QgsColorRampShader fcn( groupMin, groupMax, _createDefaultColorRamp() );
  fcn.classifyColorRamp( 5, -1, QgsRectangle(), nullptr );

  QgsMeshRendererScalarSettings scalarSettings;
  scalarSettings.setClassificationMinimumMaximum( groupMin, groupMax );
  scalarSettings.setColorRampShader( fcn );
  mRendererSettings.setScalarSettings( groupIndex, scalarSettings );
}

QgsMapLayerRenderer *QgsMeshLayer::createMapRenderer( QgsRenderContext &rendererContext )
{
  // Native mesh
  if ( !mNativeMesh )
  {
    // lazy loading of mesh data
    fillNativeMesh();
  }

  // Triangular mesh
  if ( !mTriangularMesh )
    mTriangularMesh.reset( new QgsTriangularMesh() );

  mTriangularMesh->update( mNativeMesh.get(), &rendererContext );

  // Cache
  if ( !mRendererCache )
    mRendererCache.reset( new QgsMeshLayerRendererCache() );

  return new QgsMeshLayerRenderer( this, rendererContext );
}

bool QgsMeshLayer::readSymbology( const QDomNode &node, QString &errorMessage,
                                  QgsReadWriteContext &context, QgsMapLayer::StyleCategories categories )
{
  Q_UNUSED( errorMessage );
  // TODO: implement categories for raster layer

  QDomElement elem = node.toElement();

  readCommonStyle( elem, context, categories );

  QDomElement elemRendererSettings = elem.firstChildElement( "mesh-renderer-settings" );
  if ( !elemRendererSettings.isNull() )
    mRendererSettings.readXml( elemRendererSettings );

  // get and set the blend mode if it exists
  QDomNode blendModeNode = node.namedItem( QStringLiteral( "blendMode" ) );
  if ( !blendModeNode.isNull() )
  {
    QDomElement e = blendModeNode.toElement();
    setBlendMode( QgsPainting::getCompositionMode( static_cast< QgsPainting::BlendMode >( e.text().toInt() ) ) );
  }

  return true;
}

bool QgsMeshLayer::writeSymbology( QDomNode &node, QDomDocument &doc, QString &errorMessage,
                                   const QgsReadWriteContext &context, QgsMapLayer::StyleCategories categories ) const
{
  Q_UNUSED( errorMessage );
  // TODO: implement categories for raster layer

  QDomElement elem = node.toElement();

  writeCommonStyle( elem, doc, context, categories );

  QDomElement elemRendererSettings = mRendererSettings.writeXml( doc );
  elem.appendChild( elemRendererSettings );

  // add blend mode node
  QDomElement blendModeElement  = doc.createElement( QStringLiteral( "blendMode" ) );
  QDomText blendModeText = doc.createTextNode( QString::number( QgsPainting::getBlendModeEnum( blendMode() ) ) );
  blendModeElement.appendChild( blendModeText );
  node.appendChild( blendModeElement );

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
#ifdef QGISDEBUG
      QgsDebugMsg( QStringLiteral( "extra dataset (res %1): %2" ).arg( res ).arg( uri ) );
#else
      ( void )res; // avoid unused warning in release builds
#endif

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

  setCrs( mDataProvider->crs() );

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
