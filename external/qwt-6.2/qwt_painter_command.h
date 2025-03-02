/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_PAINTER_COMMAND_H
#define QWT_PAINTER_COMMAND_H

#include "qwt_global.h"

#include <qpaintengine.h>
#include <qpixmap.h>
#include <qimage.h>
#include <qpolygon.h>
#include <qpainterpath.h>

class QPainterPath;

/*!
   QwtPainterCommand represents the attributes of a paint operation
   how it is used between QPainter and QPaintDevice

   It is used by QwtGraphic to record and replay paint operations

   \sa QwtGraphic::commands()
 */

class QWT_EXPORT QwtPainterCommand
{
  public:
    //! Type of the paint command
    enum Type
    {
        //! Invalid command
        Invalid = -1,

        //! Draw a QPainterPath
        Path,

        //! Draw a QPixmap
        Pixmap,

        //! Draw a QImage
        Image,

        //! QPainter state change
        State
    };

    //! Attributes how to paint a QPixmap
    struct PixmapData
    {
        QRectF rect;
        QPixmap pixmap;
        QRectF subRect;
    };

    //! Attributes how to paint a QImage
    struct ImageData
    {
        QRectF rect;
        QImage image;
        QRectF subRect;
        Qt::ImageConversionFlags flags;
    };

    //! Attributes of a state change
    struct StateData
    {
        QPaintEngine::DirtyFlags flags;

        QPen pen;
        QBrush brush;
        QPointF brushOrigin;
        QBrush backgroundBrush;
        Qt::BGMode backgroundMode;
        QFont font;
        QTransform transform;

        Qt::ClipOperation clipOperation;
        QRegion clipRegion;
        QPainterPath clipPath;
        bool isClipEnabled;

        QPainter::RenderHints renderHints;
        QPainter::CompositionMode compositionMode;
        qreal opacity;
    };

    QwtPainterCommand();
    QwtPainterCommand(const QwtPainterCommand&);

    explicit QwtPainterCommand( const QPainterPath& );

    QwtPainterCommand( const QRectF& rect,
        const QPixmap&, const QRectF& subRect );

    QwtPainterCommand( const QRectF& rect,
        const QImage&, const QRectF& subRect,
        Qt::ImageConversionFlags );

    explicit QwtPainterCommand( const QPaintEngineState& );

    ~QwtPainterCommand();

    QwtPainterCommand& operator=(const QwtPainterCommand& );

    Type type() const;

    QPainterPath* path();
    const QPainterPath* path() const;

    PixmapData* pixmapData();
    const PixmapData* pixmapData() const;

    ImageData* imageData();
    const ImageData* imageData() const;

    StateData* stateData();
    const StateData* stateData() const;

  private:
    void copy( const QwtPainterCommand& );
    void reset();

    Type m_type;

    union
    {
        QPainterPath* m_path;
        PixmapData* m_pixmapData;
        ImageData* m_imageData;
        StateData* m_stateData;
    };
};

//! \return Type of the command
inline QwtPainterCommand::Type QwtPainterCommand::type() const
{
    return m_type;
}

//! \return Painter path to be painted
inline const QPainterPath* QwtPainterCommand::path() const
{
    return m_path;
}

//! \return Attributes how to paint a QPixmap
inline const QwtPainterCommand::PixmapData*
QwtPainterCommand::pixmapData() const
{
    return m_pixmapData;
}

//! \return Attributes how to paint a QImage
inline const QwtPainterCommand::ImageData*
QwtPainterCommand::imageData() const
{
    return m_imageData;
}

//! \return Attributes of a state change
inline const QwtPainterCommand::StateData*
QwtPainterCommand::stateData() const
{
    return m_stateData;
}

#endif
