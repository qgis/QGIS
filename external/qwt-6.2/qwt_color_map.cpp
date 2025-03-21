/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_color_map.h"
#include "qwt_interval.h"

#include <qvector.h>

static inline QRgb qwtHsvToRgb( int h, int s, int v, int a )
{
#if 0
    return QColor::fromHsv( h, s, v, a ).rgb();
#else

    const double vs = v * s / 255.0;
    const int p = v - qRound( vs );

    switch( h / 60 )
    {
        case 0:
        {
            const double r = ( 60 - h ) / 60.0;
            return qRgba( v, v - qRound( r * vs ), p, a );
        }
        case 1:
        {
            const double r = ( h - 60 ) / 60.0;
            return qRgba( v - qRound( r * vs ), v, p, a );
        }
        case 2:
        {
            const double r = ( 180 - h ) / 60.0;
            return qRgba( p, v, v - qRound( r * vs ), a );
        }
        case 3:
        {
            const double r = ( h - 180 ) / 60.0;
            return qRgba( p, v - qRound( r * vs ), v, a );
        }
        case 4:
        {
            const double r = ( 300 - h ) / 60.0;
            return qRgba( v - qRound( r * vs ), p, v, a );
        }
        case 5:
        default:
        {
            const double r = ( h - 300 ) / 60.0;
            return qRgba( v, p, v - qRound( r * vs ), a );
        }
    }
#endif
}

class QwtLinearColorMap::ColorStops
{
  public:
    ColorStops():
        m_doAlpha( false )
    {
        m_stops.reserve( 256 );
    }

    void insert( double pos, const QColor& color );
    QRgb rgb( QwtLinearColorMap::Mode, double pos ) const;

    QVector< double > stops() const;

  private:

    class ColorStop
    {
      public:
        ColorStop():
            pos( 0.0 ),
            rgb( 0 )
        {
        };

        ColorStop( double p, const QColor& c ):
            pos( p ),
            rgb( c.rgba() )
        {
            r = qRed( rgb );
            g = qGreen( rgb );
            b = qBlue( rgb );
            a = qAlpha( rgb );

            /*
                when mapping a value to rgb we will have to calculate:
                   - const int v = int( ( s1.v0 + ratio * s1.vStep ) + 0.5 );

                Thus adding 0.5 ( for rounding ) can be done in advance
             */
            r0 = r + 0.5;
            g0 = g + 0.5;
            b0 = b + 0.5;
            a0 = a + 0.5;

            rStep = gStep = bStep = aStep = 0.0;
            posStep = 0.0;
        }

        void updateSteps( const ColorStop& nextStop )
        {
            rStep = nextStop.r - r;
            gStep = nextStop.g - g;
            bStep = nextStop.b - b;
            aStep = nextStop.a - a;
            posStep = nextStop.pos - pos;
        }

        double pos;
        QRgb rgb;
        int r, g, b, a;

        // precalculated values
        double rStep, gStep, bStep, aStep;
        double r0, g0, b0, a0;
        double posStep;
    };

    inline int findUpper( double pos ) const;
    QVector< ColorStop > m_stops;
    bool m_doAlpha;
};

void QwtLinearColorMap::ColorStops::insert( double pos, const QColor& color )
{
    // Lookups need to be very fast, insertions are not so important.
    // Anyway, a balanced tree is what we need here. TODO ...

    if ( pos < 0.0 || pos > 1.0 )
        return;

    int index;
    if ( m_stops.size() == 0 )
    {
        index = 0;
        m_stops.resize( 1 );
    }
    else
    {
        index = findUpper( pos );
        if ( index == m_stops.size() ||
            qAbs( m_stops[index].pos - pos ) >= 0.001 )
        {
            m_stops.resize( m_stops.size() + 1 );
            for ( int i = m_stops.size() - 1; i > index; i-- )
                m_stops[i] = m_stops[i - 1];
        }
    }

    m_stops[index] = ColorStop( pos, color );
    if ( color.alpha() != 255 )
        m_doAlpha = true;

    if ( index > 0 )
        m_stops[index - 1].updateSteps( m_stops[index] );

    if ( index < m_stops.size() - 1 )
        m_stops[index].updateSteps( m_stops[index + 1] );
}

inline QVector< double > QwtLinearColorMap::ColorStops::stops() const
{
    QVector< double > positions( m_stops.size() );
    for ( int i = 0; i < m_stops.size(); i++ )
        positions[i] = m_stops[i].pos;
    return positions;
}

inline int QwtLinearColorMap::ColorStops::findUpper( double pos ) const
{
    int index = 0;
    int n = m_stops.size();

    const ColorStop* stops = m_stops.data();

    while ( n > 0 )
    {
        const int half = n >> 1;
        const int middle = index + half;

        if ( stops[middle].pos <= pos )
        {
            index = middle + 1;
            n -= half + 1;
        }
        else
            n = half;
    }

    return index;
}

inline QRgb QwtLinearColorMap::ColorStops::rgb(
    QwtLinearColorMap::Mode mode, double pos ) const
{
    if ( pos <= 0.0 )
        return m_stops[0].rgb;
    if ( pos >= 1.0 )
        return m_stops[ m_stops.size() - 1 ].rgb;

    const int index = findUpper( pos );
    if ( mode == FixedColors )
    {
        return m_stops[index - 1].rgb;
    }
    else
    {
        const ColorStop& s1 = m_stops[index - 1];

        const double ratio = ( pos - s1.pos ) / ( s1.posStep );

        const int r = int( s1.r0 + ratio * s1.rStep );
        const int g = int( s1.g0 + ratio * s1.gStep );
        const int b = int( s1.b0 + ratio * s1.bStep );

        if ( m_doAlpha )
        {
            if ( s1.aStep )
            {
                const int a = int( s1.a0 + ratio * s1.aStep );
                return qRgba( r, g, b, a );
            }
            else
            {
                return qRgba( r, g, b, s1.a );
            }
        }
        else
        {
            return qRgb( r, g, b );
        }
    }
}

/*!
   Constructor
   \param format Format of the color map
 */
QwtColorMap::QwtColorMap( Format format )
    : m_format( format )
{
}

//! Destructor
QwtColorMap::~QwtColorMap()
{
}

/*!
   Set the format of the color map

   \param format Format of the color map
 */
void QwtColorMap::setFormat( Format format )
{
    m_format = format;
}

/*!
   \brief Map a value of a given interval into a color index

   \param numColors Number of colors
   \param interval Range for all values
   \param value Value to map into a color index

   \return Index, between 0 and numColors - 1, or -1 for an invalid value
 */
uint QwtColorMap::colorIndex( int numColors,
    const QwtInterval& interval, double value ) const
{
    const double width = interval.width();
    if ( width <= 0.0 )
        return 0;

    if ( value <= interval.minValue() )
        return 0;

    const int maxIndex = numColors - 1;
    if ( value >= interval.maxValue() )
        return maxIndex;

    const double v = maxIndex * ( ( value - interval.minValue() ) / width );
    return static_cast< unsigned int >( v + 0.5 );
}

/*!
   Build and return a color map of 256 colors

   The color table is needed for rendering indexed images in combination
   with using colorIndex().

   \return A color table, that can be used for a QImage
 */
QVector< QRgb > QwtColorMap::colorTable256() const
{
    QVector< QRgb > table( 256 );

    const QwtInterval interval( 0, 256 );

    for ( int i = 0; i < 256; i++ )
        table[i] = rgb( interval, i );

    return table;
}

/*!
   Build and return a color map of arbitrary number of colors

   The color table is needed for rendering indexed images in combination
   with using colorIndex().

   \param numColors Number of colors
   \return A color table
 */
QVector< QRgb > QwtColorMap::colorTable( int numColors ) const
{
    QVector< QRgb > table( numColors );

    const QwtInterval interval( 0.0, 1.0 );

    const double step = 1.0 / ( numColors - 1 );
    for ( int i = 0; i < numColors; i++ )
        table[i] = rgb( interval, step * i );

    return table;
}

class QwtLinearColorMap::PrivateData
{
  public:
    ColorStops colorStops;
    QwtLinearColorMap::Mode mode;
};

/*!
   Build a color map with two stops at 0.0 and 1.0. The color
   at 0.0 is Qt::blue, at 1.0 it is Qt::yellow.

   \param format Preferred format of the color map
 */
QwtLinearColorMap::QwtLinearColorMap( QwtColorMap::Format format ):
    QwtColorMap( format )
{
    m_data = new PrivateData;
    m_data->mode = ScaledColors;

    setColorInterval( Qt::blue, Qt::yellow );
}

/*!
   Build a color map with two stops at 0.0 and 1.0.

   \param color1 Color used for the minimum value of the value interval
   \param color2 Color used for the maximum value of the value interval
   \param format Preferred format for the color map
 */
QwtLinearColorMap::QwtLinearColorMap( const QColor& color1,
        const QColor& color2, QwtColorMap::Format format )
    : QwtColorMap( format )
{
    m_data = new PrivateData;
    m_data->mode = ScaledColors;
    setColorInterval( color1, color2 );
}

//! Destructor
QwtLinearColorMap::~QwtLinearColorMap()
{
    delete m_data;
}

/*!
   \brief Set the mode of the color map

   FixedColors means the color is calculated from the next lower
   color stop. ScaledColors means the color is calculated
   by interpolating the colors of the adjacent stops.

   \sa mode()
 */
void QwtLinearColorMap::setMode( Mode mode )
{
    m_data->mode = mode;
}

/*!
   \return Mode of the color map
   \sa setMode()
 */
QwtLinearColorMap::Mode QwtLinearColorMap::mode() const
{
    return m_data->mode;
}

/*!
   Set the color range

   Add stops at 0.0 and 1.0.

   \param color1 Color used for the minimum value of the value interval
   \param color2 Color used for the maximum value of the value interval

   \sa color1(), color2()
 */
void QwtLinearColorMap::setColorInterval(
    const QColor& color1, const QColor& color2 )
{
    m_data->colorStops = ColorStops();
    m_data->colorStops.insert( 0.0, color1 );
    m_data->colorStops.insert( 1.0, color2 );
}

/*!
   Add a color stop

   The value has to be in the range [0.0, 1.0].
   F.e. a stop at position 17.0 for a range [10.0,20.0] must be
   passed as: (17.0 - 10.0) / (20.0 - 10.0)

   \param value Value between [0.0, 1.0]
   \param color Color stop
 */
void QwtLinearColorMap::addColorStop( double value, const QColor& color )
{
    if ( value >= 0.0 && value <= 1.0 )
        m_data->colorStops.insert( value, color );
}

/*!
   \return Positions of color stops in increasing order
 */
QVector< double > QwtLinearColorMap::colorStops() const
{
    return m_data->colorStops.stops();
}

/*!
   \return the first color of the color range
   \sa setColorInterval()
 */
QColor QwtLinearColorMap::color1() const
{
    return QColor::fromRgba( m_data->colorStops.rgb( m_data->mode, 0.0 ) );
}

/*!
   \return the second color of the color range
   \sa setColorInterval()
 */
QColor QwtLinearColorMap::color2() const
{
    return QColor::fromRgba( m_data->colorStops.rgb( m_data->mode, 1.0 ) );
}

/*!
   Map a value of a given interval into a RGB value

   \param interval Range for all values
   \param value Value to map into a RGB value

   \return RGB value for value
 */
QRgb QwtLinearColorMap::rgb(
    const QwtInterval& interval, double value ) const
{
    const double width = interval.width();
    if ( width <= 0.0 )
        return 0u;

    const double ratio = ( value - interval.minValue() ) / width;
    return m_data->colorStops.rgb( m_data->mode, ratio );
}

/*!
   \brief Map a value of a given interval into a color index

   \param numColors Size of the color table
   \param interval Range for all values
   \param value Value to map into a color index

   \return Index, between 0 and 255
   \note NaN values are mapped to 0
 */
uint QwtLinearColorMap::colorIndex( int numColors,
    const QwtInterval& interval, double value ) const
{
    const double width = interval.width();
    if ( width <= 0.0 )
        return 0;

    if ( value <= interval.minValue() )
        return 0;

    if ( value >= interval.maxValue() )
        return numColors - 1;

    const double v = ( numColors - 1 ) * ( value - interval.minValue() ) / width;
    return static_cast< unsigned int >( ( m_data->mode == FixedColors ) ? v : v + 0.5 );
}

class QwtAlphaColorMap::PrivateData
{
  public:
    PrivateData()
        : alpha1(0)
        , alpha2(255)
    {
    }

    int alpha1, alpha2;

    QColor color;
    QRgb rgb;

    QRgb rgbMin;
    QRgb rgbMax;
};


/*!
   \brief Constructor

   The alpha interval is initialized by 0 to 255.

   \param color Color of the map

   \sa setColor(), setAlphaInterval()
 */
QwtAlphaColorMap::QwtAlphaColorMap( const QColor& color )
    : QwtColorMap( QwtColorMap::RGB )
{
    m_data = new PrivateData;
    setColor( color );
}

//! Destructor
QwtAlphaColorMap::~QwtAlphaColorMap()
{
    delete m_data;
}

/*!
   Set the color

   \param color Color
   \sa color()
 */
void QwtAlphaColorMap::setColor( const QColor& color )
{
    m_data->color = color;
    m_data->rgb = color.rgb() & qRgba( 255, 255, 255, 0 );

    m_data->rgbMin = m_data->rgb | ( m_data->alpha1 << 24 );
    m_data->rgbMax = m_data->rgb | ( m_data->alpha2 << 24 );
}

/*!
   \return the color
   \sa setColor()
 */
QColor QwtAlphaColorMap::color() const
{
    return m_data->color;
}

/*!
   Set the interval for the alpha coordinate

   alpha1/alpha2 need to be in the range 0 to 255,
   where 255 means opaque and 0 means transparent.

   \param alpha1 First alpha coordinate
   \param alpha2 Second alpha coordinate

   \sa alpha1(), alpha2()
 */
void QwtAlphaColorMap::setAlphaInterval( int alpha1, int alpha2 )
{
    m_data->alpha1 = qBound( 0, alpha1, 255 );
    m_data->alpha2 = qBound( 0, alpha2, 255 );

    m_data->rgbMin = m_data->rgb | ( alpha1 << 24 );
    m_data->rgbMax = m_data->rgb | ( alpha2 << 24 );
}

/*!
   \return First alpha coordinate
   \sa setAlphaInterval()
 */
int QwtAlphaColorMap::alpha1() const
{
    return m_data->alpha1;
}

/*!
   \return Second alpha coordinate
   \sa setAlphaInterval()
 */
int QwtAlphaColorMap::alpha2() const
{
    return m_data->alpha2;
}

/*!
   \brief Map a value of a given interval into a alpha value

   \param interval Range for all values
   \param value Value to map into a RGB value

   \return RGB value, with an alpha value
 */
QRgb QwtAlphaColorMap::rgb( const QwtInterval& interval, double value ) const
{
    const double width = interval.width();
    if ( width <= 0.0 )
        return 0u;

    if ( value <= interval.minValue() )
        return m_data->rgb;

    if ( value >= interval.maxValue() )
        return m_data->rgbMax;

    const double ratio = ( value - interval.minValue() ) / width;
    const int alpha = m_data->alpha1 + qRound( ratio * ( m_data->alpha2 - m_data->alpha1 ) );

    return m_data->rgb | ( alpha << 24 );
}

class QwtHueColorMap::PrivateData
{
  public:
    PrivateData();

    void updateTable();

    int hue1, hue2;
    int saturation;
    int value;
    int alpha;

    QRgb rgbMin;
    QRgb rgbMax;

    QRgb rgbTable[360];
};

QwtHueColorMap::PrivateData::PrivateData()
    : hue1(0)
    , hue2(359)
    , saturation(255)
    , value(255)
    , alpha(255)
{
    updateTable();
}

void QwtHueColorMap::PrivateData::updateTable()
{
    const int p = qRound( value * ( 255 - saturation ) / 255.0 );
    const double vs = value * saturation / 255.0;

    for ( int i = 0; i < 60; i++ )
    {
        const double r = ( 60 - i ) / 60.0;
        rgbTable[i] = qRgba( value, qRound( value - r * vs ), p, alpha );
    }

    for ( int i = 60; i < 120; i++ )
    {
        const double r = ( i - 60 ) / 60.0;
        rgbTable[i] = qRgba( qRound( value - r * vs ), value, p, alpha );
    }

    for ( int i = 120; i < 180; i++ )
    {
        const double r = ( 180 - i ) / 60.0;
        rgbTable[i] = qRgba( p, value, qRound( value - r * vs ), alpha );
    }

    for ( int i = 180; i < 240; i++ )
    {
        const double r = ( i - 180 ) / 60.0;
        rgbTable[i] = qRgba( p, qRound( value - r * vs ), value, alpha );
    }

    for ( int i = 240; i < 300; i++ )
    {
        const double r = ( 300 - i ) / 60.0;
        rgbTable[i] = qRgba( qRound( value - r * vs ), p, value, alpha );
    }

    for ( int i = 300; i < 360; i++ )
    {
        const double r = ( i - 300 ) / 60.0;
        rgbTable[i] = qRgba( value, p, qRound( value - r * vs ), alpha );
    }

    rgbMin = rgbTable[ hue1 % 360 ];
    rgbMax = rgbTable[ hue2 % 360 ];
}

/*!
   \brief Constructor

   The hue interval is initialized by 0 to 359. All other coordinates
   are set to 255.

   \param format Format of the color map

   \sa setHueInterval(), setSaturation(), setValue(), setValue()
 */
QwtHueColorMap::QwtHueColorMap( QwtColorMap::Format format )
    : QwtColorMap( format )
{
    m_data = new PrivateData;
}

//! Destructor
QwtHueColorMap::~QwtHueColorMap()
{
    delete m_data;
}

/*!
   Set the interval for the hue coordinate

   hue1/hue2 need to be positive number and can be > 360 to define cycles.
   F.e. 420 to 240 defines a map yellow/red/magenta/blue.

   \param hue1 First hue coordinate
   \param hue2 Second hue coordinate

   \sa hue1(), hue2()
 */
void QwtHueColorMap::setHueInterval( int hue1, int hue2 )
{
    m_data->hue1 = qMax( hue1, 0 );
    m_data->hue2 = qMax( hue2, 0 );

    m_data->rgbMin = m_data->rgbTable[ hue1 % 360 ];
    m_data->rgbMax = m_data->rgbTable[ hue2 % 360 ];
}

/*!
   \brief Set the the saturation coordinate

   saturation needs to be in the range 0 to 255,

   \param saturation Saturation coordinate

   \sa saturation()
 */
void QwtHueColorMap::setSaturation( int saturation )
{
    saturation = qBound( 0, saturation, 255 );

    if ( saturation != m_data->saturation )
    {
        m_data->saturation = saturation;
        m_data->updateTable();
    }
}

/*!
   \brief Set the the value coordinate

   value needs to be in the range 0 to 255,

   \param value Value coordinate

   \sa value()
 */
void QwtHueColorMap::setValue( int value )
{
    value = qBound( 0, value, 255 );

    if ( value != m_data->value )
    {
        m_data->value = value;
        m_data->updateTable();
    }
}

/*!
   \brief Set the the alpha coordinate

   alpha needs to be in the range 0 to 255,
   where 255 means opaque and 0 means transparent.

   \param alpha Alpha coordinate

   \sa alpha()
 */
void QwtHueColorMap::setAlpha( int alpha )
{
    alpha = qBound( 0, alpha, 255 );

    if ( alpha != m_data->alpha )
    {
        m_data->alpha = alpha;
        m_data->updateTable();
    }
}

/*!
   \return First hue coordinate
   \sa setHueInterval()
 */
int QwtHueColorMap::hue1() const
{
    return m_data->hue1;
}

/*!
   \return Second hue coordinate
   \sa setHueInterval()
 */
int QwtHueColorMap::hue2() const
{
    return m_data->hue2;
}

/*!
   \return Saturation coordinate
   \sa setSaturation()
 */
int QwtHueColorMap::saturation() const
{
    return m_data->saturation;
}

/*!
   \return Value coordinate
   \sa setValue()
 */
int QwtHueColorMap::value() const
{
    return m_data->value;
}

/*!
   \return Alpha coordinate
   \sa setAlpha()
 */
int QwtHueColorMap::alpha() const
{
    return m_data->alpha;
}

/*!
   Map a value of a given interval into a RGB value

   \param interval Range for all values
   \param value Value to map into a RGB value

   \return RGB value for value
 */
QRgb QwtHueColorMap::rgb( const QwtInterval& interval, double value ) const
{
    const double width = interval.width();
    if ( width <= 0 )
        return 0u;

    if ( value <= interval.minValue() )
        return m_data->rgbMin;

    if ( value >= interval.maxValue() )
        return m_data->rgbMax;

    const double ratio = ( value - interval.minValue() ) / width;

    int hue = m_data->hue1 + qRound( ratio * ( m_data->hue2 - m_data->hue1 ) );
    if ( hue >= 360 )
    {
        hue -= 360;

        if ( hue >= 360 )
            hue = hue % 360;
    }

    return m_data->rgbTable[hue];
}

class QwtSaturationValueColorMap::PrivateData
{
  public:
    PrivateData()
        : hue(0)
        , sat1(255)
        , sat2(255)
        , value1(0)
        , value2(255)
        , alpha(255)
        , tableType(Invalid)
    {
        updateTable();
    }

    void updateTable()
    {
        tableType = Invalid;

        if ( ( value1 == value2 ) && ( sat1 != sat2 ) )
        {
            rgbTable.resize( 256 );

            for ( int i = 0; i < 256; i++ )
                rgbTable[i] = qwtHsvToRgb( hue, i, value1, alpha );

            tableType = Saturation;
        }
        else if ( ( value1 != value2 ) && ( sat1 == sat2 ) )
        {
            rgbTable.resize( 256 );

            for ( int i = 0; i < 256; i++ )
                rgbTable[i] = qwtHsvToRgb( hue, sat1, i, alpha );

            tableType = Value;
        }
        else
        {
            rgbTable.resize( 256 * 256 );

            for ( int s = 0; s < 256; s++ )
            {
                const int v0 = s * 256;

                for ( int v = 0; v < 256; v++ )
                    rgbTable[v0 + v] = qwtHsvToRgb( hue, s, v, alpha );
            }
        }
    }

    int hue;
    int sat1, sat2;
    int value1, value2;
    int alpha;

    enum
    {
        Invalid,
        Value,
        Saturation

    } tableType;

    QVector< QRgb > rgbTable;
};

/*!
   \brief Constructor

   The value interval is initialized by 0 to 255,
   saturation by 255 to 255. Hue to 0 and alpha to 255.

   So the default setting interpolates the value coordinate only.

   \sa setHueInterval(), setSaturation(), setValue(), setValue()
 */
QwtSaturationValueColorMap::QwtSaturationValueColorMap()
{
    m_data = new PrivateData;
}

//! Destructor
QwtSaturationValueColorMap::~QwtSaturationValueColorMap()
{
    delete m_data;
}

/*!
   \brief Set the the hue coordinate

   Hue coordinates outside 0 to 359 will be interpreted as hue % 360..

   \param hue Hue coordinate

   \sa hue()
 */
void QwtSaturationValueColorMap::setHue( int hue )
{
    hue = hue % 360;

    if ( hue != m_data->hue )
    {
        m_data->hue = hue;
        m_data->updateTable();
    }
}

/*!
   \brief Set the interval for the saturation coordinate

   When saturation1 == saturation2 the map interpolates between
   the value coordinates only

   saturation1/saturation2 need to be in the range 0 to 255.

   \param saturation1 First saturation
   \param saturation2 Second saturation

   \sa saturation1(), saturation2(), setValueInterval()
 */
void QwtSaturationValueColorMap::setSaturationInterval(
    int saturation1, int saturation2 )
{
    saturation1 = qBound( 0, saturation1, 255 );
    saturation2 = qBound( 0, saturation2, 255 );

    if ( ( saturation1 != m_data->sat1 ) || ( saturation2 != m_data->sat2 ) )
    {
        m_data->sat1 = saturation1;
        m_data->sat2 = saturation2;

        m_data->updateTable();
    }
}

/*!
   \brief Set the interval for the value coordinate

   When value1 == value2 the map interpolates between the saturation coordinates only.

   value1/value2 need to be in the range 0 to 255.

   \param value1 First value
   \param value2 Second value

   \sa value1(), value2(), setSaturationInterval()
 */
void QwtSaturationValueColorMap::setValueInterval( int value1, int value2 )
{
    value1 = qBound( 0, value1, 255 );
    value2 = qBound( 0, value2, 255 );

    if ( ( value1 != m_data->value1 ) || ( value2 != m_data->value2 ) )
    {
        m_data->value1 = value1;
        m_data->value2 = value2;

        m_data->updateTable();
    }
}

/*!
   \brief Set the the alpha coordinate

   alpha needs to be in the range 0 to 255,
   where 255 means opaque and 0 means transparent.

   \param alpha Alpha coordinate

   \sa alpha()
 */
void QwtSaturationValueColorMap::setAlpha( int alpha )
{
    alpha = qBound( 0, alpha, 255 );

    if ( alpha != m_data->alpha )
    {
        m_data->alpha = alpha;
        m_data->updateTable();
    }
}

/*!
   \return Hue coordinate
   \sa setHue()
 */
int QwtSaturationValueColorMap::hue() const
{
    return m_data->hue;
}

/*!
   \return First saturation coordinate
   \sa setSaturationInterval()
 */
int QwtSaturationValueColorMap::saturation1() const
{
    return m_data->sat1;
}

/*!
   \return Second saturation coordinate
   \sa setSaturationInterval()
 */
int QwtSaturationValueColorMap::saturation2() const
{
    return m_data->sat2;
}

/*!
   \return First value coordinate
   \sa setValueInterval()
 */
int QwtSaturationValueColorMap::value1() const
{
    return m_data->value1;
}

/*!
   \return Second value coordinate
   \sa setValueInterval()
 */
int QwtSaturationValueColorMap::value2() const
{
    return m_data->value2;
}

/*!
   \return Alpha coordinate
   \sa setAlpha()
 */
int QwtSaturationValueColorMap::alpha() const
{
    return m_data->alpha;
}

/*!
   Map a value of a given interval into a RGB value

   \param interval Range for all values
   \param value Value to map into a RGB value

   \return RGB value for value
 */
QRgb QwtSaturationValueColorMap::rgb(
    const QwtInterval& interval, double value ) const
{
    const double width = interval.width();
    if ( width <= 0 )
        return 0u;

    const QRgb* rgbTable = m_data->rgbTable.constData();

    switch( m_data->tableType )
    {
        case PrivateData::Saturation:
        {
            if ( value <= interval.minValue() )
                return m_data->rgbTable[m_data->sat1];

            if ( value >= interval.maxValue() )
                return m_data->rgbTable[m_data->sat2];

            const double ratio = ( value - interval.minValue() ) / width;
            const int sat = m_data->sat1
                + qRound( ratio * ( m_data->sat2 - m_data->sat1 ) );

            return rgbTable[sat];
        }
        case PrivateData::Value:
        {
            if ( value <= interval.minValue() )
                return m_data->rgbTable[m_data->value1];

            if ( value >= interval.maxValue() )
                return m_data->rgbTable[m_data->value2];

            const double ratio = ( value - interval.minValue() ) / width;
            const int v = m_data->value1 +
                qRound( ratio * ( m_data->value2 - m_data->value1 ) );

            return rgbTable[ v ];
        }
        default:
        {
            int s, v;
            if ( value <= interval.minValue() )
            {
                s = m_data->sat1;
                v = m_data->value1;
            }
            else if ( value >= interval.maxValue() )
            {
                s = m_data->sat2;
                v = m_data->value2;
            }
            else
            {
                const double ratio = ( value - interval.minValue() ) / width;

                v = m_data->value1 + qRound( ratio * ( m_data->value2 - m_data->value1 ) );
                s = m_data->sat1 + qRound( ratio * ( m_data->sat2 - m_data->sat1 ) );
            }

            return rgbTable[ 256 * s + v ];
        }
    }
}
