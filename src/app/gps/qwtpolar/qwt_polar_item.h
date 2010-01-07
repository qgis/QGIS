/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * QwtPolar Widget Library
 * Copyright (C) 2008   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_POLAR_ITEM_H
#define QWT_POLAR_ITEM_H

#include "qwt_polar_global.h"
#include "qwt_text.h"
#include "qwt_legend_itemmanager.h"
#include "qwt_double_interval.h"
#include "qwt_double_rect.h"

class QString;
class QRect;
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
class QWT_POLAR_EXPORT QwtPolarItem: public QwtLegendItemManager
{
  public:
    /*!
        \brief Runtime type information

        RttiValues is used to cast plot items, without
        having to enable runtime type information of the compiler.
     */
    enum RttiValues
    {
      Rtti_PolarItem = 0,

      Rtti_PolarGrid,
      Rtti_PolarScale,
      Rtti_PolarMarker,
      Rtti_PolarCurve,
      Rtti_PolarSpectrogram,

      Rtti_PolarUserItem = 1000
    };

    /*!
       \brief Plot Item Attributes

       - Legend\n
         The item is represented on the legend.
       - AutoScale \n
         The boundingRect() of the item is included in the
         autoscaling calculation.

       \sa setItemAttribute(), testItemAttribute()
     */
    enum ItemAttribute
    {
      Legend = 1,
      AutoScale = 2
    };

#if QT_VERSION >= 0x040000
    //! Render hints
    enum RenderHint
    {
      RenderAntialiased = 1
    };
#endif

    explicit QwtPolarItem( const QwtText &title = QwtText() );
    virtual ~QwtPolarItem();

    void attach( QwtPolarPlot *plot );

    /*!
       \brief This method detaches a QwtPolarItem from any QwtPolarPlot it
              has been associated with.

       detach() is equivalent to calling attach( NULL )
       \sa attach( QwtPolarPlot* plot )
    */
    void detach() { attach( NULL ); }

    QwtPolarPlot *plot() const;

    void setTitle( const QString &title );
    void setTitle( const QwtText &title );
    const QwtText &title() const;

    virtual int rtti() const;

    void setItemAttribute( ItemAttribute, bool on = true );
    bool testItemAttribute( ItemAttribute ) const;

#if QT_VERSION >= 0x040000
    void setRenderHint( RenderHint, bool on = true );
    bool testRenderHint( RenderHint ) const;
#endif

    double z() const;
    void setZ( double z );

    void show();
    void hide();
    virtual void setVisible( bool );
    bool isVisible() const;

    virtual void itemChanged();

    /*!
      \brief Draw the item

      \param painter Painter
      \param azimuthMap Maps azimuth values to values related to 0.0, M_2PI
      \param radialMap Maps radius values into painter coordinates.
      \param pole Position of the pole in painter coordinates
      \param radius Radius of the complete plot area in painter coordinates
      \param canvasRect Contents rect of the canvas in painter coordinates
    */
    virtual void draw( QPainter *painter,
                       const QwtScaleMap &azimuthMap, const QwtScaleMap &radialMap,
                       const QwtDoublePoint &pole, double radius,
                       const QwtDoubleRect &canvasRect ) const = 0;

    virtual QwtDoubleInterval boundingInterval( int scaleId ) const;

    virtual QWidget *legendItem() const;

    virtual void updateLegend( QwtLegend * ) const;
    virtual void updateScaleDiv( const QwtScaleDiv &,
                                 const QwtScaleDiv &, const QwtDoubleInterval & );

    virtual int marginHint() const;

  private:
    // Disabled copy constructor and operator=
    QwtPolarItem( const QwtPolarItem & );
    QwtPolarItem &operator=( const QwtPolarItem & );

    class PrivateData;
    PrivateData *d_data;
};

#endif
