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

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
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


QgsRubberBand3D::QgsRubberBand3D( Qgs3DMapSettings &map, QgsWindow3DEngine *engine, Qt3DCore::QEntity *parentEntity )
{
  mMapSettings = &map;
  mEngine = engine;

  // Rubberband line
  mLineEntity = new Qt3DCore::QEntity( parentEntity );

  QgsLineVertexData dummyLineData;
  mGeometry = dummyLineData.createGeometry( mLineEntity );

  Q_ASSERT( mGeometry->attributes().count() == 2 );
  mPositionAttribute = mGeometry->attributes()[0];
  mIndexAttribute = mGeometry->attributes()[1];

  mGeomRenderer = new Qt3DRender::QGeometryRenderer;
  mGeomRenderer->setPrimitiveType( Qt3DRender::QGeometryRenderer::LineStripAdjacency );
  mGeomRenderer->setGeometry( mGeometry );
  mGeomRenderer->setPrimitiveRestartEnabled( true );
  mGeomRenderer->setRestartIndexValue( 0 );

  mLineEntity->addComponent( mGeomRenderer );

  mLineMaterial = new QgsLineMaterial;
  mLineMaterial->setLineWidth( 3 );
  mLineMaterial->setLineColor( Qt::red );

  QObject::connect( engine, &QgsAbstract3DEngine::sizeChanged, mLineMaterial, [this, engine]
  {
    mLineMaterial->setViewportSize( engine->size() );
  } );
  mLineMaterial->setViewportSize( engine->size() );

  mLineEntity->addComponent( mLineMaterial );

  // Rubberband vertex markers
  mMarkerEntity = new Qt3DCore::QEntity( parentEntity );
  mMarkerGeometry = new QgsBillboardGeometry();
  mMarkerGeometryRenderer = new Qt3DRender::QGeometryRenderer;
  mMarkerGeometryRenderer->setPrimitiveType( Qt3DRender::QGeometryRenderer::Points );
  mMarkerGeometryRenderer->setGeometry( mMarkerGeometry );
  mMarkerGeometryRenderer->setVertexCount( mMarkerGeometry->count() );

  const QVariantMap props
  {
    {QStringLiteral( "color" ), QStringLiteral( "red" ) },
    {QStringLiteral( "size" ), 6 },
    {QStringLiteral( "outline_color" ), QStringLiteral( "green" ) },
    {QStringLiteral( "outline_width" ), 0.5 }
  };

  mMarkerSymbol = QgsMarkerSymbol::createSimple( props );
  updateMarkerMaterial();
  mMarkerEntity->addComponent( mMarkerGeometryRenderer );
}

QgsRubberBand3D::~QgsRubberBand3D()
{
  delete mLineEntity;
  delete mMarkerEntity;
  delete mMarkerSymbol;
}

float QgsRubberBand3D::width() const
{
  return mLineMaterial->lineWidth();
}

void QgsRubberBand3D::setWidth( float width )
{
  mLineMaterial->setLineWidth( width );
  mMarkerSymbol->setSize( width );
  updateMarkerMaterial();
}

QColor QgsRubberBand3D::color() const
{
  return mLineMaterial->lineColor();
}

void QgsRubberBand3D::setColor( QColor color )
{
  mLineMaterial->setLineColor( color );
  mMarkerSymbol->setColor( color.lighter( 130 ) );
  if ( mMarkerSymbol->symbolLayerCount() > 0 && mMarkerSymbol->symbolLayer( 0 )->layerType() == QLatin1String( "SimpleMarker" ) )
  {
    static_cast<QgsMarkerSymbolLayer *>( mMarkerSymbol->symbolLayer( 0 ) )->setStrokeColor( color );
  }
  updateMarkerMaterial();
}

void QgsRubberBand3D::reset()
{
  mLineString.clear();
  mShowLastMarker = false;
  updateGeometry();
}

void QgsRubberBand3D::addPoint( const QgsPoint &pt )
{
  mLineString.addVertex( pt );
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
  lineData.init( Qgis::AltitudeClamping::Absolute, Qgis::AltitudeBinding::Vertex, 0, Qgs3DRenderContext::fromMapSettings( mMapSettings ) );
  lineData.addLineString( mLineString );

  mPositionAttribute->buffer()->setData( lineData.createVertexBuffer() );
  mIndexAttribute->buffer()->setData( lineData.createIndexBuffer() );
  mGeomRenderer->setVertexCount( lineData.indexes.count() );

  // first entry is empty for primitive restart
  lineData.vertices.pop_front();

  // we may not want a marker on the last point as it's tracked by the mouse cursor
  if ( !mShowLastMarker && !lineData.vertices.isEmpty() )
    lineData.vertices.pop_back();

  mMarkerGeometry->setPoints( lineData.vertices );
  mMarkerGeometryRenderer->setVertexCount( lineData.vertices.count() );
}

void QgsRubberBand3D::updateMarkerMaterial()
{
  delete mMarkerMaterial;
  mMarkerMaterial = new QgsPoint3DBillboardMaterial();
  mMarkerMaterial->setTexture2DFromSymbol( mMarkerSymbol, Qgs3DRenderContext::fromMapSettings( mMapSettings ) );
  mMarkerEntity->addComponent( mMarkerMaterial );

  //TODO: QgsAbstract3DEngine::sizeChanged should have const QSize &size param
  QObject::connect( mEngine, &QgsAbstract3DEngine::sizeChanged, mMarkerMaterial, [this]
  {
    mMarkerMaterial->setViewportSize( mEngine->size() );
  } );
  mMarkerMaterial->setViewportSize( mEngine->size() );
}
/// @endcond
