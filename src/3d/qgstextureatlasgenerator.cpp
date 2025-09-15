/***************************************************************************
  qgstextureatlasgenerator.cpp
  --------------------------------------
  Date                 : September 2025
  Copyright            : (C) 2025 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstextureatlasgenerator.h"
#include "qgscolorrampimpl.h"
#include <QPainter>

// rectpack2D library
#include <finders_interface.h>

///@cond PRIVATE

class QgsTextureRect
{
  public:
    QgsTextureRect( const rectpack2D::rect_xywh &rect, int id, const QImage &image = QImage() )
      : rect( rect )
      , id( id )
      , image( image )
    {
    }

    // get_rect must be implemented for rectpack2D compatibility:
    auto &get_rect()
    {
      return rect;
    }
    const auto &get_rect() const
    {
      return rect;
    }

    QRect asQRect() const
    {
      return QRect( rect.x, rect.y, rect.w, rect.h );
    }

    rectpack2D::rect_xywh rect;
    int id = 0;
    QImage image;
};

///@endcond

QgsTextureAtlasGenerator::QgsTextureAtlasGenerator() = default;
QgsTextureAtlasGenerator::~QgsTextureAtlasGenerator() = default;


int QgsTextureAtlasGenerator::appendRect( const QRect &rect )
{
  const int id = static_cast< int >( mRects.size() );
  mRects.emplace_back( QgsTextureRect( rectpack2D::rect_xywh( 0, 0, rect.width(), rect.height() ), id ) );
  return id;
}

int QgsTextureAtlasGenerator::appendImage( const QImage &image )
{
  const int id = static_cast< int >( mRects.size() );
  mRects.emplace_back( QgsTextureRect( rectpack2D::rect_xywh( 0, 0, image.width(), image.height() ), id, image ) );
  return id;
}

bool QgsTextureAtlasGenerator::generateAtlas( int maxSide )
{
  using spacesType = rectpack2D::empty_spaces<false, rectpack2D::default_empty_spaces>;

  bool result = true;
  auto reportSuccessful = []( rectpack2D::rect_xywh & ) {
    return rectpack2D::callback_result::CONTINUE_PACKING;
  };

  auto reportUnsuccessful = [&result]( rectpack2D::rect_xywh & ) {
    result = false;
    return rectpack2D::callback_result::ABORT_PACKING;
  };

  const auto discardStep = -4;

  auto byWidth = []( const rectpack2D::rect_xywh *a, const rectpack2D::rect_xywh *b ) {
    return a->w > b->w;
  };

  const rectpack2D::rect_wh resultSize = rectpack2D::find_best_packing<spacesType>(
    mRects,
    rectpack2D::make_finder_input(
      maxSide,
      discardStep,
      reportSuccessful,
      reportUnsuccessful,
      rectpack2D::flipping_option::DISABLED
    ),
    byWidth
  );

  if ( !result )
    return false;

  mIdToIndex.clear();
  int index = 0;
  for ( const QgsTextureRect &rect : mRects )
  {
    mIdToIndex.insert( rect.id, index );
    index++;
  }
  mAtlasSize = QSize( resultSize.w, resultSize.h );
  return true;
}

QRect QgsTextureAtlasGenerator::rect( int id ) const
{
  const auto it = mIdToIndex.constFind( id );
  if ( it == mIdToIndex.constEnd() )
    return QRect();

  return mRects[*it].asQRect();
}

QImage QgsTextureAtlasGenerator::atlasTexture() const
{
  if ( mAtlasSize.isEmpty() )
    return QImage();

  QImage res( mAtlasSize, QImage::Format_ARGB32 );
  res.fill( Qt::transparent );

  QPainter painter( &res );
  for ( const QgsTextureRect &rect : mRects )
  {
    if ( !rect.image.isNull() )
    {
      painter.drawImage( rect.asQRect(), rect.image );
    }
  }
  painter.end();

  return res;
}

QImage QgsTextureAtlasGenerator::debugTexture() const
{
  if ( mAtlasSize.isEmpty() )
    return QImage();

  QImage res( mAtlasSize, QImage::Format_ARGB32 );
  res.fill( Qt::transparent );

  QPainter painter( &res );
  painter.setPen( Qt::NoPen );
  QgsRandomColorRamp ramp;
  ramp.setTotalColorCount( static_cast< int >( mRects.size() ) );
  double index = 0;
  for ( const QgsTextureRect &rect : mRects )
  {
    const QColor color = ramp.color( index / ( static_cast< int >( mRects.size() ) - 1 ) );
    index += 1;
    painter.setBrush( QBrush( color ) );
    painter.drawRect( rect.asQRect() );
  }
  painter.end();

  return res;
}
