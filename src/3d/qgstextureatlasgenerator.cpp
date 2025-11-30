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
    [[nodiscard]] const auto &get_rect() const
    {
      return rect;
    }

    [[nodiscard]] QRect asQRect() const
    {
      return QRect( rect.x, rect.y, rect.w, rect.h );
    }

    rectpack2D::rect_xywh rect;
    int id = 0;
    QImage image;
};

///@endcond


QgsTextureAtlas::QgsTextureAtlas() = default;
QgsTextureAtlas::~QgsTextureAtlas() = default;
QgsTextureAtlas::QgsTextureAtlas( const QgsTextureAtlas &other ) = default;
QgsTextureAtlas &QgsTextureAtlas::operator=( const QgsTextureAtlas &other ) = default;

int QgsTextureAtlas::count() const
{
  return static_cast< int >( mRects.size() );
}

QRect QgsTextureAtlas::rect( int id ) const
{
  return mRects[id].asQRect();
}

QImage QgsTextureAtlas::renderAtlasTexture() const
{
  if ( mAtlasSize.isEmpty() )
    return QImage();

  QImage res( mAtlasSize, QImage::Format_ARGB32_Premultiplied );
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

QImage QgsTextureAtlas::renderDebugTexture() const
{
  if ( mAtlasSize.isEmpty() )
    return QImage();

  QImage res( mAtlasSize, QImage::Format_ARGB32_Premultiplied );
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


//
// QgsTextureAtlasGenerator
//

QgsTextureAtlas QgsTextureAtlasGenerator::createFromRects( const QVector<QRect> &rectangles, int maxSide )
{
  std::vector< QgsTextureRect > rects;
  rects.reserve( rectangles.size() );
  int index = 0;
  for ( const QRect &rect : rectangles )
  {
    rects.emplace_back( QgsTextureRect( rectpack2D::rect_xywh( 0, 0, rect.width(), rect.height() ), index++ ) );
  }
  return generateAtlas( std::move( rects ), maxSide );
}

QgsTextureAtlas QgsTextureAtlasGenerator::createFromImages( const QVector<QImage> &images, int maxSide )
{
  std::vector< QgsTextureRect > rects;
  rects.reserve( images.size() );
  int index = 0;
  for ( const QImage &image : images )
  {
    rects.emplace_back( QgsTextureRect( rectpack2D::rect_xywh( 0, 0, image.width(), image.height() ), index++, image ) );
  }
  return generateAtlas( std::move( rects ), maxSide );
}

QgsTextureAtlas QgsTextureAtlasGenerator::generateAtlas( std::vector< QgsTextureRect > rects, int maxSide )
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
    rects,
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
    return QgsTextureAtlas();

  // rectpack2D::find_best_packing will have rearranged rects. Sort it back to the original order
  // so that we can retrieve the results by their original indices.
  std::sort( rects.begin(), rects.end(), []( const QgsTextureRect &a, const QgsTextureRect &b ) {
    return a.id < b.id;
  } );

  QgsTextureAtlas res;
  res.mRects = std::move( rects );
  res.mAtlasSize = QSize( resultSize.w, resultSize.h );
  return res;
}
