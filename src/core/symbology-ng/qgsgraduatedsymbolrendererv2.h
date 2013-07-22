/***************************************************************************
    qgsgraduatedsymbolrendererv2.h
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
#ifndef QGSGRADUATEDSYMBOLRENDERERV2_H
#define QGSGRADUATEDSYMBOLRENDERERV2_H

#include "qgssymbolv2.h"
#include "qgsrendererv2.h"

class CORE_EXPORT QgsRendererRangeV2
{
  public:
    QgsRendererRangeV2();
    QgsRendererRangeV2( double lowerValue, double upperValue, QgsSymbolV2* symbol, QString label );
    QgsRendererRangeV2( const QgsRendererRangeV2& range );

    ~QgsRendererRangeV2();

    QgsRendererRangeV2& operator=( const QgsRendererRangeV2& range );

    double lowerValue() const;
    double upperValue() const;

    QgsSymbolV2* symbol() const;
    QString label() const;

    void setSymbol( QgsSymbolV2* s );
    void setLabel( QString label );
    void setLowerValue( double lowerValue );
    void setUpperValue( double upperValue );

    // debugging
    QString dump() const;

    void toSld( QDomDocument& doc, QDomElement &element, QgsStringMap props ) const;

  protected:
    double mLowerValue, mUpperValue;
    QgsSymbolV2* mSymbol;
    QString mLabel;
};

typedef QList<QgsRendererRangeV2> QgsRangeList;

class QgsVectorLayer;
class QgsVectorColorRampV2;

class CORE_EXPORT QgsGraduatedSymbolRendererV2 : public QgsFeatureRendererV2
{
  public:
    QgsGraduatedSymbolRendererV2( QString attrName = QString(), QgsRangeList ranges = QgsRangeList() );

    virtual ~QgsGraduatedSymbolRendererV2();

    virtual QgsSymbolV2* symbolForFeature( QgsFeature& feature );

    virtual void startRender( QgsRenderContext& context, const QgsVectorLayer *vlayer );

    virtual void stopRender( QgsRenderContext& context );

    virtual QList<QString> usedAttributes();

    virtual QString dump() const;

    virtual QgsFeatureRendererV2* clone();

    virtual void toSld( QDomDocument& doc, QDomElement &element ) const;

    //! returns bitwise OR-ed capabilities of the renderer
    //! \note added in 2.0
    virtual int capabilities() { return SymbolLevels | RotationField | Filter; }

    virtual QgsSymbolV2List symbols();

    QString classAttribute() const { return mAttrName; }
    void setClassAttribute( QString attr ) { mAttrName = attr; }

    const QgsRangeList& ranges() { return mRanges; }

    bool updateRangeSymbol( int rangeIndex, QgsSymbolV2* symbol );
    bool updateRangeLabel( int rangeIndex, QString label );
    bool updateRangeUpperValue( int rangeIndex, double value );
    bool updateRangeLowerValue( int rangeIndex, double value );

    void addClass( QgsSymbolV2* symbol );
    void deleteClass( int idx );
    void deleteAllClasses();

    //! Moves the category at index position from to index position to.
    void moveClass( int from, int to );

    void sortByValue( Qt::SortOrder order = Qt::AscendingOrder );
    void sortByLabel( Qt::SortOrder order = Qt::AscendingOrder );

    enum Mode
    {
      EqualInterval,
      Quantile,
      Jenks,
      StdDev,
      Pretty,
      Custom
    };

    Mode mode() const { return mMode; }
    void setMode( Mode mode ) { mMode = mode; }

    static QgsGraduatedSymbolRendererV2* createRenderer(
      QgsVectorLayer* vlayer,
      QString attrName,
      int classes,
      Mode mode,
      QgsSymbolV2* symbol,
      QgsVectorColorRampV2* ramp );

    //! create renderer from XML element
    static QgsFeatureRendererV2* create( QDomElement& element );

    //! store renderer info to XML element
    virtual QDomElement save( QDomDocument& doc );

    //! return a list of symbology items for the legend
    virtual QgsLegendSymbologyList legendSymbologyItems( QSize iconSize );

    //! return a list of item text / symbol
    //! @note: this method was added in version 1.5
    //! @note not available in python bindings
    virtual QgsLegendSymbolList legendSymbolItems();

    QgsSymbolV2* sourceSymbol();
    void setSourceSymbol( QgsSymbolV2* sym );

    QgsVectorColorRampV2* sourceColorRamp();
    void setSourceColorRamp( QgsVectorColorRampV2* ramp );

    /** Update the color ramp used. Also updates all symbols colors.
      * Doesn't alter current breaks.
      */
    void updateColorRamp( QgsVectorColorRampV2* ramp );

    /** Update the all symbols but leave breaks and colors. */
    void updateSymbols( QgsSymbolV2* sym );

    //! @note added in 1.6
    void setRotationField( QString fieldName ) { mRotationField = fieldName; }
    //! @note added in 1.6
    QString rotationField() const { return mRotationField; }

    //! @note added in 1.6
    void setSizeScaleField( QString fieldName ) { mSizeScaleField = fieldName; }
    //! @note added in 1.6
    QString sizeScaleField() const { return mSizeScaleField; }

    //! @note added in 2.0
    void setScaleMethod( QgsSymbolV2::ScaleMethod scaleMethod );
    //! @note added in 2.0
    QgsSymbolV2::ScaleMethod scaleMethod() const { return mScaleMethod; }


  protected:
    QString mAttrName;
    QgsRangeList mRanges;
    Mode mMode;
    QgsSymbolV2* mSourceSymbol;
    QgsVectorColorRampV2* mSourceColorRamp;
    QString mRotationField;
    QString mSizeScaleField;
    QgsSymbolV2::ScaleMethod mScaleMethod;

    //! attribute index (derived from attribute name in startRender)
    int mAttrNum;
    int mRotationFieldIdx, mSizeScaleFieldIdx;

    //! temporary symbols, used for data-defined rotation and scaling
#if QT_VERSION < 0x40600
    QMap<QgsSymbolV2*, QgsSymbolV2*> mTempSymbols;
#else
    QHash<QgsSymbolV2*, QgsSymbolV2*> mTempSymbols;
#endif

    QgsSymbolV2* symbolForValue( double value );
};

#endif // QGSGRADUATEDSYMBOLRENDERERV2_H
