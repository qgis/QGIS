/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_COLOR_MAP_H
#define QWT_COLOR_MAP_H

#include "qwt_global.h"
#include <qcolor.h>

class QwtInterval;

#if QT_VERSION < 0x060000
template< typename T > class QVector;
#endif

/*!
   \brief QwtColorMap is used to map values into colors.

   For displaying 3D data on a 2D plane the 3rd dimension is often
   displayed using colors, like f.e in a spectrogram.

   Each color map is optimized to return colors for only one of the
   following image formats:

   - QImage::Format_Indexed8\n
   - QImage::Format_ARGB32\n

   \sa QwtPlotSpectrogram, QwtScaleWidget
 */

class QWT_EXPORT QwtColorMap
{
  public:
    /*!
        Format for color mapping
        \sa rgb(), colorIndex(), colorTable()
     */

    enum Format
    {
        //! The map is intended to map into RGB values.
        RGB,

        /*!
           Map values into 8 bit values, that
           are used as indexes into the color table.

           Indexed color maps are used to generate QImage::Format_Indexed8
           images. The calculation of the color index is usually faster
           and the resulting image has a lower memory footprint.

           \sa colorIndex(), colorTable()
         */
        Indexed
    };

    explicit QwtColorMap( Format = QwtColorMap::RGB );
    virtual ~QwtColorMap();

    void setFormat( Format );
    Format format() const;

    /*!
       Map a value of a given interval into a RGB value.

       \param interval Range for the values
       \param value Value
       \return RGB value, corresponding to value
     */
    virtual QRgb rgb( const QwtInterval& interval, double value ) const = 0;

    virtual uint colorIndex( int numColors,
        const QwtInterval& interval, double value ) const;

    QColor color( const QwtInterval&, double value ) const;
    virtual QVector< QRgb > colorTable( int numColors ) const;
    virtual QVector< QRgb > colorTable256() const;

  private:
    Q_DISABLE_COPY(QwtColorMap)

    Format m_format;
};

/*!
   \brief QwtLinearColorMap builds a color map from color stops.

   A color stop is a color at a specific position. The valid
   range for the positions is [0.0, 1.0]. When mapping a value
   into a color it is translated into this interval according to mode().
 */
class QWT_EXPORT QwtLinearColorMap : public QwtColorMap
{
  public:
    /*!
       Mode of color map
       \sa setMode(), mode()
     */
    enum Mode
    {
        //! Return the color from the next lower color stop
        FixedColors,

        //! Interpolating the colors of the adjacent stops.
        ScaledColors
    };

    explicit QwtLinearColorMap( QwtColorMap::Format = QwtColorMap::RGB );

    QwtLinearColorMap( const QColor& from, const QColor& to,
        QwtColorMap::Format = QwtColorMap::RGB );

    virtual ~QwtLinearColorMap();

    void setMode( Mode );
    Mode mode() const;

    void setColorInterval( const QColor& color1, const QColor& color2 );
    void addColorStop( double value, const QColor& );
    QVector< double > colorStops() const;

    QColor color1() const;
    QColor color2() const;

    virtual QRgb rgb( const QwtInterval&,
        double value ) const QWT_OVERRIDE;

    virtual uint colorIndex( int numColors,
        const QwtInterval&, double value ) const QWT_OVERRIDE;

    class ColorStops;

  private:
    class PrivateData;
    PrivateData* m_data;
};

/*!
   \brief QwtAlphaColorMap varies the alpha value of a color
 */
class QWT_EXPORT QwtAlphaColorMap : public QwtColorMap
{
  public:
    explicit QwtAlphaColorMap( const QColor& = QColor( Qt::gray ) );
    virtual ~QwtAlphaColorMap();

    void setAlphaInterval( int alpha1, int alpha2 );

    int alpha1() const;
    int alpha2() const;

    void setColor( const QColor& );
    QColor color() const;

    virtual QRgb rgb( const QwtInterval&,
        double value ) const QWT_OVERRIDE;

  private:
    class PrivateData;
    PrivateData* m_data;
};

/*!
   \brief QwtHueColorMap varies the hue value of the HSV color model.

   QwtHueColorMap can be used to set up a color map easily, that runs cyclic over
   all colors. Each cycle has 360 different steps.

   The values for value and saturation are in the range of 0 to 255 and doesn't
   depend on the data value to be mapped.

   \sa QwtSaturationValueColorMap
 */
class QWT_EXPORT QwtHueColorMap : public QwtColorMap
{
  public:
    explicit QwtHueColorMap( QwtColorMap::Format = QwtColorMap::RGB );
    virtual ~QwtHueColorMap();

    void setHueInterval( int hue1, int hue2 ); // direction ?
    void setSaturation( int saturation );
    void setValue( int value );
    void setAlpha( int alpha );

    int hue1() const;
    int hue2() const;
    int saturation() const;
    int value() const;
    int alpha() const;

    virtual QRgb rgb( const QwtInterval&,
        double value ) const QWT_OVERRIDE;

  private:
    class PrivateData;
    PrivateData* m_data;
};

/*!
   \brief QwtSaturationValueColorMap varies the saturation and/or value for a given
         hue in the HSV color model.

   Value and saturation are in the range of 0 to 255 while hue is in the range
   of 0 to 259.

   \sa QwtHueColorMap
 */
class QWT_EXPORT QwtSaturationValueColorMap : public QwtColorMap
{
  public:
    QwtSaturationValueColorMap();
    virtual ~QwtSaturationValueColorMap();

    void setHue( int hue );
    void setSaturationInterval( int sat1, int sat2 );
    void setValueInterval( int value1, int value2 );
    void setAlpha( int alpha );

    int hue() const;
    int saturation1() const;
    int saturation2() const;
    int value1() const;
    int value2() const;
    int alpha() const;

    virtual QRgb rgb( const QwtInterval&,
        double value ) const QWT_OVERRIDE;

  private:
    class PrivateData;
    PrivateData* m_data;
};

/*!
   Map a value into a color

   \param interval Valid interval for values
   \param value Value

   \return Color corresponding to value
 */
inline QColor QwtColorMap::color( const QwtInterval& interval, double value ) const
{
    return QColor::fromRgba( rgb( interval, value ) );
}

/*!
   \return Intended format of the color map
   \sa Format
 */
inline QwtColorMap::Format QwtColorMap::format() const
{
    return m_format;
}

#endif
