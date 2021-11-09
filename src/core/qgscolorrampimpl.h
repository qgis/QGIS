/***************************************************************************
    qgscolorrampimpl.h
    ---------------------
    begin                : November 2009
    copyright            : (C) 2009 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCOLORRAMPIMPL_H
#define QGSCOLORRAMPIMPL_H

#include "qgis_core.h"
#include <QColor>
#include <QGradient>
#include "qgis.h"
#include "qgscolorscheme.h"
#include "qgscolorramp.h"

#ifndef SIP_RUN
///@cond PRIVATE
typedef QColor( *InterpolateColorFunc )( const QColor &c1, const QColor &c2, const double value, Qgis::AngularDirection direction );
///@endcond
#endif

/**
 * \ingroup core
 * \class QgsGradientStop
 * \brief Represents a color stop within a QgsGradientColorRamp color ramp.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsGradientStop
{
  public:

    /**
     * Constructor for QgsGradientStop
     * \param offset positional offset for stop, between 0 and 1.0
     * \param color color for stop
     */
    QgsGradientStop( double offset, const QColor &color );

    //! Relative positional offset, between 0 and 1
    double offset;
    //! Gradient color at stop
    QColor color;

    bool operator==( const QgsGradientStop &other ) const
    {
      return other.color == color && qgsDoubleNear( other.offset, offset ) && other.mColorSpec == mColorSpec && other.mDirection == mDirection;
    }

    bool operator!=( const QgsGradientStop &other ) const
    {
      return !( *this == other );
    }

    /**
     * Returns the color specification in which the color component interpolation will occur.
     *
     * For multi-stop gradients this color spec will be used for the portion of the color ramp
     * leading into the current stop.
     *
     * \see setColorSpec()
     * \since QGIS 3.24
     */
    QColor::Spec colorSpec() const { return mColorSpec; }

    /**
     * Sets the color specification in which the color component interpolation will occur.
     *
     * Only QColor::Spec::Rgb, QColor::Spec::Hsv and QColor::Spec::Hsl are currently supported.
     *
     * For multi-stop gradients this color spec will be used for the portion of the color ramp
     * leading into the current stop.
     *
     * \see colorSpec()
     * \since QGIS 3.24
     */
    void setColorSpec( QColor::Spec spec );

    /**
     * Returns the direction to traverse the color wheel using when interpolating hue-based color
     * specifications.
     *
     * For multi-stop gradients this direction will be used for the portion of the color ramp
     * leading into the current stop.
     *
     * \see setDirection()
     * \since QGIS 3.24
     */
    Qgis::AngularDirection direction() const { return mDirection; }

    /**
     * Sets the \a direction to traverse the color wheel using when interpolating hue-based color
     * specifications.
     *
     * For multi-stop gradients this direction will be used for the portion of the color ramp
     * leading into the current stop.
     *
     * \see direction()
     * \since QGIS 3.24
     */
    void setDirection( Qgis::AngularDirection direction ) { mDirection = direction; }

  private:

    QColor::Spec mColorSpec = QColor::Spec::Rgb;
    Qgis::AngularDirection mDirection = Qgis::AngularDirection::CounterClockwise;
    InterpolateColorFunc mFunc = nullptr;

    friend class QgsGradientColorRamp;
};

//! List of gradient stops
typedef QList<QgsGradientStop> QgsGradientStopsList;

// these are the QGIS branding colors, exaggerated a bit to make a default ramp with greater color variation
// then the official QGIS color gradient!
#define DEFAULT_GRADIENT_COLOR1 QColor(69, 116, 40)
#define DEFAULT_GRADIENT_COLOR2 QColor(188, 220, 60)

/**
 * \ingroup core
 * \class QgsGradientColorRamp
 * \brief Gradient color ramp, which smoothly interpolates between two colors and also
 * supports optional extra color stops.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsGradientColorRamp : public QgsColorRamp
{
  public:

    /**
     * Constructor for QgsGradientColorRamp
     * \param color1 start color, corresponding to a position of 0.0
     * \param color2 end color, corresponding to a position of 1.0
     * \param discrete set to TRUE for discrete interpolation instead of smoothly
     * interpolating between colors
     * \param stops optional list of additional color stops
     */
    QgsGradientColorRamp( const QColor &color1 = DEFAULT_GRADIENT_COLOR1,
                          const QColor &color2 = DEFAULT_GRADIENT_COLOR2,
                          bool discrete = false,
                          const QgsGradientStopsList &stops = QgsGradientStopsList() );

    //! Creates a new QgsColorRamp from a map of properties
    static QgsColorRamp *create( const QVariantMap &properties = QVariantMap() ) SIP_FACTORY;

    int count() const override { return mStops.count() + 2; }
    double value( int index ) const override;
    QColor color( double value ) const override;

    /**
     * Returns the string identifier for QgsGradientColorRamp.
     *
     * \since QGIS 3.16
     */
    static QString typeString() { return QStringLiteral( "gradient" ); }

    QString type() const override;
    void invert() override;
    QgsGradientColorRamp *clone() const override SIP_FACTORY;
    QVariantMap properties() const override;

    /**
     * Returns the gradient start color.
     * \see setColor1()
     * \see color2()
     */
    QColor color1() const { return mColor1; }

    /**
     * Returns the gradient end color.
     * \see setColor2()
     * \see color1()
     */
    QColor color2() const { return mColor2; }

    /**
     * Sets the gradient start color.
     * \param color start color
     * \see color1()
     * \see setColor2()
     */
    void setColor1( const QColor &color ) { mColor1 = color; }

    /**
     * Sets the gradient end color.
     * \param color end color
     * \see color2()
     * \see setColor1()
     */
    void setColor2( const QColor &color ) { mColor2 = color; }

    /**
     * Returns TRUE if the gradient is using discrete interpolation, rather than
     * smoothly interpolating between colors.
     * \see setDiscrete()
     */
    bool isDiscrete() const { return mDiscrete; }

    /**
     * Sets whether the gradient should use discrete interpolation, rather than
     * smoothly interpolating between colors.
     * \param discrete set to TRUE to use discrete interpolation
     * \see convertToDiscrete()
     * \see isDiscrete()
     */
    void setDiscrete( bool discrete ) { mDiscrete = discrete; }

    /**
     * Converts a gradient with existing color stops to or from discrete
     * interpolation.
     * \param discrete set to TRUE to convert the gradient stops to discrete,
     * or FALSE to convert them to smooth interpolation
     * \see isDiscrete()
     */
    void convertToDiscrete( bool discrete );

    /**
     * Sets the list of intermediate gradient stops for the ramp.
     * \param stops list of stops. Any existing color stops will be replaced. The stop
     * list will be automatically reordered so that stops are listed in ascending offset
     * order.
     * \see stops()
     */
    void setStops( const QgsGradientStopsList &stops );

    /**
     * Returns the list of intermediate gradient stops for the ramp.
     * \see setStops()
     */
    QgsGradientStopsList stops() const { return mStops; }

    /**
     * Returns any additional info attached to the gradient ramp (e.g., authorship notes)
     * \see setInfo()
     */
    QgsStringMap info() const { return mInfo; }

    /**
     * Sets additional info to attach to the gradient ramp (e.g., authorship notes)
     * \param info map of string info to attach
     * \see info()
     */
    void setInfo( const QgsStringMap &info ) { mInfo = info; }

    /**
     * Copy color ramp stops to a QGradient
     * \param gradient gradient to copy stops into
     * \param opacity opacity multiplier. Opacity of colors will be multiplied
     * by this factor before adding to the gradient.
     * \since QGIS 2.1
     */
    void addStopsToGradient( QGradient *gradient, double opacity = 1 ) const;

    /**
     * Returns the color specification in which the color component interpolation will occur.
     *
     * For multi-stop gradients this color spec will be used for the portion of the color ramp
     * leading into the final stop (i.e. color2()).
     *
     * \see setColorSpec()
     * \since QGIS 3.24
     */
    QColor::Spec colorSpec() const { return mColorSpec; }

    /**
     * Sets the color specification in which the color component interpolation will occur.
     *
     * Only QColor::Spec::Rgb, QColor::Spec::Hsv and QColor::Spec::Hsl are currently supported.
     *
     * For multi-stop gradients this color spec will be used for the portion of the color ramp
     * leading into the final stop (i.e. color2()).
     *
     * \see colorSpec()
     * \since QGIS 3.24
     */
    void setColorSpec( QColor::Spec spec );

    /**
     * Returns the direction to traverse the color wheel using when interpolating hue-based color
     * specifications.
     *
     * For multi-stop gradients this direction will be used for the portion of the color ramp
     * leading into the final stop (i.e. color2()).
     *
     * \see setDirection()
     * \since QGIS 3.24
     */
    Qgis::AngularDirection direction() const { return mDirection; }

    /**
     * Sets the \a direction to traverse the color wheel using when interpolating hue-based color
     * specifications.
     *
     * For multi-stop gradients this direction will be used for the portion of the color ramp
     * leading into the final stop (i.e. color2()).
     *
     * \see direction()
     * \since QGIS 3.24
     */
    void setDirection( Qgis::AngularDirection direction ) { mDirection = direction; }

  protected:
    QColor mColor1;
    QColor mColor2;
    bool mDiscrete;
    QgsGradientStopsList mStops;
    QgsStringMap mInfo;
    QColor::Spec mColorSpec = QColor::Spec::Rgb;
    Qgis::AngularDirection mDirection = Qgis::AngularDirection::CounterClockwise;

    InterpolateColorFunc mFunc = nullptr;
};

Q_DECLARE_METATYPE( QgsGradientColorRamp )

#define DEFAULT_RANDOM_COUNT   10
#define DEFAULT_RANDOM_HUE_MIN 0
#define DEFAULT_RANDOM_HUE_MAX 359
#define DEFAULT_RANDOM_VAL_MIN 200
#define DEFAULT_RANDOM_VAL_MAX 240
#define DEFAULT_RANDOM_SAT_MIN 100
#define DEFAULT_RANDOM_SAT_MAX 240

/**
 * \ingroup core
 * \class QgsLimitedRandomColorRamp
 * \brief Constrained random color ramp, which returns random colors based on preset parameters.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsLimitedRandomColorRamp : public QgsColorRamp
{
  public:

    /**
     * Constructor for QgsLimitedRandomColorRamp
     * \param count number of colors in ramp
     * \param hueMin minimum hue
     * \param hueMax maximum hue
     * \param satMin minimum saturation
     * \param satMax maximum saturation
     * \param valMin minimum color value
     * \param valMax maximum color value
     */
    QgsLimitedRandomColorRamp( int count = DEFAULT_RANDOM_COUNT,
                               int hueMin = DEFAULT_RANDOM_HUE_MIN, int hueMax = DEFAULT_RANDOM_HUE_MAX,
                               int satMin = DEFAULT_RANDOM_SAT_MIN, int satMax = DEFAULT_RANDOM_SAT_MAX,
                               int valMin = DEFAULT_RANDOM_VAL_MIN, int valMax = DEFAULT_RANDOM_VAL_MAX );

    /**
     * Returns a new QgsLimitedRandomColorRamp color ramp created using the properties encoded in a string
     * map.
     * \param properties color ramp properties
     * \see properties()
     */
    static QgsColorRamp *create( const QVariantMap &properties = QVariantMap() ) SIP_FACTORY;

    double value( int index ) const override;
    QColor color( double value ) const override;

    /**
     * Returns the string identifier for QgsLimitedRandomColorRamp.
     *
     * \since QGIS 3.16
     */
    static QString typeString() { return QStringLiteral( "random" ); }

    QString type() const override;
    QgsLimitedRandomColorRamp *clone() const override SIP_FACTORY;
    QVariantMap properties() const override;
    int count() const override { return mCount; }

    /**
     * Gets a list of random colors
     * \since QGIS 2.4
     */
    static QList<QColor> randomColors( int count,
                                       int hueMax = DEFAULT_RANDOM_HUE_MAX, int hueMin = DEFAULT_RANDOM_HUE_MIN,
                                       int satMax = DEFAULT_RANDOM_SAT_MAX, int satMin = DEFAULT_RANDOM_SAT_MIN,
                                       int valMax = DEFAULT_RANDOM_VAL_MAX, int valMin = DEFAULT_RANDOM_VAL_MIN );

    /**
     * Must be called after changing the properties of the color ramp
     * to regenerate the list of random colors.
     */
    void updateColors();

    /**
     * Returns the minimum hue for generated colors
     * \see setHueMin()
     */
    int hueMin() const { return mHueMin; }

    /**
     * Returns the maximum hue for generated colors
     * \see setHueMax()
     */
    int hueMax() const { return mHueMax; }

    /**
     * Returns the minimum saturation for generated colors
     * \see setSatMin()
     */
    int satMin() const { return mSatMin; }

    /**
     * Returns the maximum saturation for generated colors
     * \see setSatMax()
     */
    int satMax() const { return mSatMax; }

    /**
     * Returns the minimum value for generated colors
     * \see setValMin()
     */
    int valMin() const { return mValMin; }

    /**
     * Returns the maximum value for generated colors
     * \see setValMax()
     */
    int valMax() const { return mValMax; }

    /**
     * Sets the number of colors contained in the ramp.
     */
    void setCount( int val ) { mCount = val; }

    /**
     * Sets the minimum hue for generated colors
     * \see hueMin()
     */
    void setHueMin( int val ) { mHueMin = val; }

    /**
     * Sets the maximum hue for generated colors
     * \see hueMax()
     */
    void setHueMax( int val ) { mHueMax = val; }

    /**
     * Sets the minimum saturation for generated colors
     * \see satMin()
     */
    void setSatMin( int val ) { mSatMin = val; }

    /**
     * Sets the maximum saturation for generated colors
     * \see satMax()
     */
    void setSatMax( int val ) { mSatMax = val; }

    /**
     * Sets the minimum value for generated colors
     * \see valMin()
     */
    void setValMin( int val ) { mValMin = val; }

    /**
     * Sets the maximum value for generated colors
     * \see valMax()
     */
    void setValMax( int val ) { mValMax = val; }

  protected:
    int mCount;
    int mHueMin;
    int mHueMax;
    int mSatMin;
    int mSatMax;
    int mValMin;
    int mValMax;
    QList<QColor> mColors;
};

/**
 * \ingroup core
 * \class QgsRandomColorRamp
 * \brief Totally random color ramp. Returns colors generated at random, but constrained
 * to some hardcoded saturation and value ranges to prevent ugly color generation.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsRandomColorRamp: public QgsColorRamp
{
  public:

    /**
     * Constructor for QgsRandomColorRamp.
     */
    QgsRandomColorRamp() = default;

    int count() const override;

    double value( int index ) const override;

    QColor color( double value ) const override;

    /**
     * Sets the desired total number of unique colors for the resultant ramp. Calling
     * this method pregenerates a set of visually distinct colors which are returned
     * by subsequent calls to color().
     * \param colorCount number of unique colors
     * \since QGIS 2.5
     */
    virtual void setTotalColorCount( int colorCount );

    /**
     * Returns the string identifier for QgsRandomColorRamp.
     *
     * \since QGIS 3.16
     */
    static QString typeString() { return QStringLiteral( "randomcolors" ); }

    QString type() const override;

    QgsRandomColorRamp *clone() const override SIP_FACTORY;

    QVariantMap properties() const override;

  protected:

    int mTotalColorCount = 0;
    QList<QColor> mPrecalculatedColors;

};


/**
 * \ingroup core
 * \class QgsPresetSchemeColorRamp
 * \brief A scheme based color ramp consisting of a list of predefined colors.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsPresetSchemeColorRamp : public QgsColorRamp, public QgsColorScheme
{
  public:

    /**
     * Constructor for QgsPresetSchemeColorRamp.
     * \param colors list of colors in ramp
     */
    QgsPresetSchemeColorRamp( const QList< QColor > &colors = QList< QColor >() );

    /**
     * Constructor for QgsPresetColorRamp.
     * \param colors list of named colors in ramp
     */
    QgsPresetSchemeColorRamp( const QgsNamedColorList &colors );

    /**
     * Returns a new QgsPresetSchemeColorRamp color ramp created using the properties encoded in a string
     * map.
     * \param properties color ramp properties
     * \see properties()
     */
    static QgsColorRamp *create( const QVariantMap &properties = QVariantMap() ) SIP_FACTORY;

    /**
     * Sets the list of colors used by the ramp.
     * \param colors list of colors
     * \see colors()
     */
    bool setColors( const QgsNamedColorList &colors, const QString & = QString(), const QColor & = QColor() ) override { mColors = colors; return true; }

    /**
     * Returns the list of colors used by the ramp.
     * \see setColors()
     */
    QList< QColor > colors() const;

    double value( int index ) const override;
    QColor color( double value ) const override;

    /**
     * Returns the string identifier for QgsPresetSchemeColorRamp.
     *
     * \since QGIS 3.16
     */
    static QString typeString() { return QStringLiteral( "preset" ); }

    QString type() const override;
    void invert() override;
    QgsPresetSchemeColorRamp *clone() const override SIP_FACTORY;
    QVariantMap properties() const override;
    int count() const override;

    QString schemeName() const override { return QStringLiteral( "preset" ); }
    QgsNamedColorList fetchColors( const QString &context = QString(), const QColor &baseColor = QColor() ) override;
    bool isEditable() const override { return true; }

  private:

    QgsNamedColorList mColors;
};


#define DEFAULT_COLORBREWER_SCHEMENAME "Spectral"
#define DEFAULT_COLORBREWER_COLORS     5

/**
 * \ingroup core
 * \class QgsColorBrewerColorRamp
 * \brief Color ramp utilising "Color Brewer" preset color schemes.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsColorBrewerColorRamp : public QgsColorRamp
{
  public:

    /**
     * Constructor for QgsColorBrewerColorRamp
     * \param schemeName color brewer scheme name
     * \param colors number of colors in ramp
     * \param inverted invert ramp ordering
     */
    QgsColorBrewerColorRamp( const QString &schemeName = DEFAULT_COLORBREWER_SCHEMENAME,
                             int colors = DEFAULT_COLORBREWER_COLORS,
                             bool inverted = false );

    /**
     * Returns a new QgsColorBrewerColorRamp color ramp created using the properties encoded in a string
     * map.
     * \param properties color ramp properties
     * \see properties()
     */
    static QgsColorRamp *create( const QVariantMap &properties = QVariantMap() ) SIP_FACTORY;

    double value( int index ) const override;
    QColor color( double value ) const override;

    /**
     * Returns the string identifier for QgsColorBrewerColorRamp.
     *
     * \since QGIS 3.16
     */
    static QString typeString() { return QStringLiteral( "colorbrewer" ); }

    QString type() const override { return QgsColorBrewerColorRamp::typeString(); }
    void invert() override;
    QgsColorBrewerColorRamp *clone() const override SIP_FACTORY;
    QVariantMap properties() const override;
    int count() const override { return mColors; }

    /**
     * Returns the name of the color brewer color scheme.
     * \see setSchemeName()
     */
    QString schemeName() const { return mSchemeName; }

    /**
     * Returns the number of colors in the ramp.
     * \see setColors()
     */
    int colors() const { return mColors; }

    /**
     * Sets the name of the color brewer color scheme.
     * \param schemeName scheme name, must match a valid color brewer scheme name
     * \see schemeName()
     * \see listSchemeNames()
     */
    void setSchemeName( const QString &schemeName ) { mSchemeName = schemeName; loadPalette(); }

    /**
     * Sets the number of colors in the ramp.
     * \param colors number of colors. Must match a valid value for the scheme,
     * which can be retrieved using listSchemeVariants()
     * \see colors()
     */
    void setColors( int colors ) { mColors = colors; loadPalette(); }

    /**
     * Returns a list of all valid color brewer scheme names.
     * \see listSchemeVariants()
     */
    static QStringList listSchemeNames();

    /**
     * Returns a list of the valid variants (numbers of colors) for a specified
     * color brewer scheme name
     * \param schemeName color brewer scheme name
     * \see listSchemeNames()
     */
    static QList<int> listSchemeVariants( const QString &schemeName );

  protected:

    //! Generates the scheme using the current name and number of colors
    void loadPalette();

    QString mSchemeName;
    int mColors;
    QList<QColor> mPalette;
    bool mInverted;
};


#define DEFAULT_CPTCITY_SCHEMENAME "cb/div/BrBG_" //change this
#define DEFAULT_CPTCITY_VARIANTNAME "05"

/**
 * \ingroup core
 * \class QgsCptCityColorRamp
 */
class CORE_EXPORT QgsCptCityColorRamp : public QgsGradientColorRamp
{
  public:

    /**
     * Constructor for QgsCptCityColorRamp
     * \param schemeName cpt-city scheme name
     * \param variantName cpt-city variant name
     * \param inverted invert ramp ordering
     * \param doLoadFile load cpt-city ramp from file
     */
    QgsCptCityColorRamp( const QString &schemeName = DEFAULT_CPTCITY_SCHEMENAME,
                         const QString &variantName = DEFAULT_CPTCITY_VARIANTNAME,
                         bool inverted = false,
                         bool doLoadFile = true );

    /**
     * Constructor for QgsCptCityColorRamp
     * \param schemeName cpt-city scheme name
     * \param variantList cpt-city variant list
     * \param variantName cpt-city variant name
     * \param inverted invert ramp ordering
     * \param doLoadFile load cpt-city ramp from file
     */
    QgsCptCityColorRamp( const QString &schemeName, const QStringList &variantList,
                         const QString &variantName = QString(), bool inverted = false,
                         bool doLoadFile = true );

    //! Creates the symbol layer
    static QgsColorRamp *create( const QVariantMap &properties = QVariantMap() ) SIP_FACTORY;

    /**
     * Returns the string identifier for QgsCptCityColorRamp.
     *
     * \since QGIS 3.16
     */
    static QString typeString() { return QStringLiteral( "cpt-city" ); }

    QString type() const override;

    void invert() override;

    QgsCptCityColorRamp *clone() const override SIP_FACTORY;
    void copy( const QgsCptCityColorRamp *other );
    QgsGradientColorRamp *cloneGradientRamp() const SIP_FACTORY;

    QVariantMap properties() const override;

    QString schemeName() const { return mSchemeName; }
    QString variantName() const { return mVariantName; }
    QStringList variantList() const { return mVariantList; }

    // lazy loading - have to call loadPalette() explicitly
    void setSchemeName( const QString &schemeName ) { mSchemeName = schemeName; mFileLoaded = false; }
    void setVariantName( const QString &variantName ) { mVariantName = variantName; mFileLoaded = false; }
    void setVariantList( const QStringList &variantList ) { mVariantList = variantList; }
    void setName( const QString &schemeName, const QString &variantName = QString(), const QStringList &variantList = QStringList() )
    { mSchemeName = schemeName; mVariantName = variantName; mVariantList = variantList; mFileLoaded = false; }

    void loadPalette() { loadFile(); }
    bool hasMultiStops() const { return mMultiStops; }

    QString fileName() const;
    bool loadFile();
    bool fileLoaded() const { return mFileLoaded; }

    QString copyingFileName() const;
    QString descFileName() const;
    QgsStringMap copyingInfo() const;

  protected:
    QString mSchemeName;
    QString mVariantName;
    QStringList mVariantList;
    bool mFileLoaded = false;
    bool mMultiStops = false;
    bool mInverted;

};

// clazy:excludeall=qstring-allocations

#endif
