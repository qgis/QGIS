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

class CORE_EXPORT QgsVectorColorRampV2
{
  public:

    virtual ~QgsVectorColorRampV2() {}

    // Number of defined colors
    virtual int count() const = 0;

    // Relative value (0,1) of color at index
    virtual double value( int index ) const = 0;

    virtual QColor color( double value ) const = 0;

    virtual QString type() const = 0;

    virtual QgsVectorColorRampV2* clone() const = 0;

    virtual QgsStringMap properties() const = 0;

};

struct QgsGradientStop
{
  double offset; // relative (0,1)
  QColor color;
  QgsGradientStop( double o, const QColor& c ) : offset( o ), color( c ) { }
};

typedef QList<QgsGradientStop> QgsGradientStopsList;

#define DEFAULT_GRADIENT_COLOR1 QColor(0,0,255)
#define DEFAULT_GRADIENT_COLOR2 QColor(0,255,0)

class CORE_EXPORT QgsVectorGradientColorRampV2 : public QgsVectorColorRampV2
{
  public:
    QgsVectorGradientColorRampV2( QColor color1 = DEFAULT_GRADIENT_COLOR1,
                                  QColor color2 = DEFAULT_GRADIENT_COLOR2,
                                  bool discrete = false,
                                  QgsGradientStopsList stops = QgsGradientStopsList() );

    static QgsVectorColorRampV2* create( const QgsStringMap& properties = QgsStringMap() );

    virtual int count() const override { return mStops.count() + 2; }

    virtual double value( int index ) const override;

    virtual QColor color( double value ) const override;

    virtual QString type() const override { return "gradient"; }

    virtual QgsVectorColorRampV2* clone() const override;

    virtual QgsStringMap properties() const override;

    QColor color1() const { return mColor1; }
    QColor color2() const { return mColor2; }
    void setColor1( QColor color ) { mColor1 = color; }
    void setColor2( QColor color ) { mColor2 = color; }

    bool isDiscrete() const { return mDiscrete; }
    void setDiscrete( bool discrete ) { mDiscrete = discrete; }
    void convertToDiscrete( bool discrete );

    void setStops( const QgsGradientStopsList& stops ) { mStops = stops; }
    const QgsGradientStopsList& stops() const { return mStops; }

    QgsStringMap info() const { return mInfo; }
    void setInfo( const QgsStringMap& info ) { mInfo = info; }

    /**copy color ramp stops to a QGradient
    * @note added in 2.1 */
    void addStopsToGradient( QGradient* gradient, double alpha = 1 );

  protected:
    QColor mColor1, mColor2;
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

    virtual QgsVectorColorRampV2* clone() const override;

    virtual QgsStringMap properties() const override;

    /** get a list of random colors
    * @note added in 2.4 */
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

class CORE_EXPORT QgsRandomColorsV2: public QgsVectorColorRampV2
{
  public:
    QgsRandomColorsV2();
    ~QgsRandomColorsV2();

    int count() const override;

    double value( int index ) const override;

    QColor color( double value ) const override;

    /*Sets the desired total number of unique colors for the resultant ramp. Calling
     * this method pregenerates a set of visually distinct colors which are returned
     * by subsequent calls to color().
     * @param colorCount number of unique colors
     * @note added in QGIS 2.5
     */
    virtual void setTotalColorCount( const int colorCount );

    QString type() const override;

    QgsVectorColorRampV2* clone() const override;

    QgsStringMap properties() const override;

  protected:

    int mTotalColorCount;
    QList<QColor> mPrecalculatedColors;

};


#define DEFAULT_COLORBREWER_SCHEMENAME "Spectral"
#define DEFAULT_COLORBREWER_COLORS     5

class CORE_EXPORT QgsVectorColorBrewerColorRampV2 : public QgsVectorColorRampV2
{
  public:
    QgsVectorColorBrewerColorRampV2( QString schemeName = DEFAULT_COLORBREWER_SCHEMENAME,
                                     int colors = DEFAULT_COLORBREWER_COLORS );

    static QgsVectorColorRampV2* create( const QgsStringMap& properties = QgsStringMap() );

    virtual double value( int index ) const override;

    virtual QColor color( double value ) const override;

    virtual QString type() const override { return "colorbrewer"; }

    virtual QgsVectorColorRampV2* clone() const override;

    virtual QgsStringMap properties() const override;

    QString schemeName() const { return mSchemeName; }
    virtual int count() const override { return mColors; }
    int colors() const { return mColors; }

    void setSchemeName( QString schemeName ) { mSchemeName = schemeName; loadPalette(); }
    void setColors( int colors ) { mColors = colors; loadPalette(); }

    static QStringList listSchemeNames();
    static QList<int> listSchemeVariants( QString schemeName );

  protected:

    void loadPalette();

    QString mSchemeName;
    int mColors;
    QList<QColor> mPalette;
};


#define DEFAULT_CPTCITY_SCHEMENAME "cb/div/BrBG_" //change this
#define DEFAULT_CPTCITY_VARIANTNAME "05"

class CORE_EXPORT QgsCptCityColorRampV2 : public QgsVectorGradientColorRampV2
{
  public:
    QgsCptCityColorRampV2( QString schemeName = DEFAULT_CPTCITY_SCHEMENAME,
                           QString variantName = DEFAULT_CPTCITY_VARIANTNAME,
                           bool doLoadFile = true );
    QgsCptCityColorRampV2( QString schemeName, QStringList variantList,
                           QString variantName = QString(), bool doLoadFile = true );

    static QgsVectorColorRampV2* create( const QgsStringMap& properties = QgsStringMap() );

    virtual QString type() const override { return "cpt-city"; }

    virtual QgsVectorColorRampV2* clone() const override;
    void copy( const QgsCptCityColorRampV2* other );
    QgsVectorGradientColorRampV2* cloneGradientRamp() const;

    virtual QgsStringMap properties() const override;

    QString schemeName() const { return mSchemeName; }
    QString variantName() const { return mVariantName; }
    QStringList variantList() const { return mVariantList; }

    /* lazy loading - have to call loadPalette() explicitly */
    void setSchemeName( QString schemeName ) { mSchemeName = schemeName; mFileLoaded = false; }
    void setVariantName( QString variantName ) { mVariantName = variantName; mFileLoaded = false; }
    void setVariantList( QStringList variantList ) { mVariantList = variantList; }
    void setName( QString schemeName, QString variantName = "", QStringList variantList = QStringList() )
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
