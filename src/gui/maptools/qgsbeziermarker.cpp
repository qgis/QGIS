/***************************************************************************
    qgsbeziermarker.cpp  -  Visualization for Poly-Bézier curve digitizing
    ---------------------
    begin                : December 2025
    copyright            : (C) 2025 by Loïc Bartoletti
    email                : loic dot bartoletti at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

///@cond PRIVATE

#include "qgsbeziermarker.h"

#include "qgsmapcanvas.h"
#include "qgsrubberband.h"
#include "qgssettingsentryimpl.h"
#include "qgssettingsregistrycore.h"
#include "qgsvertexmarker.h"

QgsBezierMarker::QgsBezierMarker( QgsMapCanvas *canvas, QObject *parent )
  : QObject( parent )
  , mCanvas( canvas )
{
  mCurveRubberBand = new QgsRubberBand( canvas, Qgis::GeometryType::Line );
  mCurveRubberBand->setColor( QgsSettingsRegistryCore::settingsDigitizingLineColor->value() );
  mCurveRubberBand->setWidth( QgsSettingsRegistryCore::settingsDigitizingLineWidth->value() );
}

QgsBezierMarker::~QgsBezierMarker()
{
  clear();
  delete mCurveRubberBand;
}

QgsVertexMarker *QgsBezierMarker::createAnchorMarker()
{
  QgsVertexMarker *marker = new QgsVertexMarker( mCanvas );
  marker->setIconType( QgsVertexMarker::ICON_BOX );
  marker->setIconSize( 10 );
  const QColor snapColor = QgsSettingsRegistryCore::settingsDigitizingSnapColor->value();
  marker->setColor( snapColor );
  QColor fillColor = snapColor;
  fillColor.setAlpha( 100 );
  marker->setFillColor( fillColor );
  marker->setPenWidth( 2 );
  return marker;
}

QgsVertexMarker *QgsBezierMarker::createHandleMarker()
{
  QgsVertexMarker *marker = new QgsVertexMarker( mCanvas );
  marker->setIconType( QgsVertexMarker::ICON_CIRCLE );
  marker->setIconSize( 8 );
  QColor lineColor = QgsSettingsRegistryCore::settingsDigitizingLineColor->value();
  int h, s, v, a;
  lineColor.getHsv( &h, &s, &v, &a );
  QColor handleColor = QColor::fromHsv( ( h + 120 ) % 360, s, v, a );
  marker->setColor( handleColor );
  QColor fillColor = handleColor;
  fillColor.setAlpha( 100 );
  marker->setFillColor( fillColor );
  marker->setPenWidth( 2 );
  return marker;
}

QgsRubberBand *QgsBezierMarker::createHandleLine()
{
  QgsRubberBand *rb = new QgsRubberBand( mCanvas, Qgis::GeometryType::Line );
  QColor lineColor = QgsSettingsRegistryCore::settingsDigitizingLineColor->value();
  lineColor.setAlpha( 150 );
  rb->setColor( lineColor );
  rb->setWidth( 1 );
  rb->setLineStyle( Qt::DashLine );
  return rb;
}

void QgsBezierMarker::updateFromData( const QgsBezierData &data )
{
  updateAnchorMarkers( data );
  updateHandleMarkers( data );
  updateHandleLines( data );
  updateCurve( data );
}

void QgsBezierMarker::updateCurve( const QgsBezierData &data )
{
  mCurveRubberBand->reset( Qgis::GeometryType::Line );

  if ( data.anchorCount() < 1 )
    return;

  QgsPointSequence points = data.interpolate();

  for ( const QgsPoint &pt : std::as_const( points ) )
  {
    mCurveRubberBand->addPoint( QgsPointXY( pt ) );
  }

  mCurveRubberBand->setVisible( mVisible );
}

void QgsBezierMarker::updateAnchorMarkers( const QgsBezierData &data )
{
  while ( mAnchorMarkers.count() < data.anchorCount() )
  {
    mAnchorMarkers.append( createAnchorMarker() );
  }
  while ( mAnchorMarkers.count() > data.anchorCount() )
  {
    delete mAnchorMarkers.takeLast();
  }

  const QColor snapColor = QgsSettingsRegistryCore::settingsDigitizingSnapColor->value();
  QColor snapFillColor = snapColor;
  snapFillColor.setAlpha( 100 );

  for ( int i = 0; i < data.anchorCount(); ++i )
  {
    const QgsPoint &anchor = data.anchor( i );
    mAnchorMarkers[i]->setCenter( QgsPointXY( anchor ) );
    mAnchorMarkers[i]->setVisible( mVisible );

    if ( i == mHighlightedAnchor )
    {
      const QColor selColor = mCanvas->selectionColor();
      mAnchorMarkers[i]->setColor( selColor );
      QColor selFillColor = selColor;
      selFillColor.setAlpha( 150 );
      mAnchorMarkers[i]->setFillColor( selFillColor );
    }
    else
    {
      mAnchorMarkers[i]->setColor( snapColor );
      mAnchorMarkers[i]->setFillColor( snapFillColor );
    }
  }
}

void QgsBezierMarker::updateHandleMarkers( const QgsBezierData &data )
{
  while ( mHandleMarkers.count() < data.handleCount() )
  {
    mHandleMarkers.append( createHandleMarker() );
  }
  while ( mHandleMarkers.count() > data.handleCount() )
  {
    delete mHandleMarkers.takeLast();
  }

  QColor lineColor = QgsSettingsRegistryCore::settingsDigitizingLineColor->value();
  int h, s, v, a;
  lineColor.getHsv( &h, &s, &v, &a );
  QColor handleColor = QColor::fromHsv( ( h + 120 ) % 360, s, v, a );
  QColor handleFillColor = handleColor;
  handleFillColor.setAlpha( 100 );

  for ( int i = 0; i < data.handleCount(); ++i )
  {
    const QgsPoint &handle = data.handle( i );
    const int anchorIdx = i / 2;
    if ( anchorIdx >= data.anchorCount() )
    {
      mHandleMarkers[i]->setVisible( false );
      continue;
    }
    const QgsPoint &anchor = data.anchor( anchorIdx );

    const bool isRetracted = qFuzzyCompare( handle.x(), anchor.x() ) && qFuzzyCompare( handle.y(), anchor.y() );

    mHandleMarkers[i]->setCenter( QgsPointXY( handle ) );
    mHandleMarkers[i]->setVisible( mVisible && mHandlesVisible && !isRetracted );

    if ( i == mHighlightedHandle )
    {
      const QColor selColor = mCanvas->selectionColor();
      mHandleMarkers[i]->setColor( selColor );
      QColor selFillColor = selColor;
      selFillColor.setAlpha( 150 );
      mHandleMarkers[i]->setFillColor( selFillColor );
    }
    else
    {
      mHandleMarkers[i]->setColor( handleColor );
      mHandleMarkers[i]->setFillColor( handleFillColor );
    }
  }
}

void QgsBezierMarker::updateHandleLines( const QgsBezierData &data )
{
  while ( mHandleLines.count() < data.handleCount() )
  {
    mHandleLines.append( createHandleLine() );
  }
  while ( mHandleLines.count() > data.handleCount() )
  {
    delete mHandleLines.takeLast();
  }

  for ( int i = 0; i < data.handleCount(); ++i )
  {
    mHandleLines[i]->reset( Qgis::GeometryType::Line );

    const QgsPoint &handle = data.handle( i );
    const int anchorIdx = i / 2;
    if ( anchorIdx >= data.anchorCount() )
    {
      mHandleLines[i]->setVisible( false );
      continue;
    }
    const QgsPoint &anchor = data.anchor( anchorIdx );

    const bool isRetracted = qFuzzyCompare( handle.x(), anchor.x() ) && qFuzzyCompare( handle.y(), anchor.y() );

    if ( !isRetracted && mVisible && mHandlesVisible )
    {
      mHandleLines[i]->addPoint( QgsPointXY( anchor ) );
      mHandleLines[i]->addPoint( QgsPointXY( handle ) );
      mHandleLines[i]->setVisible( true );
    }
    else
    {
      mHandleLines[i]->setVisible( false );
    }
  }
}

void QgsBezierMarker::setVisible( bool visible )
{
  mVisible = visible;

  for ( QgsVertexMarker *marker : std::as_const( mAnchorMarkers ) )
    marker->setVisible( visible );

  for ( QgsVertexMarker *marker : std::as_const( mHandleMarkers ) )
    marker->setVisible( visible && mHandlesVisible );

  for ( QgsRubberBand *rb : std::as_const( mHandleLines ) )
    rb->setVisible( visible && mHandlesVisible );

  mCurveRubberBand->setVisible( visible );
}

void QgsBezierMarker::setHandlesVisible( bool visible )
{
  mHandlesVisible = visible;

  for ( QgsVertexMarker *marker : std::as_const( mHandleMarkers ) )
    marker->setVisible( mVisible && visible );

  for ( QgsRubberBand *rb : std::as_const( mHandleLines ) )
    rb->setVisible( mVisible && visible );
}

void QgsBezierMarker::clear()
{
  qDeleteAll( mAnchorMarkers );
  mAnchorMarkers.clear();

  qDeleteAll( mHandleMarkers );
  mHandleMarkers.clear();

  qDeleteAll( mHandleLines );
  mHandleLines.clear();

  if ( mCurveRubberBand )
    mCurveRubberBand->reset( Qgis::GeometryType::Line );

  mHighlightedAnchor = -1;
  mHighlightedHandle = -1;
}

void QgsBezierMarker::highlightAnchor( int idx )
{
  mHighlightedAnchor = idx;
}

void QgsBezierMarker::highlightHandle( int idx )
{
  mHighlightedHandle = idx;
}

///@endcond PRIVATE

#include "moc_qgsbeziermarker.cpp"
