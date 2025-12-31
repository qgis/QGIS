/***************************************************************************
  qgsrubberband3d.cpp
  --------------------------------------
  Date                 : June 2021
  Copyright            : (C) 2021 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrubberband3d.h"

#include "qgs3dmapsettings.h"
#include "qgs3dutils.h"
#include "qgsabstract3dengine.h"
#include "qgsbillboardgeometry.h"
#include "qgsgeotransform.h"
#include "qgslinematerial_p.h"
#include "qgslinestring.h"
#include "qgslinevertexdata_p.h"
#include "qgsmarkersymbol.h"
#include "qgsmessagelog.h"
#include "qgspoint3dbillboardmaterial.h"
#include "qgspolygon.h"
#include "qgssymbollayer.h"
#include "qgssymbollayerutils.h"
#include "qgstessellatedpolygongeometry.h"
#include "qgstessellator.h"
#include "qgsvertexid.h"

#include <QColor>
#include <Qt3DCore/QAttribute>
#include <Qt3DCore/QBuffer>
#include <Qt3DCore/QEntity>
#include <Qt3DCore/QGeometry>
#include <Qt3DRender/QGeometryRenderer>

/// @cond PRIVATE


QgsRubberBand3D::QgsRubberBand3D( Qgs3DMapSettings &map, QgsAbstract3DEngine *engine, Qt3DCore::QEntity *parentEntity, const Qgis::GeometryType geometryType )
  : mMapSettings( &map )
  , mEngine( engine )
  , mGeometryType( geometryType )
{
  switch ( mGeometryType )
  {
    case Qgis::GeometryType::Point:
      setupMarker( parentEntity );
      break;
    case Qgis::GeometryType::Line:
      setupLine( parentEntity, engine );
      setupMarker( parentEntity );
      break;
    case Qgis::GeometryType::Polygon:
      setupMarker( parentEntity );
      setupLine( parentEntity, engine );
      setupPolygon( parentEntity );
      break;
    case Qgis::GeometryType::Null:
    case Qgis::GeometryType::Unknown:
      QgsDebugError( "Unknown GeometryType used in QgsRubberband3D" );
      break;
  }
}

void QgsRubberBand3D::setupMarker( Qt3DCore::QEntity *parentEntity )
{
  mMarkerEntity.reset( new Qt3DCore::QEntity( parentEntity ) );
  mMarkerGeometry = new QgsBillboardGeometry();
  mMarkerGeometryRenderer = new Qt3DRender::QGeometryRenderer;
  mMarkerGeometryRenderer->setPrimitiveType( Qt3DRender::QGeometryRenderer::Points );
  mMarkerGeometryRenderer->setGeometry( mMarkerGeometry );
  mMarkerGeometryRenderer->setVertexCount( mMarkerGeometry->count() );

  setMarkerType( mMarkerType );
  mMarkerEntity->addComponent( mMarkerGeometryRenderer );

  mMarkerTransform = new QgsGeoTransform;
  mMarkerTransform->setOrigin( mMapSettings->origin() );
  mMarkerEntity->addComponent( mMarkerTransform );
}

void QgsRubberBand3D::setupLine( Qt3DCore::QEntity *parentEntity, QgsAbstract3DEngine *engine )
{
  mLineEntity.reset( new Qt3DCore::QEntity( parentEntity ) );

  QgsLineVertexData dummyLineData;
  mLineGeometry = dummyLineData.createGeometry( mLineEntity );

  Q_ASSERT( mLineGeometry->attributes().count() == 2 );
  mPositionAttribute = mLineGeometry->attributes().at( 0 );
  mIndexAttribute = mLineGeometry->attributes().at( 1 );

  mLineGeometryRenderer = new Qt3DRender::QGeometryRenderer;
  mLineGeometryRenderer->setPrimitiveType( Qt3DRender::QGeometryRenderer::LineStripAdjacency );
  mLineGeometryRenderer->setGeometry( mLineGeometry );
  mLineGeometryRenderer->setPrimitiveRestartEnabled( true );
  mLineGeometryRenderer->setRestartIndexValue( 0 );

  mLineEntity->addComponent( mLineGeometryRenderer );

  mLineMaterial = new QgsLineMaterial;
  mLineMaterial->setLineWidth( mWidth );
  mLineMaterial->setLineColor( mColor );

  QObject::connect( engine, &QgsAbstract3DEngine::sizeChanged, mLineMaterial, [this, engine] {
    mLineMaterial->setViewportSize( engine->size() );
  } );
  mLineMaterial->setViewportSize( engine->size() );

  mLineEntity->addComponent( mLineMaterial );

  mLineTransform = new QgsGeoTransform( mLineEntity );
  mLineTransform->setOrigin( mMapSettings->origin() );
  mLineEntity->addComponent( mLineTransform );
}

void QgsRubberBand3D::setupPolygon( Qt3DCore::QEntity *parentEntity )
{
  mPolygonEntity.reset( new Qt3DCore::QEntity( parentEntity ) );

  mPolygonGeometry = new QgsTessellatedPolygonGeometry();

  Qt3DRender::QGeometryRenderer *polygonGeometryRenderer = new Qt3DRender::QGeometryRenderer;
  polygonGeometryRenderer->setPrimitiveType( Qt3DRender::QGeometryRenderer::Triangles );
  polygonGeometryRenderer->setGeometry( mPolygonGeometry );
  mPolygonEntity->addComponent( polygonGeometryRenderer );

  QgsPhongMaterialSettings polygonMaterialSettings = QgsPhongMaterialSettings();
  polygonMaterialSettings.setAmbient( mColor );
  polygonMaterialSettings.setDiffuse( mColor );
  polygonMaterialSettings.setOpacity( DEFAULT_POLYGON_OPACITY );
  mPolygonMaterial = polygonMaterialSettings.toMaterial( QgsMaterialSettingsRenderingTechnique::Triangles, QgsMaterialContext() );
  mPolygonEntity->addComponent( mPolygonMaterial );

  mPolygonTransform = new QgsGeoTransform;
  mPolygonTransform->setOrigin( mMapSettings->origin() );
  mPolygonEntity->addComponent( mPolygonTransform );
}

void QgsRubberBand3D::removePoint( int index )
{
  if ( QgsPolygon *polygon = qgsgeometry_cast<QgsPolygon *>( mGeometry.get() ) )
  {
    QgsLineString *lineString = qgsgeometry_cast<QgsLineString *>( polygon->exteriorRing() );
    const int vertexIndex = index < 0 ? lineString->numPoints() - 1 + index : index;
    lineString->deleteVertex( QgsVertexId( 0, 0, vertexIndex ) );

    if ( lineString->numPoints() < 3 )
    {
      mGeometry.set( new QgsLineString( *lineString ) );
    }
  }
  else if ( QgsLineString *lineString = qgsgeometry_cast<QgsLineString *>( mGeometry.get() ) )
  {
    const int vertexIndex = index < 0 ? lineString->numPoints() + index : index;
    lineString->deleteVertex( QgsVertexId( 0, 0, vertexIndex ) );
  }
  else
  {
    return;
  }

  updateGeometry();
}

QgsRubberBand3D::~QgsRubberBand3D()
{
  if ( mPolygonEntity )
  {
    mPolygonEntity.reset();
  }
  if ( mLineEntity )
  {
    mLineEntity.reset();
  }
  if ( mMarkerEntity )
  {
    mMarkerEntity.reset();
  }
}


float QgsRubberBand3D::width() const
{
  return mWidth;
}

void QgsRubberBand3D::setWidth( float width )
{
  const bool isLineOrPolygon = mGeometryType == Qgis::GeometryType::Line || mGeometryType == Qgis::GeometryType::Polygon;
  mWidth = width;

  if ( isLineOrPolygon && mEdgesEnabled )
  {
    // when highlighting lines, the vertex markers should be wider
    mLineMaterial->setLineWidth( width );
    width *= 3;
  }

  mMarkerSymbol->setSize( width );
  updateMarkerMaterial();
}

QColor QgsRubberBand3D::color() const
{
  return mColor;
}

void QgsRubberBand3D::setColor( const QColor color )
{
  const bool isLineOrPolygon = mGeometryType == Qgis::GeometryType::Line || mGeometryType == Qgis::GeometryType::Polygon;
  mColor = color;

  if ( mEdgesEnabled && isLineOrPolygon )
  {
    mLineMaterial->setLineColor( color );
  }

  if ( isLineOrPolygon )
  {
    mMarkerSymbol->setColor( color.lighter( 130 ) );
  }
  else
  {
    mMarkerSymbol->setColor( color );
  }

  if ( mMarkerSymbol->symbolLayerCount() > 0 && mMarkerSymbol->symbolLayer( 0 )->layerType() == "SimpleMarker"_L1 && !mOutlineColor.value() )
  {
    mMarkerSymbol->symbolLayer( 0 )->setStrokeColor( color );
  }
  updateMarkerMaterial();

  if ( mGeometryType == Qgis::GeometryType::Polygon )
  {
    if ( mPolygonMaterial )
      mPolygonEntity->removeComponent( mPolygonMaterial );

    if ( mPolygonFillEnabled )
    {
      QgsPhongMaterialSettings polygonMaterialSettings;
      polygonMaterialSettings.setAmbient( mColor );
      polygonMaterialSettings.setDiffuse( mColor );
      polygonMaterialSettings.setOpacity( DEFAULT_POLYGON_OPACITY );
      mPolygonMaterial = polygonMaterialSettings.toMaterial( QgsMaterialSettingsRenderingTechnique::Triangles, QgsMaterialContext() );
      mPolygonEntity->addComponent( mPolygonMaterial );
    }
  }
}

QColor QgsRubberBand3D::outlineColor() const
{
  return mOutlineColor;
}

void QgsRubberBand3D::setOutlineColor( const QColor color )
{
  mOutlineColor = color;

  if ( mMarkerSymbol->symbolLayerCount() > 0 && mMarkerSymbol->symbolLayer( 0 )->layerType() == "SimpleMarker"_L1 )
  {
    mMarkerSymbol->symbolLayer( 0 )->setStrokeColor( color );
  }
  updateMarkerMaterial();
}

void QgsRubberBand3D::setMarkerType( const MarkerType marker )
{
  mMarkerType = marker;

  const bool lineOrPolygon = mGeometryType == Qgis::GeometryType::Line || mGeometryType == Qgis::GeometryType::Polygon;

  const QVariantMap props {
    { u"color"_s, lineOrPolygon ? mColor.lighter( 130 ).name() : mColor.name() },
    { u"size_unit"_s, u"pixel"_s },
    { u"size"_s, QString::number( lineOrPolygon ? mWidth * 3.f : mWidth ) },
    { u"outline_color"_s, mOutlineColor.value() ? mOutlineColor.name() : mColor.name() },
    { u"outline_style"_s, QgsSymbolLayerUtils::encodePenStyle( mMarkerOutlineStyle ) },
    { u"outline_width"_s, QString::number( lineOrPolygon ? 0.5 : 1 ) },
    { u"name"_s, mMarkerType == Square ? u"square"_s : u"circle"_s }
  };

  mMarkerSymbol = QgsMarkerSymbol::createSimple( props );
  updateMarkerMaterial();
}

QgsRubberBand3D::MarkerType QgsRubberBand3D::markerType() const
{
  return mMarkerType;
}

void QgsRubberBand3D::setMarkerOutlineStyle( const Qt::PenStyle style )
{
  mMarkerOutlineStyle = style;
  setMarkerType( markerType() );
}

Qt::PenStyle QgsRubberBand3D::markerOutlineStyle() const
{
  return mMarkerOutlineStyle;
}

void QgsRubberBand3D::setMarkersEnabled( const bool enable )
{
  mMarkerEnabled = enable;
  updateMarkerMaterial();
}

bool QgsRubberBand3D::hasMarkersEnabled() const
{
  return mMarkerEnabled;
}

void QgsRubberBand3D::setEdgesEnabled( const bool enable )
{
  mEdgesEnabled = enable;
  setColor( mColor );
}

bool QgsRubberBand3D::hasEdgesEnabled() const
{
  return mEdgesEnabled;
}

void QgsRubberBand3D::setFillEnabled( const bool enable )
{
  mPolygonFillEnabled = enable;
  setColor( mColor );
}

bool QgsRubberBand3D::hasFillEnabled() const
{
  return mPolygonFillEnabled;
}

void QgsRubberBand3D::reset()
{
  mGeometry.set( nullptr );
  updateGeometry();
}

void QgsRubberBand3D::addPoint( const QgsPoint &pt )
{
  if ( QgsPolygon *polygon = qgsgeometry_cast<QgsPolygon *>( mGeometry.get() ) )
  {
    QgsLineString *exteriorRing = qgsgeometry_cast<QgsLineString *>( polygon->exteriorRing() );
    const int lastVertexIndex = exteriorRing->numPoints() - 1;
    exteriorRing->insertVertex( QgsVertexId( 0, 0, lastVertexIndex ), pt );
  }
  else if ( QgsLineString *lineString = qgsgeometry_cast<QgsLineString *>( mGeometry.get() ) )
  {
    lineString->addVertex( pt );
    // transform linestring to polygon if we have enough vertices
    if ( mGeometryType == Qgis::GeometryType::Polygon && lineString->numPoints() >= 3 )
    {
      mGeometry.set( new QgsPolygon( lineString->clone() ) );
    }
  }
  else if ( !mGeometry.constGet() )
  {
    mGeometry.set( new QgsLineString( QVector<QgsPoint> { pt } ) );
  }

  updateGeometry();
}

void QgsRubberBand3D::setGeometry( const QgsGeometry &geometry )
{
  mGeometry = geometry;
  mGeometryType = geometry.type();

  updateGeometry();
}

void QgsRubberBand3D::removeLastPoint()
{
  removePoint( -1 );
}

void QgsRubberBand3D::removePenultimatePoint()
{
  removePoint( -2 );
}

void QgsRubberBand3D::moveLastPoint( const QgsPoint &pt )
{
  if ( QgsPolygon *polygon = qgsgeometry_cast<QgsPolygon *>( mGeometry.get() ) )
  {
    QgsLineString *lineString = qgsgeometry_cast<QgsLineString *>( polygon->exteriorRing() );
    const int lastVertexIndex = lineString->numPoints() - 2;
    lineString->moveVertex( QgsVertexId( 0, 0, lastVertexIndex ), pt );
  }
  else if ( QgsLineString *lineString = qgsgeometry_cast<QgsLineString *>( mGeometry.get() ) )
  {
    const int lastVertexIndex = lineString->numPoints() - 1;
    lineString->moveVertex( QgsVertexId( 0, 0, lastVertexIndex ), pt );
  }
  else
  {
    return;
  }

  updateGeometry();
}

void QgsRubberBand3D::updateGeometry()
{
  // figure out a reasonable origin for the coordinates to keep them as small as possible
  const QgsBox3D box = mGeometry.constGet() ? mGeometry.constGet()->boundingBox3D() : QgsBox3D();
  const QgsVector3D dataOrigin = box.isNull() ? mMapSettings->origin() : box.center();

  QgsLineVertexData lineData;
  lineData.withAdjacency = true;
  lineData.geocentricCoordinates = mMapSettings->sceneMode() == Qgis::SceneMode::Globe;
  lineData.init( Qgis::AltitudeClamping::Absolute, Qgis::AltitudeBinding::Vertex, 0, Qgs3DRenderContext::fromMapSettings( mMapSettings ), dataOrigin );
  if ( const QgsPolygon *polygon = qgsgeometry_cast<const QgsPolygon *>( mGeometry.constGet() ) )
  {
    std::unique_ptr< QgsLineString > lineString( qgsgeometry_cast<QgsLineString *>( polygon->exteriorRing()->clone() ) );
    const int lastVertexIndex = lineString->numPoints() - 1;
    lineString->deleteVertex( QgsVertexId( 0, 0, lastVertexIndex ) );
    lineData.addLineString( *lineString, 0, true );
  }
  else if ( const QgsLineString *lineString = qgsgeometry_cast<const QgsLineString *>( mGeometry.constGet() ) )
  {
    lineData.addLineString( *lineString, 0, false );
  }


  if ( mEdgesEnabled && ( mGeometryType == Qgis::GeometryType::Line || mGeometryType == Qgis::GeometryType::Polygon ) )
  {
    mPositionAttribute->buffer()->setData( lineData.createVertexBuffer() );
    mIndexAttribute->buffer()->setData( lineData.createIndexBuffer() );
    mLineGeometryRenderer->setVertexCount( lineData.indexes.count() );
    mLineTransform->setGeoTranslation( dataOrigin );
  }

  // first entry is empty for primitive restart
  lineData.vertices.pop_front();

  // we may not want a marker on the last point as it's tracked by the mouse cursor
  if ( mHideLastMarker && !lineData.vertices.isEmpty() )
    lineData.vertices.pop_back();

  mMarkerGeometry->setPositions( lineData.vertices );
  mMarkerGeometryRenderer->setVertexCount( lineData.vertices.count() );
  mMarkerTransform->setGeoTranslation( dataOrigin );

  if ( mGeometryType == Qgis::GeometryType::Polygon )
  {
    if ( const QgsPolygon *polygon = qgsgeometry_cast<const QgsPolygon *>( mGeometry.constGet() ) )
    {
      QgsTessellator tessellator;
      tessellator.setOrigin( mMapSettings->origin() );
      tessellator.setAddNormals( true );
      tessellator.setOutputZUp( true );
      tessellator.addPolygon( *polygon, 0 );
      if ( !tessellator.error().isEmpty() )
      {
        QgsMessageLog::logMessage( tessellator.error(), QObject::tr( "3D" ) );
      }
      // extract vertex buffer data from tessellator
      const QByteArray data( reinterpret_cast<const char *>( tessellator.data().constData() ), static_cast<int>( tessellator.data().count() * sizeof( float ) ) );
      const int vertexCount = data.count() / tessellator.stride();
      mPolygonGeometry->setData( data, vertexCount, QVector<QgsFeatureId>(), QVector<uint>() );
      mPolygonTransform->setGeoTranslation( mMapSettings->origin() );
    }
    else
    {
      mPolygonGeometry->setData( QByteArray(), 0, QVector<QgsFeatureId>(), QVector<uint>() );
    }
  }
}

void QgsRubberBand3D::updateMarkerMaterial()
{
  if ( mMarkerEnabled )
  {
    mMarkerMaterial = new QgsPoint3DBillboardMaterial();
    mMarkerMaterial->setTexture2DFromSymbol( mMarkerSymbol.get(), Qgs3DRenderContext::fromMapSettings( mMapSettings ) );
    mMarkerEntity->addComponent( mMarkerMaterial );

    //TODO: QgsAbstract3DEngine::sizeChanged should have const QSize &size param
    QObject::connect( mEngine, &QgsAbstract3DEngine::sizeChanged, mMarkerMaterial, [this] {
      mMarkerMaterial->setViewportSize( mEngine->size() );
    } );
    mMarkerMaterial->setViewportSize( mEngine->size() );
  }
  else
  {
    mMarkerEntity->removeComponent( mMarkerMaterial );
    QObject::disconnect( mEngine, nullptr, mMarkerMaterial, nullptr );
  }
}
/// @endcond
