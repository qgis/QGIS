/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_WIDGET_OVERLAY_H
#define QWT_WIDGET_OVERLAY_H

#include "qwt_global.h"
#include <qwidget.h>

class QPainter;
class QRegion;

/*!
   \brief An overlay for a widget

   The main use case of an widget overlay is to avoid
   heavy repaint operation of the widget below.

   F.e. in combination with the plot canvas an overlay
   avoid replots as the content of the canvas can be restored from
   its backing store.

   QwtWidgetOverlay is an abstract base class. Deriving classes are
   supposed to reimplement the following methods:

   - drawOverlay()
   - maskHint()

   Internally QwtPlotPicker uses overlays for displaying
   the rubber band and the tracker text.

   \sa QwtPlotCanvas::BackingStore
 */
class QWT_EXPORT QwtWidgetOverlay : public QWidget
{
  public:
    /*!
       \brief Mask mode

       When using masks the widget below gets paint events for
       the masked regions of the overlay only. Otherwise
       Qt triggers full repaints. On less powerful hardware
       ( f.e embedded systems ) - or when using the raster paint
       engine on a remote desktop - bit blitting is a noticeable
       operation, that needs to be avoided.

       If and how to mask depends on how expensive the calculation
       of the mask is and how many pixels can be excluded by the mask.

       The default setting is MaskHint.

       \sa setMaskMode(), maskMode()
     */
    enum MaskMode
    {
        //! Don't use a mask.
        NoMask,

        /*!
           \brief Use maskHint() as mask

           For many situations a fast approximation is good enough
           and it is not necessary to build a more detailed mask
           ( f.e the bounding rectangle of a text ).
         */
        MaskHint,

        /*!
           \brief Calculate a mask by checking the alpha values

           Sometimes it is not possible to give a fast approximation
           and the mask needs to be calculated by drawing the overlay
           and testing the result.

           When a valid maskHint() is available
           only pixels inside this approximation are checked.
         */
        AlphaMask
    };

    /*!
       \brief Render mode

       For calculating the alpha mask the overlay has already
       been painted to a temporary QImage. Instead of rendering
       the overlay twice this buffer can be copied for drawing
       the overlay.

       On graphic systems using the raster paint engine ( QWS, Windows )
       it means usually copying some memory only. On X11 it results in an
       expensive operation building a pixmap and for simple overlays
       it might not be recommended.

       \note The render mode has no effect, when maskMode() != AlphaMask.
     */
    enum RenderMode
    {
        //! Copy the buffer, when using the raster paint engine.
        AutoRenderMode,

        //! Always copy the buffer
        CopyAlphaMask,

        //! Never copy the buffer
        DrawOverlay
    };

    explicit QwtWidgetOverlay( QWidget* );
    virtual ~QwtWidgetOverlay();

    void setMaskMode( MaskMode );
    MaskMode maskMode() const;

    void setRenderMode( RenderMode );
    RenderMode renderMode() const;

    virtual bool eventFilter( QObject*, QEvent*) QWT_OVERRIDE;

  public Q_SLOTS:
    void updateOverlay();

  protected:
    virtual void paintEvent( QPaintEvent* ) QWT_OVERRIDE;
    virtual void resizeEvent( QResizeEvent* ) QWT_OVERRIDE;

    virtual QRegion maskHint() const;

    /*!
       Draw the widget overlay
       \param painter Painter
     */
    virtual void drawOverlay( QPainter* painter ) const = 0;

  private:
    void updateMask();
    void draw( QPainter* ) const;

  private:
    class PrivateData;
    PrivateData* m_data;
};

#endif
