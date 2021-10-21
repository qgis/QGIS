/***************************************************************************
 qgsdecorationtilegrid.cpp
 -------------------------
 Date: 24-Nov-2021
 Copyright: (C) 2021 by Jochen Topf
 Email: jochen@topf.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdecorationtilegrid.h"
#include "qgsdecorationtilegriddialog.h"

#include "qgisapp.h"
#include "qgslinesymbol.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayer.h"
#include "qgsmaptopixel.h"
#include "qgsmarkersymbol.h"
#include "qgsmarkersymbollayer.h"
#include "qgspathresolver.h"
#include "qgspointxy.h"
#include "qgsproject.h"
#include "qgsreadwritecontext.h"
#include "qgsrendercontext.h"
#include "qgssymbol.h"
#include "qgssymbollayerutils.h"
#include "qgstextrenderer.h"
#include "qgstiles.h"

#include <QFont>
#include <QPolygon>
#include <QString>

#include <array>

static constexpr const int MinZoomLevel = 0;
static constexpr const int MaxZoomLevel = 22;
static constexpr const int MaxGridLines = 50;

static const std::array<QString, 3> annotationNames = { "None", "Center", "Border" };

// The tile grid decoration only works in WebMercator CRS
static bool allowedCrs( const QgsMapSettings &mapSettings )
{
  return mapSettings.destinationCrs().srsid() == 3857;
}

QgsDecorationTileGrid::QgsDecorationTileGrid( QObject *parent )
  : QgsDecorationItem( parent )
{
  setDisplayName( tr( "TileGrid" ) );
  mConfigurationName = QStringLiteral( "TileGrid" );

  projectRead();

  connect( QgisApp::instance()->mapCanvas(), &QgsMapCanvas::destinationCrsChanged,
           this, &QgsDecorationTileGrid::mapCrsChanged );
}

QgsDecorationTileGrid::~QgsDecorationTileGrid() = default;

void QgsDecorationTileGrid::setLineSymbol( QgsLineSymbol *symbol )
{
  mLineSymbol.reset( symbol );
}

void QgsDecorationTileGrid::setMarkerSymbol( QgsMarkerSymbol *symbol )
{
  mMarkerSymbol.reset( symbol );
}

void QgsDecorationTileGrid::projectRead()
{
  QgsDecorationItem::projectRead();

  const auto gridStyle = QgsProject::instance()->readEntry( mConfigurationName, QStringLiteral( "/Style" ) );
  if ( gridStyle == "Marker" )
    mGridStyle = GridStyle::Marker;
  else
    mGridStyle = GridStyle::Line;

  const auto dynamicOrStatic = QgsProject::instance()->readEntry( mConfigurationName, QStringLiteral( "/DynamicOrStatic" ) );
  if ( dynamicOrStatic == "Static" )
    mDynamicOrStaticGrid = DynamicOrStaticGrid::Static;
  else
    mDynamicOrStaticGrid = DynamicOrStaticGrid::Dynamic;

  mZoomFactor = QgsProject::instance()->readDoubleEntry( mConfigurationName, QStringLiteral( "/ZoomFactor" ), 1.0 );
  mZoomLevel = QgsProject::instance()->readNumEntry( mConfigurationName, QStringLiteral( "/ZoomLevel" ), 3 );

  const auto gridAnnotationStyle = QgsProject::instance()->readEntry( mConfigurationName, QStringLiteral( "/GridAnnotationStyle" ), QStringLiteral( "None" ) );
  for ( std::size_t i = 0; i < annotationNames.size(); ++i )
  {
    if ( gridAnnotationStyle == annotationNames[i] )
    {
      mGridAnnotationStyle = static_cast<GridAnnotationStyle>( i );
      break;
    }
  }

  QgsReadWriteContext rwContext;
  rwContext.setPathResolver( QgsProject::instance()->pathResolver() );
  QDomDocument doc;

  const QString textXml = QgsProject::instance()->readEntry( mConfigurationName, QStringLiteral( "/Font" ) );
  if ( !textXml.isEmpty() )
  {
    doc.setContent( textXml );
    const auto elem = doc.documentElement();
    mTextFormat.readXml( elem, rwContext );
  }

  // read symbol info from xml
  QString xml;

  if ( mLineSymbol )
    mLineSymbol.reset();
  xml = QgsProject::instance()->readEntry( mConfigurationName, QStringLiteral( "/LineSymbol" ) );
  if ( !xml.isEmpty() )
  {
    doc.setContent( xml );
    const auto elem = doc.documentElement();
    mLineSymbol.reset( QgsSymbolLayerUtils::loadSymbol<QgsLineSymbol>( elem, rwContext ) );
  }
  if ( ! mLineSymbol )
    mLineSymbol = std::make_unique<QgsLineSymbol>();

  if ( mMarkerSymbol )
    mMarkerSymbol.reset();
  xml = QgsProject::instance()->readEntry( mConfigurationName, QStringLiteral( "/MarkerSymbol" ) );
  if ( !xml.isEmpty() )
  {
    doc.setContent( xml );
    const auto elem = doc.documentElement();
    mMarkerSymbol.reset( QgsSymbolLayerUtils::loadSymbol<QgsMarkerSymbol>( elem, rwContext ) );
  }
  if ( ! mMarkerSymbol )
  {
    // set default symbol : cross with width=3
    QgsSymbolLayerList symbolList;
    symbolList << new QgsSimpleMarkerSymbolLayer( Qgis::MarkerShape::Cross, 3, 0 );
    mMarkerSymbol = std::make_unique<QgsMarkerSymbol>( symbolList );
  }
}

void QgsDecorationTileGrid::saveToProject()
{
  QgsDecorationItem::saveToProject();

  QgsProject::instance()->writeEntry( mConfigurationName, QStringLiteral( "/Style" ), mGridStyle == GridStyle::Line ? QStringLiteral( "Line" ) : QStringLiteral( "Marker" ) );
  QgsProject::instance()->writeEntry( mConfigurationName, QStringLiteral( "/DynamicOrStatic" ), mDynamicOrStaticGrid == DynamicOrStaticGrid::Dynamic ? QStringLiteral( "Dynamic" ) : QStringLiteral( "Static" ) );
  QgsProject::instance()->writeEntry( mConfigurationName, QStringLiteral( "/ZoomFactor" ), mZoomFactor );
  QgsProject::instance()->writeEntry( mConfigurationName, QStringLiteral( "/ZoomLevel" ), mZoomLevel );

  QgsProject::instance()->writeEntry( mConfigurationName, QStringLiteral( "/GridAnnotationStyle" ), annotationNames[ static_cast<int>( mGridAnnotationStyle ) ] );

  QgsReadWriteContext rwContext;
  rwContext.setPathResolver( QgsProject::instance()->pathResolver() );

  {
    QDomDocument doc;
    const auto elem = mTextFormat.writeXml( doc, rwContext );
    doc.appendChild( elem );
    QgsProject::instance()->writeEntry( mConfigurationName, QStringLiteral( "/Font" ), doc.toString() );
  }

  if ( mLineSymbol )
  {
    QDomDocument doc;
    const auto elem = QgsSymbolLayerUtils::saveSymbol( QStringLiteral( "line symbol" ), mLineSymbol.get(), doc, rwContext );
    doc.appendChild( elem );
    QgsProject::instance()->writeEntry( mConfigurationName, QStringLiteral( "/LineSymbol" ), doc.toString() );
  }

  if ( mMarkerSymbol )
  {
    QDomDocument doc;
    const auto elem = QgsSymbolLayerUtils::saveSymbol( QStringLiteral( "marker symbol" ), mMarkerSymbol.get(), doc, rwContext );
    doc.appendChild( elem );
    QgsProject::instance()->writeEntry( mConfigurationName, QStringLiteral( "/MarkerSymbol" ), doc.toString() );
  }
}

void QgsDecorationTileGrid::run()
{
  QgsDecorationTileGridDialog dialog( *this );

  if ( dialog.exec() )
  {
    update();
  }
}

int QgsDecorationTileGrid::zoomFromVisibleExtent( const QgsMapSettings &mapSettings ) const
{
  const auto matrix = QgsTileMatrix::fromWebMercator( 0 );
  const QgsRectangle extent = mapSettings.visibleExtent();
  const double minWidth = std::min( extent.width(), extent.height() );
  const double ratio = matrix.extent().width() / minWidth;
  return std::min( std::max( static_cast<int>( std::log2( ratio * mZoomFactor ) ) + 2, MinZoomLevel ), MaxZoomLevel );
}

static QgsRectangle rectangleToPixel( const QgsMapToPixel &m2p, const QgsRectangle &rect )
{
  const auto topLeft = m2p.transform( rect.xMinimum(), rect.yMinimum() );
  const auto bottomRight = m2p.transform( rect.xMaximum(), rect.yMaximum() );
  return { topLeft, bottomRight };
}

static void drawLines( QgsRenderContext &context, QgsLineSymbol *symbol, const QgsTileMatrix &matrix, const QgsTileRange &range )
{
  const int zoom = matrix.zoomLevel();
  const auto &m2p = context.mapToPixel();

  std::vector<QPolygonF> lines;
  lines.reserve( ( range.endColumn() - range.startColumn() + 1 ) +
                 ( range.endRow() - range.startRow() + 1 )
                 + 1 );
  {
    lines.emplace_back( 3 );
    auto &poly = lines.back();
    const auto extent = rectangleToPixel( m2p, matrix.extent() );

    poly[0] = QPointF( extent.xMaximum(), extent.yMaximum() );
    poly[1] = QPointF( extent.xMinimum(), extent.yMaximum() );
    poly[2] = QPointF( extent.xMinimum(), extent.yMinimum() );
  }

  for ( int c = range.startColumn(); c <= range.endColumn(); ++c )
  {
    QgsTileXYZ tile_start( c, range.startRow(), zoom );
    QgsTileXYZ tile_end( c, range.endRow(), zoom );

    lines.emplace_back( 2 );
    auto &poly = lines.back();
    poly[0] = m2p.transform( matrix.tileExtent( tile_start ).xMaximum(),
                             matrix.tileExtent( tile_start ).yMaximum() ).toQPointF();
    poly[1] = m2p.transform( matrix.tileExtent( tile_end ).xMaximum(),
                             matrix.tileExtent( tile_end ).yMinimum() ).toQPointF();
  }

  for ( int r = range.startRow(); r <= range.endRow(); ++r )
  {
    QgsTileXYZ tile_start( range.startColumn(), r, zoom );
    QgsTileXYZ tile_end( range.endColumn(), r, zoom );

    lines.emplace_back( 2 );
    auto &poly = lines.back();
    poly[0] = m2p.transform( matrix.tileExtent( tile_start ).xMinimum(),
                             matrix.tileExtent( tile_start ).yMaximum() ).toQPointF();
    poly[1] = m2p.transform( matrix.tileExtent( tile_end ).xMaximum(),
                             matrix.tileExtent( tile_end ).yMaximum() ).toQPointF();
  }

  symbol->startRender( context );
  for ( int layer = 0; layer < symbol->symbolLayerCount(); ++layer )
  {
    for ( const auto &line : lines )
    {
      symbol->renderPolyline( line, nullptr, context, layer );
    }
  }
  symbol->stopRender( context );
}

static void drawMarkers( QgsRenderContext &context, QgsMarkerSymbol *symbol, const QgsTileMatrix &matrix, const QgsTileRange &range )
{
  const int zoom = matrix.zoomLevel();
  const auto &m2p = context.mapToPixel();

  std::vector<QPointF> points;
  points.reserve( ( range.endColumn() - range.startColumn() + 2 ) *
                  ( range.endRow() - range.startRow() + 2 ) );

  for ( int c = range.startColumn(); c <= range.endColumn(); ++c )
  {
    for ( int r = range.startRow(); r <= range.endRow(); ++r )
    {
      const auto extent = matrix.tileExtent( QgsTileXYZ( c, r, zoom ) );
      points.push_back( m2p.transform( extent.xMinimum(), extent.yMinimum() ).toQPointF() );

      if ( c == range.endColumn() )
      {
        points.push_back( m2p.transform( extent.xMaximum(), extent.yMinimum() ).toQPointF() );
      }
    }
    const auto extent = matrix.tileExtent( QgsTileXYZ( c, range.startRow(), zoom ) );
    points.push_back( m2p.transform( extent.xMinimum(), extent.yMaximum() ).toQPointF() );
  }
  const auto extent = matrix.tileExtent( QgsTileXYZ( range.endColumn(), range.startRow(), zoom ) );
  points.push_back( m2p.transform( extent.xMaximum(), extent.yMaximum() ).toQPointF() );

  symbol->startRender( context );
  for ( int layer = 0; layer < symbol->symbolLayerCount(); ++layer )
  {
    for ( const auto point : points )
    {
      symbol->renderPoint( point, nullptr, context, layer );
    }
  }
  symbol->stopRender( context );
}

void QgsDecorationTileGrid::render( const QgsMapSettings &mapSettings, QgsRenderContext &context )
{
  if ( ! mEnabled || !allowedCrs( mapSettings ) )
    return;

  const auto zoom = mDynamicOrStaticGrid == DynamicOrStaticGrid::Dynamic ? zoomFromVisibleExtent( mapSettings ) : mZoomLevel;

  const auto matrix = QgsTileMatrix::fromWebMercator( zoom );
  const auto range = matrix.tileRangeFromExtent( context.extent() );

  // Don't draw grid if there would be too many grid lines/markers obscuring the map
  if ( ( range.endColumn() - range.startColumn() > MaxGridLines ) ||
       ( range.endRow() - range.startRow() > MaxGridLines ) )
    return;

  if ( mGridStyle == GridStyle::Line && mLineSymbol )
    drawLines( context, mLineSymbol.get(), matrix, range );
  else if ( mGridStyle == GridStyle::Marker && mMarkerSymbol )
    drawMarkers( context, mMarkerSymbol.get(), matrix, range );

  if ( showGridAnnotation() )
    drawTileCoordinates( context, zoom );
}

static double tileWidthInPixels( const QgsMapToPixel &m2p, int zoom )
{
  const auto matrix = QgsTileMatrix::fromWebMercator( zoom );
  const QgsTileXYZ tile( 0, 0, zoom );
  return matrix.tileExtent( tile ).width() / m2p.mapUnitsPerPixel();
}

void QgsDecorationTileGrid::drawTileCoordinates( QgsRenderContext &context, int zoom ) const
{
  constexpr const int borderWidth = 10;
  constexpr const int minSizeToDraw = 200;

  auto matrix = QgsTileMatrix::fromWebMercator( zoom );
  const auto range = matrix.tileRangeFromExtent( context.extent() );
  const QgsMapToPixel &m2p = context.mapToPixel();

  if ( tileWidthInPixels( m2p, zoom ) < minSizeToDraw )
    return;

  if ( mGridAnnotationStyle == GridAnnotationStyle::Border )
  {
    QStringList outputStringList;
    outputStringList << ( QString( "Z" ) + QString::number( zoom ) );
    QgsTextRenderer::drawText( QRectF( borderWidth, borderWidth, 100, 100 ), 0, QgsTextRenderer::AlignLeft, outputStringList, context, mTextFormat );

    for ( int c = range.startColumn(); c <= range.endColumn(); ++c )
    {
      QgsTileXYZ tile( c, 0, zoom );
      auto extent = rectangleToPixel( m2p, matrix.tileExtent( tile ) );
      extent.setYMinimum( borderWidth );
      extent.setYMaximum( qInf() );
      if ( extent.xMinimum() < 100 )
        extent.setXMinimum( 100 );
      if ( extent.width() >= minSizeToDraw )
      {
        outputStringList[0].setNum( tile.column() );
        QgsTextRenderer::drawText( extent.toRectF(), 0, QgsTextRenderer::AlignCenter, outputStringList, context, mTextFormat, true, QgsTextRenderer::AlignTop );
      }
    }

    for ( int r = range.startRow(); r <= range.endRow(); ++r )
    {
      QgsTileXYZ tile( 0, r, zoom );
      auto extent = rectangleToPixel( m2p, matrix.tileExtent( tile ) );
      extent.setXMinimum( borderWidth );
      extent.setXMaximum( qInf() );
      if ( extent.yMinimum() < 100 )
        extent.setYMinimum( 100 );
      if ( extent.height() >= minSizeToDraw )
      {
        outputStringList[0].setNum( tile.row() );
        QgsTextRenderer::drawText( extent.toRectF(), 0, QgsTextRenderer::AlignLeft, outputStringList, context, mTextFormat, true, QgsTextRenderer::AlignVCenter );
      }
    }
  }
  else if ( mGridAnnotationStyle == GridAnnotationStyle::Center )
  {
    for ( int c = range.startColumn(); c <= range.endColumn(); ++c )
    {
      for ( int r = range.startRow(); r <= range.endRow(); ++r )
      {
        QgsTileXYZ tile( c, r, zoom );
        const auto center = m2p.transform( matrix.tileCenter( tile ) ).toQPointF();
        const QStringList annotationStringList = QStringList() << tile.toString();
        QgsTextRenderer::drawText( center, 0, QgsTextRenderer::AlignCenter, annotationStringList, context, mTextFormat );
      }
    }
  }
}

void QgsDecorationTileGrid::mapCrsChanged()
{
  if ( ! allowedCrs( QgisApp::instance()->mapCanvas()->mapSettings() ) )
    mEnabled = false;
}

