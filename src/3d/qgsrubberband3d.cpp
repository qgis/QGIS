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

#include "qgsbillboardgeometry.h"
#include "qgspoint3dbillboardmaterial.h"
#include "qgsmarkersymbol.h"
#include "qgswindow3dengine.h"
#include "qgslinevertexdata_p.h"
#include "qgslinematerial_p.h"
#include "qgsvertexid.h"
#include "qgssymbollayer.h"
#include "qgs3dmapsettings.h"
#include "qgs3dutils.h"
#include "qgslinestring.h"
#include "qgsmessagelog.h"
#include "qgspolygon.h"
#include "qgssymbollayerutils.h"
#include "qgstessellatedpolygongeometry.h"
#include "qgstessellator.h"

#include <Qt3DCore/QEntity>

#if QT_VERSION < QT_VERSION_CHECK( 6, 0, 0 )
#include <Qt3DRender/QAttribute>
#include <Qt3DRender/QBuffer>
#include <Qt3DRender/QGeometry>
#else
#include <Qt3DCore/QAttribute>
#include <Qt3DCore/QBuffer>
#include <Qt3DCore/QGeometry>
#endif

#include <Qt3DRender/QGeometryRenderer>
#include <QColor>


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
  mMarkerEntity = new Qt3DCore::QEntity( parentEntity );
  mMarkerGeometry = new QgsBillboardGeometry();
  mMarkerGeometryRenderer = new Qt3DRender::QGeometryRenderer;
  mMarkerGeometryRenderer->setPrimitiveType( Qt3DRender::QGeometryRenderer::Points );
  mMarkerGeometryRenderer->setGeometry( mMarkerGeometry );
  mMarkerGeometryRenderer->setVertexCount( mMarkerGeometry->count() );

  setMarkerType( mMarkerType );
  mMarkerEntity->addComponent( mMarkerGeometryRenderer );
}

void QgsRubberBand3D::setupLine( Qt3DCore::QEntity *parentEntity, QgsAbstract3DEngine *engine )
{
  mLineEntity = new Qt3DCore::QEntity( parentEntity );

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
}

void QgsRubberBand3D::setupPolygon( Qt3DCore::QEntity *parentEntity )
{
  mPolygonEntity = new Qt3DCore::QEntity( parentEntity );

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
}

QgsRubberBand3D::~QgsRubberBand3D()
{
  if ( mPolygonEntity )
    mPolygonEntity->deleteLater();
  if ( mLineEntity )
    mLineEntity->deleteLater();
  if ( mMarkerEntity )
    mMarkerEntity->deleteLater();
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

  if ( mMarkerSymbol->symbolLayerCount() > 0 && mMarkerSymbol->symbolLayer( 0 )->layerType() == QLatin1String( "SimpleMarker" ) && !mOutlineColor.value() )
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

  if ( mMarkerSymbol->symbolLayerCount() > 0 && mMarkerSymbol->symbolLayer( 0 )->layerType() == QLatin1String( "SimpleMarker" ) )
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
    { QStringLiteral( "color" ), lineOrPolygon ? mColor.lighter( 130 ).name() : mColor.name() },
    { QStringLiteral( "size_unit" ), QStringLiteral( "pixel" ) },
    { QStringLiteral( "size" ), QString::number( lineOrPolygon ? mWidth * 3.f : mWidth ) },
    { QStringLiteral( "outline_color" ), mOutlineColor.value() ? mOutlineColor.name() : mColor.name() },
    { QStringLiteral( "outline_style" ), QgsSymbolLayerUtils::encodePenStyle( mMarkerOutlineStyle ) },
    { QStringLiteral( "outline_width" ), QString::number( lineOrPolygon ? 0.5 : 1 ) },
    { QStringLiteral( "name" ), mMarkerType == Square ? QStringLiteral( "square" ) : QStringLiteral( "circle" ) }
  };

  mMarkerSymbol.reset( QgsMarkerSymbol::createSimple( props ) );
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
  if ( QgsPolygon *polygon = qgsgeometry_cast<QgsPolygon *>( mGeometry.constGet() ) )
  {
    QgsLineString *exteriorRing = qgsgeometry_cast<QgsLineString *>( polygon->exteriorRing() );
    const int lastVertexIndex = exteriorRing->numPoints() - 1;
    exteriorRing->insertVertex( QgsVertexId( 0, 0, lastVertexIndex ), pt );
  }
  else if ( QgsLineString *lineString = qgsgeometry_cast<QgsLineString *>( mGeometry.constGet() ) )
  {
    lineString->addVertex( pt );
    // transform linestring to polygon if we have enough vertices
    if ( mGeometryType == Qgis::GeometryType::Polygon && lineString->numPoints() >= 3 )
    {
      mGeometry.set( new QgsPolygon( lineString->clone() ) );
    }
  }
  else if ( !mGeometry.get() )
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
  QgsLineString *lineString = nullptr;
  if ( QgsPolygon *polygon = qgsgeometry_cast<QgsPolygon *>( mGeometry.constGet() ) )
  {
    lineString = qgsgeometry_cast<QgsLineString *>( polygon->exteriorRing() );
    const int lastVertexIndex = lineString->numPoints() - 2;
    lineString->deleteVertex( QgsVertexId( 0, 0, lastVertexIndex ) );
  }
  else if ( ( lineString = qgsgeometry_cast<QgsLineString *>( mGeometry.constGet() ) ) )
  {
    const int lastVertexIndex = lineString->numPoints() - 1;
    lineString->deleteVertex( QgsVertexId( 0, 0, lastVertexIndex ) );
  }
  else
  {
    return;
  }

  if ( lineString->numPoints() < 3 && !qgsgeometry_cast<QgsLineString *>( mGeometry.constGet() ) )
  {
    mGeometry.set( new QgsLineString( *lineString ) );
  }

  updateGeometry();
}

void QgsRubberBand3D::moveLastPoint( const QgsPoint &pt )
{
  QgsLineString *lineString = nullptr;
  if ( QgsPolygon *polygon = qgsgeometry_cast<QgsPolygon *>( mGeometry.constGet() ) )
  {
    lineString = qgsgeometry_cast<QgsLineString *>( polygon->exteriorRing() );
    const int lastVertexIndex = lineString->numPoints() - 2;
    lineString->moveVertex( QgsVertexId( 0, 0, lastVertexIndex ), pt );
  }
  else if ( ( lineString = qgsgeometry_cast<QgsLineString *>( mGeometry.constGet() ) ) )
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
  QgsLineVertexData lineData;
  lineData.withAdjacency = true;
  lineData.init( Qgis::AltitudeClamping::Absolute, Qgis::AltitudeBinding::Vertex, 0, Qgs3DRenderContext::fromMapSettings( mMapSettings ), mMapSettings->origin() );
  if ( QgsPolygon *polygon = qgsgeometry_cast<QgsPolygon *>( mGeometry.constGet() ) )
  {
    QgsLineString *lineString = qgsgeometry_cast<QgsLineString *>( polygon->exteriorRing()->clone() );
    const int lastVertexIndex = lineString->numPoints() - 1;
    lineString->deleteVertex( QgsVertexId( 0, 0, lastVertexIndex ) );
    lineData.addLineString( *lineString, 0, true );
  }
  else if ( const QgsLineString *lineString = qgsgeometry_cast<QgsLineString *>( mGeometry.constGet() ) )
  {
    lineData.addLineString( *lineString, 0, false );
  }


  if ( mEdgesEnabled && ( mGeometryType == Qgis::GeometryType::Line || mGeometryType == Qgis::GeometryType::Polygon ) )
  {
    mPositionAttribute->buffer()->setData( lineData.createVertexBuffer() );
    mIndexAttribute->buffer()->setData( lineData.createIndexBuffer() );
    mLineGeometryRenderer->setVertexCount( lineData.indexes.count() );
  }

  // first entry is empty for primitive restart
  lineData.vertices.pop_front();

  // we may not want a marker on the last point as it's tracked by the mouse cursor
  if ( mHideLastMarker && !lineData.vertices.isEmpty() )
    lineData.vertices.pop_back();

  mMarkerGeometry->setPoints( lineData.vertices );
  mMarkerGeometryRenderer->setVertexCount( lineData.vertices.count() );


  if ( mGeometryType == Qgis::GeometryType::Polygon )
  {
    if ( const QgsPolygon *polygon = qgsgeometry_cast<QgsPolygon *>( mGeometry.constGet() ) )
    {
      QgsTessellator tessellator( mMapSettings->origin().x(), mMapSettings->origin().y(), true );
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
