/***************************************************************************
                         qgsdecorationtilegrid.cpp
                         ----------------------
    begin                : May 10, 2012
    copyright            : (C) 2012 by Etienne Tourigny
    email                : etourigny.dev at gmail dot com

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
#include "qgslogger.h"
#include "qgsmaplayer.h"
#include "qgsmapcanvas.h"
#include "qgsmaptopixel.h"
#include "qgspathresolver.h"
#include "qgspointxy.h"
#include "qgsproject.h"
#include "qgssymbollayerutils.h"
#include "qgssymbol.h"
#include "qgsmarkersymbollayer.h"
#include "qgsreadwritecontext.h"
#include "qgsrendercontext.h"
#include "qgstextrenderer.h"
#include "qgstiles.h"
#include "qgslinesymbol.h"
#include "qgsmarkersymbol.h"

#include <QPainter>
#include <QAction>
#include <QPen>
#include <QPolygon>
#include <QString>
#include <QFontMetrics>
#include <QFont>
#include <QColor>
#include <QMenu>
#include <QFile>
#include <QLocale>
#include <QDomDocument>
#include <QMessageBox>

//non qt includes
#include <cmath>


QgsDecorationTileGrid::QgsDecorationTileGrid( QObject *parent )
  : QgsDecorationItem( parent )
{
  setDisplayName( tr( "TileGrid" ) );
  mConfigurationName = QStringLiteral( "TileGrid" );

  projectRead();

  connect( QgisApp::instance()->mapCanvas(), &QgsMapCanvas::destinationCrsChanged,
           this, &QgsDecorationTileGrid::checkMapUnitsChanged );
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

  mEnabled = QgsProject::instance()->readBoolEntry( mConfigurationName, QStringLiteral( "/Enabled" ), false );
  mMapUnits = static_cast< QgsUnitTypes::DistanceUnit >( QgsProject::instance()->readNumEntry( mConfigurationName, QStringLiteral( "/MapUnits" ),
              QgsUnitTypes::DistanceUnknownUnit ) );
  mGridStyle = static_cast< GridStyle >( QgsProject::instance()->readNumEntry( mConfigurationName, QStringLiteral( "/Style" ),
                                         QgsDecorationTileGrid::Line ) );
  mDynamicOrStaticGrid = static_cast< DynamicOrStaticGrid >( QgsProject::instance()->readNumEntry( mConfigurationName, QStringLiteral( "/DynamicOrStatic" ),
                         DynamicOrStaticGrid::Dynamic ) );
  mZoomFactor = QgsProject::instance()->readDoubleEntry( mConfigurationName, QStringLiteral( "/ZoomFactor" ), 1.0 );
  mZoomLevel = QgsProject::instance()->readNumEntry( mConfigurationName, QStringLiteral( "/ZoomLevel" ), 0 );
  mShowGridAnnotation = QgsProject::instance()->readBoolEntry( mConfigurationName, QStringLiteral( "/ShowAnnotation" ), false );

  QDomDocument doc;
  QDomElement elem;
  const QString textXml = QgsProject::instance()->readEntry( mConfigurationName, QStringLiteral( "/Font" ) );
  if ( !textXml.isEmpty() )
  {
    doc.setContent( textXml );
    elem = doc.documentElement();
    QgsReadWriteContext rwContext;
    rwContext.setPathResolver( QgsProject::instance()->pathResolver() );
    mTextFormat.readXml( elem, rwContext );
  }

  // read symbol info from xml
  QString xml;
  QgsReadWriteContext rwContext;
  rwContext.setPathResolver( QgsProject::instance()->pathResolver() );

  if ( mLineSymbol )
    setLineSymbol( nullptr );
  xml = QgsProject::instance()->readEntry( mConfigurationName, QStringLiteral( "/LineSymbol" ) );
  if ( !xml.isEmpty() )
  {
    doc.setContent( xml );
    elem = doc.documentElement();
    mLineSymbol.reset( QgsSymbolLayerUtils::loadSymbol<QgsLineSymbol>( elem, rwContext ) );
  }
  if ( ! mLineSymbol )
    mLineSymbol = std::make_unique<QgsLineSymbol>();

  if ( mMarkerSymbol )
    setMarkerSymbol( nullptr );
  xml = QgsProject::instance()->readEntry( mConfigurationName, QStringLiteral( "/MarkerSymbol" ) );
  if ( !xml.isEmpty() )
  {
    doc.setContent( xml );
    elem = doc.documentElement();
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
  QgsProject::instance()->writeEntry( mConfigurationName, QStringLiteral( "/Enabled" ), mEnabled );
  QgsProject::instance()->writeEntry( mConfigurationName, QStringLiteral( "/MapUnits" ), static_cast< int >( mMapUnits ) );
  QgsProject::instance()->writeEntry( mConfigurationName, QStringLiteral( "/Style" ), static_cast< int >( mGridStyle ) );
  QgsProject::instance()->writeEntry( mConfigurationName, QStringLiteral( "/DynamicOrStatic" ), static_cast< int >( mDynamicOrStaticGrid ) );
  QgsProject::instance()->writeEntry( mConfigurationName, QStringLiteral( "/ZoomFactor" ), mZoomFactor );
  QgsProject::instance()->writeEntry( mConfigurationName, QStringLiteral( "/ZoomLevel" ), mZoomLevel );
  QgsProject::instance()->writeEntry( mConfigurationName, QStringLiteral( "/ShowAnnotation" ), mShowGridAnnotation );
  QgsProject::instance()->writeEntry( mConfigurationName, QStringLiteral( "/AnnotationFont" ), mGridAnnotationFont.toString() );

  QDomDocument textDoc;
  QgsReadWriteContext rwContext;
  rwContext.setPathResolver( QgsProject::instance()->pathResolver() );
  const QDomElement textElem = mTextFormat.writeXml( textDoc, rwContext );
  textDoc.appendChild( textElem );
  QgsProject::instance()->writeEntry( mConfigurationName, QStringLiteral( "/Font" ), textDoc.toString() );

  // write symbol info to xml
  QDomDocument doc;
  QDomElement elem;
  rwContext.setPathResolver( QgsProject::instance()->pathResolver() );
  if ( mLineSymbol )
  {
    elem = QgsSymbolLayerUtils::saveSymbol( QStringLiteral( "line symbol" ), mLineSymbol.get(), doc, rwContext );
    doc.appendChild( elem );
    // FIXME this works, but XML will not be valid as < is replaced by &lt;
    QgsProject::instance()->writeEntry( mConfigurationName, QStringLiteral( "/LineSymbol" ), doc.toString() );
  }
  if ( mMarkerSymbol )
  {
    doc.setContent( QString() );
    elem = QgsSymbolLayerUtils::saveSymbol( QStringLiteral( "marker symbol" ), mMarkerSymbol.get(), doc, rwContext );
    doc.appendChild( elem );
    QgsProject::instance()->writeEntry( mConfigurationName, QStringLiteral( "/MarkerSymbol" ), doc.toString() );
  }

}

void QgsDecorationTileGrid::run()
{
  QgsDecorationTileGridDialog dlg( *this );

  if ( dlg.exec() )
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
  return std::max( static_cast<int>( std::log2( ratio * mZoomFactor ) ) + 2, 1 );
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
    poly[0] = m2p.transform( matrix.extent().xMinimum(),
                             matrix.extent().yMaximum() ).toQPointF();
    poly[1] = m2p.transform( matrix.extent().xMinimum(),
                             matrix.extent().yMinimum() ).toQPointF();
    poly[2] = m2p.transform( matrix.extent().xMaximum(),
                             matrix.extent().yMinimum() ).toQPointF();
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
  if ( ! mEnabled || mapSettings.destinationCrs().srsid() != 3857 )
    return;

  const auto zoom = mDynamicOrStaticGrid == DynamicOrStaticGrid::Dynamic ? zoomFromVisibleExtent( mapSettings ) : mZoomLevel;

  const auto matrix = QgsTileMatrix::fromWebMercator( zoom );
  const auto range = matrix.tileRangeFromExtent( context.extent() );

  switch ( mGridStyle )
  {
    case Line:
      if ( ! mLineSymbol )
        return;
      drawLines( context, mLineSymbol.get(), matrix, range );
      break;
    case Marker:
      if ( ! mMarkerSymbol )
        return;
      drawMarkers( context, mMarkerSymbol.get(), matrix, range );
      break;
  }

  if ( mShowGridAnnotation )
  {
    drawTileCoordinates( context, zoom );
  }
}

static double tileWidthInPixels( const QgsMapToPixel &m2p, int zoom )
{
  const auto matrix = QgsTileMatrix::fromWebMercator( zoom );
  const QgsTileXYZ tile( 0, 0, zoom );
  return matrix.tileExtent( tile ).width() / m2p.mapUnitsPerPixel();
}

void QgsDecorationTileGrid::drawTileCoordinates( QgsRenderContext &context, int zoom )
{
  auto matrix = QgsTileMatrix::fromWebMercator( zoom );
  const auto range = matrix.tileRangeFromExtent( context.extent() );
  const QgsMapToPixel &m2p = context.mapToPixel();

  if ( tileWidthInPixels( m2p, zoom ) < 400 )
    return;

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

void QgsDecorationTileGrid::checkMapUnitsChanged()
{
  // disable grid if units changed and grid is enabled, and update the canvas
  // this is to avoid problems when CRS changes
  const QgsUnitTypes::DistanceUnit mapUnits = QgisApp::instance()->mapCanvas()->mapSettings().mapUnits();
  if ( mEnabled && ( mMapUnits != mapUnits ) )
  {
    mEnabled = false;
    mMapUnits = QgsUnitTypes::DistanceUnknownUnit; // make sure isDirty() returns true
    if ( ! QgisApp::instance()->mapCanvas()->isFrozen() )
    {
      update();
    }
  }
}

bool QgsDecorationTileGrid::isDirty()
{
  // checks if stored map units is undefined or different from canvas map units
  // or if interval is 0
  return mMapUnits == QgsUnitTypes::DistanceUnknownUnit ||
         mMapUnits != QgisApp::instance()->mapCanvas()->mapSettings().mapUnits();
}

void QgsDecorationTileGrid::setDirty( bool dirty )
{
  if ( dirty )
  {
    mMapUnits = QgsUnitTypes::DistanceUnknownUnit;
  }
  else
  {
    mMapUnits = QgisApp::instance()->mapCanvas()->mapSettings().mapUnits();
  }
}
