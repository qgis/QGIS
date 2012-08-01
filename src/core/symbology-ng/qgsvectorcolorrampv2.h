/***************************************************************************
    qgsvectorcolorrampv2.h
    ---------------------
    begin                : November 2009
    copyright            : (C) 2009 by Martin Dobias
    email                : wonder.sk at gmail.com
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

#include "qgssymbollayerv2.h" // for QgsStringMap

class CORE_EXPORT QgsVectorColorRampV2
{
  public:
    virtual ~QgsVectorColorRampV2() {}

    virtual QColor color( double value ) const = 0;

    virtual QString type() const = 0;

    virtual QgsVectorColorRampV2* clone() const = 0;

    virtual QgsStringMap properties() const = 0;

};

#define DEFAULT_GRADIENT_COLOR1 QColor(0,0,255)
#define DEFAULT_GRADIENT_COLOR2 QColor(0,255,0)

class CORE_EXPORT QgsVectorGradientColorRampV2 : public QgsVectorColorRampV2
{
  public:
    QgsVectorGradientColorRampV2( QColor color1 = DEFAULT_GRADIENT_COLOR1,
                                  QColor color2 = DEFAULT_GRADIENT_COLOR2 );

    static QgsVectorColorRampV2* create( const QgsStringMap& properties = QgsStringMap() );

    virtual QColor color( double value ) const;

    virtual QString type() const { return "gradient"; }

    virtual QgsVectorColorRampV2* clone() const;

    virtual QgsStringMap properties() const;

    QColor color1() const { return mColor1; }
    QColor color2() const { return mColor2; }

    void setColor1( QColor color ) { mColor1 = color; }
    void setColor2( QColor color ) { mColor2 = color; }

    typedef QMap<double, QColor> StopsMap;

    void setStops( const StopsMap& stops ) { mStops = stops; }
    const StopsMap& stops() const { return mStops; }

  protected:
    QColor mColor1, mColor2;
    StopsMap mStops;
};

#define DEFAULT_RANDOM_COUNT   10
#define DEFAULT_RANDOM_HUE_MIN 0
#define DEFAULT_RANDOM_HUE_MAX 359
#define DEFAULT_RANDOM_VAL_MIN 0
#define DEFAULT_RANDOM_VAL_MAX 255
#define DEFAULT_RANDOM_SAT_MIN 0
#define DEFAULT_RANDOM_SAT_MAX 255

class CORE_EXPORT QgsVectorRandomColorRampV2 : public QgsVectorColorRampV2
{
  public:
    QgsVectorRandomColorRampV2( int count = DEFAULT_RANDOM_COUNT,
                                int hueMin = DEFAULT_RANDOM_HUE_MIN, int hueMax = DEFAULT_RANDOM_HUE_MAX,
                                int satMin = DEFAULT_RANDOM_SAT_MIN, int satMax = DEFAULT_RANDOM_SAT_MAX,
                                int valMin = DEFAULT_RANDOM_VAL_MIN, int valMax = DEFAULT_RANDOM_VAL_MAX );

    static QgsVectorColorRampV2* create( const QgsStringMap& properties = QgsStringMap() );

    virtual QColor color( double value ) const;

    virtual QString type() const { return "random"; }

    virtual QgsVectorColorRampV2* clone() const;

    virtual QgsStringMap properties() const;

    void updateColors();

    int count() const { return mCount; }
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


#define DEFAULT_COLORBREWER_SCHEMENAME "Spectral"
#define DEFAULT_COLORBREWER_COLORS     5

class CORE_EXPORT QgsVectorColorBrewerColorRampV2 : public QgsVectorColorRampV2
{
  public:
    QgsVectorColorBrewerColorRampV2( QString schemeName = DEFAULT_COLORBREWER_SCHEMENAME,
                                     int colors = DEFAULT_COLORBREWER_COLORS );

    static QgsVectorColorRampV2* create( const QgsStringMap& properties = QgsStringMap() );

    virtual QColor color( double value ) const;

    virtual QString type() const { return "colorbrewer"; }

    virtual QgsVectorColorRampV2* clone() const;

    virtual QgsStringMap properties() const;

    QString schemeName() const { return mSchemeName; }
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

class CORE_EXPORT QgsCptCityColorRampV2 : public QgsVectorColorRampV2
{
  public:
    QgsCptCityColorRampV2( QString schemeName = DEFAULT_CPTCITY_SCHEMENAME,
                           QString variantName = DEFAULT_CPTCITY_VARIANTNAME );

    static QgsVectorColorRampV2* create( const QgsStringMap& properties = QgsStringMap() );

    virtual QColor color( double value ) const;

    virtual QString type() const { return "cpt-city"; }

    virtual QgsVectorColorRampV2* clone() const;

    virtual QgsStringMap properties() const;

    int count() const { return mPalette.count(); }

    QString schemeName() const { return mSchemeName; }
    QString variantName() const { return mVariantName; }

    /* void setSchemeName( QString schemeName ) { mSchemeName = schemeName; loadPalette(); } */
    /* void setVariantName( QString variantName ) { mVariantName = variantName; loadPalette(); } */
    /* lazy loading - have to call loadPalette() explicitly */
    void setSchemeName( QString schemeName ) { mSchemeName = schemeName; }
    void setVariantName( QString variantName ) { mVariantName = variantName; }
    void setName( QString schemeName, QString variantName = "" )
    { mSchemeName = schemeName; mVariantName = variantName; loadPalette(); }

    void loadPalette();
    bool isContinuous() const { return mContinuous; }

    QString getFilename() const;
    bool loadFile( QString filename = "" );

    /* static QList<QColor> listSchemeColors(  QString schemeName, int colors ); */
    static QList<int> listSchemeVariants( QString schemeName );

    static QString getSchemeBaseDir();
    static void loadSchemes( QString rootDir = "", bool reset = false );
    static QStringList listSchemeCollections( QString collectionName = "", bool recursive = false );
    static QStringList listSchemeNames( QString collectionName );
    static QgsCptCityColorRampV2* colorRampFromSVGFile( QString svgFile );
    static QgsCptCityColorRampV2* colorRampFromSVGString( QString svgString );

    static const QMap< QString, QStringList > schemeMap() { return mSchemeMap; }
    /* static const QMap< QString, int > schemeNumColors() { return mSchemeNumColors; } */
    static const QMap< QString, QStringList > schemeVariants() { return mSchemeVariants; }
    static const QMap< QString, QString > collectionNames() { return mCollectionNames; }
    static const QMap< QString, QStringList > collectionSelections() { return mCollectionSelections; }

  protected:

    typedef QMap<double, QColor> StopsMap;

    QString mSchemeName;
    /* int mColors; //remove! */
    QString mVariantName;
    bool mContinuous;
    QList< QColor > mPalette;
    QList< double > mPaletteStops;
    /* QMap< double, QColor > mPalette; */

    static QStringList mCollections;
    static QMap< QString, QStringList > mSchemeMap; //key is collection, value is schemes
    /* mSchemeNumColors removed, instead read on demand */
    /* static QMap< QString, int > mSchemeNumColors; //key is scheme, value is # colors (if no variants) */
    static QMap< QString, QStringList > mSchemeVariants; //key is scheme, value is variants
    static QMap< QString, QString > mCollectionNames; //key is name, value is description
    static QMap< QString, QStringList > mCollectionSelections;

};

#endif
