/***************************************************************************
    qgsbeziermarker.cpp  -  Visualization for Poly-Bézier curve digitizing
    ---------------------
    begin                : December 2025
    copyright            : (C) 2025 by Loïc Bartoletti
                           Adapted from BezierEditing plugin work by Takayuki Mizutani
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
  , mCurveRubberBand( new QgsRubberBand( canvas, Qgis::GeometryType::Line ) )
{
  mCurveRubberBand->setColor( QgsSettingsRegistryCore::settingsDigitizingLineColor->value() );
  mCurveRubberBand->setWidth( QgsSettingsRegistryCore::settingsDigitizingLineWidth->value() );
}

QgsBezierMarker::~QgsBezierMarker()
{
  qDeleteAll( mAnchorMarkers );
  qDeleteAll( mHandleMarkers );
}

QgsVertexMarker *QgsBezierMarker::createAnchorMarker()
{
  QgsVertexMarker *marker = new QgsVertexMarker( mCanvas );
  marker->setIconType( QgsVertexMarker::ICON_BOX );
  marker->setIconSize( MarkerConfig::AnchorMarkerSize );
  const QColor snapColor = QgsSettingsRegistryCore::settingsDigitizingSnapColor->value();
  marker->setColor( snapColor );
  QColor fillColor = snapColor;
  fillColor.setAlpha( MarkerConfig::FillAlpha );
  marker->setFillColor( fillColor );
  marker->setPenWidth( MarkerConfig::MarkerPenWidth );
  return marker;
}

QgsVertexMarker *QgsBezierMarker::createHandleMarker()
{
  QgsVertexMarker *marker = new QgsVertexMarker( mCanvas );
  marker->setIconType( QgsVertexMarker::ICON_CIRCLE );
  marker->setIconSize( MarkerConfig::HandleMarkerSize );
  QColor lineColor = QgsSettingsRegistryCore::settingsDigitizingLineColor->value();
  int h, s, v, a;
  lineColor.getHsv( &h, &s, &v, &a );
  QColor handleColor = QColor::fromHsv( ( h + 120 ) % 360, s, v, a );
  marker->setColor( handleColor );
  QColor fillColor = handleColor;
  fillColor.setAlpha( MarkerConfig::FillAlpha );
  marker->setFillColor( fillColor );
  marker->setPenWidth( MarkerConfig::MarkerPenWidth );
  return marker;
}

QObjectUniquePtr<QgsRubberBand> QgsBezierMarker::createHandleLine()
{
  QObjectUniquePtr<QgsRubberBand> rb( new QgsRubberBand( mCanvas, Qgis::GeometryType::Line ) );
  QColor lineColor = QgsSettingsRegistryCore::settingsDigitizingLineColor->value();
  lineColor.setAlpha( MarkerConfig::HandleLineAlpha );
  rb->setColor( lineColor );
  rb->setWidth( MarkerConfig::HandleLineWidth );
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

  QgsPointSequence points = data.interpolateLine();

  for ( const QgsPoint &pt : std::as_const( points ) )
  {
    mCurveRubberBand->addPoint( QgsPointXY( pt ) );
  }

  mCurveRubberBand->setVisible( mVisible );
}

void QgsBezierMarker::updateAnchorMarkers( const QgsBezierData &data )
{
  while ( static_cast<int>( mAnchorMarkers.size() ) < data.anchorCount() )
  {
    mAnchorMarkers.push_back( createAnchorMarker() );
  }
  while ( static_cast<int>( mAnchorMarkers.size() ) > data.anchorCount() )
  {
    delete mAnchorMarkers.back();
    mAnchorMarkers.pop_back();
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
      selFillColor.setAlpha( MarkerConfig::SelectionAlpha );
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
  while ( static_cast<int>( mHandleMarkers.size() ) < data.handleCount() )
  {
    mHandleMarkers.push_back( createHandleMarker() );
  }
  while ( static_cast<int>( mHandleMarkers.size() ) > data.handleCount() )
  {
    delete mHandleMarkers.back();
    mHandleMarkers.pop_back();
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
    const int anchorIndex = i / 2;
    if ( anchorIndex >= data.anchorCount() )
    {
      mHandleMarkers[i]->setVisible( false );
      continue;
    }
    const QgsPoint &anchor = data.anchor( anchorIndex );

    const bool isRetracted = qgsDoubleNear( handle.x(), anchor.x() ) && qgsDoubleNear( handle.y(), anchor.y() );

    mHandleMarkers[i]->setCenter( QgsPointXY( handle ) );
    mHandleMarkers[i]->setVisible( mVisible && mHandlesVisible && !isRetracted );

    if ( i == mHighlightedHandle )
    {
      const QColor selColor = mCanvas->selectionColor();
      mHandleMarkers[i]->setColor( selColor );
      QColor selFillColor = selColor;
      selFillColor.setAlpha( MarkerConfig::SelectionAlpha );
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
  while ( static_cast<int>( mHandleLines.size() ) < data.handleCount() )
  {
    mHandleLines.push_back( createHandleLine() );
  }
  while ( static_cast<int>( mHandleLines.size() ) > data.handleCount() )
  {
    mHandleLines.pop_back();
  }

  for ( int i = 0; i < data.handleCount(); ++i )
  {
    mHandleLines[i]->reset( Qgis::GeometryType::Line );

    const QgsPoint &handle = data.handle( i );
    const int anchorIndex = i / 2;
    if ( anchorIndex >= data.anchorCount() )
    {
      mHandleLines[i]->setVisible( false );
      continue;
    }
    const QgsPoint &anchor = data.anchor( anchorIndex );

    const bool isRetracted = qgsDoubleNear( handle.x(), anchor.x() ) && qgsDoubleNear( handle.y(), anchor.y() );

    if ( !isRetracted && mVisible && mHandlesVisible )
    {
      mHandleLines[i]->addPoint( QgsPointXY( anchor ) );
      mHandleLines[i]->addPoint( QgsPointXY( handle ) );
      mHandleLines[i]->setVisible( true );
    }
  }
}

void QgsBezierMarker::setVisible( bool visible )
{
  mVisible = visible;

  for ( const auto &marker : mAnchorMarkers )
    marker->setVisible( visible );

  for ( const auto &marker : mHandleMarkers )
    marker->setVisible( visible && mHandlesVisible );

  for ( const auto &rb : mHandleLines )
    rb->setVisible( visible && mHandlesVisible );

  mCurveRubberBand->setVisible( visible );
}

void QgsBezierMarker::setHandlesVisible( bool visible )
{
  mHandlesVisible = visible;

  for ( const auto &marker : mHandleMarkers )
    marker->setVisible( mVisible && visible );

  for ( const auto &rb : mHandleLines )
    rb->setVisible( mVisible && visible );
}

void QgsBezierMarker::clear()
{
  qDeleteAll( mAnchorMarkers );
  mAnchorMarkers.clear();
  qDeleteAll( mHandleMarkers );
  mHandleMarkers.clear();
  mHandleLines.clear();

  if ( mCurveRubberBand )
    mCurveRubberBand->reset( Qgis::GeometryType::Line );

  mHighlightedAnchor = -1;
  mHighlightedHandle = -1;
}

void QgsBezierMarker::setHighlightedAnchor( int idx )
{
  mHighlightedAnchor = idx;
}

void QgsBezierMarker::setHighlightedHandle( int idx )
{
  mHighlightedHandle = idx;
}

///@endcond PRIVATE

#include "moc_qgsbeziermarker.cpp"
