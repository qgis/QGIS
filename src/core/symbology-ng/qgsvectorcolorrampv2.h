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
#include "qgslogger.h"

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

#define DEFAULT_CPTCITY_COLLECTION "default"

class CORE_EXPORT QgsCptCityCollection
{
  public:
    QgsCptCityCollection( QString collectionName = DEFAULT_CPTCITY_COLLECTION,
                          QString baseDir = QString() );
    ~QgsCptCityCollection();

    QString baseDir() const;
    static QString baseDir( QString collectionName );
    static QString defaultBaseDir();
    void setBaseDir( QString dirName ) { mBaseDir = dirName; }
    bool loadSchemes( QString rootDir = "", bool reset = false );
    /** Is the minimal (free to distribute) set of schemes available?
     * Currently returns hasAllSchemes, because we don't have a minimal set yet. */
    bool hasBasicSchemes();
    /** Is the entire archive available? Currently tests that there is at least one scheme. */
    bool hasAllSchemes();
    QStringList listSchemeCollections( QString collectionName = "", bool recursive = false );
    QStringList listSchemeNames( QString collectionName );
    QgsCptCityCollection* colorRampFromSVGFile( QString svgFile );
    QgsCptCityCollection* colorRampFromSVGString( QString svgString );

    QString copyingFileName( const QString& dirName ) const;
    QString descFileName( const QString& dirName ) const;
    static QMap< QString, QString > copyingInfo( const QString& copyingFileName );

    QString collectionName() const { return mCollectionName; }
    QMap< QString, QStringList > schemeMap() const { return mSchemeMap; }
    QMap< QString, QStringList > schemeVariants() const { return mSchemeVariants; }
    QMap< QString, QString > collectionNames() const { return mCollectionNames; }
    QMap< QString, QStringList > collectionSelections() const { return mCollectionSelections; }

    static void initCollection( QString collectionName = DEFAULT_CPTCITY_COLLECTION,
                                QString collectionBaseDir = baseDir( DEFAULT_CPTCITY_COLLECTION ) );
    static void initCollections( ) { initCollection(); }
    static void initCollections( QMap< QString, QString > collectionsDefs );
    static QgsCptCityCollection* defaultCollection();
    static QMap< QString, QgsCptCityCollection* > collectionRegistry();

  protected:

    QString mCollectionName;
    QString mBaseDir;
    QStringList mCollections;
    QMap< QString, QStringList > mSchemeMap; //key is collection, value is schemes
    QMap< QString, QStringList > mSchemeVariants; //key is scheme, value is variants
    QMap< QString, QString > mCollectionNames; //key is name, value is description
    QMap< QString, QStringList > mCollectionSelections;
    static QgsCptCityCollection* mDefaultCollection;
    static QMap< QString, QgsCptCityCollection* > mCollectionRegistry;
    static QMap< QString, QMap< QString, QString > > mCopyingInfoMap; // mapping of copyinginfo, key is fileName
};


#define DEFAULT_CPTCITY_SCHEMENAME "cb/div/BrBG_" //change this
#define DEFAULT_CPTCITY_VARIANTNAME "05"

class CORE_EXPORT QgsCptCityColorRampV2 : public QgsVectorColorRampV2
{
  public:
    QgsCptCityColorRampV2( QString schemeName = DEFAULT_CPTCITY_SCHEMENAME,
                           QString variantName = DEFAULT_CPTCITY_VARIANTNAME,
                           QString collectionName = DEFAULT_CPTCITY_COLLECTION );


    enum GradientType
    {
      Discrete, //discrete stops, e.g. Color Brewer
      Continuous, //continuous, e.g. QgsVectorColorRampV2
      ContinuousMulti //continuous with 2 values in intermediate stops
    };
    typedef QList< QPair < double, QColor > > GradientList;


    static QgsVectorColorRampV2* create( const QgsStringMap& properties = QgsStringMap() );

    virtual QColor color( double value ) const;

    virtual QString type() const { return "cpt-city"; }

    virtual QgsVectorColorRampV2* clone() const;

    virtual QgsStringMap properties() const;

    int count() const { return mPalette.count(); }

    QString schemeName() const { return mSchemeName; }
    QString variantName() const { return mVariantName; }
    QStringList variantList() const { return mVariantList; }
    /* QgsCptCityCollection* collection() const { return mCollection; } */
    QString collectionName() const { return mCollectionName; }
    QgsCptCityCollection* collection() const
    { return QgsCptCityCollection::collectionRegistry().value( mCollectionName ); }

    /* lazy loading - have to call loadPalette() explicitly */
    void setSchemeName( QString schemeName ) { mSchemeName = schemeName; mFileLoaded = false; }
    void setVariantName( QString variantName ) { mVariantName = variantName; mFileLoaded = false; }
    void setName( QString schemeName, QString variantName = "" )
    { mSchemeName = schemeName; mVariantName = variantName; mFileLoaded = false; }

    void loadPalette() { loadFile(); }
    /* bool isContinuous() const { return mContinuous; } */
    GradientType gradientType() const { return mGradientType; }

    QString fileName() const;
    bool loadFile();

    QString copyingFileName() const;
    QString descFileName() const;
    QMap< QString, QString > copyingInfo() const;

  protected:

    QString mSchemeName;
    QString mVariantName;
    QString mCollectionName;
    /* QgsCptCityCollection* mCollection; */
    GradientType mGradientType;
    GradientList mPalette;
    QStringList mVariantList;
    bool mFileLoaded;
};


#endif
