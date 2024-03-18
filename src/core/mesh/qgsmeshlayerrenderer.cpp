/***************************************************************************
                         qgsmeshlayerrenderer.cpp
                         ------------------------
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

#include <memory>
#include <QSet>
#include <QPair>
#include <QLinearGradient>
#include <QBrush>
#include <QPointer>
#include <algorithm>
#include <QElapsedTimer>

#include "qgsmeshlayerrenderer.h"

#include "qgslogger.h"
#include "qgsmeshlayer.h"
#include "qgspointxy.h"
#include "qgssinglebandpseudocolorrenderer.h"
#include "qgsrastershader.h"
#include "qgsmeshlayerinterpolator.h"
#include "qgsmeshlayerutils.h"
#include "qgsmeshvectorrenderer.h"
#include "qgsmeshlayerlabeling.h"
#include "qgsmeshlayerlabelprovider.h"
#include "qgsmapclippingutils.h"
#include "qgscolorrampshader.h"
#include "qgsmaplayerelevationproperties.h"
#include "qgsapplication.h"
#include "qgsruntimeprofiler.h"
#include "qgsexpressioncontextutils.h"
#include "qgsmeshlayerelevationproperties.h"

QgsMeshLayerRenderer::QgsMeshLayerRenderer(
  QgsMeshLayer *layer,
  QgsRenderContext &context )
  : QgsMapLayerRenderer( layer->id(), &context )
  , mIsEditable( layer->isEditable() )
  , mLayerName( layer->name() )
  , mFeedback( new QgsMeshLayerRendererFeedback )
  , mRendererSettings( layer->rendererSettings() )
  , mEnableProfile( context.flags() & Qgis::RenderContextFlag::RecordProfile )
  , mLayerOpacity( layer->opacity() )
{
  QElapsedTimer timer;
  timer.start();

  // make copies for mesh data
  // cppcheck-suppress assertWithSideEffect
  Q_ASSERT( layer->nativeMesh() );
  // cppcheck-suppress assertWithSideEffect
  Q_ASSERT( layer->triangularMesh() );
  // cppcheck-suppress assertWithSideEffect
  Q_ASSERT( layer->rendererCache() );
  // cppcheck-suppress assertWithSideEffect
  Q_ASSERT( layer->dataProvider() );

  mReadyToCompose = false;

  // copy native mesh
  mNativeMesh = *( layer->nativeMesh() );
  mLayerExtent = layer->extent();

  // copy triangular mesh
  copyTriangularMeshes( layer, context );

  // copy datasets
  copyScalarDatasetValues( layer );
  copyVectorDatasetValues( layer );

  calculateOutputSize();

  QSet<QString> attrs;
  prepareLabeling( layer, attrs );

  mClippingRegions = QgsMapClippingUtils::collectClippingRegionsForLayer( *renderContext(), layer );

  if ( layer->elevationProperties() && layer->elevationProperties()->hasElevation() )
  {
    QgsMeshLayerElevationProperties *elevProp = qobject_cast<QgsMeshLayerElevationProperties *>( layer->elevationProperties() );

    mRenderElevationMap = true;
    mElevationScale = elevProp->zScale();
    mElevationOffset = elevProp->zOffset();

    if ( !context.zRange().isInfinite() )
    {
      switch ( elevProp->mode() )
      {
        case Qgis::MeshElevationMode::FixedElevationRange:
          // don't need to handle anything here -- the layer renderer will never be created if the
          // render context range doesn't match the layer's fixed elevation range
          break;

        case Qgis::MeshElevationMode::FromVertices:
        {
          // TODO -- filtering by mesh z values is not currently implemented
          break;
        }
      }
    }
  }

  mPreparationTime = timer.elapsed();
}

void QgsMeshLayerRenderer::copyTriangularMeshes( QgsMeshLayer *layer, QgsRenderContext &context )
{
  // handle level of details of mesh
  const QgsMeshSimplificationSettings simplificationSettings = layer->meshSimplificationSettings();
  if ( simplificationSettings.isEnabled() )
  {
    const double triangleSize = simplificationSettings.meshResolution() * context.mapToPixel().mapUnitsPerPixel();
    mTriangularMesh = *( layer->triangularMesh( triangleSize ) );
    mIsMeshSimplificationActive = true;
  }
  else
  {
    mTriangularMesh = *( layer->triangularMesh() );
  }
}

QgsFeedback *QgsMeshLayerRenderer::feedback() const
{
  return mFeedback.get();
}

void QgsMeshLayerRenderer::calculateOutputSize()
{
  // figure out image size
  const QgsRenderContext &context = *renderContext();
  const QgsRectangle extent = context.mapExtent();
  const QgsMapToPixel mapToPixel = context.mapToPixel();
  const QgsRectangle screenBBox = QgsMeshLayerUtils::boundingBoxToScreenRectangle( mapToPixel, extent, context.devicePixelRatio() );
  const int width = int( screenBBox.width() );
  const int height = int( screenBBox.height() );
  mOutputSize = QSize( width, height );
}

void QgsMeshLayerRenderer::copyScalarDatasetValues( QgsMeshLayer *layer )
{
  QgsMeshDatasetIndex datasetIndex;
  if ( renderContext()->isTemporal() )
    datasetIndex = layer->activeScalarDatasetAtTime( renderContext()->temporalRange() );
  else
    datasetIndex = layer->staticScalarDatasetIndex();

  // Find out if we can use cache up to date. If yes, use it and return
  const int datasetGroupCount = layer->datasetGroupCount();
  const QgsMeshRendererScalarSettings::DataResamplingMethod method = mRendererSettings.scalarSettings( datasetIndex.group() ).dataResamplingMethod();
  QgsMeshLayerRendererCache *cache = layer->rendererCache();
  if ( ( cache->mDatasetGroupsCount == datasetGroupCount ) &&
       ( cache->mActiveScalarDatasetIndex == datasetIndex ) &&
       ( cache->mDataInterpolationMethod ==  method ) &&
       ( QgsMesh3DAveragingMethod::equals( cache->mScalarAveragingMethod.get(), mRendererSettings.averagingMethod() ) )
     )
  {
    mScalarDatasetValues = cache->mScalarDatasetValues;
    mScalarActiveFaceFlagValues = cache->mScalarActiveFaceFlagValues;
    mScalarDataType = cache->mScalarDataType;
    mScalarDatasetMinimum = cache->mScalarDatasetMinimum;
    mScalarDatasetMaximum = cache->mScalarDatasetMaximum;
    return;
  }

  // Cache is not up-to-date, gather data
  if ( datasetIndex.isValid() )
  {
    const QgsMeshDatasetGroupMetadata metadata = layer->datasetGroupMetadata( datasetIndex.group() );
    mScalarDataType = QgsMeshLayerUtils::datasetValuesType( metadata.dataType() );

    // populate scalar values
    const int count = QgsMeshLayerUtils::datasetValuesCount( &mNativeMesh, mScalarDataType );
    const QgsMeshDataBlock vals = QgsMeshLayerUtils::datasetValues(
                                    layer,
                                    datasetIndex,
                                    0,
                                    count );

    if ( vals.isValid() )
    {
      // vals could be scalar or vectors, for contour rendering we want always magnitude
      mScalarDatasetValues = QgsMeshLayerUtils::calculateMagnitudes( vals );
    }
    else
    {
      mScalarDatasetValues = QVector<double>( count, std::numeric_limits<double>::quiet_NaN() );
    }

    // populate face active flag, always defined on faces
    mScalarActiveFaceFlagValues = layer->areFacesActive(
                                    datasetIndex,
                                    0,
                                    mNativeMesh.faces.count() );

    // for data on faces, there could be request to interpolate the data to vertices
    if ( method != QgsMeshRendererScalarSettings::NoResampling )
    {
      if ( mScalarDataType == QgsMeshDatasetGroupMetadata::DataType::DataOnFaces )
      {
        mScalarDataType = QgsMeshDatasetGroupMetadata::DataType::DataOnVertices;
        mScalarDatasetValues = QgsMeshLayerUtils::interpolateFromFacesData(
                                 mScalarDatasetValues,
                                 &mNativeMesh,
                                 &mTriangularMesh,
                                 &mScalarActiveFaceFlagValues,
                                 method
                               );
      }
      else if ( mScalarDataType == QgsMeshDatasetGroupMetadata::DataType::DataOnVertices )
      {
        mScalarDataType = QgsMeshDatasetGroupMetadata::DataType::DataOnFaces;
        mScalarDatasetValues = QgsMeshLayerUtils::resampleFromVerticesToFaces(
                                 mScalarDatasetValues,
                                 &mNativeMesh,
                                 &mTriangularMesh,
                                 &mScalarActiveFaceFlagValues,
                                 method
                               );
      }
    }

    const QgsMeshDatasetMetadata datasetMetadata = layer->datasetMetadata( datasetIndex );
    mScalarDatasetMinimum = datasetMetadata.minimum();
    mScalarDatasetMaximum = datasetMetadata.maximum();
  }

  // update cache
  cache->mDatasetGroupsCount = datasetGroupCount;
  cache->mActiveScalarDatasetIndex = datasetIndex;
  cache->mDataInterpolationMethod = method;
  cache->mScalarDatasetValues = mScalarDatasetValues;
  cache->mScalarActiveFaceFlagValues = mScalarActiveFaceFlagValues;
  cache->mScalarDataType = mScalarDataType;
  cache->mScalarDatasetMinimum = mScalarDatasetMinimum;
  cache->mScalarDatasetMaximum = mScalarDatasetMaximum;
  cache->mScalarAveragingMethod.reset( mRendererSettings.averagingMethod() ? mRendererSettings.averagingMethod()->clone() : nullptr );
}


void QgsMeshLayerRenderer::copyVectorDatasetValues( QgsMeshLayer *layer )
{
  QgsMeshDatasetIndex datasetIndex;
  if ( renderContext()->isTemporal() )
    datasetIndex = layer->activeVectorDatasetAtTime( renderContext()->temporalRange() );
  else
    datasetIndex = layer->staticVectorDatasetIndex();

  // Find out if we can use cache up to date. If yes, use it and return
  const int datasetGroupCount = layer->datasetGroupCount();
  QgsMeshLayerRendererCache *cache = layer->rendererCache();
  if ( ( cache->mDatasetGroupsCount == datasetGroupCount ) &&
       ( cache->mActiveVectorDatasetIndex == datasetIndex ) &&
       ( QgsMesh3DAveragingMethod::equals( cache->mVectorAveragingMethod.get(), mRendererSettings.averagingMethod() ) )
     )
  {
    mVectorDatasetValues = cache->mVectorDatasetValues;
    mVectorDatasetValuesMag = cache->mVectorDatasetValuesMag;
    mVectorDatasetMagMinimum = cache->mVectorDatasetMagMinimum;
    mVectorDatasetMagMaximum = cache->mVectorDatasetMagMaximum;
    mVectorDatasetGroupMagMinimum = cache->mVectorDatasetMagMinimum;
    mVectorDatasetGroupMagMaximum = cache->mVectorDatasetMagMaximum;
    mVectorActiveFaceFlagValues = cache->mVectorActiveFaceFlagValues;
    mVectorDataType = cache->mVectorDataType;
    return;
  }

  // Cache is not up-to-date, gather data
  if ( datasetIndex.isValid() )
  {
    const QgsMeshDatasetGroupMetadata metadata = layer->datasetGroupMetadata( datasetIndex );

    const bool isScalar = metadata.isScalar();
    if ( isScalar )
    {
      QgsDebugError( QStringLiteral( "Dataset has no vector values" ) );
    }
    else
    {
      mVectorDataType = QgsMeshLayerUtils::datasetValuesType( metadata.dataType() );

      mVectorDatasetGroupMagMinimum = metadata.minimum();
      mVectorDatasetGroupMagMaximum = metadata.maximum();

      const int count = QgsMeshLayerUtils::datasetValuesCount( &mNativeMesh, mVectorDataType );
      mVectorDatasetValues = QgsMeshLayerUtils::datasetValues(
                               layer,
                               datasetIndex,
                               0,
                               count );

      if ( mVectorDatasetValues.isValid() )
        mVectorDatasetValuesMag = QgsMeshLayerUtils::calculateMagnitudes( mVectorDatasetValues );
      else
        mVectorDatasetValuesMag = QVector<double>( count, std::numeric_limits<double>::quiet_NaN() );

      // populate face active flag
      mVectorActiveFaceFlagValues = layer->areFacesActive( datasetIndex, 0, mNativeMesh.faces.count() );

      const QgsMeshDatasetMetadata datasetMetadata = layer->datasetMetadata( datasetIndex );
      mVectorDatasetMagMinimum = datasetMetadata.minimum();
      mVectorDatasetMagMaximum = datasetMetadata.maximum();
    }
  }

  // update cache
  cache->mDatasetGroupsCount = datasetGroupCount;
  cache->mActiveVectorDatasetIndex = datasetIndex;
  cache->mVectorDatasetValues = mVectorDatasetValues;
  cache->mVectorDatasetValuesMag = mVectorDatasetValuesMag;
  cache->mVectorDatasetMagMinimum = mVectorDatasetMagMinimum;
  cache->mVectorDatasetMagMaximum = mVectorDatasetMagMaximum;
  cache->mVectorDatasetGroupMagMinimum = mVectorDatasetMagMinimum;
  cache->mVectorDatasetGroupMagMaximum = mVectorDatasetMagMaximum;
  cache->mVectorActiveFaceFlagValues = mVectorActiveFaceFlagValues;
  cache->mVectorDataType = mVectorDataType;
  cache->mVectorAveragingMethod.reset( mRendererSettings.averagingMethod() ? mRendererSettings.averagingMethod()->clone() : nullptr );
}

bool QgsMeshLayerRenderer::render()
{
  std::unique_ptr< QgsScopedRuntimeProfile > profile;
  std::unique_ptr< QgsScopedRuntimeProfile > preparingProfile;
  if ( mEnableProfile )
  {
    profile = std::make_unique< QgsScopedRuntimeProfile >( mLayerName, QStringLiteral( "rendering" ), layerId() );
    if ( mPreparationTime > 0 )
      QgsApplication::profiler()->record( QObject::tr( "Create renderer" ), mPreparationTime / 1000.0, QStringLiteral( "rendering" ) );
    preparingProfile = std::make_unique< QgsScopedRuntimeProfile >( QObject::tr( "Preparing render" ), QStringLiteral( "rendering" ) );
  }

  mReadyToCompose = false;
  const QgsScopedQPainterState painterState( renderContext()->painter() );
  if ( !mClippingRegions.empty() )
  {
    bool needsPainterClipPath = false;
    const QPainterPath path = QgsMapClippingUtils::calculatePainterClipRegion( mClippingRegions, *renderContext(), Qgis::LayerType::Mesh, needsPainterClipPath );
    if ( needsPainterClipPath )
      renderContext()->painter()->setClipPath( path, Qt::IntersectClip );
  }

  preparingProfile.reset();

  renderScalarDataset();
  mReadyToCompose = true;
  renderMesh();
  renderVectorDataset();

  registerLabelFeatures();

  return !renderContext()->renderingStopped();
}

bool QgsMeshLayerRenderer::forceRasterRender() const
{
  return renderContext()->testFlag( Qgis::RenderContextFlag::UseAdvancedEffects ) && ( !qgsDoubleNear( mLayerOpacity, 1.0 ) );
}

void QgsMeshLayerRenderer::renderMesh()
{
  if ( !mRendererSettings.nativeMeshSettings().isEnabled() && !mIsEditable &&
       !mRendererSettings.edgeMeshSettings().isEnabled() &&
       !mRendererSettings.triangularMeshSettings().isEnabled() )
    return;

  std::unique_ptr< QgsScopedRuntimeProfile > renderProfile;
  if ( mEnableProfile )
  {
    renderProfile = std::make_unique< QgsScopedRuntimeProfile >( QObject::tr( "Rendering mesh" ), QStringLiteral( "rendering" ) );
  }

  // triangular mesh
  const QList<int> trianglesInExtent = mTriangularMesh.faceIndexesForRectangle( renderContext()->mapExtent() );
  if ( mRendererSettings.triangularMeshSettings().isEnabled() )
  {
    renderFaceMesh(
      mRendererSettings.triangularMeshSettings(),
      mTriangularMesh.triangles(),
      trianglesInExtent );
  }

  // native mesh
  if ( ( mRendererSettings.nativeMeshSettings().isEnabled() || mIsEditable ) &&
       mTriangularMesh.levelOfDetail() == 0 )
  {
    const QSet<int> nativeFacesInExtent = QgsMeshUtils::nativeFacesFromTriangles( trianglesInExtent,
                                          mTriangularMesh.trianglesToNativeFaces() );

    renderFaceMesh(
      mRendererSettings.nativeMeshSettings(),
      mNativeMesh.faces,
      nativeFacesInExtent.values() );
  }

  // edge mesh
  if ( mRendererSettings.edgeMeshSettings().isEnabled() )
  {
    const QList<int> edgesInExtent = mTriangularMesh.edgeIndexesForRectangle( renderContext()->mapExtent() );
    renderEdgeMesh( mRendererSettings.edgeMeshSettings(), edgesInExtent );
  }
}

static QPainter *_painterForMeshFrame( QgsRenderContext &context, const QgsMeshRendererMeshSettings &settings )
{
  // Set up the render configuration options
  QPainter *painter = context.painter();

  painter->save();
  context.setPainterFlagsUsingContext( painter );

  QPen pen = painter->pen();
  pen.setCapStyle( Qt::FlatCap );
  pen.setJoinStyle( Qt::MiterJoin );

  const double penWidth = context.convertToPainterUnits( settings.lineWidth(), settings.lineWidthUnit() );
  pen.setWidthF( penWidth );
  pen.setColor( settings.color() );
  painter->setPen( pen );
  return painter;
}

void QgsMeshLayerRenderer::renderEdgeMesh( const QgsMeshRendererMeshSettings &settings, const QList<int> &edgesInExtent )
{
  Q_ASSERT( settings.isEnabled() );

  if ( !mTriangularMesh.contains( QgsMesh::ElementType::Edge ) )
    return;

  QgsRenderContext &context = *renderContext();
  QPainter *painter = _painterForMeshFrame( context, settings );

  const QVector<QgsMeshEdge> edges = mTriangularMesh.edges();
  const QVector<QgsMeshVertex> vertices = mTriangularMesh.vertices();

  for ( const int i : edgesInExtent )
  {
    if ( context.renderingStopped() )
      break;

    if ( i >= edges.size() )
      continue;

    const QgsMeshEdge &edge = edges[i];
    const int startVertexIndex = edge.first;
    const int endVertexIndex = edge.second;

    if ( ( startVertexIndex >= vertices.size() ) || endVertexIndex >= vertices.size() )
      continue;

    const QgsMeshVertex &startVertex = vertices[startVertexIndex];
    const QgsMeshVertex &endVertex = vertices[endVertexIndex];
    const QgsPointXY lineStart = context.mapToPixel().transform( startVertex.x(), startVertex.y() );
    const QgsPointXY lineEnd = context.mapToPixel().transform( endVertex.x(), endVertex.y() );
    painter->drawLine( lineStart.toQPointF(), lineEnd.toQPointF() );
  }
  painter->restore();
};

void QgsMeshLayerRenderer::renderFaceMesh(
  const QgsMeshRendererMeshSettings &settings,
  const QVector<QgsMeshFace> &faces,
  const QList<int> &facesInExtent )
{
  Q_ASSERT( settings.isEnabled() || mIsEditable );

  if ( !mTriangularMesh.contains( QgsMesh::ElementType::Face ) )
    return;

  QgsRenderContext &context = *renderContext();
  QPainter *painter = _painterForMeshFrame( context, settings );

  const QVector<QgsMeshVertex> &vertices = mTriangularMesh.vertices(); //Triangular mesh vertices contains also native mesh vertices
  QSet<QPair<int, int>> drawnEdges;

  for ( const int i : facesInExtent )
  {
    if ( context.renderingStopped() )
      break;

    if ( i < 0 || i >= faces.count() )
      continue;

    const QgsMeshFace &face = faces[i];
    if ( face.size() < 2 )
      continue;

    for ( int j = 0; j < face.size(); ++j )
    {
      const int startVertexId = face[j];
      const int endVertexId = face[( j + 1 ) % face.size()];
      const QPair<int, int> thisEdge( startVertexId, endVertexId );
      const QPair<int, int> thisEdgeReversed( endVertexId, startVertexId );
      if ( drawnEdges.contains( thisEdge ) || drawnEdges.contains( thisEdgeReversed ) )
        continue;
      drawnEdges.insert( thisEdge );
      drawnEdges.insert( thisEdgeReversed );

      const QgsMeshVertex &startVertex = vertices[startVertexId];
      const QgsMeshVertex &endVertex = vertices[endVertexId];
      const QgsPointXY lineStart = context.mapToPixel().transform( startVertex.x(), startVertex.y() );
      const QgsPointXY lineEnd = context.mapToPixel().transform( endVertex.x(), endVertex.y() );
      painter->drawLine( lineStart.toQPointF(), lineEnd.toQPointF() );
    }
  }

  painter->restore();
}

void QgsMeshLayerRenderer::renderScalarDataset()
{
  if ( mScalarDatasetValues.isEmpty() )
    return; // activeScalarDataset == NO_ACTIVE_MESH_DATASET

  if ( std::isnan( mScalarDatasetMinimum ) || std::isnan( mScalarDatasetMaximum ) )
    return; // only NODATA values

  const int groupIndex = mRendererSettings.activeScalarDatasetGroup();
  if ( groupIndex < 0 )
    return; // no shader

  std::unique_ptr< QgsScopedRuntimeProfile > renderProfile;
  if ( mEnableProfile )
  {
    renderProfile = std::make_unique< QgsScopedRuntimeProfile >( QObject::tr( "Rendering scalar datasets" ), QStringLiteral( "rendering" ) );
  }

  const QgsMeshRendererScalarSettings scalarSettings = mRendererSettings.scalarSettings( groupIndex );

  if ( ( mTriangularMesh.contains( QgsMesh::ElementType::Face ) ) &&
       ( mScalarDataType != QgsMeshDatasetGroupMetadata::DataType::DataOnEdges ) )
  {
    renderScalarDatasetOnFaces( scalarSettings );
  }

  if ( ( mTriangularMesh.contains( QgsMesh::ElementType::Edge ) ) &&
       ( mScalarDataType != QgsMeshDatasetGroupMetadata::DataType::DataOnFaces ) )
  {
    renderScalarDatasetOnEdges( scalarSettings );
  }
}

void QgsMeshLayerRenderer::renderScalarDatasetOnEdges( const QgsMeshRendererScalarSettings &scalarSettings )
{
  QgsRenderContext &context = *renderContext();
  const QVector<QgsMeshEdge> edges = mTriangularMesh.edges();
  const QVector<QgsMeshVertex> vertices = mTriangularMesh.vertices();
  const QList<int> edgesInExtent = mTriangularMesh.edgeIndexesForRectangle( context.mapExtent() );

  QgsInterpolatedLineRenderer edgePlotter;
  edgePlotter.setInterpolatedColor( QgsInterpolatedLineColor( scalarSettings.colorRampShader() ) );
  edgePlotter.setInterpolatedWidth( QgsInterpolatedLineWidth( scalarSettings.edgeStrokeWidth() ) );
  edgePlotter.setWidthUnit( scalarSettings.edgeStrokeWidthUnit() );

  for ( const int i : edgesInExtent )
  {
    if ( context.renderingStopped() )
      break;

    if ( i >= edges.size() )
      continue;

    const QgsMeshEdge &edge = edges[i];
    const int startVertexIndex = edge.first;
    const int endVertexIndex = edge.second;

    if ( ( startVertexIndex >= vertices.size() ) || endVertexIndex >= vertices.size() )
      continue;

    const QgsMeshVertex &startVertex = vertices[startVertexIndex];
    const QgsMeshVertex &endVertex = vertices[endVertexIndex];

    if ( mScalarDataType == QgsMeshDatasetGroupMetadata::DataType::DataOnEdges )
    {
      edgePlotter.render( mScalarDatasetValues[i], mScalarDatasetValues[i], startVertex, endVertex, context );
    }
    else
    {
      edgePlotter.render( mScalarDatasetValues[startVertexIndex], mScalarDatasetValues[endVertexIndex], startVertex, endVertex, context );
    }
  }
}

QColor QgsMeshLayerRenderer::colorAt( QgsColorRampShader *shader, double val ) const
{
  int r, g, b, a;
  if ( shader->shade( val, &r, &g, &b, &a ) )
  {
    return QColor( r, g, b, a );
  }
  return QColor();
}

QgsPointXY QgsMeshLayerRenderer::fractionPoint( const QgsPointXY &p1, const QgsPointXY &p2, double fraction ) const
{
  const QgsPointXY pt( p1.x() + fraction * ( p2.x() - p1.x() ),
                       p1.y() + fraction * ( p2.y() - p1.y() ) );
  return pt;
}

void QgsMeshLayerRenderer::renderScalarDatasetOnFaces( const QgsMeshRendererScalarSettings &scalarSettings )
{
  QgsRenderContext &context = *renderContext();

  QgsColorRampShader *fcn = new QgsColorRampShader( scalarSettings.colorRampShader() );
  QgsRasterShader *sh = new QgsRasterShader();
  sh->setRasterShaderFunction( fcn );  // takes ownership of fcn
  QgsMeshLayerInterpolator interpolator( mTriangularMesh,
                                         mScalarDatasetValues,
                                         mScalarActiveFaceFlagValues,
                                         mScalarDataType,
                                         context,
                                         mOutputSize );
  interpolator.setSpatialIndexActive( mIsMeshSimplificationActive );
  interpolator.setElevationMapSettings( mRenderElevationMap, mElevationScale, mElevationOffset );
  QgsSingleBandPseudoColorRenderer renderer( &interpolator, 0, sh );  // takes ownership of sh
  renderer.setClassificationMin( scalarSettings.classificationMinimum() );
  renderer.setClassificationMax( scalarSettings.classificationMaximum() );
  renderer.setOpacity( scalarSettings.opacity() );

  std::unique_ptr<QgsRasterBlock> bl( renderer.block( 0, context.mapExtent(), mOutputSize.width(), mOutputSize.height(), mFeedback.get() ) );
  QImage img = bl->image();
  img.setDevicePixelRatio( context.devicePixelRatio() );

  context.painter()->drawImage( 0, 0, img );
}

void QgsMeshLayerRenderer::renderVectorDataset()
{
  const int groupIndex = mRendererSettings.activeVectorDatasetGroup();
  if ( groupIndex < 0 )
    return;

  if ( !mVectorDatasetValues.isValid() )
    return; // no data at all

  if ( std::isnan( mVectorDatasetMagMinimum ) || std::isnan( mVectorDatasetMagMaximum ) )
    return; // only NODATA values

  if ( !( mVectorDatasetMagMaximum > 0 ) )
    return; //all vector are null vector

  std::unique_ptr< QgsScopedRuntimeProfile > renderProfile;
  if ( mEnableProfile )
  {
    renderProfile = std::make_unique< QgsScopedRuntimeProfile >( QObject::tr( "Rendering vector datasets" ), QStringLiteral( "rendering" ) );
  }

  std::unique_ptr<QgsMeshVectorRenderer> renderer( QgsMeshVectorRenderer::makeVectorRenderer(
        mTriangularMesh,
        mVectorDatasetValues,
        mVectorActiveFaceFlagValues,
        mVectorDatasetValuesMag,
        mVectorDatasetMagMaximum,
        mVectorDatasetMagMinimum,
        mVectorDataType,
        mRendererSettings.vectorSettings( groupIndex ),
        *renderContext(),
        mLayerExtent,
        mFeedback.get(),
        mOutputSize ) );

  if ( renderer )
    renderer->draw();
}

void QgsMeshLayerRenderer::prepareLabeling( QgsMeshLayer *layer, QSet<QString> &attributeNames )
{
  QgsRenderContext &context = *renderContext();

  if ( QgsLabelingEngine *engine = context.labelingEngine() )
  {
    if ( layer->labelsEnabled() )
    {
      mLabelProvider = layer->labeling()->provider( layer );
      if ( mLabelProvider )
      {
        auto c = context.expressionContext();

        c.appendScope( QgsExpressionContextUtils::meshExpressionScope( mLabelProvider->labelFaces() ? QgsMesh::Face : QgsMesh::Vertex ) );
        c.lastScope()->setVariable( QStringLiteral( "_native_mesh" ), QVariant::fromValue( mNativeMesh ) );
        context.setExpressionContext( c );

        engine->addProvider( mLabelProvider );
        if ( !mLabelProvider->prepare( context, attributeNames ) )
        {
          engine->removeProvider( mLabelProvider );
          mLabelProvider = nullptr; // deleted by engine
        }
      }
    }
  }
}

void QgsMeshLayerRenderer::registerLabelFeatures()
{
  if ( !mLabelProvider )
    return;

  QgsRenderContext &context = *renderContext();

  QgsExpressionContextScope *scope = context.expressionContext().activeScopeForVariable( QStringLiteral( "_native_mesh" ) );

  const QList<int> trianglesInExtent = mTriangularMesh.faceIndexesForRectangle( renderContext()->mapExtent() );

  if ( mLabelProvider->labelFaces() )
  {
    if ( !mTriangularMesh.contains( QgsMesh::ElementType::Face ) )
      return;
    const QSet<int> nativeFacesInExtent = QgsMeshUtils::nativeFacesFromTriangles( trianglesInExtent,
                                          mTriangularMesh.trianglesToNativeFaces() );

    for ( const int i : nativeFacesInExtent )
    {
      if ( context.renderingStopped() )
        break;

      if ( i < 0 || i >= mNativeMesh.faces.count() )
        continue;

      scope->setVariable( QStringLiteral( "_mesh_face_index" ), i, false );

      QgsFeature f( i );
      QgsGeometry geom = QgsMeshUtils::toGeometry( mNativeMesh.face( i ), mNativeMesh.vertices );
      f.setGeometry( geom );
      mLabelProvider->registerFeature( f, context );
    }
  }
  else
  {
    if ( !mTriangularMesh.contains( QgsMesh::ElementType::Vertex ) )
      return;
    const QSet<int> nativeVerticesInExtent = QgsMeshUtils::nativeVerticesFromTriangles( trianglesInExtent,
        mTriangularMesh.triangles() );

    for ( const int i : nativeVerticesInExtent )
    {
      if ( context.renderingStopped() )
        break;

      if ( i < 0 || i >= mNativeMesh.vertexCount() )
        continue;

      scope->setVariable( QStringLiteral( "_mesh_vertex_index" ), i, false );

      QgsFeature f( i );
      QgsGeometry geom = QgsGeometry( new QgsPoint( mNativeMesh.vertex( i ) ) );
      f.setGeometry( geom );
      mLabelProvider->registerFeature( f, context );
    }
  }
}
