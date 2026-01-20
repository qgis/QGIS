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
#include "qgspallabeling.h"
#include "qgsrendercontext.h"
#include "qgstextformat.h"
#include "qgstextrenderer.h"

#include <QPainter>
#include <QRegularExpressionMatch>
#include <QSet>

// rectpack2D library
#include <finders_interface.h>

///@cond PRIVATE

class QgsCharTextureRect
{
  public:
    QgsCharTextureRect( const QString &grapheme, const QSize &boundingRectSize, const QPoint &characterOffsetFromOrigin, int texturePaddingPixels )
      : grapheme( grapheme )
      , boundingRectSize( boundingRectSize )
      , characterOffsetFromOrigin( characterOffsetFromOrigin )
    {
      paddedRect = rectpack2D::rect_xywh( 0, 0, boundingRectSize.width() + 2 * texturePaddingPixels, boundingRectSize.height() + 2 * texturePaddingPixels );
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

    QString grapheme;
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

QRect QgsFontTextureAtlas::rect( const QString &grapheme ) const
{
  auto it = mGraphemeIndices.constFind( grapheme );
  if ( it == mGraphemeIndices.constEnd() )
    return QRect();

  return mRects[it.value()].asQRect();
}

int QgsFontTextureAtlas::graphemeCount( const QString &string ) const
{
  auto it = mStringMetrics.constFind( string );
  if ( it == mStringMetrics.constEnd() )
    return 0;

  return it->graphemeMetrics.count();
}

int QgsFontTextureAtlas::totalWidth( const QString &string ) const
{
  auto it = mStringMetrics.constFind( string );
  if ( it == mStringMetrics.constEnd() )
    return 0;

  return it->totalWidth;
}

QPoint QgsFontTextureAtlas::pixelOffsetForGrapheme( const QString &string, int graphemeIndex ) const
{
  auto it = mStringMetrics.constFind( string );
  if ( it == mStringMetrics.constEnd() )
    return QPoint();

  const GraphemeMetric &graphemeMetrics = it.value().graphemeMetrics[graphemeIndex];
  auto charIt = mGraphemeIndices.constFind( graphemeMetrics.grapheme );
  if ( charIt == mGraphemeIndices.constEnd() )
    return QPoint();

  return QPoint( graphemeMetrics.horizontalAdvance, -( mRects[charIt.value()].boundingRectSize.height() + mRects[charIt.value()].characterOffsetFromOrigin.y() ) );
}

QRect QgsFontTextureAtlas::textureRectForGrapheme( const QString &string, int graphemeIndex ) const
{
  auto it = mStringMetrics.constFind( string );
  if ( it == mStringMetrics.constEnd() )
    return QRect();

  const GraphemeMetric &graphemeMetrics = it.value().graphemeMetrics.value( graphemeIndex );
  auto charIt = mGraphemeIndices.constFind( graphemeMetrics.grapheme );
  if ( charIt == mGraphemeIndices.constEnd() )
    return QRect();

  return mRects[charIt.value()].asQRect();
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
    QgsTextRenderer::drawText( QPointF( -rect.characterOffsetFromOrigin.x() + rect.paddedRect.x + mTexturePaddingPixels, -rect.characterOffsetFromOrigin.y() + rect.paddedRect.y + mTexturePaddingPixels ), 0, Qgis::TextHorizontalAlignment::Left, { rect.grapheme }, context, mFormat );
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

  int texturePaddingPixels = 2;
  if ( format.buffer().enabled() )
  {
    texturePaddingPixels += static_cast< int >( std::ceil( context.convertToPainterUnits( format.buffer().size(), format.buffer().sizeUnit() ) ) );
  }

  // collect unique graphemes from all strings
  QSet<QString> uniqueGraphemes;
  QMap< QString, QgsFontTextureAtlas::StringMetrics > stringMetrics;
  for ( const QString &string : strings )
  {
    const QStringList graphemes = QgsPalLabeling::splitToGraphemes( string );

    QgsFontTextureAtlas::StringMetrics thisStringMetrics;
    thisStringMetrics.totalWidth = fontMetrics.boundingRect( string ).width();
    thisStringMetrics.graphemeMetrics.reserve( graphemes.size() );
    QString currentString;
    for ( const QString &grapheme : graphemes )
    {
      uniqueGraphemes.insert( grapheme );
      thisStringMetrics.graphemeMetrics << QgsFontTextureAtlas::GraphemeMetric( static_cast< int >( std::round( fontMetrics.horizontalAdvance( currentString ) ) ), grapheme );
      currentString += grapheme;
    }
    stringMetrics.insert( string, thisStringMetrics );
  }

  if ( uniqueGraphemes.isEmpty() )
  {
    return QgsFontTextureAtlas();
  }

  // get bounding rectangles for all the unique characters we need to render
  std::vector<QgsCharTextureRect> charRects;
  charRects.reserve( uniqueGraphemes.size() );
  for ( const QString &c : uniqueGraphemes )
  {
    const thread_local QRegularExpression sWhitespaceRx( u"^\\s+$"_s );
    if ( sWhitespaceRx.match( c ).hasMatch() )
      continue;

    const QRect boundingRect = c.size() == 1 ? fontMetrics.boundingRect( c.at( 0 ) ).toRect() : fontMetrics.boundingRect( c ).toRect();
    charRects.emplace_back( QgsCharTextureRect( c, boundingRect.size(), boundingRect.topLeft(), texturePaddingPixels ) );
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
  res.mStringMetrics = std::move( stringMetrics );
  res.mTexturePaddingPixels = texturePaddingPixels;

  int index = 0;
  for ( const QgsCharTextureRect &r : res.mRects )
  {
    res.mGraphemeIndices.insert( r.grapheme, index++ );
  }

  return res;
}
