/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_painter_command.h"

//! Construct an invalid command
QwtPainterCommand::QwtPainterCommand()
    : m_type( Invalid )
{
}

//! Copy constructor
QwtPainterCommand::QwtPainterCommand( const QPainterPath& path )
    : m_type( Path )
{
    m_path = new QPainterPath( path );
}

/*!
   Constructor for Pixmap paint operation

   \param rect Target rectangle
   \param pixmap Pixmap
   \param subRect Rectangle inside the pixmap

   \sa QPainter::drawPixmap()
 */
QwtPainterCommand::QwtPainterCommand( const QRectF& rect,
        const QPixmap& pixmap, const QRectF& subRect )
    : m_type( Pixmap )
{
    m_pixmapData = new PixmapData();
    m_pixmapData->rect = rect;
    m_pixmapData->pixmap = pixmap;
    m_pixmapData->subRect = subRect;
}

/*!
   Constructor for Image paint operation

   \param rect Target rectangle
   \param image Image
   \param subRect Rectangle inside the image
   \param flags Conversion flags

   \sa QPainter::drawImage()
 */
QwtPainterCommand::QwtPainterCommand( const QRectF& rect,
        const QImage& image, const QRectF& subRect,
        Qt::ImageConversionFlags flags )
    : m_type( Image )
{
    m_imageData = new ImageData();
    m_imageData->rect = rect;
    m_imageData->image = image;
    m_imageData->subRect = subRect;
    m_imageData->flags = flags;
}

/*!
   Constructor for State paint operation
   \param state Paint engine state
 */
QwtPainterCommand::QwtPainterCommand( const QPaintEngineState& state )
    : m_type( State )
{
    m_stateData = new StateData();

    m_stateData->flags = state.state();

    if ( m_stateData->flags & QPaintEngine::DirtyPen )
        m_stateData->pen = state.pen();

    if ( m_stateData->flags & QPaintEngine::DirtyBrush )
        m_stateData->brush = state.brush();

    if ( m_stateData->flags & QPaintEngine::DirtyBrushOrigin )
        m_stateData->brushOrigin = state.brushOrigin();

    if ( m_stateData->flags & QPaintEngine::DirtyFont )
        m_stateData->font = state.font();

    if ( m_stateData->flags & QPaintEngine::DirtyBackground )
    {
        m_stateData->backgroundMode = state.backgroundMode();
        m_stateData->backgroundBrush = state.backgroundBrush();
    }

    if ( m_stateData->flags & QPaintEngine::DirtyTransform )
        m_stateData->transform = state.transform();

    if ( m_stateData->flags & QPaintEngine::DirtyClipEnabled )
        m_stateData->isClipEnabled = state.isClipEnabled();

    if ( m_stateData->flags & QPaintEngine::DirtyClipRegion )
    {
        m_stateData->clipRegion = state.clipRegion();
        m_stateData->clipOperation = state.clipOperation();
    }

    if ( m_stateData->flags & QPaintEngine::DirtyClipPath )
    {
        m_stateData->clipPath = state.clipPath();
        m_stateData->clipOperation = state.clipOperation();
    }

    if ( m_stateData->flags & QPaintEngine::DirtyHints )
        m_stateData->renderHints = state.renderHints();

    if ( m_stateData->flags & QPaintEngine::DirtyCompositionMode )
        m_stateData->compositionMode = state.compositionMode();

    if ( m_stateData->flags & QPaintEngine::DirtyOpacity )
        m_stateData->opacity = state.opacity();
}

/*!
   Copy constructor
   \param other Command to be copied

 */
QwtPainterCommand::QwtPainterCommand( const QwtPainterCommand& other )
{
    copy( other );
}

//! Destructor
QwtPainterCommand::~QwtPainterCommand()
{
    reset();
}

/*!
   Assignment operator

   \param other Command to be copied
   \return Modified command
 */
QwtPainterCommand& QwtPainterCommand::operator=( const QwtPainterCommand& other )
{
    reset();
    copy( other );

    return *this;
}

void QwtPainterCommand::copy( const QwtPainterCommand& other )
{
    m_type = other.m_type;

    switch( other.m_type )
    {
        case Path:
        {
            m_path = new QPainterPath( *other.m_path );
            break;
        }
        case Pixmap:
        {
            m_pixmapData = new PixmapData( *other.m_pixmapData );
            break;
        }
        case Image:
        {
            m_imageData = new ImageData( *other.m_imageData );
            break;
        }
        case State:
        {
            m_stateData = new StateData( *other.m_stateData );
            break;
        }
        default:
            break;
    }
}

void QwtPainterCommand::reset()
{
    switch( m_type )
    {
        case Path:
        {
            delete m_path;
            break;
        }
        case Pixmap:
        {
            delete m_pixmapData;
            break;
        }
        case Image:
        {
            delete m_imageData;
            break;
        }
        case State:
        {
            delete m_stateData;
            break;
        }
        default:
            break;
    }

    m_type = Invalid;
}

//! \return Painter path to be painted
QPainterPath* QwtPainterCommand::path()
{
    return m_path;
}

//! \return Attributes how to paint a QPixmap
QwtPainterCommand::PixmapData* QwtPainterCommand::pixmapData()
{
    return m_pixmapData;
}

//! \return Attributes how to paint a QImage
QwtPainterCommand::ImageData* QwtPainterCommand::imageData()
{
    return m_imageData;
}

//! \return Attributes of a state change
QwtPainterCommand::StateData* QwtPainterCommand::stateData()
{
    return m_stateData;
}
