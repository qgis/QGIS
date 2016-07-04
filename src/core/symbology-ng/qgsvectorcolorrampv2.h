/***************************************************************************
    qgsvectorcolorrampv2.h
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

#ifndef QGSVECTORCOLORRAMPV2_H
#define QGSVECTORCOLORRAMPV2_H

#include <QColor>
#include <QGradient>

#include "qgssymbollayerv2.h" // for QgsStringMap
#include "qgslogger.h"

/** \ingroup core
 * \class QgsVectorColorRampV2
 * \brief Abstract base class for color ramps
 */
//TODO QGIS 3.0 - rename to QgsColorRamp, since this is used by much more than just vectors
class CORE_EXPORT QgsVectorColorRampV2
{
  public:

    virtual ~QgsVectorColorRampV2() {}

    /** Returns number of defined colors, or -1 if undefined
     */
    virtual int count() const = 0;

    /** Returns relative value between [0,1] of color at specified index
     */
    virtual double value( int index ) const = 0;

    /** Returns the color corresponding to a specified value.
     * @param value value between [0, 1] inclusive
     * @returns color for value
     */
    virtual QColor color( double value ) const = 0;

    /** Returns a string representing the color ramp type.
     */
    virtual QString type() const = 0;

    /** Creates a clone of the color ramp.
     */
    virtual QgsVectorColorRampV2* clone() const = 0;

    /** Returns a string map containing all the color ramp's properties.
     */
    virtual QgsStringMap properties() const = 0;
};

/** \ingroup core
 * \class QgsGradientStop
 * \brief Represents a color stop within a gradient color ramp.
 */
class CORE_EXPORT QgsGradientStop
{
  public:

    /** Constructor for QgsGradientStop
     * @param o positional offset for stop, between 0 and 1.0
     * @param c color for stop
     */
    QgsGradientStop( double o, const QColor& c )
        : offset( o )
        , color( c )
    { }

    //! Relative positional offset, between 0 and 1
    double offset;
    //! Gradient color at stop
    QColor color;

    bool operator==( const QgsGradientStop& other ) const
    {
      return other.color == color && qgsDoubleNear( other.offset, offset );
    }
};

//! List of gradient stops
typedef QList<QgsGradientStop> QgsGradientStopsList;

#define DEFAULT_GRADIENT_COLOR1 QColor(0,0,255)
#define DEFAULT_GRADIENT_COLOR2 QColor(0,255,0)

/** \ingroup core
 * \class QgsVectorGradientColorRampV2
 * \brief Gradient color ramp, which smoothly interpolates between two colors and also
 * supports optional extra color stops.
 */
//TODO QGIS 3.0 - rename to QgsGradientColorRamp, since this is used by much more than just vectors
class CORE_EXPORT QgsVectorGradientColorRampV2 : public QgsVectorColorRampV2
{
  public:

    /** Constructor for QgsVectorGradientColorRampV2
     * @param color1 start color, corresponding to a position of 0.0
     * @param color2 end color, corresponding to a position of 1.0
     * @param discrete set to true for discrete interpolation instead of smoothly
     * interpolating between colors
     * @param stops optional list of additional color stops
     */
    QgsVectorGradientColorRampV2( const QColor& color1 = DEFAULT_GRADIENT_COLOR1,
                                  const QColor& color2 = DEFAULT_GRADIENT_COLOR2,
                                  bool discrete = false,
                                  const QgsGradientStopsList& stops = QgsGradientStopsList() );

    //! Creates a new QgsVectorColorRampV2 from a map of properties
    static QgsVectorColorRampV2* create( const QgsStringMap& properties = QgsStringMap() );

    virtual int count() const override { return mStops.count() + 2; }
    virtual double value( int index ) const override;
    virtual QColor color( double value ) const override;
    virtual QString type() const override { return "gradient"; }
    virtual QgsVectorGradientColorRampV2* clone() const override;
    virtual QgsStringMap properties() const override;

    /** Returns the gradient start color.
     * @see setColor1()
     * @see color2()
     */
    QColor color1() const { return mColor1; }

    /** Returns the gradient end color.
     * @see setColor2()
     * @see color1()
     */
    QColor color2() const { return mColor2; }

    /** Sets the gradient start color.
     * @param color start color
     * @see color1()
     * @see setColor2()
     */
    void setColor1( const QColor& color ) { mColor1 = color; }

    /** Sets the gradient end color.
     * @param color end color
     * @see color2()
     * @see setColor1()
     */
    void setColor2( const QColor& color ) { mColor2 = color; }

    /** Returns true if the gradient is using discrete interpolation, rather than
     * smoothly interpolating between colors.
     * @see setDiscrete()
     */
    bool isDiscrete() const { return mDiscrete; }

    /** Sets whether the gradient should use discrete interpolation, rather than
     * smoothly interpolating between colors.
     * @param discrete set to true to use discrete interpolation
     * @see convertToDiscrete()
     * @see isDiscrete()
     */
    void setDiscrete( bool discrete ) { mDiscrete = discrete; }

    /** Converts a gradient with existing color stops to or from discrete
     * interpolation.
     * @param discrete set to true to convert the gradient stops to discrete,
     * or false to convert them to smooth interpolation
     * @see isDiscrete()
     */
    void convertToDiscrete( bool discrete );

    /** Sets the list of intermediate gradient stops for the ramp.
     * @param stops list of stops. Any existing color stops will be replaced. The stop
     * list will be automatically reordered so that stops are listed in ascending offset
     * order.
     * @see stops()
     */
    void setStops( const QgsGradientStopsList& stops );

    /** Returns the list of intermediate gradient stops for the ramp.
     * @see setStops()
     */
    QgsGradientStopsList stops() const { return mStops; }

    /** Returns any additional info attached to the gradient ramp (eg authorship notes)
     * @see setInfo()
     */
    QgsStringMap info() const { return mInfo; }

    /** Sets additional info to attach to the gradient ramp (eg authorship notes)
     * @param info map of string info to attach
     * @see info()
     */
    void setInfo( const QgsStringMap& info ) { mInfo = info; }

    /** Copy color ramp stops to a QGradient
     * @param gradient gradient to copy stops into
     * @param alpha alpha multiplier. Opacity of colors will be multiplied
     * by this factor before adding to the gradient.
     * @note added in 2.1
     */
    void addStopsToGradient( QGradient* gradient, double alpha = 1 );

  protected:
    QColor mColor1;
    QColor mColor2;
    bool mDiscrete;
    QgsGradientStopsList mStops;
    QgsStringMap mInfo;
};

#define DEFAULT_RANDOM_COUNT   10
#define DEFAULT_RANDOM_HUE_MIN 0
#define DEFAULT_RANDOM_HUE_MAX 359
#define DEFAULT_RANDOM_VAL_MIN 200
#define DEFAULT_RANDOM_VAL_MAX 240
#define DEFAULT_RANDOM_SAT_MIN 100
#define DEFAULT_RANDOM_SAT_MAX 240

/** \ingroup core
 * \class QgsVectorRandomColorRampV2
 * \brief Random color ramp, which returns random colors based on preset parameters.
 */
//TODO QGIS 3.0 - rename to QgsRandomColorRamp, since this is used by much more than just vectors
class CORE_EXPORT QgsVectorRandomColorRampV2 : public QgsVectorColorRampV2
{
  public:
    QgsVectorRandomColorRampV2( int count = DEFAULT_RANDOM_COUNT,
                                int hueMin = DEFAULT_RANDOM_HUE_MIN, int hueMax = DEFAULT_RANDOM_HUE_MAX,
                                int satMin = DEFAULT_RANDOM_SAT_MIN, int satMax = DEFAULT_RANDOM_SAT_MAX,
                                int valMin = DEFAULT_RANDOM_VAL_MIN, int valMax = DEFAULT_RANDOM_VAL_MAX );

    static QgsVectorColorRampV2* create( const QgsStringMap& properties = QgsStringMap() );

    virtual double value( int index ) const override;

    virtual QColor color( double value ) const override;

    virtual QString type() const override { return "random"; }

    virtual QgsVectorRandomColorRampV2* clone() const override;

    virtual QgsStringMap properties() const override;

    /** Get a list of random colors
     * @note added in 2.4
     */
    static QList<QColor> randomColors( int count,
                                       int hueMax = DEFAULT_RANDOM_HUE_MAX, int hueMin = DEFAULT_RANDOM_HUE_MIN,
                                       int satMax = DEFAULT_RANDOM_SAT_MAX, int satMin = DEFAULT_RANDOM_SAT_MIN,
                                       int valMax = DEFAULT_RANDOM_VAL_MAX, int valMin = DEFAULT_RANDOM_VAL_MIN );

    void updateColors();

    int count() const override { return mCount; }
    int hueMin() const { return mHueMin; }
    int hueMax() const { return mHueMax; }
    int satMin() const { return mSatMin; }
    int satMax() const { return mSatMax; }
    int valMin() const { return mValMin; }
    int valMax() const { return mValMax; }

    void setCount( int val ) { mCount = val; }
    void setHueMin( int val ) { mHueMin = val; }
    void setHueMax( int val ) { mHueMax = val; }
    void setSatMin( int val ) { mSatMin = val; }
    void setSatMax( int val ) { mSatMax = val; }
    void setValMin( int val ) { mValMin = val; }
    void setValMax( int val ) { mValMax = val; }

  protected:
    int mCount;
    int mHueMin, mHueMax, mSatMin, mSatMax, mValMin, mValMax;
    QList<QColor> mColors;
};

/** \ingroup core
 * \class QgsRandomColorsV2
 */
class CORE_EXPORT QgsRandomColorsV2: public QgsVectorColorRampV2
{
  public:
    QgsRandomColorsV2();
    ~QgsRandomColorsV2();

    int count() const override;

    double value( int index ) const override;

    QColor color( double value ) const override;

    /** Sets the desired total number of unique colors for the resultant ramp. Calling
     * this method pregenerates a set of visually distinct colors which are returned
     * by subsequent calls to color().
     * @param colorCount number of unique colors
     * @note added in QGIS 2.5
     */
    virtual void setTotalColorCount( const int colorCount );

    QString type() const override;

    QgsRandomColorsV2* clone() const override;

    QgsStringMap properties() const override;

  protected:

    int mTotalColorCount;
    QList<QColor> mPrecalculatedColors;

};


#define DEFAULT_COLORBREWER_SCHEMENAME "Spectral"
#define DEFAULT_COLORBREWER_COLORS     5

/** \ingroup core
 * \class QgsVectorColorBrewerColorRampV2
 */
class CORE_EXPORT QgsVectorColorBrewerColorRampV2 : public QgsVectorColorRampV2
{
  public:
    QgsVectorColorBrewerColorRampV2( const QString& schemeName = DEFAULT_COLORBREWER_SCHEMENAME,
                                     int colors = DEFAULT_COLORBREWER_COLORS );

    static QgsVectorColorRampV2* create( const QgsStringMap& properties = QgsStringMap() );

    virtual double value( int index ) const override;

    virtual QColor color( double value ) const override;

    virtual QString type() const override { return "colorbrewer"; }

    virtual QgsVectorColorBrewerColorRampV2* clone() const override;

    virtual QgsStringMap properties() const override;

    QString schemeName() const { return mSchemeName; }
    virtual int count() const override { return mColors; }
    int colors() const { return mColors; }

    void setSchemeName( const QString& schemeName ) { mSchemeName = schemeName; loadPalette(); }
    void setColors( int colors ) { mColors = colors; loadPalette(); }

    static QStringList listSchemeNames();
    static QList<int> listSchemeVariants( const QString& schemeName );

  protected:

    void loadPalette();

    QString mSchemeName;
    int mColors;
    QList<QColor> mPalette;
};


#define DEFAULT_CPTCITY_SCHEMENAME "cb/div/BrBG_" //change this
#define DEFAULT_CPTCITY_VARIANTNAME "05"

/** \ingroup core
 * \class QgsCptCityColorRampV2
 */
class CORE_EXPORT QgsCptCityColorRampV2 : public QgsVectorGradientColorRampV2
{
  public:
    QgsCptCityColorRampV2( const QString& schemeName = DEFAULT_CPTCITY_SCHEMENAME,
                           const QString& variantName = DEFAULT_CPTCITY_VARIANTNAME,
                           bool doLoadFile = true );
    QgsCptCityColorRampV2( const QString& schemeName, const QStringList& variantList,
                           const QString& variantName = QString(), bool doLoadFile = true );

    static QgsVectorColorRampV2* create( const QgsStringMap& properties = QgsStringMap() );

    virtual QString type() const override { return "cpt-city"; }

    virtual QgsCptCityColorRampV2* clone() const override;
    void copy( const QgsCptCityColorRampV2* other );
    QgsVectorGradientColorRampV2* cloneGradientRamp() const;

    virtual QgsStringMap properties() const override;

    QString schemeName() const { return mSchemeName; }
    QString variantName() const { return mVariantName; }
    QStringList variantList() const { return mVariantList; }

    /* lazy loading - have to call loadPalette() explicitly */
    void setSchemeName( const QString& schemeName ) { mSchemeName = schemeName; mFileLoaded = false; }
    void setVariantName( const QString& variantName ) { mVariantName = variantName; mFileLoaded = false; }
    void setVariantList( const QStringList& variantList ) { mVariantList = variantList; }
    void setName( const QString& schemeName, const QString& variantName = "", const QStringList& variantList = QStringList() )
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
    bool mFileLoaded;
    bool mMultiStops;
};


#endif
