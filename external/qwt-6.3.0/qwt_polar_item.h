/******************************************************************************
 * QwtPolar Widget Library
 * Copyright (C) 2008   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_POLAR_ITEM_H
#define QWT_POLAR_ITEM_H

#include "qwt_global.h"
#include "qwt_text.h"
#include "qwt_legend_data.h"
#include "qwt_graphic.h"
#include "qwt_interval.h"

class QString;
class QRect;
class QPointF;
class QPainter;
class QwtPolarPlot;
class QwtScaleMap;
class QwtScaleDiv;

/*!
   \brief Base class for items on a polar plot

   A QwtPolarItem is "something that can be painted on the canvas".
   It is connected to the QwtPolar framework by a couple of virtual
   methods, that are individually implemented in derived item classes.

   QwtPolar offers an implementation of the most common types of items,
   but deriving from QwtPolarItem makes it easy to implement additional
   types of items.
 */
class QWT_EXPORT QwtPolarItem
{
  public:
    /*!
        \brief Runtime type information

        RttiValues is used to cast plot items, without
        having to enable runtime type information of the compiler.
     */
    enum RttiValues
    {
        //! Unspecific value, that can be used, when it doesn't matter
        Rtti_PolarItem = 0,

        //! For QwtPolarGrid
        Rtti_PolarGrid,

        //! For QwtPolarMarker
        Rtti_PolarMarker,

        //! For QwtPolarCurve
        Rtti_PolarCurve,

        //! For QwtPolarSpectrogram
        Rtti_PolarSpectrogram,

        /*!
           Values >= Rtti_PolarUserItem are reserved for plot items
           not implemented in the QwtPolar library.
         */
        Rtti_PolarUserItem = 1000
    };

    /*!
       \brief Plot Item Attributes
       \sa setItemAttribute(), testItemAttribute()
     */
    enum ItemAttribute
    {
        //! The item is represented on the legend.
        Legend    = 0x01,

        /*!
           The boundingRect() of the item is included in the
           autoscaling calculation.
         */
        AutoScale = 0x02
    };

    Q_DECLARE_FLAGS( ItemAttributes, ItemAttribute )

    /*!
       \brief Render hints
       \sa setRenderHint(), testRenderHint()
     */
    enum RenderHint
    {
        //! Enable antialiasing
        RenderAntialiased = 0x01
    };

    Q_DECLARE_FLAGS( RenderHints, RenderHint )

    explicit QwtPolarItem( const QwtText& title = QwtText() );
    virtual ~QwtPolarItem();

    void attach( QwtPolarPlot* plot );
    void detach();

    QwtPolarPlot* plot() const;

    void setTitle( const QString& title );
    void setTitle( const QwtText& title );
    const QwtText& title() const;

    virtual int rtti() const;

    void setItemAttribute( ItemAttribute, bool on = true );
    bool testItemAttribute( ItemAttribute ) const;

    void setRenderHint( RenderHint, bool on = true );
    bool testRenderHint( RenderHint ) const;

    void setRenderThreadCount( uint numThreads );
    uint renderThreadCount() const;

    double z() const;
    void setZ( double z );

    void show();
    void hide();
    virtual void setVisible( bool );
    bool isVisible () const;

    virtual void itemChanged();
    virtual void legendChanged();

    /*!
       \brief Draw the item

       \param painter Painter
       \param azimuthMap Maps azimuth values to values related to 0.0, M_2PI
       \param radialMap Maps radius values into painter coordinates.
       \param pole Position of the pole in painter coordinates
       \param radius Radius of the complete plot area in painter coordinates
       \param canvasRect Contents rect of the canvas in painter coordinates
     */
    virtual void draw( QPainter* painter,
        const QwtScaleMap& azimuthMap, const QwtScaleMap& radialMap,
        const QPointF& pole, double radius,
        const QRectF& canvasRect ) const = 0;

    virtual QwtInterval boundingInterval( int scaleId ) const;

    virtual void updateScaleDiv( const QwtScaleDiv&,
        const QwtScaleDiv&, const QwtInterval& );

    virtual int marginHint() const;

    void setLegendIconSize( const QSize& );
    QSize legendIconSize() const;

    virtual QList< QwtLegendData > legendData() const;
    virtual QwtGraphic legendIcon( int index, const QSizeF& ) const;

  private:
    Q_DISABLE_COPY( QwtPolarItem )

    class PrivateData;
    PrivateData* m_data;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QwtPolarItem::ItemAttributes )
Q_DECLARE_OPERATORS_FOR_FLAGS( QwtPolarItem::RenderHints )

Q_DECLARE_METATYPE( QwtPolarItem* )

#endif
