/***************************************************************************
  qgs3dutils.cpp
  --------------------------------------
  Date                 : July 2017
  Copyright            : (C) 2017 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgs3dutils.h"

#include "qgs3dmapcanvas.h"
#include "qgs3dmapscene.h"
#include "qgsabstract3dengine.h"
#include "qgsabstractgeometry.h"
#include "qgsabstractterrainsettings.h"
#include "qgsapplication.h"
#include "qgscameracontroller.h"
#include "qgschunkedentity.h"
#include "qgsfeature.h"
#include "qgsfeatureiterator.h"
#include "qgsfeaturerequest.h"
#include "qgsfeedback.h"
#include "qgsglobechunkedentity.h"
#include "qgslinestring.h"
#include "qgsoffscreen3dengine.h"
#include "qgspointcloud3dsymbol.h"
#include "qgspointcloudattributebyramprenderer.h"
#include "qgspointcloudclassifiedrenderer.h"
#include "qgspointcloudlayer3drenderer.h"
#include "qgspointcloudrenderer.h"
#include "qgspointcloudrgbrenderer.h"
#include "qgspolygon.h"
#include "qgsraycastcontext.h"
#include "qgsraycastresult.h"
#include "qgsterrainentity.h"
#include "qgsterraingenerator.h"
#include "qgsvectorlayer.h"

#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <Qt3DCore/QBuffer>
#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DLogic/QFrameAction>
#include <Qt3DRender/QRenderSettings>
#include <QtMath>

#if !defined( Q_OS_MAC )
#include <GL/gl.h>
#endif


// declared here as Qgs3DTypes has no cpp file
const char *Qgs3DTypes::PROP_NAME_3D_RENDERER_FLAG = "PROP_NAME_3D_RENDERER_FLAG";

void Qgs3DUtils::waitForFrame( QgsAbstract3DEngine &engine, Qgs3DMapScene *scene )
{
  // Set policy to always render frame, so we don't wait forever.
  Qt3DRender::QRenderSettings::RenderPolicy oldPolicy = engine.renderSettings()->renderPolicy();
  engine.renderSettings()->setRenderPolicy( Qt3DRender::QRenderSettings::RenderPolicy::Always );

  // Wait for at least one frame to render
  Qt3DLogic::QFrameAction *frameAction = new Qt3DLogic::QFrameAction();
  scene->addComponent( frameAction );
  QEventLoop evLoop;
  QObject::connect( frameAction, &Qt3DLogic::QFrameAction::triggered, &evLoop, &QEventLoop::quit );
  evLoop.exec();
  scene->removeComponent( frameAction );
  frameAction->deleteLater();

  engine.renderSettings()->setRenderPolicy( oldPolicy );
}

void Qgs3DUtils::waitForEntitiesLoaded( Qgs3DMapScene *scene )
{
  while ( scene->totalPendingJobsCount() > 0 )
  {
    QgsApplication::processEvents();
  }
}

QImage Qgs3DUtils::captureSceneImage( QgsAbstract3DEngine &engine, Qgs3DMapScene *scene )
{
  QImage resImage;
  QEventLoop evLoop;

  // We need to change render policy to RenderPolicy::Always, since otherwise render capture node won't work
  engine.renderSettings()->setRenderPolicy( Qt3DRender::QRenderSettings::RenderPolicy::Always );

  waitForFrame( engine, scene );

  auto saveImageFcn = [&evLoop, &resImage]( const QImage &img ) {
    resImage = img;
    evLoop.quit();
  };

  const QMetaObject::Connection conn1 = QObject::connect( &engine, &QgsAbstract3DEngine::imageCaptured, saveImageFcn );
  QMetaObject::Connection conn2;

  auto requestImageFcn = [&engine, scene] {
    if ( scene->sceneState() == Qgs3DMapScene::Ready )
    {
      engine.renderSettings()->setRenderPolicy( Qt3DRender::QRenderSettings::RenderPolicy::OnDemand );
      engine.requestCaptureImage();
    }
  };

  if ( scene->sceneState() == Qgs3DMapScene::Ready )
  {
    requestImageFcn();
  }
  else
  {
    // first wait until scene is loaded
    conn2 = QObject::connect( scene, &Qgs3DMapScene::sceneStateChanged, requestImageFcn );
  }

  evLoop.exec();

  QObject::disconnect( conn1 );
  if ( conn2 )
    QObject::disconnect( conn2 );

  engine.renderSettings()->setRenderPolicy( Qt3DRender::QRenderSettings::RenderPolicy::OnDemand );
  return resImage;
}

QImage Qgs3DUtils::captureSceneDepthBuffer( QgsAbstract3DEngine &engine, Qgs3DMapScene *scene )
{
  QImage resImage;
  QEventLoop evLoop;

  // We need to change render policy to RenderPolicy::Always, since otherwise render capture node won't work
  engine.renderSettings()->setRenderPolicy( Qt3DRender::QRenderSettings::RenderPolicy::Always );

  auto requestImageFcn = [&engine, scene] {
    if ( scene->sceneState() == Qgs3DMapScene::Ready )
    {
      engine.renderSettings()->setRenderPolicy( Qt3DRender::QRenderSettings::RenderPolicy::OnDemand );
      engine.requestDepthBufferCapture();
    }
  };

  auto saveImageFcn = [&evLoop, &resImage]( const QImage &img ) {
    resImage = img;
    evLoop.quit();
  };

  QMetaObject::Connection conn1 = QObject::connect( &engine, &QgsAbstract3DEngine::depthBufferCaptured, saveImageFcn );
  QMetaObject::Connection conn2;

  // Make sure once-per-frame functions run
  waitForFrame( engine, scene );
  if ( scene->sceneState() == Qgs3DMapScene::Ready )
  {
    requestImageFcn();
  }
  else
  {
    // first wait until scene is loaded
    conn2 = QObject::connect( scene, &Qgs3DMapScene::sceneStateChanged, requestImageFcn );
  }

  evLoop.exec();

  QObject::disconnect( conn1 );
  if ( conn2 )
    QObject::disconnect( conn2 );

  engine.renderSettings()->setRenderPolicy( Qt3DRender::QRenderSettings::RenderPolicy::OnDemand );
  return resImage;
}


double Qgs3DUtils::calculateEntityGpuMemorySize( Qt3DCore::QEntity *entity )
{
  long long usedGpuMemory = 0;
  for ( Qt3DCore::QBuffer *buffer : entity->findChildren<Qt3DCore::QBuffer *>() )
  {
    usedGpuMemory += buffer->data().size();
  }
  for ( Qt3DRender::QTexture2D *tex : entity->findChildren<Qt3DRender::QTexture2D *>() )
  {
    // TODO : lift the assumption that the texture is RGBA
    usedGpuMemory += static_cast< long long >( tex->width() ) * static_cast< long long >( tex->height() ) * 4;
  }
  return usedGpuMemory / 1024.0 / 1024.0;
}


bool Qgs3DUtils::exportAnimation( const Qgs3DAnimationSettings &animationSettings, Qgs3DMapSettings &mapSettings, int framesPerSecond, const QString &outputDirectory, const QString &fileNameTemplate, const QSize &outputSize, QString &error, QgsFeedback *feedback )
{
  if ( animationSettings.keyFrames().size() < 2 )
  {
    error = QObject::tr( "Unable to export 3D animation. Add at least 2 keyframes" );
    return false;
  }

  const float duration = animationSettings.duration(); //in seconds
  if ( duration <= 0 )
  {
    error = QObject::tr( "Unable to export 3D animation (invalid duration)." );
    return false;
  }

  float time = 0;
  int frameNo = 0;
  const int totalFrames = static_cast<int>( duration * framesPerSecond );

  if ( fileNameTemplate.isEmpty() )
  {
    error = QObject::tr( "Filename template is empty" );
    return false;
  }

  const int numberOfDigits = fileNameTemplate.count( QLatin1Char( '#' ) );
  if ( numberOfDigits < 0 )
  {
    error = QObject::tr( "Wrong filename template format (must contain #)" );
    return false;
  }
  const QString token( numberOfDigits, QLatin1Char( '#' ) );
  if ( !fileNameTemplate.contains( token ) )
  {
    error = QObject::tr( "Filename template must contain all # placeholders in one continuous group." );
    return false;
  }

  if ( !QDir().exists( outputDirectory ) )
  {
    if ( !QDir().mkpath( outputDirectory ) )
    {
      error = QObject::tr( "Output directory could not be created." );
      return false;
    }
  }

  QgsOffscreen3DEngine engine;
  engine.setSize( outputSize );
  Qgs3DMapScene *scene = new Qgs3DMapScene( mapSettings, &engine );
  engine.setRootEntity( scene );
  // We need to change render policy to RenderPolicy::Always, since otherwise render capture node won't work
  engine.renderSettings()->setRenderPolicy( Qt3DRender::QRenderSettings::RenderPolicy::Always );

  while ( time <= duration )
  {
    if ( feedback )
    {
      if ( feedback->isCanceled() )
      {
        error = QObject::tr( "Export canceled" );
        return false;
      }
      feedback->setProgress( frameNo / static_cast<double>( totalFrames ) * 100 );
    }
    ++frameNo;

    const Qgs3DAnimationSettings::Keyframe kf = animationSettings.interpolate( time );
    scene->cameraController()->setLookingAtMapPoint( kf.point, kf.dist, kf.pitch, kf.yaw );

    QString fileName( fileNameTemplate );
    const QString frameNoPaddedLeft( u"%1"_s.arg( frameNo, numberOfDigits, 10, QChar( '0' ) ) ); // e.g. 0001
    fileName.replace( token, frameNoPaddedLeft );
    const QString path = QDir( outputDirectory ).filePath( fileName );

    const QImage img = Qgs3DUtils::captureSceneImage( engine, scene );

    img.save( path );

    time += 1.0f / static_cast<float>( framesPerSecond );
  }

  return true;
}


int Qgs3DUtils::maxZoomLevel( double tile0width, double tileResolution, double maxError )
{
  if ( maxError <= 0 || tileResolution <= 0 || tile0width <= 0 )
    return 0; // invalid input

  // derived from:
  // tile width [map units] = tile0width / 2^zoomlevel
  // tile error [map units] = tile width / tile resolution
  // + re-arranging to get zoom level if we know tile error we want to get
  const double zoomLevel = -log( tileResolution * maxError / tile0width ) / log( 2 );
  return round( zoomLevel ); // we could use ceil() here if we wanted to always get to the desired error
}

QString Qgs3DUtils::altClampingToString( Qgis::AltitudeClamping altClamp )
{
  switch ( altClamp )
  {
    case Qgis::AltitudeClamping::Absolute:
      return u"absolute"_s;
    case Qgis::AltitudeClamping::Relative:
      return u"relative"_s;
    case Qgis::AltitudeClamping::Terrain:
      return u"terrain"_s;
  }
  BUILTIN_UNREACHABLE
}


Qgis::AltitudeClamping Qgs3DUtils::altClampingFromString( const QString &str )
{
  if ( str == "absolute"_L1 )
    return Qgis::AltitudeClamping::Absolute;
  else if ( str == "terrain"_L1 )
    return Qgis::AltitudeClamping::Terrain;
  else // "relative"  (default)
    return Qgis::AltitudeClamping::Relative;
}


QString Qgs3DUtils::altBindingToString( Qgis::AltitudeBinding altBind )
{
  switch ( altBind )
  {
    case Qgis::AltitudeBinding::Vertex:
      return u"vertex"_s;
    case Qgis::AltitudeBinding::Centroid:
      return u"centroid"_s;
  }
  BUILTIN_UNREACHABLE
}


Qgis::AltitudeBinding Qgs3DUtils::altBindingFromString( const QString &str )
{
  if ( str == "vertex"_L1 )
    return Qgis::AltitudeBinding::Vertex;
  else // "centroid"  (default)
    return Qgis::AltitudeBinding::Centroid;
}

QString Qgs3DUtils::cullingModeToString( Qgs3DTypes::CullingMode mode )
{
  switch ( mode )
  {
    case Qgs3DTypes::NoCulling:
      return u"no-culling"_s;
    case Qgs3DTypes::Front:
      return u"front"_s;
    case Qgs3DTypes::Back:
      return u"back"_s;
    case Qgs3DTypes::FrontAndBack:
      return u"front-and-back"_s;
  }
  BUILTIN_UNREACHABLE
}

Qgs3DTypes::CullingMode Qgs3DUtils::cullingModeFromString( const QString &str )
{
  if ( str == "front"_L1 )
    return Qgs3DTypes::Front;
  else if ( str == "back"_L1 )
    return Qgs3DTypes::Back;
  else if ( str == "front-and-back"_L1 )
    return Qgs3DTypes::FrontAndBack;
  else
    return Qgs3DTypes::NoCulling;
}

float Qgs3DUtils::clampAltitude( const QgsPoint &p, Qgis::AltitudeClamping altClamp, Qgis::AltitudeBinding altBind, float offset, const QgsPoint &centroid, const Qgs3DRenderContext &context )
{
  float terrainZ = 0;
  switch ( altClamp )
  {
    case Qgis::AltitudeClamping::Relative:
    case Qgis::AltitudeClamping::Terrain:
    {
      const QgsPointXY pt = altBind == Qgis::AltitudeBinding::Vertex ? p : centroid;
      terrainZ = context.terrainRenderingEnabled() && context.terrainGenerator() ? context.terrainGenerator()->heightAt( pt.x(), pt.y(), context ) : 0;
      break;
    }

    case Qgis::AltitudeClamping::Absolute:
      break;
  }

  float geomZ = 0;
  if ( p.is3D() )
  {
    switch ( altClamp )
    {
      case Qgis::AltitudeClamping::Absolute:
      case Qgis::AltitudeClamping::Relative:
        geomZ = p.z();
        break;

      case Qgis::AltitudeClamping::Terrain:
        break;
    }
  }

  const float z = ( terrainZ + geomZ ) * static_cast<float>( context.terrainSettings()->verticalScale() ) + offset;
  return z;
}

void Qgs3DUtils::clampAltitudes( QgsLineString *lineString, Qgis::AltitudeClamping altClamp, Qgis::AltitudeBinding altBind, const QgsPoint &centroid, float offset, const Qgs3DRenderContext &context )
{
  for ( int i = 0; i < lineString->nCoordinates(); ++i )
  {
    float terrainZ = 0;
    switch ( altClamp )
    {
      case Qgis::AltitudeClamping::Relative:
      case Qgis::AltitudeClamping::Terrain:
      {
        QgsPointXY pt;
        switch ( altBind )
        {
          case Qgis::AltitudeBinding::Vertex:
            pt.setX( lineString->xAt( i ) );
            pt.setY( lineString->yAt( i ) );
            break;

          case Qgis::AltitudeBinding::Centroid:
            pt.set( centroid.x(), centroid.y() );
            break;
        }

        terrainZ = context.terrainRenderingEnabled() && context.terrainGenerator() ? context.terrainGenerator()->heightAt( pt.x(), pt.y(), context ) : 0;
        break;
      }

      case Qgis::AltitudeClamping::Absolute:
        break;
    }

    float geomZ = 0;

    switch ( altClamp )
    {
      case Qgis::AltitudeClamping::Absolute:
      case Qgis::AltitudeClamping::Relative:
        geomZ = lineString->zAt( i );
        break;

      case Qgis::AltitudeClamping::Terrain:
        break;
    }

    const float z = ( terrainZ + geomZ ) * static_cast<float>( context.terrainSettings()->verticalScale() ) + offset;
    lineString->setZAt( i, z );
  }
}


bool Qgs3DUtils::clampAltitudes( QgsPolygon *polygon, Qgis::AltitudeClamping altClamp, Qgis::AltitudeBinding altBind, float offset, const Qgs3DRenderContext &context )
{
  if ( !polygon->is3D() )
    polygon->addZValue( 0 );

  QgsPoint centroid;
  switch ( altBind )
  {
    case Qgis::AltitudeBinding::Vertex:
      break;

    case Qgis::AltitudeBinding::Centroid:
      centroid = polygon->centroid();
      break;
  }

  QgsCurve *curve = const_cast<QgsCurve *>( polygon->exteriorRing() );
  QgsLineString *lineString = qgsgeometry_cast<QgsLineString *>( curve );
  if ( !lineString )
    return false;

  clampAltitudes( lineString, altClamp, altBind, centroid, offset, context );

  for ( int i = 0; i < polygon->numInteriorRings(); ++i )
  {
    QgsCurve *curve = const_cast<QgsCurve *>( polygon->interiorRing( i ) );
    QgsLineString *lineString = qgsgeometry_cast<QgsLineString *>( curve );
    if ( !lineString )
      return false;

    clampAltitudes( lineString, altClamp, altBind, centroid, offset, context );
  }
  return true;
}


QString Qgs3DUtils::matrix4x4toString( const QMatrix4x4 &m )
{
  const float *d = m.constData();
  QStringList elems;
  elems.reserve( 16 );
  for ( int i = 0; i < 16; ++i )
    elems << QString::number( d[i] );
  return elems.join( ' ' );
}

QMatrix4x4 Qgs3DUtils::stringToMatrix4x4( const QString &str )
{
  QMatrix4x4 m;
  float *d = m.data();
  QStringList elems = str.split( ' ' );
  for ( int i = 0; i < 16; ++i )
    d[i] = elems[i].toFloat();
  return m;
}

void Qgs3DUtils::extractPointPositions( const QgsFeature &f, const Qgs3DRenderContext &context, const QgsVector3D &chunkOrigin, Qgis::AltitudeClamping altClamp, QVector<QVector3D> &positions )
{
  const QgsAbstractGeometry *g = f.geometry().constGet();
  for ( auto it = g->vertices_begin(); it != g->vertices_end(); ++it )
  {
    const QgsPoint pt = *it;
    float geomZ = 0;
    if ( pt.is3D() )
    {
      geomZ = pt.z();
    }
    const float terrainZ = context.terrainRenderingEnabled() && context.terrainGenerator() ? static_cast<float>( context.terrainGenerator()->heightAt( pt.x(), pt.y(), context ) * context.terrainSettings()->verticalScale() ) : 0.f;
    float h = 0.0f;
    switch ( altClamp )
    {
      case Qgis::AltitudeClamping::Absolute:
        h = geomZ;
        break;
      case Qgis::AltitudeClamping::Terrain:
        h = terrainZ;
        break;
      case Qgis::AltitudeClamping::Relative:
        h = terrainZ + geomZ;
        break;
    }
    positions.append( QVector3D(
      static_cast<float>( pt.x() - chunkOrigin.x() ),
      static_cast<float>( pt.y() - chunkOrigin.y() ),
      h
    ) );
    QgsDebugMsgLevel( u"%1 %2 %3"_s.arg( positions.last().x() ).arg( positions.last().y() ).arg( positions.last().z() ), 2 );
  }
}

/**
 * copied from https://searchcode.com/codesearch/view/35195518/
 * qt3d /src/threed/painting/qglpainter.cpp
 * no changes in the code
 */
static inline uint outcode( QVector4D v )
{
  // For a discussion of outcodes see pg 388 Dunn & Parberry.
  // For why you can't just test if the point is in a bounding box
  // consider the case where a view frustum with view-size 1.5 x 1.5
  // is tested against a 2x2 box which encloses the near-plane, while
  // all the points in the box are outside the frustum.
  // TODO: optimise this with assembler - according to D&P this can
  // be done in one line of assembler on some platforms
  uint code = 0;
  if ( v.x() < -v.w() )
    code |= 0x01;
  if ( v.x() > v.w() )
    code |= 0x02;
  if ( v.y() < -v.w() )
    code |= 0x04;
  if ( v.y() > v.w() )
    code |= 0x08;
  if ( v.z() < -v.w() )
    code |= 0x10;
  if ( v.z() > v.w() )
    code |= 0x20;
  return code;
}


/**
 * coarse box vs frustum test for culling.
 * corners of oriented box are transformed to clip space
 * and there is a test that all points are on the wrong side of the same plane
 * see http://www.lighthouse3d.com/tutorials/view-frustum-culling/geometric-approach-testing-boxes/
 *
 * should be equivalent to https://searchcode.com/codesearch/view/35195518/
 * qt3d /src/threed/painting/qglpainter.cpp
 * bool QGLPainter::isCullable(const QBox3D& box) const
 */
bool Qgs3DUtils::isCullable( const QgsAABB &bbox, const QMatrix4x4 &viewProjectionMatrix )
{
  uint out = 0xff;

  for ( int i = 0; i < 8; ++i )
  {
    const QVector4D p( ( ( i >> 0 ) & 1 ) ? bbox.xMin : bbox.xMax, ( ( i >> 1 ) & 1 ) ? bbox.yMin : bbox.yMax, ( ( i >> 2 ) & 1 ) ? bbox.zMin : bbox.zMax, 1 );
    const QVector4D pc = viewProjectionMatrix * p;

    // if the logical AND of all the outcodes is non-zero then the BB is
    // definitely outside the view frustum.
    out = out & outcode( pc );
  }
  return out;
}

QgsVector3D Qgs3DUtils::mapToWorldCoordinates( const QgsVector3D &mapCoords, const QgsVector3D &origin )
{
  return QgsVector3D( mapCoords.x() - origin.x(), mapCoords.y() - origin.y(), mapCoords.z() - origin.z() );
}

QgsVector3D Qgs3DUtils::worldToMapCoordinates( const QgsVector3D &worldCoords, const QgsVector3D &origin )
{
  return QgsVector3D( worldCoords.x() + origin.x(), worldCoords.y() + origin.y(), worldCoords.z() + origin.z() );
}

QgsRectangle Qgs3DUtils::tryReprojectExtent2D( const QgsRectangle &extent, const QgsCoordinateReferenceSystem &crs1, const QgsCoordinateReferenceSystem &crs2, const QgsCoordinateTransformContext &context )
{
  QgsRectangle extentMapCrs( extent );
  if ( crs1 != crs2 )
  {
    // reproject if necessary
    QgsCoordinateTransform ct( crs1, crs2, context );
    ct.setBallparkTransformsAreAppropriate( true );
    try
    {
      extentMapCrs = ct.transformBoundingBox( extentMapCrs );
    }
    catch ( const QgsCsException & )
    {
      // bad luck, can't reproject for some reason
      QgsDebugError( u"3D utils: transformation of extent failed: "_s + extentMapCrs.toString( -1 ) );
    }
  }
  return extentMapCrs;
}

QgsAABB Qgs3DUtils::layerToWorldExtent( const QgsRectangle &extent, double zMin, double zMax, const QgsCoordinateReferenceSystem &layerCrs, const QgsVector3D &mapOrigin, const QgsCoordinateReferenceSystem &mapCrs, const QgsCoordinateTransformContext &context )
{
  const QgsRectangle extentMapCrs( Qgs3DUtils::tryReprojectExtent2D( extent, layerCrs, mapCrs, context ) );
  return mapToWorldExtent( extentMapCrs, zMin, zMax, mapOrigin );
}

QgsRectangle Qgs3DUtils::worldToLayerExtent( const QgsAABB &bbox, const QgsCoordinateReferenceSystem &layerCrs, const QgsVector3D &mapOrigin, const QgsCoordinateReferenceSystem &mapCrs, const QgsCoordinateTransformContext &context )
{
  const QgsRectangle extentMap = worldToMapExtent( bbox, mapOrigin );
  return Qgs3DUtils::tryReprojectExtent2D( extentMap, mapCrs, layerCrs, context );
}

QgsAABB Qgs3DUtils::mapToWorldExtent( const QgsRectangle &extent, double zMin, double zMax, const QgsVector3D &mapOrigin )
{
  const QgsVector3D extentMin3D( extent.xMinimum(), extent.yMinimum(), zMin );
  const QgsVector3D extentMax3D( extent.xMaximum(), extent.yMaximum(), zMax );
  const QgsVector3D worldExtentMin3D = mapToWorldCoordinates( extentMin3D, mapOrigin );
  const QgsVector3D worldExtentMax3D = mapToWorldCoordinates( extentMax3D, mapOrigin );
  QgsAABB rootBbox( worldExtentMin3D.x(), worldExtentMin3D.y(), worldExtentMin3D.z(), worldExtentMax3D.x(), worldExtentMax3D.y(), worldExtentMax3D.z() );
  return rootBbox;
}

QgsAABB Qgs3DUtils::mapToWorldExtent( const QgsBox3D &box3D, const QgsVector3D &mapOrigin )
{
  const QgsVector3D extentMin3D( box3D.xMinimum(), box3D.yMinimum(), box3D.zMinimum() );
  const QgsVector3D extentMax3D( box3D.xMaximum(), box3D.yMaximum(), box3D.zMaximum() );
  const QgsVector3D worldExtentMin3D = mapToWorldCoordinates( extentMin3D, mapOrigin );
  const QgsVector3D worldExtentMax3D = mapToWorldCoordinates( extentMax3D, mapOrigin );
  // casting to float should be ok, assuming that the map origin is not too far from the box
  return QgsAABB( static_cast<float>( worldExtentMin3D.x() ), static_cast<float>( worldExtentMin3D.y() ), static_cast<float>( worldExtentMin3D.z() ), static_cast<float>( worldExtentMax3D.x() ), static_cast<float>( worldExtentMax3D.y() ), static_cast<float>( worldExtentMax3D.z() ) );
}

QgsRectangle Qgs3DUtils::worldToMapExtent( const QgsAABB &bbox, const QgsVector3D &mapOrigin )
{
  const QgsVector3D worldExtentMin3D = Qgs3DUtils::worldToMapCoordinates( QgsVector3D( bbox.xMin, bbox.yMin, bbox.zMin ), mapOrigin );
  const QgsVector3D worldExtentMax3D = Qgs3DUtils::worldToMapCoordinates( QgsVector3D( bbox.xMax, bbox.yMax, bbox.zMax ), mapOrigin );
  const QgsRectangle extentMap( worldExtentMin3D.x(), worldExtentMin3D.y(), worldExtentMax3D.x(), worldExtentMax3D.y() );
  // we discard zMin/zMax here because we don't need it
  return extentMap;
}


QgsVector3D Qgs3DUtils::transformWorldCoordinates( const QgsVector3D &worldPoint1, const QgsVector3D &origin1, const QgsCoordinateReferenceSystem &crs1, const QgsVector3D &origin2, const QgsCoordinateReferenceSystem &crs2, const QgsCoordinateTransformContext &context )
{
  const QgsVector3D mapPoint1 = worldToMapCoordinates( worldPoint1, origin1 );
  QgsVector3D mapPoint2 = mapPoint1;
  if ( crs1 != crs2 )
  {
    // reproject if necessary
    const QgsCoordinateTransform ct( crs1, crs2, context );
    try
    {
      const QgsPointXY pt = ct.transform( QgsPointXY( mapPoint1.x(), mapPoint1.y() ) );
      mapPoint2.set( pt.x(), pt.y(), mapPoint1.z() );
    }
    catch ( const QgsCsException & )
    {
      // bad luck, can't reproject for some reason
    }
  }
  return mapToWorldCoordinates( mapPoint2, origin2 );
}

void Qgs3DUtils::estimateVectorLayerZRange( QgsVectorLayer *layer, double &zMin, double &zMax )
{
  if ( !QgsWkbTypes::hasZ( layer->wkbType() ) )
  {
    zMin = 0;
    zMax = 0;
    return;
  }

  zMin = std::numeric_limits<double>::max();
  zMax = std::numeric_limits<double>::lowest();

  QgsFeature f;
  QgsFeatureIterator it = layer->getFeatures( QgsFeatureRequest().setNoAttributes().setLimit( 100 ) );
  while ( it.nextFeature( f ) )
  {
    const QgsGeometry g = f.geometry();
    for ( auto vit = g.vertices_begin(); vit != g.vertices_end(); ++vit )
    {
      const double z = ( *vit ).z();
      if ( z < zMin )
        zMin = z;
      if ( z > zMax )
        zMax = z;
    }
  }

  if ( zMin == std::numeric_limits<double>::max() && zMax == std::numeric_limits<double>::lowest() )
  {
    zMin = 0;
    zMax = 0;
  }
}

QgsPhongMaterialSettings Qgs3DUtils::phongMaterialFromQt3DComponent( Qt3DExtras::QPhongMaterial *material )
{
  QgsPhongMaterialSettings settings;
  settings.setAmbient( material->ambient() );
  settings.setDiffuse( material->diffuse() );
  settings.setSpecular( material->specular() );
  settings.setShininess( material->shininess() );
  return settings;
}

QgsRay3D Qgs3DUtils::rayFromScreenPoint( const QPoint &point, const QSize &windowSize, Qt3DRender::QCamera *camera )
{
  const QVector3D deviceCoords( point.x(), point.y(), 0.0 );
  // normalized device coordinates
  const QVector3D normDeviceCoords( 2.0 * deviceCoords.x() / windowSize.width() - 1.0f, 1.0f - 2.0 * deviceCoords.y() / windowSize.height(), camera->nearPlane() );
  // clip coordinates
  const QVector4D rayClip( normDeviceCoords.x(), normDeviceCoords.y(), -1.0, 0.0 );

  const QMatrix4x4 invertedProjMatrix = camera->projectionMatrix().inverted();
  const QMatrix4x4 invertedViewMatrix = camera->viewMatrix().inverted();

  // ray direction in view coordinates
  QVector4D rayDirView = invertedProjMatrix * rayClip;
  // ray origin in world coordinates
  const QVector4D rayOriginWorld = invertedViewMatrix * QVector4D( 0.0f, 0.0f, 0.0f, 1.0f );

  // ray direction in world coordinates
  rayDirView.setZ( -1.0f );
  rayDirView.setW( 0.0f );
  const QVector4D rayDirWorld4D = invertedViewMatrix * rayDirView;
  QVector3D rayDirWorld( rayDirWorld4D.x(), rayDirWorld4D.y(), rayDirWorld4D.z() );
  rayDirWorld = rayDirWorld.normalized();

  return QgsRay3D( QVector3D( rayOriginWorld ), rayDirWorld );
}

QVector3D Qgs3DUtils::screenPointToWorldPos( const QPoint &screenPoint, double depth, const QSize &screenSize, Qt3DRender::QCamera *camera )
{
  // Transform pixel coordinates and [0.0, 1.0]-range sampled depth to [-1.0, 1.0]
  // normalised device coordinates used by projection matrix.
  QVector3D screenPointNdc {
    ( static_cast<float>( screenPoint.x() ) / ( static_cast<float>( screenSize.width() ) / 2.0f ) - 1.0f ),
    -( static_cast<float>( screenPoint.y() ) / ( static_cast<float>( screenSize.height() ) / 2.0f ) - 1.0f ),
    static_cast<float>( depth * 2 - 1 ),
  };

  // Apply inverse of projection matrix, then view matrix, to get from NDC to world coords.
  return camera->viewMatrix().inverted() * camera->projectionMatrix().inverted() * screenPointNdc;
}

void Qgs3DUtils::pitchAndYawFromViewVector( QVector3D vect, double &pitch, double &yaw )
{
  vect.normalize();

  pitch = qRadiansToDegrees( qAcos( vect.y() ) );
  yaw = qRadiansToDegrees( qAtan2( -vect.z(), vect.x() ) ) + 90;
}

QVector2D Qgs3DUtils::screenToTextureCoordinates( QVector2D screenXY, QSize winSize )
{
  return QVector2D( screenXY.x() / winSize.width(), 1 - screenXY.y() / winSize.width() );
}

QVector2D Qgs3DUtils::textureToScreenCoordinates( QVector2D textureXY, QSize winSize )
{
  return QVector2D( textureXY.x() * winSize.width(), ( 1 - textureXY.y() ) * winSize.height() );
}

std::unique_ptr<QgsPointCloudLayer3DRenderer> Qgs3DUtils::convert2DPointCloudRendererTo3D( QgsPointCloudRenderer *renderer )
{
  if ( !renderer )
    return nullptr;

  std::unique_ptr<QgsPointCloud3DSymbol> symbol3D;
  if ( renderer->type() == "ramp"_L1 )
  {
    const QgsPointCloudAttributeByRampRenderer *renderer2D = qgis::down_cast<const QgsPointCloudAttributeByRampRenderer *>( renderer );
    symbol3D = std::make_unique<QgsColorRampPointCloud3DSymbol>();
    QgsColorRampPointCloud3DSymbol *symbol = static_cast<QgsColorRampPointCloud3DSymbol *>( symbol3D.get() );
    symbol->setAttribute( renderer2D->attribute() );
    symbol->setColorRampShaderMinMax( renderer2D->minimum(), renderer2D->maximum() );
    symbol->setColorRampShader( renderer2D->colorRampShader() );
  }
  else if ( renderer->type() == "rgb"_L1 )
  {
    const QgsPointCloudRgbRenderer *renderer2D = qgis::down_cast<const QgsPointCloudRgbRenderer *>( renderer );
    symbol3D = std::make_unique<QgsRgbPointCloud3DSymbol>();
    QgsRgbPointCloud3DSymbol *symbol = static_cast<QgsRgbPointCloud3DSymbol *>( symbol3D.get() );
    symbol->setRedAttribute( renderer2D->redAttribute() );
    symbol->setGreenAttribute( renderer2D->greenAttribute() );
    symbol->setBlueAttribute( renderer2D->blueAttribute() );

    symbol->setRedContrastEnhancement( renderer2D->redContrastEnhancement() ? new QgsContrastEnhancement( *renderer2D->redContrastEnhancement() ) : nullptr );
    symbol->setGreenContrastEnhancement( renderer2D->greenContrastEnhancement() ? new QgsContrastEnhancement( *renderer2D->greenContrastEnhancement() ) : nullptr );
    symbol->setBlueContrastEnhancement( renderer2D->blueContrastEnhancement() ? new QgsContrastEnhancement( *renderer2D->blueContrastEnhancement() ) : nullptr );
  }
  else if ( renderer->type() == "classified"_L1 )
  {
    const QgsPointCloudClassifiedRenderer *renderer2D = qgis::down_cast<const QgsPointCloudClassifiedRenderer *>( renderer );
    symbol3D = std::make_unique<QgsClassificationPointCloud3DSymbol>();
    QgsClassificationPointCloud3DSymbol *symbol = static_cast<QgsClassificationPointCloud3DSymbol *>( symbol3D.get() );
    symbol->setAttribute( renderer2D->attribute() );
    symbol->setCategoriesList( renderer2D->categories() );
  }

  if ( symbol3D )
  {
    auto renderer3D = std::make_unique<QgsPointCloudLayer3DRenderer>();
    renderer3D->setSymbol( symbol3D.release() );
    return renderer3D;
  }
  return nullptr;
}

QgsRayCastResult Qgs3DUtils::castRay( Qgs3DMapScene *scene, const QgsRay3D &ray, const QgsRayCastContext &context )
{
  QgsRayCastResult results;
  const QList<QgsMapLayer *> keys = scene->layers();
  for ( QgsMapLayer *layer : keys )
  {
    Qt3DCore::QEntity *entity = scene->layerEntity( layer );

    if ( QgsChunkedEntity *chunkedEntity = qobject_cast<QgsChunkedEntity *>( entity ) )
    {
      const QList<QgsRayCastHit> hits = chunkedEntity->rayIntersection( ray, context );

      if ( !hits.isEmpty() )
        results.addLayerHits( layer, hits );
    }
  }
  if ( QgsTerrainEntity *terrain = scene->terrainEntity() )
  {
    const QList<QgsRayCastHit> hits = terrain->rayIntersection( ray, context );

    if ( !hits.isEmpty() )
      results.addTerrainHits( hits );
  }
  if ( QgsGlobeEntity *globe = scene->globeEntity() )
  {
    const QList<QgsRayCastHit> hits = globe->rayIntersection( ray, context );

    if ( !hits.isEmpty() )
      results.addTerrainHits( hits );
  }
  return results;
}

float Qgs3DUtils::screenSpaceError( float epsilon, float distance, int screenSize, float fov )
{
  /* This routine approximately calculates how an error (epsilon) of an object in world coordinates
   * at given distance (between camera and the object) will look like in screen coordinates.
   *
   * the math below simply uses triangle similarity:
   *
   *             epsilon                       phi
   *   -----------------------------  = ----------------
   *   [ frustum width at distance ]    [ screen width ]
   *
   * Then we solve for phi, substituting [frustum width at distance] = 2 * distance * tan(fov / 2)
   *
   *  ________xxx__      xxx = real world error (epsilon)
   *  \     |     /        x = screen space error (phi)
   *   \    |    /
   *    \___|_x_/   near plane (screen space)
   *     \  |  /
   *      \ | /
   *       \|/    angle = field of view
   *       camera
   */
  float phi = epsilon * static_cast<float>( screenSize ) / static_cast<float>( 2 * distance * tan( fov * M_PI / ( 2 * 180 ) ) );
  return phi;
}

void Qgs3DUtils::computeBoundingBoxNearFarPlanes( const QgsAABB &bbox, const QMatrix4x4 &viewMatrix, float &fnear, float &ffar )
{
  fnear = 1e9;
  ffar = 0;

  for ( int i = 0; i < 8; ++i )
  {
    const QVector4D p( ( ( i >> 0 ) & 1 ) ? bbox.xMin : bbox.xMax, ( ( i >> 1 ) & 1 ) ? bbox.yMin : bbox.yMax, ( ( i >> 2 ) & 1 ) ? bbox.zMin : bbox.zMax, 1 );

    const QVector4D pc = viewMatrix * p;

    const float dst = -pc.z(); // in camera coordinates, x grows right, y grows down, z grows to the back
    fnear = std::min( fnear, dst );
    ffar = std::max( ffar, dst );
  }
}

Qt3DRender::QCullFace::CullingMode Qgs3DUtils::qt3DcullingMode( Qgs3DTypes::CullingMode mode )
{
  switch ( mode )
  {
    case Qgs3DTypes::NoCulling:
      return Qt3DRender::QCullFace::NoCulling;
    case Qgs3DTypes::Front:
      return Qt3DRender::QCullFace::Front;
    case Qgs3DTypes::Back:
      return Qt3DRender::QCullFace::Back;
    case Qgs3DTypes::FrontAndBack:
      return Qt3DRender::QCullFace::FrontAndBack;
  }
  return Qt3DRender::QCullFace::NoCulling;
}


QByteArray Qgs3DUtils::addDefinesToShaderCode( const QByteArray &shaderCode, const QStringList &defines )
{
  // There is one caveat to take care of - GLSL source code needs to start with #version as
  // a first directive, otherwise we get the old GLSL 100 version. So we can't just prepend the
  // shader source code, but insert our defines at the right place.

  QStringList defineLines;
  for ( const QString &define : defines )
    defineLines += "#define " + define + "\n";

  QString definesText = defineLines.join( QString() );

  QByteArray newShaderCode = shaderCode;
  int versionIndex = shaderCode.indexOf( "#version " );
  int insertionIndex = versionIndex == -1 ? 0 : shaderCode.indexOf( '\n', versionIndex + 1 ) + 1;
  newShaderCode.insert( insertionIndex, definesText.toLatin1() );
  return newShaderCode;
}

QByteArray Qgs3DUtils::removeDefinesFromShaderCode( const QByteArray &shaderCode, const QStringList &defines )
{
  QByteArray newShaderCode = shaderCode;

  for ( const QString &define : defines )
  {
    const QString defineLine = "#define " + define + "\n";
    const int defineLineIndex = newShaderCode.indexOf( defineLine.toUtf8() );
    if ( defineLineIndex != -1 )
    {
      newShaderCode.remove( defineLineIndex, defineLine.size() );
    }
  }

  return newShaderCode;
}

void Qgs3DUtils::decomposeTransformMatrix( const QMatrix4x4 &matrix, QVector3D &translation, QQuaternion &rotation, QVector3D &scale )
{
  // decompose the transform matrix
  // assuming the last row has values [0 0 0 1]
  // see https://math.stackexchange.com/questions/237369/given-this-transformation-matrix-how-do-i-decompose-it-into-translation-rotati
  const float *md = matrix.data(); // returns data in column-major order
  const float sx = QVector3D( md[0], md[1], md[2] ).length();
  const float sy = QVector3D( md[4], md[5], md[6] ).length();
  const float sz = QVector3D( md[8], md[9], md[10] ).length();
  float rd[9] = {
    md[0] / sx,
    md[4] / sy,
    md[8] / sz,
    md[1] / sx,
    md[5] / sy,
    md[9] / sz,
    md[2] / sx,
    md[6] / sy,
    md[10] / sz,
  };
  const QMatrix3x3 rot3x3( rd ); // takes data in row-major order

  scale = QVector3D( sx, sy, sz );
  rotation = QQuaternion::fromRotationMatrix( rot3x3 );
  translation = QVector3D( md[12], md[13], md[14] );
}

int Qgs3DUtils::openGlMaxClipPlanes( QSurface *surface )
{
  int numPlanes = 6;

  QOpenGLContext context;
  context.setFormat( QSurfaceFormat::defaultFormat() );
  if ( context.create() )
  {
    if ( context.makeCurrent( surface ) )
    {
      QOpenGLFunctions *funcs = context.functions();
      funcs->glGetIntegerv( GL_MAX_CLIP_PLANES, &numPlanes );
    }
  }

  return numPlanes;
}

QQuaternion Qgs3DUtils::rotationFromPitchHeadingAngles( float pitchAngle, float headingAngle )
{
  return QQuaternion::fromAxisAndAngle( QVector3D( 0, 0, 1 ), headingAngle ) * QQuaternion::fromAxisAndAngle( QVector3D( 1, 0, 0 ), pitchAngle );
}

QgsPoint Qgs3DUtils::screenPointToMapCoordinates( const QPoint &screenPoint, const QSize size, const QgsCameraController *cameraController, const Qgs3DMapSettings *mapSettings )
{
  const QgsRay3D ray = rayFromScreenPoint( screenPoint, size, cameraController->camera() );

  // pick an arbitrary point mid-way between near and far plane
  const float pointDistance = ( cameraController->camera()->farPlane() + cameraController->camera()->nearPlane() ) / 2;
  const QVector3D worldPoint = ray.origin() + pointDistance * ray.direction().normalized();
  const QgsVector3D mapTransform = worldToMapCoordinates( worldPoint, mapSettings->origin() );
  const QgsPoint mapPoint( mapTransform.x(), mapTransform.y(), mapTransform.z() );
  return mapPoint;
}

// computes the portion of the Y=y plane the camera is looking at
void Qgs3DUtils::calculateViewExtent( const Qt3DRender::QCamera *camera, float maxRenderingDistance, float z, float &minX, float &maxX, float &minY, float &maxY, float &minZ, float &maxZ )
{
  const QVector3D cameraPos = camera->position();
  const QMatrix4x4 projectionMatrix = camera->projectionMatrix();
  const QMatrix4x4 viewMatrix = camera->viewMatrix();
  float depth = 1.0f;
  QVector4D viewCenter = viewMatrix * QVector4D( camera->viewCenter(), 1.0f );
  viewCenter /= viewCenter.w();
  viewCenter = projectionMatrix * viewCenter;
  viewCenter /= viewCenter.w();
  depth = viewCenter.z();
  QVector<QVector3D> viewFrustumPoints = {
    QVector3D( 0.0f, 0.0f, depth ),
    QVector3D( 0.0f, 1.0f, depth ),
    QVector3D( 1.0f, 0.0f, depth ),
    QVector3D( 1.0f, 1.0f, depth ),
    QVector3D( 0.0f, 0.0f, 0 ),
    QVector3D( 0.0f, 1.0f, 0 ),
    QVector3D( 1.0f, 0.0f, 0 ),
    QVector3D( 1.0f, 1.0f, 0 )
  };
  maxX = std::numeric_limits<float>::lowest();
  maxY = std::numeric_limits<float>::lowest();
  maxZ = std::numeric_limits<float>::lowest();
  minX = std::numeric_limits<float>::max();
  minY = std::numeric_limits<float>::max();
  minZ = std::numeric_limits<float>::max();
  for ( int i = 0; i < viewFrustumPoints.size(); ++i )
  {
    // convert from view port space to world space
    viewFrustumPoints[i] = viewFrustumPoints[i].unproject( viewMatrix, projectionMatrix, QRect( 0, 0, 1, 1 ) );
    minX = std::min( minX, viewFrustumPoints[i].x() );
    maxX = std::max( maxX, viewFrustumPoints[i].x() );
    minY = std::min( minY, viewFrustumPoints[i].y() );
    maxY = std::max( maxY, viewFrustumPoints[i].y() );
    minZ = std::min( minZ, viewFrustumPoints[i].z() );
    maxZ = std::max( maxZ, viewFrustumPoints[i].z() );
    // find the intersection between the line going from cameraPos to the frustum quad point
    // and the horizontal plane Z=z
    // if the intersection is on the back side of the viewing panel we get a point that is
    // maxRenderingDistance units in front of the camera
    const QVector3D pt = cameraPos;
    const QVector3D vect = ( viewFrustumPoints[i] - pt ).normalized();
    float t = ( z - pt.z() ) / vect.z();
    if ( t < 0 )
      t = maxRenderingDistance;
    else
      t = std::min( t, maxRenderingDistance );
    viewFrustumPoints[i] = pt + t * vect;
    minX = std::min( minX, viewFrustumPoints[i].x() );
    maxX = std::max( maxX, viewFrustumPoints[i].x() );
    minY = std::min( minY, viewFrustumPoints[i].y() );
    maxY = std::max( maxY, viewFrustumPoints[i].y() );
    minZ = std::min( minZ, viewFrustumPoints[i].z() );
    maxZ = std::max( maxZ, viewFrustumPoints[i].z() );
  }
}

QList<QVector4D> Qgs3DUtils::lineSegmentToClippingPlanes( const QgsVector3D &startPoint, const QgsVector3D &endPoint, const double distance, const QgsVector3D &origin )
{
  // return empty vector if distance is negative
  if ( distance < 0 )
    return QList<QVector4D>();

  QgsVector3D lineDirection( endPoint - startPoint );
  lineDirection.normalize();
  const QgsVector lineDirection2DPerp = QgsVector( lineDirection.x(), lineDirection.y() ).perpVector();
  const QgsVector3D linePerp( lineDirection2DPerp.x(), lineDirection2DPerp.y(), 0 );

  QList<QVector4D> clippingPlanes;
  QgsVector3D planePoint;
  double originDistance;

  // the naming is assigned according to line direction
  //! back clip plane
  planePoint = startPoint;
  originDistance = QgsVector3D::dotProduct( planePoint - origin, lineDirection );
  clippingPlanes << QVector4D( static_cast<float>( lineDirection.x() ), static_cast<float>( lineDirection.y() ), 0, static_cast<float>( -originDistance ) );

  //! left clip plane
  planePoint = startPoint + linePerp * distance;
  originDistance = QgsVector3D::dotProduct( planePoint - origin, -linePerp );
  clippingPlanes << QVector4D( static_cast<float>( -linePerp.x() ), static_cast<float>( -linePerp.y() ), 0, static_cast<float>( -originDistance ) );

  //! front clip plane
  planePoint = endPoint;
  originDistance = QgsVector3D::dotProduct( planePoint - origin, -lineDirection );
  clippingPlanes << QVector4D( static_cast<float>( -lineDirection.x() ), static_cast<float>( -lineDirection.y() ), 0, static_cast<float>( -originDistance ) );

  //! right clip plane
  planePoint = startPoint - linePerp * distance;
  originDistance = QgsVector3D::dotProduct( planePoint - origin, linePerp );
  clippingPlanes << QVector4D( static_cast<float>( linePerp.x() ), static_cast<float>( linePerp.y() ), 0, static_cast<float>( -originDistance ) );

  return clippingPlanes;
}

QgsCameraPose Qgs3DUtils::lineSegmentToCameraPose( const QgsVector3D &startPoint, const QgsVector3D &endPoint, const QgsDoubleRange &elevationRange, const float fieldOfView, const QgsVector3D &worldOrigin )
{
  QgsCameraPose cameraPose;
  // we tilt the view slightly to see flat layers if the elevationRange is infinite (scene has flat terrain, vector layers...)
  elevationRange.isInfinite() ? cameraPose.setPitchAngle( 89 ) : cameraPose.setPitchAngle( 90 );

  // calculate the middle of the front side defined by clipping planes
  QgsVector linePerpVec( ( endPoint - startPoint ).x(), ( endPoint - startPoint ).y() );
  linePerpVec = -linePerpVec.normalized().perpVector();
  const QgsVector3D linePerpVec3D( linePerpVec.x(), linePerpVec.y(), 0 );
  QgsVector3D middle( startPoint + ( endPoint - startPoint ) / 2 );

  double elevationRangeHalf;
  elevationRange.isInfinite() ? elevationRangeHalf = 0 : elevationRangeHalf = ( elevationRange.upper() - elevationRange.lower() ) / 2;
  const double side = std::max( middle.distance( startPoint ), elevationRangeHalf );
  const double distance = ( side / std::tan( fieldOfView / 2 * M_PI / 180 ) ) * 1.05;
  cameraPose.setDistanceFromCenterPoint( static_cast<float>( distance ) );

  elevationRange.isInfinite() ? middle.setZ( 0 ) : middle.setZ( elevationRange.lower() + ( elevationRange.upper() - elevationRange.lower() ) / 2 );
  cameraPose.setCenterPoint( mapToWorldCoordinates( middle, worldOrigin ) );

  const QgsVector3D northDirectionVec( 0, -1, 0 );
  // calculate the angle between vector pointing to the north and vector pointing from the front side of clipped area
  float yawAngle = static_cast<float>( acos( QgsVector3D::dotProduct( linePerpVec3D, northDirectionVec ) ) * 180 / M_PI );
  // check if the angle between the view point is to the left or right of the scene north, apply angle offset if necessary for camera
  if ( QgsVector3D::crossProduct( linePerpVec3D, northDirectionVec ).z() > 0 )
  {
    yawAngle = 360 - yawAngle;
  }
  cameraPose.setHeadingAngle( yawAngle );

  return cameraPose;
}

std::unique_ptr<Qt3DRender::QCamera> Qgs3DUtils::copyCamera( Qt3DRender::QCamera *cam )
{
  auto copy = std::make_unique<Qt3DRender::QCamera>();
  copy->setPosition( cam->position() );
  copy->setViewCenter( cam->viewCenter() );
  copy->setUpVector( cam->upVector() );
  copy->setProjectionMatrix( cam->projectionMatrix() );
  copy->setNearPlane( cam->nearPlane() );
  copy->setFarPlane( cam->farPlane() );
  copy->setAspectRatio( cam->aspectRatio() );
  copy->setFieldOfView( cam->fieldOfView() );
  return copy;
}
