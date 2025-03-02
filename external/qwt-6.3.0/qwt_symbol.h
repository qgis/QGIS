/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_SYMBOL_H
#define QWT_SYMBOL_H

#include "qwt_global.h"

#include <qpolygon.h>
#include <qpen.h>
#include <qbrush.h>

class QPainter;
class QSize;
class QBrush;
class QPen;
class QColor;
class QPointF;
class QPainterPath;
class QPixmap;
class QByteArray;
class QwtGraphic;

//! A class for drawing symbols
class QWT_EXPORT QwtSymbol
{
  public:
    /*!
       Symbol Style
       \sa setStyle(), style()
     */
    enum Style
    {
        //! No Style. The symbol cannot be drawn.
        NoSymbol = -1,

        //! Ellipse or circle
        Ellipse,

        //! Rectangle
        Rect,

        //!  Diamond
        Diamond,

        //! Triangle pointing upwards
        Triangle,

        //! Triangle pointing downwards
        DTriangle,

        //! Triangle pointing upwards
        UTriangle,

        //! Triangle pointing left
        LTriangle,

        //! Triangle pointing right
        RTriangle,

        //! Cross (+)
        Cross,

        //! Diagonal cross (X)
        XCross,

        //! Horizontal line
        HLine,

        //! Vertical line
        VLine,

        //! X combined with +
        Star1,

        //! Six-pointed star
        Star2,

        //! Hexagon
        Hexagon,

        /*!
           The symbol is represented by a painter path, where the
           origin ( 0, 0 ) of the path coordinate system is mapped to
           the position of the symbol.

           \sa setPath(), path()
         */
        Path,

        /*!
           The symbol is represented by a pixmap. The pixmap is centered
           or aligned to its pin point.

           \sa setPinPoint()
         */
        Pixmap,

        /*!
           The symbol is represented by a graphic. The graphic is centered
           or aligned to its pin point.

           \sa setPinPoint()
         */
        Graphic,

        /*!
           The symbol is represented by a SVG graphic. The graphic is centered
           or aligned to its pin point.

           \sa setPinPoint()
         */
        SvgDocument,

        /*!
           Styles >= QwtSymbol::UserSymbol are reserved for derived
           classes of QwtSymbol that overload drawSymbols() with
           additional application specific symbol types.
         */
        UserStyle = 1000
    };

    /*!
       Depending on the render engine and the complexity of the
       symbol shape it might be faster to render the symbol
       to a pixmap and to paint this pixmap.

       F.e. the raster paint engine is a pure software renderer
       where in cache mode a draw operation usually ends in
       raster operation with the the backing store, that are usually
       faster, than the algorithms for rendering polygons.
       But the opposite can be expected for graphic pipelines
       that can make use of hardware acceleration.

       The default setting is AutoCache

       \sa setCachePolicy(), cachePolicy()

       \note The policy has no effect, when the symbol is painted
            to a vector graphics format ( PDF, SVG ).
       \warning Since Qt 4.8 raster is the default backend on X11
     */

    enum CachePolicy
    {
        //! Don't use a pixmap cache
        NoCache,

        //! Always use a pixmap cache
        Cache,

        /*!
           Use a cache when one of the following conditions is true:

           - The symbol is rendered with the software
             renderer ( QPaintEngine::Raster )
         */
        AutoCache
    };

  public:
    explicit QwtSymbol( Style = NoSymbol );
    QwtSymbol( Style, const QBrush&, const QPen&, const QSize& );
    QwtSymbol( const QPainterPath&, const QBrush&, const QPen& );

    virtual ~QwtSymbol();

    void setCachePolicy( CachePolicy );
    CachePolicy cachePolicy() const;

    void setSize( const QSize& );
    void setSize( int width, int height = -1 );
    const QSize& size() const;

    void setPinPoint( const QPointF& pos, bool enable = true );
    QPointF pinPoint() const;

    void setPinPointEnabled( bool );
    bool isPinPointEnabled() const;

    virtual void setColor( const QColor& );

    void setBrush( const QBrush& );
    const QBrush& brush() const;

    void setPen( const QColor&, qreal width = 0.0, Qt::PenStyle = Qt::SolidLine );
    void setPen( const QPen& );
    const QPen& pen() const;

    void setStyle( Style );
    Style style() const;

    void setPath( const QPainterPath& );
    const QPainterPath& path() const;

    void setPixmap( const QPixmap& );
    const QPixmap& pixmap() const;

    void setGraphic( const QwtGraphic& );
    const QwtGraphic& graphic() const;

#ifndef QWT_NO_SVG
    void setSvgDocument( const QByteArray& );
#endif

    void drawSymbol( QPainter*, const QRectF& ) const;
    void drawSymbol( QPainter*, const QPointF& ) const;
    void drawSymbols( QPainter*, const QPolygonF& ) const;
    void drawSymbols( QPainter*,
        const QPointF*, int numPoints ) const;

    virtual QRect boundingRect() const;
    void invalidateCache();

  protected:
    virtual void renderSymbols( QPainter*,
        const QPointF*, int numPoints ) const;

  private:
    Q_DISABLE_COPY(QwtSymbol)

    class PrivateData;
    PrivateData* m_data;
};

/*!
   \brief Draw the symbol at a specified position

   \param painter Painter
   \param pos Position of the symbol in screen coordinates
 */
inline void QwtSymbol::drawSymbol(
    QPainter* painter, const QPointF& pos ) const
{
    drawSymbols( painter, &pos, 1 );
}

/*!
   \brief Draw symbols at the specified points

   \param painter Painter
   \param points Positions of the symbols in screen coordinates
 */

inline void QwtSymbol::drawSymbols(
    QPainter* painter, const QPolygonF& points ) const
{
    drawSymbols( painter, points.data(), points.size() );
}

#endif
