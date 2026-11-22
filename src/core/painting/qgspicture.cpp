/***************************************************************************
  qgspicture.cpp
  --------------------------------------
  Date                 : November 2026
  Copyright            : (C) 2026 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspicture.h"

#include <QBrush>
#include <QColor>
#include <QFont>
#include <QFontMetricsF>
#include <QGlyphRun>
#include <QImage>
#include <QLineF>
#include <QList>
#include <QPaintEngine>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QPointF>
#include <QPolygonF>
#include <QRect>
#include <QRectF>
#include <QRegion>
#include <QSharedData>
#include <QString>
#include <QTransform>

Q_GUI_EXPORT extern int qt_defaultDpiX();
Q_GUI_EXPORT extern int qt_defaultDpiY();

///@cond PRIVATE

// NOTE: These classes started life as direct 1:1 copies of the QPicture equivalents,
// but then were heavily revised to remove all the assumptions about encoding to binary
// formats (and removal of non-thread safe QBuffer use)
// Some of this still bares resemblances to QPicture, but the backend for storage
// is completely in-memory based around QList and other implicitly shared and memory-optimised
// objects only...

// Lossless storage container for QPainter commands
class QgsPicturePrivate : public QSharedData
{
  public:
    // indicates which drawing command or property setter was used on the QPainter
    enum class CommandType : quint8
    {
      DrawPoint = 1,
      DrawPoints,
      DrawLine,
      DrawLines,
      DrawRect,
      DrawRects,
      DrawEllipse,
      DrawPolyline,
      DrawPolygonOddEven,
      DrawPolygonWinding,
      DrawConvexPolygon,
      DrawPath,
      DrawImage,
      DrawTiledImage,
      DrawTextItem,

      SetBackgroundColor,
      SetBackgroundMode,
      SetBrushOrigin,
      SetPen,
      SetBrush,
      SetTransform,
      SetClipRegion,
      SetClipPath,
      SetClipEnabled,
      SetRenderHints,
      SetCompositionMode,
      SetOpacity
    };

    // Encapsulates the data for the QPainter command. Intentionally packed to be as SMALL as possible
    // to fit what we need -- the larger objects (like QImage, QBrush, etc) are all stored in separate QLists
    struct Command
    {
        CommandType type;
        quint8 arg1 = 0;
        quint16 arg2 = 0;
        quint32 index = 0;
    };
    static_assert( sizeof( Command ) == 8 );

    struct ImageData
    {
        QRectF target;
        int imageIndex = 0;
        QRectF source;
        Qt::ImageConversionFlags flags = Qt::AutoColor;
    };

    struct TiledImageData
    {
        QRectF rect;
        int imageIndex = 0;
        QPointF offset;
    };

    struct TextItemData
    {
        QPointF pos;
        QString text;
        QFont font;
        qreal justificationWidth = 0.0;
    };

    struct ClipRegionData
    {
        QRegion region;
        Qt::ClipOperation op = Qt::ReplaceClip;
    };

    struct ClipPathData
    {
        QPainterPath path;
        Qt::ClipOperation op = Qt::ReplaceClip;
    };

    QgsPicturePrivate() = default;
    QgsPicturePrivate( const QgsPicturePrivate &other ) = default;

    void clear()
    {
      mBoundingRect = QRectF();
      mOverriddenBoundingRect = QRect();

      mCommands.clear();
      mPoints.clear();
      mLines.clear();
      mRects.clear();
      mPaths.clear();
      mPolygons.clear();
      mTransforms.clear();
      mColors.clear();
      mImages.clear();
      mImageData.clear();
      mTiledImages.clear();
      mBrushes.clear();
      mPens.clear();
      mTextItems.clear();
      mClipRegions.clear();
      mClipPaths.clear();
    }

    void appendCommand( CommandType op, quint32 index = 0, quint8 arg1 = 0, quint16 arg2 = 0 ) { mCommands.append( Command { op, arg1, arg2, index } ); }

    void growBoundingRect( const QRectF &localRect, const QTransform &transform, bool hasClip, const QRectF &clipBounds, qreal penWidth = 0.0 )
    {
      if ( localRect.width() < 0.0 || localRect.height() < 0.0 )
        return;

      QRectF r = localRect;
      if ( penWidth > 0.0 )
      {
        const qreal halfPen = penWidth * 0.5;
        r.adjust( -halfPen, -halfPen, halfPen, halfPen );
      }

      if ( r.width() == 0.0 )
        r.setWidth( 1e-6 );
      if ( r.height() == 0.0 )
        r.setHeight( 1e-6 );

      r = transform.mapRect( r );
      if ( hasClip )
      {
        r &= clipBounds;
      }

      if ( r.width() > 0.0 || r.height() > 0.0 )
      {
        mBoundingRect |= r;
      }
    }

    int dpi = 0;
    QRectF mBoundingRect;
    QRect mOverriddenBoundingRect;

    QList<Command> mCommands;

    QList<QPointF> mPoints;
    QList<QLineF> mLines;
    QList<QRectF> mRects;
    QList<QPainterPath> mPaths;
    QList<QPolygonF> mPolygons;
    QList<QTransform> mTransforms;
    QList<QColor> mColors;
    QList<QImage> mImages;
    QList<ImageData> mImageData;
    QList<TiledImageData> mTiledImages;
    QList<QBrush> mBrushes;
    QList<QPen> mPens;
    QList<TextItemData> mTextItems;
    QList<ClipRegionData> mClipRegions;
    QList<ClipPathData> mClipPaths;
};

//
// QgsPicturePaintEngine
//

class QgsPicturePaintEngine final : public QPaintEngine
{
  public:
    QgsPicturePaintEngine()
      : QPaintEngine( AllFeatures )
    {}

    Type type() const override { return Picture; }

    bool begin( QPaintDevice *pdev ) override
    {
      QgsPicture *pic = static_cast<QgsPicture *>( pdev );
      // starting to record, so we need to detach as we'll be modifying the state
      pic->detach();
      mPicD = pic->d_func();

      // TODO test-- does QPicture reset when starting again?
      mPicD->clear();

      // reset state
      mPendingPen = QPen();
      mPendingBrush = QBrush();
      mPendingBrushOrigin = QPointF();
      mPendingTransform = QTransform();
      mPendingHints = QPainter::RenderHints();
      mPendingComposition = QPainter::CompositionMode_SourceOver;
      mPendingOpacity = 1.0;
      mPendingBgColor = QColor();
      mPendingBgMode = Qt::TransparentMode;

      mRecordedPen.reset();
      mRecordedBrush.reset();
      mRecordedBrushOrigin.reset();
      mRecordedTransform = QTransform();
      mRecordedHints = QPainter::RenderHints();
      mRecordedComposition = QPainter::CompositionMode_SourceOver;
      mRecordedOpacity = 1;
      mRecordedBgColor.reset();
      mRecordedBgMode = Qt::TransparentMode;

      mRecordedClipEnabled = false;

      setActive( true );
      return true;
    }

    bool end() override
    {
      mPicD = nullptr;
      setActive( false );
      return true;
    }

    void updateState( const QPaintEngineState &state ) override
    {
      // NOTE: we don't immediately record the state change. Instead we set the "pending" state,
      // and defer recording state till a drawing operation happens (via flushState()). This allows us to compress
      // multiple state changes to one change, removing unneeded changes (such as setting the brush
      // twice without drawing in between). End result is a smaller QgsPicture, and faster playback...
      const QPaintEngine::DirtyFlags flags = state.state();
      if ( flags & DirtyPen )
        mPendingPen = state.pen();
      if ( flags & DirtyBrush )
        mPendingBrush = state.brush();
      if ( flags & DirtyBrushOrigin )
        mPendingBrushOrigin = state.brushOrigin();
      if ( flags & ( DirtyBackground | DirtyBackgroundMode ) )
      {
        mPendingBgColor = state.backgroundBrush().color();
        mPendingBgMode = state.backgroundMode();
      }
      if ( flags & DirtyTransform )
        mPendingTransform = state.transform();

      if ( flags & ( DirtyClipRegion | DirtyClipPath | DirtyClipEnabled ) )
      {
        const bool clipEnabled = state.isClipEnabled() && state.clipOperation() != Qt::NoClip;
        if ( !clipEnabled )
        {
          if ( mRecordedClipEnabled )
          {
            updateClipEnabled( false );
            mRecordedClipEnabled = false;
          }
        }
        else
        {
          if ( flags & ( DirtyClipRegion | DirtyClipPath ) )
          {
            const bool usePath = ( flags & DirtyClipPath ) && ( !( flags & DirtyClipRegion ) || !state.clipPath().isEmpty() );
            if ( usePath )
              updateClipPath( state.clipPath(), state.clipOperation() );
            else
              updateClipRegion( state.clipRegion(), state.clipOperation() );

            mRecordedClipEnabled = true;
          }
          else if ( !mRecordedClipEnabled )
          {
            updateClipEnabled( true );
            mRecordedClipEnabled = true;
          }
        }
      }

      if ( flags & DirtyHints )
        mPendingHints = state.renderHints();
      if ( flags & DirtyCompositionMode )
        mPendingComposition = state.compositionMode();
      if ( flags & DirtyOpacity )
        mPendingOpacity = state.opacity();
    }

    void drawPixmap( const QRectF &r, const QPixmap &pm, const QRectF &sr ) override
    {
      // force conversion of pixmap to image, because QPixmap is NOT safe
      // to use on non-main thread
      drawImage( r, pm.toImage(), sr, Qt::AutoColor );
    }

    void drawTiledPixmap( const QRectF &r, const QPixmap &pixmap, const QPointF &s ) override
    {
      if ( pixmap.isNull() )
        return;

      flushState( false );
      const int imageIndex = static_cast< int >( mPicD->mImages.size() );
      mPicD->mImages.append( pixmap.toImage() );

      const quint32 commandIndex = static_cast<quint32>( mPicD->mTiledImages.size() );
      mPicD->mTiledImages.append( QgsPicturePrivate::TiledImageData { r, imageIndex, s } );
      mPicD->appendCommand( QgsPicturePrivate::CommandType::DrawTiledImage, commandIndex );
      recordBounds( r, false );
    }

    void drawImage( const QRectF &r, const QImage &image, const QRectF &sr, Qt::ImageConversionFlags flags = Qt::AutoColor ) override
    {
      if ( image.isNull() )
        return;

      flushState( false );
      const int imageIndex = static_cast< int >( mPicD->mImages.size() );
      mPicD->mImages.append( image );

      const quint32 commandIndex = static_cast<quint32>( mPicD->mImageData.size() );
      mPicD->mImageData.append( QgsPicturePrivate::ImageData { r, imageIndex, sr, flags } );
      mPicD->appendCommand( QgsPicturePrivate::CommandType::DrawImage, commandIndex );
      recordBounds( r, false );
    }

    void drawEllipse( const QRect &rect ) override { drawEllipse( QRectF( rect ) ); }

    void drawEllipse( const QRectF &rect ) override
    {
      flushState( true );
      const quint32 index = static_cast<quint32>( mPicD->mRects.size() );
      mPicD->mRects.append( rect );
      mPicD->appendCommand( QgsPicturePrivate::CommandType::DrawEllipse, index );
      recordBounds( rect, true );
    }

    void drawPath( const QPainterPath &path ) override
    {
      flushState( true );
      const quint32 index = static_cast<quint32>( mPicD->mPaths.size() );
      mPicD->mPaths.append( path );
      mPicD->appendCommand( QgsPicturePrivate::CommandType::DrawPath, index );
      recordBounds( path.boundingRect(), true );
    }

    void drawPoints( const QPointF *points, int pointCount ) override
    {
      if ( pointCount <= 0 )
        return;

      flushState( true );
      int offset = 0;
      while ( offset < pointCount )
      {
        const int chunkSize = std::min( pointCount - offset, static_cast<int>( std::numeric_limits<quint16>::max() ) );
        const quint32 startIndex = static_cast<quint32>( mPicD->mPoints.size() );
        mPicD->mPoints.reserve( mPicD->mPoints.size() + chunkSize );
        qreal minX = points[offset].x();
        qreal maxX = minX;
        qreal minY = points[offset].y();
        qreal maxY = minY;

        for ( int i = 0; i < chunkSize; ++i )
        {
          const QPointF &pt = points[offset + i];
          mPicD->mPoints.append( pt );
          minX = std::min( minX, pt.x() );
          maxX = std::max( maxX, pt.x() );
          minY = std::min( minY, pt.y() );
          maxY = std::max( maxY, pt.y() );
        }
        recordBounds( QRectF( minX, minY, maxX - minX + 1.0, maxY - minY + 1.0 ), true );
        if ( chunkSize == 1 )
          mPicD->appendCommand( QgsPicturePrivate::CommandType::DrawPoint, startIndex );
        else
          mPicD->appendCommand( QgsPicturePrivate::CommandType::DrawPoints, startIndex, 0, static_cast<quint16>( chunkSize ) );
        offset += chunkSize;
      }
    }

    void drawPoints( const QPoint *points, int pointCount ) override
    {
      if ( pointCount <= 0 )
        return;

      flushState( true );
      int offset = 0;
      while ( offset < pointCount )
      {
        const int chunkSize = std::min( pointCount - offset, static_cast<int>( std::numeric_limits<quint16>::max() ) );
        const quint32 startIndex = static_cast<quint32>( mPicD->mPoints.size() );
        mPicD->mPoints.reserve( mPicD->mPoints.size() + chunkSize );
        qreal minX = points[offset].x();
        qreal maxX = minX;
        qreal minY = points[offset].y();
        qreal maxY = minY;
        for ( int i = 0; i < chunkSize; ++i )
        {
          const QPointF pt( points[offset + i] );
          mPicD->mPoints.append( pt );
          minX = std::min( minX, pt.x() );
          maxX = std::max( maxX, pt.x() );
          minY = std::min( minY, pt.y() );
          maxY = std::max( maxY, pt.y() );
        }
        recordBounds( QRectF( minX, minY, maxX - minX + 1.0, maxY - minY + 1.0 ), true );
        if ( chunkSize == 1 )
          mPicD->appendCommand( QgsPicturePrivate::CommandType::DrawPoint, startIndex );
        else
          mPicD->appendCommand( QgsPicturePrivate::CommandType::DrawPoints, startIndex, 0, static_cast<quint16>( chunkSize ) );
        offset += chunkSize;
      }
    }

    void drawLines( const QLineF *lines, int lineCount ) override
    {
      if ( lineCount <= 0 )
        return;

      flushState( true );
      int offset = 0;
      while ( offset < lineCount )
      {
        const int chunkSize = std::min( lineCount - offset, static_cast<int>( std::numeric_limits<quint16>::max() ) );
        const quint32 startIndex = static_cast<quint32>( mPicD->mLines.size() );
        mPicD->mLines.reserve( mPicD->mLines.size() + chunkSize );
        qreal minX = std::min( lines[offset].x1(), lines[offset].x2() );
        qreal maxX = std::max( lines[offset].x1(), lines[offset].x2() );
        qreal minY = std::min( lines[offset].y1(), lines[offset].y2() );
        qreal maxY = std::max( lines[offset].y1(), lines[offset].y2() );
        for ( int i = 0; i < chunkSize; ++i )
        {
          const QLineF &line = lines[offset + i];
          mPicD->mLines.append( line );
          minX = std::min( { minX, line.x1(), line.x2() } );
          maxX = std::max( { maxX, line.x1(), line.x2() } );
          minY = std::min( { minY, line.y1(), line.y2() } );
          maxY = std::max( { maxY, line.y1(), line.y2() } );
        }
        recordBounds( QRectF( minX, minY, maxX - minX, maxY - minY ), true );
        if ( chunkSize == 1 )
          mPicD->appendCommand( QgsPicturePrivate::CommandType::DrawLine, startIndex );
        else
          mPicD->appendCommand( QgsPicturePrivate::CommandType::DrawLines, startIndex, 0, static_cast<quint16>( chunkSize ) );
        offset += chunkSize;
      }
    }

    void drawLines( const QLine *lines, int lineCount ) override
    {
      if ( lineCount <= 0 )
        return;

      flushState( true );
      int offset = 0;
      while ( offset < lineCount )
      {
        const int chunkSize = std::min( lineCount - offset, static_cast<int>( std::numeric_limits<quint16>::max() ) );
        const quint32 startIndex = static_cast<quint32>( mPicD->mLines.size() );
        mPicD->mLines.reserve( mPicD->mLines.size() + chunkSize );
        int minX = std::min( lines[offset].x1(), lines[offset].x2() );
        int maxX = std::max( lines[offset].x1(), lines[offset].x2() );
        int minY = std::min( lines[offset].y1(), lines[offset].y2() );
        int maxY = std::max( lines[offset].y1(), lines[offset].y2() );
        for ( int i = 0; i < chunkSize; ++i )
        {
          const QLine &line = lines[offset + i];
          mPicD->mLines.append( QLineF( line ) );
          minX = std::min( { minX, line.x1(), line.x2() } );
          maxX = std::max( { maxX, line.x1(), line.x2() } );
          minY = std::min( { minY, line.y1(), line.y2() } );
          maxY = std::max( { maxY, line.y1(), line.y2() } );
        }
        recordBounds( QRectF( minX, minY, maxX - minX, maxY - minY ), true );
        if ( chunkSize == 1 )
          mPicD->appendCommand( QgsPicturePrivate::CommandType::DrawLine, startIndex );
        else
          mPicD->appendCommand( QgsPicturePrivate::CommandType::DrawLines, startIndex, 0, static_cast<quint16>( chunkSize ) );
        offset += chunkSize;
      }
    }

    void drawRects( const QRectF *rects, int rectCount ) override
    {
      if ( rectCount <= 0 )
        return;

      flushState( true );
      int offset = 0;
      while ( offset < rectCount )
      {
        const int chunkSize = std::min( rectCount - offset, static_cast<int>( std::numeric_limits<quint16>::max() ) );
        const quint32 startIndex = static_cast<quint32>( mPicD->mRects.size() );
        mPicD->mRects.reserve( mPicD->mRects.size() + chunkSize );
        QRectF batchBounds = rects[offset].normalized();
        for ( int i = 0; i < chunkSize; ++i )
        {
          const QRectF &rect = rects[offset + i];
          mPicD->mRects.append( rect );
          const QRectF norm = rect.normalized();
          batchBounds.setLeft( std::min( batchBounds.left(), norm.left() ) );
          batchBounds.setTop( std::min( batchBounds.top(), norm.top() ) );
          batchBounds.setRight( std::max( batchBounds.right(), norm.right() ) );
          batchBounds.setBottom( std::max( batchBounds.bottom(), norm.bottom() ) );
        }
        recordBounds( batchBounds, true );
        if ( chunkSize == 1 )
          mPicD->appendCommand( QgsPicturePrivate::CommandType::DrawRect, startIndex );
        else
          mPicD->appendCommand( QgsPicturePrivate::CommandType::DrawRects, startIndex, 0, static_cast<quint16>( chunkSize ) );
        offset += chunkSize;
      }
    }

    void drawRects( const QRect *rects, int rectCount ) override
    {
      if ( rectCount <= 0 )
        return;

      flushState( true );
      int offset = 0;
      while ( offset < rectCount )
      {
        const int chunkSize = std::min( rectCount - offset, static_cast<int>( std::numeric_limits<quint16>::max() ) );
        const quint32 startIndex = static_cast<quint32>( mPicD->mRects.size() );
        mPicD->mRects.reserve( mPicD->mRects.size() + chunkSize );
        QRectF batchBounds = QRectF( rects[offset] ).normalized();
        for ( int i = 0; i < chunkSize; ++i )
        {
          const QRectF rect( rects[offset + i] );
          mPicD->mRects.append( rect );
          const QRectF norm = rect.normalized();
          batchBounds.setLeft( std::min( batchBounds.left(), norm.left() ) );
          batchBounds.setTop( std::min( batchBounds.top(), norm.top() ) );
          batchBounds.setRight( std::max( batchBounds.right(), norm.right() ) );
          batchBounds.setBottom( std::max( batchBounds.bottom(), norm.bottom() ) );
        }
        recordBounds( batchBounds, true );
        if ( chunkSize == 1 )
          mPicD->appendCommand( QgsPicturePrivate::CommandType::DrawRect, startIndex );
        else
          mPicD->appendCommand( QgsPicturePrivate::CommandType::DrawRects, startIndex, 0, static_cast<quint16>( chunkSize ) );
        offset += chunkSize;
      }
    }

    void drawPolygon( const QPointF *points, int pointCount, PolygonDrawMode mode ) override
    {
      if ( pointCount <= 0 )
        return;

      flushState( true );
      QPolygonF polygon;
      polygon.reserve( pointCount );
      for ( int i = 0; i < pointCount; ++i )
        polygon.append( points[i] );

      const QRectF bounds = polygon.boundingRect();
      const quint32 index = static_cast<quint32>( mPicD->mPolygons.size() );
      mPicD->mPolygons.append( std::move( polygon ) );

      switch ( mode )
      {
        case PolylineMode:
          mPicD->appendCommand( QgsPicturePrivate::CommandType::DrawPolyline, index );
          break;
        case OddEvenMode:
          mPicD->appendCommand( QgsPicturePrivate::CommandType::DrawPolygonOddEven, index );
          break;
        case WindingMode:
          mPicD->appendCommand( QgsPicturePrivate::CommandType::DrawPolygonWinding, index );
          break;
        case ConvexMode:
          mPicD->appendCommand( QgsPicturePrivate::CommandType::DrawConvexPolygon, index );
          break;
      }

      recordBounds( bounds, true );
    }

    void drawPolygon( const QPoint *points, int pointCount, PolygonDrawMode mode ) override
    {
      if ( pointCount <= 0 )
        return;

      flushState( true );
      QPolygonF polygon;
      polygon.reserve( pointCount );
      for ( int i = 0; i < pointCount; ++i )
        polygon.append( QPointF( points[i] ) );

      const QRectF bounds = polygon.boundingRect();
      const quint32 index = static_cast<quint32>( mPicD->mPolygons.size() );
      mPicD->mPolygons.append( std::move( polygon ) );

      switch ( mode )
      {
        case PolylineMode:
          mPicD->appendCommand( QgsPicturePrivate::CommandType::DrawPolyline, index );
          break;
        case OddEvenMode:
          mPicD->appendCommand( QgsPicturePrivate::CommandType::DrawPolygonOddEven, index );
          break;
        case WindingMode:
          mPicD->appendCommand( QgsPicturePrivate::CommandType::DrawPolygonWinding, index );
          break;
        case ConvexMode:
          mPicD->appendCommand( QgsPicturePrivate::CommandType::DrawConvexPolygon, index );
          break;
      }

      recordBounds( bounds, true );
    }

    void drawTextItem( const QPointF &p, const QTextItem &ti ) override
    {
      if ( ti.text().isEmpty() )
      {
        QPaintEngine::drawTextItem( p, ti );
        return;
      }

      flushState( true );

      QFont fnt = ti.font();
      const QFontMetricsF fm( fnt );
      const QPointF topLeft( p.x(), p.y() - fm.ascent() );

      const quint32 index = static_cast<quint32>( mPicD->mTextItems.size() );
      mPicD->mTextItems.append( QgsPicturePrivate::TextItemData { topLeft, ti.text(), fnt, 0.0 } );
      mPicD->appendCommand( QgsPicturePrivate::CommandType::DrawTextItem, index );

      recordBounds( QRectF( topLeft.x(), topLeft.y(), ti.width(), fm.height() ), false );
    }

  private:
    Q_DISABLE_COPY_MOVE( QgsPicturePaintEngine )

    static QBrush prepareBrush( const QBrush &brush )
    {
      if ( brush.style() == Qt::TexturePattern && !brush.texture().isNull() )
      {
        // we HAVE to convert a pixmap backed QBrush to a QImage backed brush, as QPixmap is NOT safe to use from non-gui threads
        QBrush sanitized = brush;
        sanitized.setTextureImage( brush.texture().toImage() );
        return sanitized;
      }
      return brush;
    }

    static QPen preparePen( const QPen &pen )
    {
      if ( pen.brush().style() == Qt::TexturePattern && !pen.brush().texture().isNull() )
      {
        // we HAVE to convert a pixmap backed QBrush to a QImage backed brush, as QPixmap is NOT safe to use from non-gui threads
        QPen sanitized = pen;
        sanitized.setBrush( prepareBrush( pen.brush() ) );
        return sanitized;
      }
      return pen;
    }

    void flushTransform()
    {
      if ( mRecordedTransform != mPendingTransform )
      {
        mRecordedTransform = mPendingTransform;
        const quint32 idx = static_cast<quint32>( mPicD->mTransforms.size() );
        mPicD->mTransforms.append( mPendingTransform );
        mPicD->appendCommand( QgsPicturePrivate::CommandType::SetTransform, idx );
      }
    }

    // ensures that the recorded (current) render state matches the pending state (ie collapses redundant setter calls to a single
    // one before doing a render operation)
    // must be called BEFORE doing any actual drawing operation on the engine
    void flushState( bool includePenAndBrush )
    {
      flushTransform();

      if ( mRecordedHints != mPendingHints )
      {
        mRecordedHints = mPendingHints;
        mPicD->appendCommand( QgsPicturePrivate::CommandType::SetRenderHints, static_cast<quint32>( mPendingHints.toInt() ) );
      }

      if ( mRecordedComposition != mPendingComposition )
      {
        mRecordedComposition = mPendingComposition;
        mPicD->appendCommand( QgsPicturePrivate::CommandType::SetCompositionMode, static_cast<quint32>( mPendingComposition ) );
      }

      if ( !qFuzzyCompare( mRecordedOpacity, mPendingOpacity ) )
      {
        mRecordedOpacity = mPendingOpacity;
        const quint16 encoded = static_cast<quint16>( std::clamp( std::lround( mPendingOpacity * 65535.0 ), 0L, 65535L ) );
        mPicD->appendCommand( QgsPicturePrivate::CommandType::SetOpacity, 0, 0, encoded );
      }

      if ( mRecordedBgMode != mPendingBgMode )
      {
        mRecordedBgMode = mPendingBgMode;
        mPicD->appendCommand( QgsPicturePrivate::CommandType::SetBackgroundMode, 0, static_cast<quint8>( mPendingBgMode ) );
      }

      if ( mPendingBgMode == Qt::OpaqueMode && ( !mRecordedBgColor.has_value() || *mRecordedBgColor != mPendingBgColor ) )
      {
        mRecordedBgColor = mPendingBgColor;
        const quint32 idx = static_cast<quint32>( mPicD->mColors.size() );
        mPicD->mColors.append( mPendingBgColor );
        mPicD->appendCommand( QgsPicturePrivate::CommandType::SetBackgroundColor, idx );
      }

      if ( includePenAndBrush )
      {
        if ( !mRecordedPen.has_value() || *mRecordedPen != mPendingPen )
        {
          mRecordedPen = mPendingPen;
          const quint32 idx = static_cast<quint32>( mPicD->mPens.size() );
          mPicD->mPens.append( preparePen( mPendingPen ) );
          mPicD->appendCommand( QgsPicturePrivate::CommandType::SetPen, idx );
        }

        if ( !mRecordedBrush.has_value() || *mRecordedBrush != mPendingBrush )
        {
          mRecordedBrush = mPendingBrush;
          const quint32 idx = static_cast<quint32>( mPicD->mBrushes.size() );
          mPicD->mBrushes.append( prepareBrush( mPendingBrush ) );
          mPicD->appendCommand( QgsPicturePrivate::CommandType::SetBrush, idx );
        }

        if ( mPendingBrush.style() != Qt::NoBrush && ( !mRecordedBrushOrigin.has_value() || *mRecordedBrushOrigin != mPendingBrushOrigin ) )
        {
          mRecordedBrushOrigin = mPendingBrushOrigin;
          const quint32 idx = static_cast<quint32>( mPicD->mPoints.size() );
          mPicD->mPoints.append( mPendingBrushOrigin );
          mPicD->appendCommand( QgsPicturePrivate::CommandType::SetBrushOrigin, idx );
        }
      }
    }

    void recordBounds( const QRectF &localRect, bool includePenWidth )
    {
      const qreal penWidth = ( includePenWidth && mPendingPen.style() != Qt::NoPen ) ? std::max( mPendingPen.widthF(), 1.0 ) : 0.0;
      const bool hasClip = painter() && painter()->hasClipping();
      const QRectF clipBounds = hasClip ? mPendingTransform.mapRect( painter()->clipBoundingRect() ) : QRectF();
      mPicD->growBoundingRect( localRect, mPendingTransform, hasClip, clipBounds, penWidth );
    }

    void updateClipRegion( const QRegion &region, Qt::ClipOperation op )
    {
      flushTransform();
      const quint32 idx = static_cast<quint32>( mPicD->mClipRegions.size() );
      mPicD->mClipRegions.append( QgsPicturePrivate::ClipRegionData { region, op } );
      mPicD->appendCommand( QgsPicturePrivate::CommandType::SetClipRegion, idx );
    }

    void updateClipPath( const QPainterPath &path, Qt::ClipOperation op )
    {
      flushTransform();
      const quint32 idx = static_cast<quint32>( mPicD->mClipPaths.size() );
      mPicD->mClipPaths.append( QgsPicturePrivate::ClipPathData { path, op } );
      mPicD->appendCommand( QgsPicturePrivate::CommandType::SetClipPath, idx );
    }

    void updateClipEnabled( bool enabled ) { mPicD->appendCommand( QgsPicturePrivate::CommandType::SetClipEnabled, 0, enabled ? 1 : 0 ); }

    QgsPicturePrivate *mPicD = nullptr;

    // pending painter states -- we don't immediately record ("commit") them, as we want to compress
    // multiple changes into a single one
    QPen mPendingPen;
    QBrush mPendingBrush;
    QPointF mPendingBrushOrigin;
    QTransform mPendingTransform;
    QPainter::RenderHints mPendingHints;
    QPainter::CompositionMode mPendingComposition = QPainter::CompositionMode_SourceOver;
    qreal mPendingOpacity = 1.0;
    QColor mPendingBgColor;
    Qt::BGMode mPendingBgMode = Qt::TransparentMode;

    // actual last committed (recorded) states
    std::optional<QPen> mRecordedPen;
    std::optional<QBrush> mRecordedBrush;
    std::optional<QPointF> mRecordedBrushOrigin;
    QTransform mRecordedTransform;
    QPainter::RenderHints mRecordedHints;
    QPainter::CompositionMode mRecordedComposition = QPainter::CompositionMode_SourceOver;
    qreal mRecordedOpacity = 1.0;
    std::optional<QColor> mRecordedBgColor;
    Qt::BGMode mRecordedBgMode = Qt::TransparentMode;

    bool mRecordedClipEnabled = false;
};

///@endcond

//
// QgsPicture
//

QgsPicture::QgsPicture()
  : QPaintDevice()
  , d_ptr( new QgsPicturePrivate )
{}

QgsPicture::~QgsPicture() = default;

QgsPicture::QgsPicture( const QgsPicture &other )
  : QPaintDevice()
  , d_ptr( other.d_ptr )
{}

QgsPicture &QgsPicture::operator=( const QgsPicture &other )
{
  if ( this != &other )
  {
    d_ptr = other.d_ptr;
    mPaintEngine.reset();
  }
  return *this;
}

QgsPicture::QgsPicture( QgsPicture &&other ) noexcept
  : QPaintDevice()
  , d_ptr( std::move( other.d_ptr ) )
{}

QgsPicture &QgsPicture::operator=( QgsPicture &&other ) noexcept
{
  if ( this != &other )
  {
    d_ptr = std::move( other.d_ptr );
    mPaintEngine.reset();
  }
  return *this;
}

int QgsPicture::devType() const
{
  return QInternal::Picture;
}

QPaintEngine *QgsPicture::paintEngine() const
{
  if ( !mPaintEngine )
    mPaintEngine = std::make_unique<QgsPicturePaintEngine>();
  return mPaintEngine.get();
}

bool QgsPicture::isNull() const
{
  return d_ptr->mCommands.isEmpty();
}

QRect QgsPicture::boundingRect() const
{
  if ( !d_ptr->mOverriddenBoundingRect.isEmpty() )
    return d_ptr->mOverriddenBoundingRect;

  return d_ptr->mBoundingRect.toAlignedRect();
}

void QgsPicture::setBoundingRect( const QRect &rect )
{
  if ( d_ptr->mOverriddenBoundingRect == rect )
    return;

  detach();
  d_ptr->mOverriddenBoundingRect = rect;
}

void QgsPicture::detach()
{
  d_ptr.detach();
}

bool QgsPicture::isDetached() const
{
  return d_ptr->ref.loadRelaxed() == 1;
}

int QgsPicture::metric( PaintDeviceMetric m ) const
{
  const QRect brect = boundingRect();
  const int dpiX = qt_defaultDpiX();
  const int dpiY = qt_defaultDpiY();

  switch ( m )
  {
    case PdmWidth:
      return brect.width();
    case PdmHeight:
      return brect.height();
    case PdmWidthMM:
      return static_cast<int>( 25.4 / dpiX * brect.width() );
    case PdmHeightMM:
      return static_cast<int>( 25.4 / dpiY * brect.height() );
    case PdmDpiX:
    case PdmPhysicalDpiX:
      return dpiX;
    case PdmDpiY:
    case PdmPhysicalDpiY:
      return dpiY;
    case PdmNumColors:
      return 16777216;
    case PdmDepth:
      return 24;
    case PdmDevicePixelRatio:
      return 1;
    case PdmDevicePixelRatioScaled:
      return static_cast<int>( 1 * QPaintDevice::devicePixelRatioFScale() );
    default:
      qWarning( "QgsPicture::metric: Invalid metric %d", m );
      return 0;
  }
}

bool QgsPicture::play( QPainter *painter ) const
{
  if ( !painter || !painter->isActive() )
    return false;

  const QgsPicturePrivate *d = d_ptr.constData();
  if ( d->mCommands.isEmpty() )
    return true;

  // record initial state
  std::optional<QTransform> worldMatrix;
  const bool hadBaseClip = painter->hasClipping();
  std::optional<QRegion> baseClipRegion;
  auto ensureBaseClip = [&]() {
    if ( !baseClipRegion.has_value() )
      baseClipRegion = painter->clipRegion();
  };
  auto restoreBaseClip = [&]() {
    if ( hadBaseClip )
    {
      ensureBaseClip();
      if ( worldMatrix.has_value() )
      {
        const QTransform currentTransform = painter->transform();
        painter->setTransform( *worldMatrix, false );
        painter->setClipRegion( *baseClipRegion, Qt::ReplaceClip );
        painter->setTransform( currentTransform, false );
      }
      else
      {
        painter->setClipRegion( *baseClipRegion, Qt::ReplaceClip );
      }
    }
    else
    {
      painter->setClipping( false );
    }
  };

  auto prepareClipOp = [&]( Qt::ClipOperation &op ) {
    if ( hadBaseClip && op == Qt::ReplaceClip )
    {
      restoreBaseClip();
      op = Qt::IntersectClip;
    }
    else if ( hadBaseClip )
    {
      ensureBaseClip();
    }
  };

  for ( const QgsPicturePrivate::Command &cmd : d->mCommands )
  {
    switch ( cmd.type )
    {
      case QgsPicturePrivate::CommandType::DrawPoint:
        painter->drawPoint( d->mPoints[cmd.index] );
        break;

      case QgsPicturePrivate::CommandType::DrawPoints:
        painter->drawPoints( d->mPoints.constData() + cmd.index, cmd.arg2 );
        break;

      case QgsPicturePrivate::CommandType::DrawLine:
        painter->drawLine( d->mLines[cmd.index] );
        break;

      case QgsPicturePrivate::CommandType::DrawLines:
        painter->drawLines( d->mLines.constData() + cmd.index, cmd.arg2 );
        break;

      case QgsPicturePrivate::CommandType::DrawRect:
        painter->drawRect( d->mRects[cmd.index] );
        break;

      case QgsPicturePrivate::CommandType::DrawRects:
        painter->drawRects( d->mRects.constData() + cmd.index, cmd.arg2 );
        break;

      case QgsPicturePrivate::CommandType::DrawEllipse:
        painter->drawEllipse( d->mRects[cmd.index] );
        break;

      case QgsPicturePrivate::CommandType::DrawPolyline:
        painter->drawPolyline( d->mPolygons[cmd.index] );
        break;

      case QgsPicturePrivate::CommandType::DrawPolygonOddEven:
        painter->drawPolygon( d->mPolygons[cmd.index], Qt::OddEvenFill );
        break;

      case QgsPicturePrivate::CommandType::DrawPolygonWinding:
        painter->drawPolygon( d->mPolygons[cmd.index], Qt::WindingFill );
        break;

      case QgsPicturePrivate::CommandType::DrawConvexPolygon:
        painter->drawConvexPolygon( d->mPolygons[cmd.index] );
        break;

      case QgsPicturePrivate::CommandType::DrawPath:
        painter->drawPath( d->mPaths[cmd.index] );
        break;

      case QgsPicturePrivate::CommandType::DrawImage:
      {
        const QgsPicturePrivate::ImageData &img = d->mImageData[cmd.index];
        painter->drawImage( img.target, d->mImages[img.imageIndex], img.source, img.flags );
        break;
      }

      case QgsPicturePrivate::CommandType::DrawTiledImage:
      {
        const QgsPicturePrivate::TiledImageData &tiled = d->mTiledImages[cmd.index];
        const QPointF prevOrigin = painter->brushOrigin();
        painter->setBrushOrigin( tiled.rect.topLeft() - tiled.offset );
        painter->fillRect( tiled.rect, QBrush( d->mImages[tiled.imageIndex] ) );
        painter->setBrushOrigin( prevOrigin );
        break;
      }

      case QgsPicturePrivate::CommandType::DrawTextItem:
      {
        const QgsPicturePrivate::TextItemData &ti = d->mTextItems[cmd.index];
        int flags = Qt::TextSingleLine | Qt::TextDontClip | Qt::TextForceLeftToRight;
        QSizeF size( 1, 1 );
        if ( ti.justificationWidth > 0 )
        {
          size.setWidth( ti.justificationWidth );
          flags |= Qt::TextJustificationForced | Qt::AlignJustify;
        }
        const QFont prevFont = painter->font();

        // we have to create a new QFont using this as the paint device, so that the font size
        // is correctly scaled to account for THIS paint device's dpi (not the target device's DPI)
        QFont scaledFont( ti.font );
        if ( scaledFont.pixelSize() == -1 && painter->device() && painter->device()->logicalDpiY() > 0 && painter->device()->logicalDpiY() != logicalDpiY() )
        {
          scaledFont.setPointSizeF( scaledFont.pointSizeF() * static_cast<qreal>( logicalDpiY() ) / painter->device()->logicalDpiY() );
        }
        painter->setFont( scaledFont );
        painter->drawText( QRectF( ti.pos, size ), flags, ti.text );
        painter->setFont( prevFont );
        break;
      }

      case QgsPicturePrivate::CommandType::SetBackgroundColor:
        painter->setBackground( d->mColors[cmd.index] );
        break;

      case QgsPicturePrivate::CommandType::SetBackgroundMode:
        painter->setBackgroundMode( static_cast<Qt::BGMode>( cmd.arg1 ) );
        break;

      case QgsPicturePrivate::CommandType::SetBrushOrigin:
        painter->setBrushOrigin( d->mPoints[cmd.index] );
        break;

      case QgsPicturePrivate::CommandType::SetPen:
        painter->setPen( d->mPens[cmd.index] );
        break;

      case QgsPicturePrivate::CommandType::SetBrush:
        painter->setBrush( d->mBrushes[cmd.index] );
        break;

      case QgsPicturePrivate::CommandType::SetTransform:
        if ( !worldMatrix.has_value() )
        {
          worldMatrix = painter->transform();
          if ( hadBaseClip )
            ensureBaseClip();
        }
        painter->setTransform( d->mTransforms[cmd.index] * ( *worldMatrix ), false );
        break;

      case QgsPicturePrivate::CommandType::SetClipRegion:
      {
        const QgsPicturePrivate::ClipRegionData &cr = d->mClipRegions[cmd.index];
        Qt::ClipOperation op = cr.op;
        prepareClipOp( op );
        painter->setClipRegion( cr.region, op );
        break;
      }

      case QgsPicturePrivate::CommandType::SetClipPath:
      {
        const QgsPicturePrivate::ClipPathData &cp = d->mClipPaths[cmd.index];
        Qt::ClipOperation op = cp.op;
        prepareClipOp( op );
        painter->setClipPath( cp.path, op );
        break;
      }

      case QgsPicturePrivate::CommandType::SetClipEnabled:
        if ( cmd.arg1 == 0 )
          restoreBaseClip();
        else
          painter->setClipping( true );
        break;

      case QgsPicturePrivate::CommandType::SetRenderHints:
      {
        const QPainter::RenderHints targetHints = QPainter::RenderHints::fromInt( cmd.index );
        const QPainter::RenderHints currentHints = painter->renderHints();
        const QPainter::RenderHints toEnable = targetHints & ~currentHints;
        const QPainter::RenderHints toDisable = currentHints & ~targetHints;
        if ( toEnable )
          painter->setRenderHints( toEnable, true );
        if ( toDisable )
          painter->setRenderHints( toDisable, false );
        break;
      }

      case QgsPicturePrivate::CommandType::SetCompositionMode:
        painter->setCompositionMode( static_cast<QPainter::CompositionMode>( cmd.index ) );
        break;

      case QgsPicturePrivate::CommandType::SetOpacity:
        painter->setOpacity( static_cast<qreal>( cmd.arg2 ) / 65535.0 );
        break;
    }
  }

  return true;
}
