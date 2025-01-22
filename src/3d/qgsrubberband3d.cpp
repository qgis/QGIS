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
#include "qgslinestring.h"
#include "qgssymbollayer.h"
#include "qgs3dmapsettings.h"

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


QgsRubberBand3D::QgsRubberBand3D( Qgs3DMapSettings &map, QgsWindow3DEngine *engine, Qt3DCore::QEntity *parentEntity, Qgis::GeometryType geometryType )
  : mMapSettings( &map )
  , mEngine( engine )
  , mGeometryType( geometryType )
{
  if ( mGeometryType == Qgis::GeometryType::Line || mGeometryType == Qgis::GeometryType::Polygon )
  {
    // Rubberband line
    mLineEntity = new Qt3DCore::QEntity( parentEntity );

    QgsLineVertexData dummyLineData;
    mGeometry = dummyLineData.createGeometry( mLineEntity );

    Q_ASSERT( mGeometry->attributes().count() == 2 );
    mPositionAttribute = mGeometry->attributes().at( 0 );
    mIndexAttribute = mGeometry->attributes().at( 1 );

    mLineGeomRenderer = new Qt3DRender::QGeometryRenderer;
    mLineGeomRenderer->setPrimitiveType( Qt3DRender::QGeometryRenderer::LineStripAdjacency );
    mLineGeomRenderer->setGeometry( mGeometry );
    mLineGeomRenderer->setPrimitiveRestartEnabled( true );
    mLineGeomRenderer->setRestartIndexValue( 0 );

    mLineEntity->addComponent( mLineGeomRenderer );

    mLineMaterial = new QgsLineMaterial;
    mLineMaterial->setLineWidth( mWidth );
    mLineMaterial->setLineColor( mColor );

    QObject::connect( engine, &QgsAbstract3DEngine::sizeChanged, mLineMaterial, [this, engine] {
      mLineMaterial->setViewportSize( engine->size() );
    } );
    mLineMaterial->setViewportSize( engine->size() );

    mLineEntity->addComponent( mLineMaterial );
  }

  // Rubberband vertex markers
  mMarkerEntity = new Qt3DCore::QEntity( parentEntity );
  mMarkerGeometry = new QgsBillboardGeometry();
  mMarkerGeometryRenderer = new Qt3DRender::QGeometryRenderer;
  mMarkerGeometryRenderer->setPrimitiveType( Qt3DRender::QGeometryRenderer::Points );
  mMarkerGeometryRenderer->setGeometry( mMarkerGeometry );
  mMarkerGeometryRenderer->setVertexCount( mMarkerGeometry->count() );

  setMarkerType( mMarkerType );
  mMarkerEntity->addComponent( mMarkerGeometryRenderer );
}

QgsRubberBand3D::~QgsRubberBand3D()
{
  if ( mLineEntity )
    mLineEntity->deleteLater();
  mMarkerEntity->deleteLater();
}

float QgsRubberBand3D::width() const
{
  return mWidth;
}

void QgsRubberBand3D::setWidth( float width )
{
  mWidth = width;

  if ( mGeometryType == Qgis::GeometryType::Line || mGeometryType == Qgis::GeometryType::Polygon )
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

void QgsRubberBand3D::setColor( QColor color )
{
  mColor = color;

  if ( mGeometryType == Qgis::GeometryType::Line || mGeometryType == Qgis::GeometryType::Polygon )
  {
    mLineMaterial->setLineColor( color );
    mMarkerSymbol->setColor( color.lighter( 130 ) );
  }
  else
  {
    mMarkerSymbol->setColor( color );
  }

  if ( mMarkerSymbol->symbolLayerCount() > 0 && mMarkerSymbol->symbolLayer( 0 )->layerType() == QLatin1String( "SimpleMarker" ) )
  {
    static_cast<QgsMarkerSymbolLayer *>( mMarkerSymbol->symbolLayer( 0 ) )->setStrokeColor( color );
  }
  updateMarkerMaterial();
}

void QgsRubberBand3D::setMarkerType( MarkerType marker )
{
  mMarkerType = marker;

  const bool lineOrPolygon = mGeometryType == Qgis::GeometryType::Line || mGeometryType == Qgis::GeometryType::Polygon;

  const QVariantMap props {
    { QStringLiteral( "color" ), lineOrPolygon ? mColor.lighter( 130 ).name() : mColor.name() },
    { QStringLiteral( "size_unit" ), QStringLiteral( "pixel" ) },
    { QStringLiteral( "size" ), QString::number( lineOrPolygon ? mWidth * 3.f : mWidth ) },
    { QStringLiteral( "outline_color" ), mColor.name() },
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

void QgsRubberBand3D::reset()
{
  mLineString.clear();
  updateGeometry();
}

void QgsRubberBand3D::addPoint( const QgsPoint &pt )
{
  mLineString.addVertex( pt );
  updateGeometry();
}

void QgsRubberBand3D::setPoints( const QgsLineString &points )
{
  mLineString = points;
  updateGeometry();
}

void QgsRubberBand3D::removeLastPoint()
{
  const int lastVertexIndex = mLineString.numPoints() - 1;
  mLineString.deleteVertex( QgsVertexId( 0, 0, lastVertexIndex ) );
  updateGeometry();
}

void QgsRubberBand3D::moveLastPoint( const QgsPoint &pt )
{
  const int lastVertexIndex = mLineString.numPoints() - 1;
  mLineString.moveVertex( QgsVertexId( 0, 0, lastVertexIndex ), pt );
  updateGeometry();
}

void QgsRubberBand3D::updateGeometry()
{
  QgsLineVertexData lineData;
  lineData.withAdjacency = true;
  lineData.init( Qgis::AltitudeClamping::Absolute, Qgis::AltitudeBinding::Vertex, 0, Qgs3DRenderContext::fromMapSettings( mMapSettings ), mMapSettings->origin() );
  const bool closed = mGeometryType == Qgis::GeometryType::Polygon;
  lineData.addLineString( mLineString, 0, closed );

  if ( mGeometryType == Qgis::GeometryType::Line || mGeometryType == Qgis::GeometryType::Polygon )
  {
    mPositionAttribute->buffer()->setData( lineData.createVertexBuffer() );
    mIndexAttribute->buffer()->setData( lineData.createIndexBuffer() );
    mLineGeomRenderer->setVertexCount( lineData.indexes.count() );
  }

  // first entry is empty for primitive restart
  lineData.vertices.pop_front();

  // we may not want a marker on the last point as it's tracked by the mouse cursor
  if ( mHideLastMarker && !lineData.vertices.isEmpty() )
    lineData.vertices.pop_back();

  mMarkerGeometry->setPoints( lineData.vertices );
  mMarkerGeometryRenderer->setVertexCount( lineData.vertices.count() );
}

void QgsRubberBand3D::updateMarkerMaterial()
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
/// @endcond
