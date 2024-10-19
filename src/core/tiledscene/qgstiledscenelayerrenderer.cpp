/***************************************************************************
                         qgstiledscenelayerrenderer.cpp
                         --------------------
    begin                : June 2023
    copyright            : (C) 2023 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgstiledscenelayerrenderer.h"
#include "qgscurve.h"
#include "qgslogger.h"
#include "qgsquantizedmeshtiles.h"
#include "qgstiledsceneboundingvolume.h"
#include "qgstiledscenelayer.h"
#include "qgsfeedback.h"
#include "qgsmapclippingutils.h"
#include "qgsrendercontext.h"
#include "qgstiledscenerequest.h"
#include "qgstiledscenetile.h"
#include "qgstiledscenerenderer.h"
#include "qgsgltfutils.h"
#include "qgscesiumutils.h"
#include "qgscurvepolygon.h"
#include "qgstextrenderer.h"
#include "qgsruntimeprofiler.h"
#include "qgsapplication.h"

#include <QMatrix4x4>
#include <qglobal.h>

#define TINYGLTF_NO_STB_IMAGE         // we use QImage-based reading of images
#define TINYGLTF_NO_STB_IMAGE_WRITE   // we don't need writing of images
#include "tiny_gltf.h"

QgsTiledSceneLayerRenderer::QgsTiledSceneLayerRenderer( QgsTiledSceneLayer *layer, QgsRenderContext &context )
  : QgsMapLayerRenderer( layer->id(), &context )
  , mLayerName( layer->name() )
  , mFeedback( new QgsFeedback )
  , mEnableProfile( context.flags() & Qgis::RenderContextFlag::RecordProfile )
{
  // We must not keep pointer to mLayer (it's dangerous) - we must copy anything we need for rendering
  // or use some locking to prevent read/write from multiple threads
  if ( !layer->dataProvider() || !layer->renderer() )
    return;

  QElapsedTimer timer;
  timer.start();

  mRenderer.reset( layer->renderer()->clone() );

  mSceneCrs = layer->dataProvider()->sceneCrs();

  mClippingRegions = QgsMapClippingUtils::collectClippingRegionsForLayer( *renderContext(), layer );
  mLayerBoundingVolume = layer->dataProvider()->boundingVolume();

  mIndex = layer->dataProvider()->index();
  mRenderTileBorders = mRenderer->isTileBorderRenderingEnabled();

  mReadyToCompose = false;

  mPreparationTime = timer.elapsed();
}

QgsTiledSceneLayerRenderer::~QgsTiledSceneLayerRenderer() = default;

bool QgsTiledSceneLayerRenderer::render()
{
  if ( !mIndex.isValid() )
    return false;

  std::unique_ptr< QgsScopedRuntimeProfile > profile;
  if ( mEnableProfile )
  {
    profile = std::make_unique< QgsScopedRuntimeProfile >( mLayerName, QStringLiteral( "rendering" ), layerId() );
    if ( mPreparationTime > 0 )
      QgsApplication::profiler()->record( QObject::tr( "Create renderer" ), mPreparationTime / 1000.0, QStringLiteral( "rendering" ) );
  }

  std::unique_ptr< QgsScopedRuntimeProfile > preparingProfile;
  if ( mEnableProfile )
  {
    preparingProfile = std::make_unique< QgsScopedRuntimeProfile >( QObject::tr( "Preparing render" ), QStringLiteral( "rendering" ) );
  }

  QgsRenderContext *rc = renderContext();
  QgsTiledSceneRenderContext context( *rc, mFeedback.get() );

  // Set up the render configuration options
  QPainter *painter = rc->painter();

  QgsScopedQPainterState painterState( painter );
  rc->setPainterFlagsUsingContext( painter );

  if ( !mClippingRegions.empty() )
  {
    bool needsPainterClipPath = false;
    const QPainterPath path = QgsMapClippingUtils::calculatePainterClipRegion( mClippingRegions, *rc, Qgis::LayerType::VectorTile, needsPainterClipPath );
    if ( needsPainterClipPath )
      rc->painter()->setClipPath( path, Qt::IntersectClip );
  }

  mElapsedTimer.start();

  mSceneToMapTransform = QgsCoordinateTransform( mSceneCrs, rc->coordinateTransform().destinationCrs(), rc->transformContext() );

  mRenderer->startRender( context );

  preparingProfile.reset();
  std::unique_ptr< QgsScopedRuntimeProfile > renderingProfile;
  if ( mEnableProfile )
  {
    renderingProfile = std::make_unique< QgsScopedRuntimeProfile >( QObject::tr( "Rendering" ), QStringLiteral( "rendering" ) );
  }

  const bool result = renderTiles( context );
  mRenderer->stopRender( context );
  mReadyToCompose = true;

  return result;
}

Qgis::MapLayerRendererFlags QgsTiledSceneLayerRenderer::flags() const
{
  // we want to show temporary incremental renders we retrieve each tile in the scene, as this can be slow and
  // we need to show the user that some activity is happening here.
  // But we can't render the final layer result incrementally, as we need to collect ALL the content from the
  // scene before we can sort it by z order and avoid random z-order stacking artifacts!
  // So we request here a preview render image for the temporary incremental updates:
  return Qgis::MapLayerRendererFlag::RenderPartialOutputs | Qgis::MapLayerRendererFlag::RenderPartialOutputOverPreviousCachedImage;
}

bool QgsTiledSceneLayerRenderer::forceRasterRender() const
{
  return mRenderer ? ( mRenderer->flags() & Qgis::TiledSceneRendererFlag::ForceRasterRender ) : false;
}

QgsTiledSceneRequest QgsTiledSceneLayerRenderer::createBaseRequest()
{
  const QgsRenderContext *context = renderContext();
  const QgsRectangle mapExtent = context->mapExtent();

  // calculate maximum screen error in METERS
  const double maximumErrorPixels = context->convertToPainterUnits( mRenderer->maximumScreenError(), mRenderer->maximumScreenErrorUnit() );
  // calculate width in meters across the middle of the map
  const double mapYCenter = 0.5 * ( mapExtent.yMinimum() + mapExtent.yMaximum() );
  double mapWidthMeters = 0;
  try
  {
    mapWidthMeters = context->distanceArea().measureLine(
                       QgsPointXY( mapExtent.xMinimum(), mapYCenter ),
                       QgsPointXY( mapExtent.xMaximum(), mapYCenter )
                     );
  }
  catch ( QgsCsException & )
  {
    // TODO report errors to user
    QgsDebugError( QStringLiteral( "An error occurred while calculating length" ) );
  }

  const double mapMetersPerPixel = mapWidthMeters / context->outputSize().width();
  const double maximumErrorInMeters = maximumErrorPixels * mapMetersPerPixel;

  QgsTiledSceneRequest request;
  request.setFeedback( feedback() );

  // TODO what z range makes sense here??
  const QVector< QgsVector3D > corners = QgsBox3D( mapExtent, -10000, 10000 ).corners();
  QVector< double > x;
  x.reserve( 8 );
  QVector< double > y;
  y.reserve( 8 );
  QVector< double > z;
  z.reserve( 8 );
  for ( int i = 0; i < 8; ++i )
  {
    const QgsVector3D &corner = corners[i];
    x.append( corner.x() );
    y.append( corner.y() );
    z.append( corner.z() );
  }
  mSceneToMapTransform.transformInPlace( x, y, z, Qgis::TransformDirection::Reverse );

  const auto minMaxX = std::minmax_element( x.constBegin(), x.constEnd() );
  const auto minMaxY = std::minmax_element( y.constBegin(), y.constEnd() );
  const auto minMaxZ = std::minmax_element( z.constBegin(), z.constEnd() );
  request.setFilterBox(
    QgsOrientedBox3D::fromBox3D( QgsBox3D( *minMaxX.first, *minMaxY.first, *minMaxZ.first, *minMaxX.second, *minMaxY.second, *minMaxZ.second ) )
  );

  request.setRequiredGeometricError( maximumErrorInMeters );

  return request;
}

bool QgsTiledSceneLayerRenderer::renderTiles( QgsTiledSceneRenderContext &context )
{
  const QgsRectangle mapExtent = context.renderContext().mapExtent();
  auto tileIsVisibleInMap = [mapExtent, this]( const QgsTiledSceneTile & tile )->bool
  {
    // the trip from map CRS to scene CRS will have expanded out the bounding volumes for the tile request, so
    // we want to cull any tiles which we've been given which don't actually intersect our visible map extent
    // when we transform them back into the destination map CRS.
    // This potentially saves us requesting data for tiles which aren't actually visible in the map.
    const QgsGeometry tileGeometry( tile.boundingVolume().as2DGeometry( mSceneToMapTransform ) );
    return tileGeometry.intersects( mapExtent );
  };

  QgsTiledSceneRequest request = createBaseRequest();
  QVector< long long > tileIds = mIndex.getTiles( request );
  while ( !tileIds.empty() )
  {
    if ( feedback() && feedback()->isCanceled() )
      return false;

    const long long tileId = tileIds.first();
    tileIds.pop_front();

    const QgsTiledSceneTile tile = mIndex.getTile( tileId );
    if ( !tile.isValid() || !tileIsVisibleInMap( tile ) )
      continue;

    switch ( mIndex.childAvailability( tileId ) )
    {
      case Qgis::TileChildrenAvailability::NoChildren:
      case Qgis::TileChildrenAvailability::Available:
      {
        renderTile( tile, context );
        break;
      }

      case Qgis::TileChildrenAvailability::NeedFetching:
      {
        if ( mIndex.fetchHierarchy( tileId, feedback() ) )
        {
          request.setParentTileId( tileId );
          const QVector< long long > newTileIdsToRender = mIndex.getTiles( request );
          tileIds.append( newTileIdsToRender );

          // do we still need to render the parent? Depends on the parent's refinement process...
          const QgsTiledSceneTile tile = mIndex.getTile( tileId );
          if ( tile.isValid() )
          {
            switch ( tile.refinementProcess() )
            {
              case Qgis::TileRefinementProcess::Replacement:
                break;
              case Qgis::TileRefinementProcess::Additive:
                renderTile( tile, context );
                break;
            }
          }
        }
        break;
      }
    }
  }
  if ( feedback() && feedback()->isCanceled() )
    return false;

  const bool needsTextures = mRenderer->flags() & Qgis::TiledSceneRendererFlag::RequiresTextures;

  std::sort( mPrimitiveData.begin(), mPrimitiveData.end(), []( const PrimitiveData & a, const PrimitiveData & b )
  {
    // this isn't an exact science ;)
    if ( qgsDoubleNear( a.z, b.z, 0.001 ) )
    {
      // for overlapping lines/triangles, ensure the line is drawn over the triangle
      if ( a.type == PrimitiveType::Line )
        return false;
      else if ( b.type == PrimitiveType::Line )
        return true;
    }
    return a.z < b.z;
  } );
  for ( const PrimitiveData &data : std::as_const( mPrimitiveData ) )
  {
    switch ( data.type )
    {
      case PrimitiveType::Line:
        mRenderer->renderLine( context, data.coordinates );
        break;

      case PrimitiveType::Triangle:
        if ( needsTextures )
        {
          context.setTextureImage( mTextures.value( data.textureId ) );
          context.setTextureCoordinates( data.textureCoords[0], data.textureCoords[1],
                                         data.textureCoords[2], data.textureCoords[3],
                                         data.textureCoords[4], data.textureCoords[5] );
        }
        mRenderer->renderTriangle( context, data.coordinates );
        break;
    }
  }

  if ( mRenderTileBorders )
  {
    QPainter *painter = renderContext()->painter();
    for ( const TileDetails &tile : std::as_const( mTileDetails ) )
    {
      QPen pen;
      QBrush brush;
      if ( tile.hasContent )
      {
        brush = QBrush( QColor( 0, 0, 255, 10 ) );
        pen = QPen( QColor( 0, 0, 255, 150 ) );
      }
      else
      {
        brush = QBrush( QColor( 255, 0, 255, 10 ) );
        pen = QPen( QColor( 255, 0, 255, 150 ) );
      }
      pen.setWidth( 2 );
      painter->setPen( pen );
      painter->setBrush( brush );
      painter->drawPolygon( tile.boundary );
#if 1
      QgsTextFormat format;
      format.setColor( QColor( 255, 0, 0 ) );
      format.buffer().setEnabled( true );

      QgsTextRenderer::drawText( QRectF( QPoint( 0, 0 ), renderContext()->outputSize() ).intersected( tile.boundary.boundingRect() ),
                                 0, Qgis::TextHorizontalAlignment::Center, { tile.id },
                                 *renderContext(), format, true, Qgis::TextVerticalAlignment::VerticalCenter );
#endif
    }
  }

  return true;
}

void QgsTiledSceneLayerRenderer::renderTile( const QgsTiledSceneTile &tile, QgsTiledSceneRenderContext &context )
{
  const bool hasContent = renderTileContent( tile, context );

  if ( mRenderTileBorders )
  {
    const QgsTiledSceneBoundingVolume &volume = tile.boundingVolume();
    try
    {
      std::unique_ptr< QgsAbstractGeometry > volumeGeometry( volume.as2DGeometry( mSceneToMapTransform ) );
      if ( QgsCurvePolygon *polygon = qgsgeometry_cast< QgsCurvePolygon * >( volumeGeometry.get() ) )
      {
        QPolygonF volumePolygon = polygon->exteriorRing()->asQPolygonF( );

        // remove non-finite points, e.g. infinite or NaN points caused by reprojecting errors
        volumePolygon.erase( std::remove_if( volumePolygon.begin(), volumePolygon.end(),
                                             []( const QPointF point )
        {
          return !std::isfinite( point.x() ) || !std::isfinite( point.y() );
        } ), volumePolygon.end() );

        QPointF *ptr = volumePolygon.data();
        for ( int i = 0; i < volumePolygon.size(); ++i, ++ptr )
        {
          renderContext()->mapToPixel().transformInPlace( ptr->rx(), ptr->ry() );
        }

        TileDetails details;
        details.boundary = volumePolygon;
        details.hasContent = hasContent;
        details.id = QString::number( tile.id() );
        mTileDetails.append( details );
      }
    }
    catch ( QgsCsException & )
    {
      QgsDebugError( QStringLiteral( "Error transforming bounding volume" ) );
    }
  }
}

bool QgsTiledSceneLayerRenderer::renderTileContent( const QgsTiledSceneTile &tile, QgsTiledSceneRenderContext &context )
{
  const QString contentUri = tile.resources().value( QStringLiteral( "content" ) ).toString();
  if ( contentUri.isEmpty() )
    return false;

  const QByteArray tileContent = mIndex.retrieveContent( contentUri, feedback() );
  // When the operation is canceled, retrieveContent() will silently return an empty array
  if ( feedback()->isCanceled() )
    return false;

  tinygltf::Model model;
  QgsVector3D centerOffset;
  mCurrentModelId++;
  // TODO: Somehow de-hardcode this switch?
  const auto &format = tile.metadata().value( QStringLiteral( "contentFormat" ) ).value<QString>();
  if ( format == QLatin1String( "quantizedmesh" ) )
  {
    try
    {
      QgsQuantizedMeshTile qmTile( tileContent );
      qmTile.removeDegenerateTriangles();
      model = qmTile.toGltf();
    }
    catch ( QgsQuantizedMeshParsingException &ex )
    {
      QgsDebugError( QStringLiteral( "Failed to parse tile from '%1'" ).arg( contentUri ) );
      return false;
    }
  }
  else if ( format == QLatin1String( "cesiumtiles" ) )
  {
    const QgsCesiumUtils::TileContents content = QgsCesiumUtils::extractGltfFromTileContent( tileContent );
    if ( content.gltf.isEmpty() )
    {
      return false;
    }
    centerOffset = content.rtcCenter;

    QString gltfErrors;
    QString gltfWarnings;
    const bool res = QgsGltfUtils::loadGltfModel( content.gltf, model,
                     &gltfErrors, &gltfWarnings );
    if ( !gltfErrors.isEmpty() )
    {
      if ( !mErrors.contains( gltfErrors ) )
        mErrors.append( gltfErrors );
      QgsDebugError( QStringLiteral( "Error raised reading %1: %2" )
                     .arg( contentUri, gltfErrors ) );
    }
    if ( !gltfWarnings.isEmpty() )
    {
      QgsDebugError( QStringLiteral( "Warnings raised reading %1: %2" )
                     .arg( contentUri, gltfWarnings ) );
    }
    if ( !res ) return false;
  }
  else
    return false;

  const QgsVector3D tileTranslationEcef =
    centerOffset +
    QgsGltfUtils::extractTileTranslation(
      model,
      static_cast<Qgis::Axis>( tile.metadata()
                               .value( QStringLiteral( "gltfUpAxis" ),
                                       static_cast<int>( Qgis::Axis::Y ) )
                               .toInt() ) );

  bool sceneOk = false;
  const std::size_t sceneIndex =
    QgsGltfUtils::sourceSceneForModel( model, sceneOk );
  if ( !sceneOk )
  {
    const QString error = QObject::tr( "No scenes found in model" );
    mErrors.append( error );
    QgsDebugError(
      QStringLiteral( "Error raised reading %1: %2" ).arg( contentUri, error ) );
  }
  else
  {
    const tinygltf::Scene &scene = model.scenes[sceneIndex];

    std::function< void( int nodeIndex, const QMatrix4x4 &transform ) > traverseNode;
    traverseNode = [&model, &context, &tileTranslationEcef, &tile, &contentUri, &traverseNode, this]( int nodeIndex, const QMatrix4x4 & parentTransform )
    {
      const tinygltf::Node &gltfNode = model.nodes[nodeIndex];
      std::unique_ptr< QMatrix4x4 > gltfLocalTransform = QgsGltfUtils::parseNodeTransform( gltfNode );

      if ( !parentTransform.isIdentity() )
      {
        if ( gltfLocalTransform )
          *gltfLocalTransform = parentTransform * *gltfLocalTransform;
        else
        {
          gltfLocalTransform.reset( new QMatrix4x4( parentTransform ) );
        }
      }

      if ( gltfNode.mesh >= 0 )
      {
        const tinygltf::Mesh &mesh = model.meshes[gltfNode.mesh];

        for ( const tinygltf::Primitive &primitive : mesh.primitives )
        {
          if ( context.renderContext().renderingStopped() )
            break;

          renderPrimitive( model, primitive, tile, tileTranslationEcef, gltfLocalTransform.get(), contentUri, context );
        }
      }

      for ( int childNode : gltfNode.children )
      {
        traverseNode( childNode, gltfLocalTransform ? *gltfLocalTransform : QMatrix4x4() );
      }
    };

    for ( int nodeIndex : scene.nodes )
    {
      traverseNode( nodeIndex, QMatrix4x4() );
    }
  }
  return true;
}

void QgsTiledSceneLayerRenderer::renderPrimitive( const tinygltf::Model &model, const tinygltf::Primitive &primitive, const QgsTiledSceneTile &tile, const QgsVector3D &tileTranslationEcef, const QMatrix4x4 *gltfLocalTransform, const QString &contentUri, QgsTiledSceneRenderContext &context )
{
  switch ( primitive.mode )
  {
    case TINYGLTF_MODE_TRIANGLES:
      if ( mRenderer->flags() & Qgis::TiledSceneRendererFlag::RendersTriangles )
        renderTrianglePrimitive( model, primitive, tile, tileTranslationEcef, gltfLocalTransform, contentUri, context );
      break;

    case TINYGLTF_MODE_LINE:
      if ( mRenderer->flags() & Qgis::TiledSceneRendererFlag::RendersLines )
        renderLinePrimitive( model, primitive, tile, tileTranslationEcef, gltfLocalTransform, contentUri, context );
      return;

    case TINYGLTF_MODE_POINTS:
      if ( !mWarnedPrimitiveTypes.contains( TINYGLTF_MODE_POINTS ) )
      {
        mErrors << QObject::tr( "Point objects in tiled scenes are not supported" );
        mWarnedPrimitiveTypes.insert( TINYGLTF_MODE_POINTS );
      }
      return;

    case TINYGLTF_MODE_LINE_LOOP:
      if ( !mWarnedPrimitiveTypes.contains( TINYGLTF_MODE_LINE_LOOP ) )
      {
        mErrors << QObject::tr( "Line loops in tiled scenes are not supported" );
        mWarnedPrimitiveTypes.insert( TINYGLTF_MODE_LINE_LOOP );
      }
      return;

    case TINYGLTF_MODE_LINE_STRIP:
      if ( !mWarnedPrimitiveTypes.contains( TINYGLTF_MODE_LINE_STRIP ) )
      {
        mErrors << QObject::tr( "Line strips in tiled scenes are not supported" );
        mWarnedPrimitiveTypes.insert( TINYGLTF_MODE_LINE_STRIP );
      }
      return;

    case TINYGLTF_MODE_TRIANGLE_STRIP:
      if ( !mWarnedPrimitiveTypes.contains( TINYGLTF_MODE_TRIANGLE_STRIP ) )
      {
        mErrors << QObject::tr( "Triangular strips in tiled scenes are not supported" );
        mWarnedPrimitiveTypes.insert( TINYGLTF_MODE_TRIANGLE_STRIP );
      }
      return;

    case TINYGLTF_MODE_TRIANGLE_FAN:
      if ( !mWarnedPrimitiveTypes.contains( TINYGLTF_MODE_TRIANGLE_FAN ) )
      {
        mErrors << QObject::tr( "Triangular fans in tiled scenes are not supported" );
        mWarnedPrimitiveTypes.insert( TINYGLTF_MODE_TRIANGLE_FAN );
      }
      return;

    default:
      if ( !mWarnedPrimitiveTypes.contains( primitive.mode ) )
      {
        mErrors << QObject::tr( "Primitive type %1 in tiled scenes are not supported" ).arg( primitive.mode );
        mWarnedPrimitiveTypes.insert( primitive.mode );
      }
      return;
  }
}

void QgsTiledSceneLayerRenderer::renderTrianglePrimitive( const tinygltf::Model &model, const tinygltf::Primitive &primitive, const QgsTiledSceneTile &tile, const QgsVector3D &tileTranslationEcef, const QMatrix4x4 *gltfLocalTransform, const QString &contentUri, QgsTiledSceneRenderContext &context )
{
  auto posIt = primitive.attributes.find( "POSITION" );
  if ( posIt == primitive.attributes.end() )
  {
    mErrors << QObject::tr( "Could not find POSITION attribute for primitive" );
    return;
  }
  int positionAccessorIndex = posIt->second;

  QVector< double > x;
  QVector< double > y;
  QVector< double > z;
  QgsGltfUtils::accessorToMapCoordinates(
    model, positionAccessorIndex, tile.transform() ? *tile.transform() : QgsMatrix4x4(),
    &mSceneToMapTransform,
    tileTranslationEcef,
    gltfLocalTransform,
    static_cast< Qgis::Axis >( tile.metadata().value( QStringLiteral( "gltfUpAxis" ), static_cast< int >( Qgis::Axis::Y ) ).toInt() ),
    x, y, z
  );

  renderContext()->mapToPixel().transformInPlace( x, y );

  const bool needsTextures = mRenderer->flags() & Qgis::TiledSceneRendererFlag::RequiresTextures;

  QImage textureImage;
  QVector< float > texturePointX;
  QVector< float > texturePointY;
  QPair< int, int > textureId{ -1, -1 };
  if ( needsTextures && primitive.material != -1 )
  {
    const tinygltf::Material &material = model.materials[primitive.material];
    const tinygltf::PbrMetallicRoughness &pbr = material.pbrMetallicRoughness;

    if ( pbr.baseColorTexture.index >= 0
         && static_cast< int >( model.textures.size() ) > pbr.baseColorTexture.index )
    {
      const tinygltf::Texture &tex = model.textures[pbr.baseColorTexture.index];

      switch ( QgsGltfUtils::imageResourceType( model, tex.source ) )
      {
        case QgsGltfUtils::ResourceType::Embedded:
          textureImage = QgsGltfUtils::extractEmbeddedImage( model, tex.source );
          break;

        case QgsGltfUtils::ResourceType::Linked:
        {
          const QString linkedPath = QgsGltfUtils::linkedImagePath( model, tex.source );
          const QString textureUri = QUrl( contentUri ).resolved( linkedPath ).toString();
          const QByteArray rep = mIndex.retrieveContent( textureUri, feedback() );
          if ( !rep.isEmpty() )
          {
            textureImage = QImage::fromData( rep );
          }
          break;
        }
      }

      if ( !textureImage.isNull() )
      {
        auto texIt = primitive.attributes.find( "TEXCOORD_0" );
        if ( texIt != primitive.attributes.end() )
        {
          QgsGltfUtils::extractTextureCoordinates(
            model, texIt->second, texturePointX, texturePointY
          );
        }

        textureId = qMakePair( mCurrentModelId, pbr.baseColorTexture.index );
      }
    }
    else if ( qgsDoubleNear( pbr.baseColorFactor[3], 0 ) )
    {
      // transparent primitive, skip
      return;
    }
  }

  const QRect outputRect = QRect( QPoint( 0, 0 ), context.renderContext().outputSize() );
  auto needTriangle = [&outputRect]( const QPolygonF & triangle ) -> bool
  {
    return triangle.boundingRect().intersects( outputRect );
  };

  const bool useTexture = !textureImage.isNull();
  bool hasStoredTexture = false;

  QVector< PrimitiveData > thisTileTriangleData;

  if ( primitive.indices == -1 )
  {
    Q_ASSERT( x.size() % 3 == 0 );

    thisTileTriangleData.reserve( x.size() );
    for ( int i = 0; i < x.size(); i += 3 )
    {
      if ( context.renderContext().renderingStopped() )
        break;

      PrimitiveData data;
      data.type = PrimitiveType::Triangle;
      data.textureId = textureId;
      if ( useTexture )
      {
        data.textureCoords[0] = texturePointX[i];
        data.textureCoords[1] = texturePointY[i];
        data.textureCoords[2] = texturePointX[i + 1];
        data.textureCoords[3] = texturePointY[i + 1];
        data.textureCoords[4] = texturePointX[i + 2];
        data.textureCoords[5] = texturePointY[i + 2];
      }
      data.coordinates = QVector<QPointF> { QPointF( x[i], y[i] ), QPointF( x[i + 1], y[i + 1] ), QPointF( x[i + 2], y[i + 2] ), QPointF( x[i], y[i] ) };
      data.z = ( z[i] + z[i + 1] + z[i + 2] ) / 3;
      if ( needTriangle( data.coordinates ) )
      {
        thisTileTriangleData.push_back( data );
        if ( !hasStoredTexture && !textureImage.isNull() )
        {
          // have to make an explicit .copy() here, as we don't necessarily own the image data
          mTextures.insert( textureId, textureImage.copy() );
          hasStoredTexture = true;
        }
      }
    }
  }
  else
  {
    const tinygltf::Accessor &primitiveAccessor = model.accessors[primitive.indices];
    const tinygltf::BufferView &bvPrimitive = model.bufferViews[primitiveAccessor.bufferView];
    const tinygltf::Buffer &bPrimitive = model.buffers[bvPrimitive.buffer];

    Q_ASSERT( ( primitiveAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT
                || primitiveAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT
                || primitiveAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE )
              && primitiveAccessor.type == TINYGLTF_TYPE_SCALAR );

    const char *primitivePtr = reinterpret_cast< const char * >( bPrimitive.data.data() ) + bvPrimitive.byteOffset + primitiveAccessor.byteOffset;

    thisTileTriangleData.reserve( primitiveAccessor.count / 3 );
    for ( std::size_t i = 0; i < primitiveAccessor.count / 3; i++ )
    {
      if ( context.renderContext().renderingStopped() )
        break;

      unsigned int index1 = 0;
      unsigned int index2 = 0;
      unsigned int index3 = 0;

      PrimitiveData data;
      data.type = PrimitiveType::Triangle;
      data.textureId = textureId;

      if ( primitiveAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT )
      {
        const unsigned short *usPtrPrimitive = reinterpret_cast< const unsigned short * >( primitivePtr );
        if ( bvPrimitive.byteStride )
          primitivePtr += bvPrimitive.byteStride;
        else
          primitivePtr += 3 * sizeof( unsigned short );

        index1 = usPtrPrimitive[0];
        index2 = usPtrPrimitive[1];
        index3 = usPtrPrimitive[2];
      }
      else if ( primitiveAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE )
      {
        const unsigned char *usPtrPrimitive = reinterpret_cast< const unsigned char * >( primitivePtr );
        if ( bvPrimitive.byteStride )
          primitivePtr += bvPrimitive.byteStride;
        else
          primitivePtr += 3 * sizeof( unsigned char );

        index1 = usPtrPrimitive[0];
        index2 = usPtrPrimitive[1];
        index3 = usPtrPrimitive[2];
      }
      else
      {
        const unsigned int *uintPtrPrimitive = reinterpret_cast< const unsigned int * >( primitivePtr );
        if ( bvPrimitive.byteStride )
          primitivePtr += bvPrimitive.byteStride;
        else
          primitivePtr += 3 * sizeof( unsigned int );

        index1 = uintPtrPrimitive[0];
        index2 = uintPtrPrimitive[1];
        index3 = uintPtrPrimitive[2];
      }

      if ( useTexture )
      {
        data.textureCoords[0] = texturePointX[index1];
        data.textureCoords[1] = texturePointY[index1];
        data.textureCoords[2] = texturePointX[index2];
        data.textureCoords[3] = texturePointY[index2];
        data.textureCoords[4] = texturePointX[index3];
        data.textureCoords[5] = texturePointY[index3];
      }

      data.coordinates = { QVector<QPointF>{ QPointF( x[index1], y[index1] ), QPointF( x[index2], y[index2] ), QPointF( x[index3], y[index3] ), QPointF( x[index1], y[index1] ) } };
      data.z = ( z[index1] + z[index2] + z[index3] ) / 3;
      if ( needTriangle( data.coordinates ) )
      {
        thisTileTriangleData.push_back( data );
        if ( !hasStoredTexture && !textureImage.isNull() )
        {
          // have to make an explicit .copy() here, as we don't necessarily own the image data
          mTextures.insert( textureId, textureImage.copy() );
          hasStoredTexture = true;
        }
      }
    }
  }

  if ( context.renderContext().previewRenderPainter() )
  {
    // swap out the destination painter for the preview render painter, and render
    // the triangles from this tile in a sorted order
    QPainter *finalPainter = context.renderContext().painter();
    context.renderContext().setPainter( context.renderContext().previewRenderPainter() );

    std::sort( thisTileTriangleData.begin(), thisTileTriangleData.end(), []( const PrimitiveData & a, const PrimitiveData & b )
    {
      return a.z < b.z;
    } );

    for ( const PrimitiveData &data : std::as_const( thisTileTriangleData ) )
    {
      if ( useTexture && data.textureId.first >= 0 )
      {
        context.setTextureImage( mTextures.value( data.textureId ) );
        context.setTextureCoordinates( data.textureCoords[0], data.textureCoords[1],
                                       data.textureCoords[2], data.textureCoords[3],
                                       data.textureCoords[4], data.textureCoords[5] );
      }
      mRenderer->renderTriangle( context, data.coordinates );
    }
    context.renderContext().setPainter( finalPainter );
  }

  mPrimitiveData.append( thisTileTriangleData );

  // as soon as first tile is rendered, we can start showing layer updates. But we still delay
  // this by e.g. 3 seconds before we start forcing progressive updates, so that we don't show the unsorted
  // z triangle render if the overall layer render only takes a second or so.
  if ( mElapsedTimer.elapsed() > MAX_TIME_TO_USE_CACHED_PREVIEW_IMAGE )
  {
    mReadyToCompose = true;
  }
}

void QgsTiledSceneLayerRenderer::renderLinePrimitive( const tinygltf::Model &model, const tinygltf::Primitive &primitive, const QgsTiledSceneTile &tile, const QgsVector3D &tileTranslationEcef, const QMatrix4x4 *gltfLocalTransform, const QString &, QgsTiledSceneRenderContext &context )
{
  auto posIt = primitive.attributes.find( "POSITION" );
  if ( posIt == primitive.attributes.end() )
  {
    mErrors << QObject::tr( "Could not find POSITION attribute for primitive" );
    return;
  }
  int positionAccessorIndex = posIt->second;

  QVector< double > x;
  QVector< double > y;
  QVector< double > z;
  QgsGltfUtils::accessorToMapCoordinates(
    model, positionAccessorIndex, tile.transform() ? *tile.transform() : QgsMatrix4x4(),
    &mSceneToMapTransform,
    tileTranslationEcef,
    gltfLocalTransform,
    static_cast< Qgis::Axis >( tile.metadata().value( QStringLiteral( "gltfUpAxis" ), static_cast< int >( Qgis::Axis::Y ) ).toInt() ),
    x, y, z
  );

  renderContext()->mapToPixel().transformInPlace( x, y );

  const QRect outputRect = QRect( QPoint( 0, 0 ), context.renderContext().outputSize() );
  auto needLine = [&outputRect]( const QPolygonF & line ) -> bool
  {
    return line.boundingRect().intersects( outputRect );
  };

  QVector< PrimitiveData > thisTileLineData;

  if ( primitive.indices == -1 )
  {
    Q_ASSERT( x.size() % 2 == 0 );

    thisTileLineData.reserve( x.size() );
    for ( int i = 0; i < x.size(); i += 2 )
    {
      if ( context.renderContext().renderingStopped() )
        break;

      PrimitiveData data;
      data.type = PrimitiveType::Line;
      data.coordinates = QVector<QPointF> { QPointF( x[i], y[i] ), QPointF( x[i + 1], y[i + 1] ) };
      // note -- we take the maximum z here, as we'd ideally like lines to be placed over similarish z valued triangles
      data.z = std::max( z[i], z[i + 1] );
      if ( needLine( data.coordinates ) )
      {
        thisTileLineData.push_back( data );
      }
    }
  }
  else
  {
    const tinygltf::Accessor &primitiveAccessor = model.accessors[primitive.indices];
    const tinygltf::BufferView &bvPrimitive = model.bufferViews[primitiveAccessor.bufferView];
    const tinygltf::Buffer &bPrimitive = model.buffers[bvPrimitive.buffer];

    Q_ASSERT( ( primitiveAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT
                || primitiveAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT
                || primitiveAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE )
              && primitiveAccessor.type == TINYGLTF_TYPE_SCALAR );

    const char *primitivePtr = reinterpret_cast< const char * >( bPrimitive.data.data() ) + bvPrimitive.byteOffset + primitiveAccessor.byteOffset;

    thisTileLineData.reserve( primitiveAccessor.count / 2 );
    for ( std::size_t i = 0; i < primitiveAccessor.count / 2; i++ )
    {
      if ( context.renderContext().renderingStopped() )
        break;

      unsigned int index1 = 0;
      unsigned int index2 = 0;

      PrimitiveData data;
      data.type = PrimitiveType::Line;

      if ( primitiveAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT )
      {
        const unsigned short *usPtrPrimitive = reinterpret_cast< const unsigned short * >( primitivePtr );
        if ( bvPrimitive.byteStride )
          primitivePtr += bvPrimitive.byteStride;
        else
          primitivePtr += 2 * sizeof( unsigned short );

        index1 = usPtrPrimitive[0];
        index2 = usPtrPrimitive[1];
      }
      else if ( primitiveAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE )
      {
        const unsigned char *usPtrPrimitive = reinterpret_cast< const unsigned char * >( primitivePtr );
        if ( bvPrimitive.byteStride )
          primitivePtr += bvPrimitive.byteStride;
        else
          primitivePtr += 2 * sizeof( unsigned char );

        index1 = usPtrPrimitive[0];
        index2 = usPtrPrimitive[1];
      }
      else
      {
        const unsigned int *uintPtrPrimitive = reinterpret_cast< const unsigned int * >( primitivePtr );
        if ( bvPrimitive.byteStride )
          primitivePtr += bvPrimitive.byteStride;
        else
          primitivePtr += 2 * sizeof( unsigned int );

        index1 = uintPtrPrimitive[0];
        index2 = uintPtrPrimitive[1];
      }

      data.coordinates = { QVector<QPointF>{ QPointF( x[index1], y[index1] ), QPointF( x[index2], y[index2] ) } };
      // note -- we take the maximum z here, as we'd ideally like lines to be placed over similarish z valued triangles
      data.z = std::max( z[index1], z[index2] );
      if ( needLine( data.coordinates ) )
      {
        thisTileLineData.push_back( data );
      }
    }
  }

  if ( context.renderContext().previewRenderPainter() )
  {
    // swap out the destination painter for the preview render painter, and render
    // the triangles from this tile in a sorted order
    QPainter *finalPainter = context.renderContext().painter();
    context.renderContext().setPainter( context.renderContext().previewRenderPainter() );

    std::sort( thisTileLineData.begin(), thisTileLineData.end(), []( const PrimitiveData & a, const PrimitiveData & b )
    {
      return a.z < b.z;
    } );

    for ( const PrimitiveData &data : std::as_const( thisTileLineData ) )
    {
      mRenderer->renderLine( context, data.coordinates );
    }
    context.renderContext().setPainter( finalPainter );
  }

  mPrimitiveData.append( thisTileLineData );

  // as soon as first tile is rendered, we can start showing layer updates. But we still delay
  // this by e.g. 3 seconds before we start forcing progressive updates, so that we don't show the unsorted
  // z primitive render if the overall layer render only takes a second or so.
  if ( mElapsedTimer.elapsed() > MAX_TIME_TO_USE_CACHED_PREVIEW_IMAGE )
  {
    mReadyToCompose = true;
  }
}
