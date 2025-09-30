/***************************************************************************
  qgsfonttextureatlasgenerator.cpp
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

#include "qgsfonttextureatlasgenerator.h"
#include "qgscolorrampimpl.h"
#include "qgstextformat.h"
#include "qgstextrenderer.h"
#include "qgsrendercontext.h"
#include <QPainter>
#include <QSet>

// rectpack2D library
#include <finders_interface.h>

///@cond PRIVATE


static constexpr int FONT_ATLAS_TEXTURE_PADDING_PIXELS = 2;

class QgsCharTextureRect
{
  public:
    QgsCharTextureRect( const QChar &character, const QSize &boundingRectSize, const QPoint &characterOffsetFromOrigin )
      : character( character )
      , boundingRectSize( boundingRectSize )
      , characterOffsetFromOrigin( characterOffsetFromOrigin )
    {
      paddedRect = rectpack2D::rect_xywh( 0, 0, boundingRectSize.width() + 2 * FONT_ATLAS_TEXTURE_PADDING_PIXELS, boundingRectSize.height() + 2 * FONT_ATLAS_TEXTURE_PADDING_PIXELS );
    }

    // get_rect must be implemented for rectpack2D compatibility:
    auto &get_rect()
    {
      return paddedRect;
    }
    const auto &get_rect() const
    {
      return paddedRect;
    }

    QRect asQRect() const
    {
      return QRect( paddedRect.x, paddedRect.y, paddedRect.w, paddedRect.h );
    }

    QChar character;
    //! Original tight bounding rect of the character glyph
    QSize boundingRectSize;
    //! Offset of character from origin
    QPoint characterOffsetFromOrigin;
    //! Texture atlas rect
    rectpack2D::rect_xywh paddedRect;
};

///@endcond


QgsFontTextureAtlas::QgsFontTextureAtlas() = default;
QgsFontTextureAtlas::~QgsFontTextureAtlas() = default;
QgsFontTextureAtlas::QgsFontTextureAtlas( const QgsFontTextureAtlas &other ) = default;
QgsFontTextureAtlas &QgsFontTextureAtlas::operator=( const QgsFontTextureAtlas &other ) = default;

int QgsFontTextureAtlas::count() const
{
  return static_cast< int >( mRects.size() );
}

QRect QgsFontTextureAtlas::rect( const QChar &character ) const
{
  auto it = mCharIndices.constFind( character );
  if ( it == mCharIndices.constEnd() )
    return QRect();

  return mRects[it.value()].asQRect();
}

QPoint QgsFontTextureAtlas::pixelOffsetForCharacter( const QString &string, int characterIndex ) const
{
  auto it = mHorizontalAdvancesForStrings.constFind( string );
  if ( it == mHorizontalAdvancesForStrings.constEnd() )
    return QPoint();

  auto charIt = mCharIndices.constFind( string.at( characterIndex ) );
  if ( charIt == mCharIndices.constEnd() )
    return QPoint();

  return QPoint( it.value().at( characterIndex ), -( mRects[charIt.value()].boundingRectSize.height() + mRects[charIt.value()].characterOffsetFromOrigin.y() ) );
}

QImage QgsFontTextureAtlas::renderAtlasTexture() const
{
  if ( mAtlasSize.isEmpty() )
    return QImage();

  QImage res( mAtlasSize, QImage::Format_ARGB32_Premultiplied );
  res.fill( Qt::transparent );

  QPainter painter( &res );
  QgsRenderContext context = QgsRenderContext::fromQPainter( &painter );
  for ( const QgsCharTextureRect &rect : mRects )
  {
    QgsTextRenderer::drawText( QPointF( -rect.characterOffsetFromOrigin.x() + rect.paddedRect.x + FONT_ATLAS_TEXTURE_PADDING_PIXELS, -rect.characterOffsetFromOrigin.y() + rect.paddedRect.y + FONT_ATLAS_TEXTURE_PADDING_PIXELS ), 0, Qgis::TextHorizontalAlignment::Left, { QString( rect.character ) }, context, mFormat );
  }
  painter.end();

  return res;
}

QImage QgsFontTextureAtlas::renderDebugTexture() const
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
  for ( const QgsCharTextureRect &rect : mRects )
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
// QgsFontTextureAtlasGenerator
//

QgsFontTextureAtlas QgsFontTextureAtlasGenerator::create( const QgsTextFormat &format, const QStringList &strings )
{
  QgsRenderContext context;
  context.setScaleFactor( 96.0 / 25.4 );
  const QFontMetricsF fontMetrics = QgsTextRenderer::fontMetrics( context, format );

  // collect unique characters from all strings
  QSet<QChar> uniqueChars;
  QMap< QString, QVector< int > > horizontalAdvancesForStrings;
  for ( const QString &string : strings )
  {
    int index = 0;
    QVector< int > horizontalAdvances;
    horizontalAdvances.reserve( string.size() );
    for ( QChar c : string )
    {
      uniqueChars.insert( c );
      horizontalAdvances << ( index > 0 ? static_cast< int >( std::round( fontMetrics.horizontalAdvance( string.left( index ) ) ) ) : 0 );
      index++;
    }
    horizontalAdvancesForStrings.insert( string, horizontalAdvances );
  }

  if ( uniqueChars.isEmpty() )
  {
    return QgsFontTextureAtlas();
  }

  // get bounding rectangles for all the unique characters we need to render
  std::vector<QgsCharTextureRect> charRects;
  charRects.reserve( uniqueChars.size() );
  for ( const QChar &c : uniqueChars )
  {
    const QRect boundingRect = fontMetrics.boundingRect( c ).toRect();
    charRects.emplace_back( QgsCharTextureRect( c, boundingRect.size(), boundingRect.topLeft() ) );
  }

  // pack character rects into an atlas
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
    charRects,
    rectpack2D::make_finder_input(
      1024,
      discardStep,
      reportSuccessful,
      reportUnsuccessful,
      rectpack2D::flipping_option::DISABLED
    ),
    byWidth
  );

  if ( !result )
    return QgsFontTextureAtlas();

  QgsFontTextureAtlas res;
  res.mFormat = format;
  res.mRects = std::move( charRects );
  res.mAtlasSize = QSize( resultSize.w, resultSize.h );
  res.mHorizontalAdvancesForStrings = std::move( horizontalAdvancesForStrings );

  int index = 0;
  for ( const QgsCharTextureRect &r : res.mRects )
  {
    res.mCharIndices.insert( r.character, index++ );
  }

  return res;
}
