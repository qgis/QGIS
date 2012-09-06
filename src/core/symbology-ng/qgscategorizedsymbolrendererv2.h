/***************************************************************************
    qgscategorizedsymbolrendererv2.h
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
#ifndef QGSCATEGORIZEDSYMBOLRENDERERV2_H
#define QGSCATEGORIZEDSYMBOLRENDERERV2_H

#include "qgssymbolv2.h"
#include "qgsrendererv2.h"

#include <QHash>

class QgsVectorColorRampV2;
class QgsVectorLayer;

/* \brief categorized renderer */
class CORE_EXPORT QgsRendererCategoryV2
{
  public:

    //! takes ownership of symbol
    QgsRendererCategoryV2( QVariant value, QgsSymbolV2* symbol, QString label );

    //! copy constructor
    QgsRendererCategoryV2( const QgsRendererCategoryV2& cat );

    ~QgsRendererCategoryV2();

    QVariant value() const;
    QgsSymbolV2* symbol() const;
    QString label() const;

    void setValue( const QVariant &value );
    void setSymbol( QgsSymbolV2* s );
    void setLabel( const QString &label );

    // debugging
    QString dump();

    void toSld( QDomDocument& doc, QDomElement &element, QgsStringMap props ) const;

  protected:
    QVariant mValue;
    QgsSymbolV2* mSymbol;
    QString mLabel;
};

typedef QList<QgsRendererCategoryV2> QgsCategoryList;

class CORE_EXPORT QgsCategorizedSymbolRendererV2 : public QgsFeatureRendererV2
{
  public:

    QgsCategorizedSymbolRendererV2( QString attrName = QString(), QgsCategoryList categories = QgsCategoryList() );

    virtual ~QgsCategorizedSymbolRendererV2();

    virtual QgsSymbolV2* symbolForFeature( QgsFeature& feature );

    virtual void startRender( QgsRenderContext& context, const QgsVectorLayer *vlayer );

    virtual void stopRender( QgsRenderContext& context );

    virtual QList<QString> usedAttributes();

    virtual QString dump();

    virtual QgsFeatureRendererV2* clone();

    virtual void toSld( QDomDocument& doc, QDomElement &element ) const;

    //! returns bitwise OR-ed capabilities of the renderer
    //! \note added in 2.0
    virtual int capabilities() { return SymbolLevels | RotationField; }

    virtual QgsSymbolV2List symbols();

    const QgsCategoryList& categories() { return mCategories; }

    //! return index of category with specified value (-1 if not found)
    int categoryIndexForValue( QVariant val );

    bool updateCategoryValue( int catIndex, const QVariant &value );
    bool updateCategorySymbol( int catIndex, QgsSymbolV2* symbol );
    bool updateCategoryLabel( int catIndex, QString label );

    void addCategory( const QgsRendererCategoryV2 &category );
    bool deleteCategory( int catIndex );
    void deleteAllCategories();

    QString classAttribute() const { return mAttrName; }
    void setClassAttribute( QString attr ) { mAttrName = attr; }

    //! create renderer from XML element
    static QgsFeatureRendererV2* create( QDomElement& element );

    //! store renderer info to XML element
    virtual QDomElement save( QDomDocument& doc );

    //! return a list of symbology items for the legend
    virtual QgsLegendSymbologyList legendSymbologyItems( QSize iconSize );

    //! return a list of item text / symbol
    //! @note: this method was added in version 1.5
    virtual QgsLegendSymbolList legendSymbolItems();

    QgsSymbolV2* sourceSymbol();
    void setSourceSymbol( QgsSymbolV2* sym );

    QgsVectorColorRampV2* sourceColorRamp();
    void setSourceColorRamp( QgsVectorColorRampV2* ramp );

    //! @note added in 1.6
    void setRotationField( QString fieldName ) { mRotationField = fieldName; }
    //! @note added in 1.6
    QString rotationField() const { return mRotationField; }

    //! @note added in 1.6
    void setSizeScaleField( QString fieldName ) { mSizeScaleField = fieldName; }
    //! @note added in 1.6
    QString sizeScaleField() const { return mSizeScaleField; }

    //! @note added in 2.0
    void setScaleMethod( QgsSymbolV2::ScaleMethod scaleMethod ) { mScaleMethod = scaleMethod; }
    //! @note added in 2.0
    QgsSymbolV2::ScaleMethod scaleMethod() const { return mScaleMethod; }

  protected:
    QString mAttrName;
    QgsCategoryList mCategories;
    QgsSymbolV2* mSourceSymbol;
    QgsVectorColorRampV2* mSourceColorRamp;
    QString mRotationField;
    QString mSizeScaleField;
    QgsSymbolV2::ScaleMethod mScaleMethod;

    //! attribute index (derived from attribute name in startRender)
    int mAttrNum;
    int mRotationFieldIdx, mSizeScaleFieldIdx;

    //! hashtable for faster access to symbols
    QHash<QString, QgsSymbolV2*> mSymbolHash;

    //! temporary symbols, used for data-defined rotation and scaling
    QHash<QString, QgsSymbolV2*> mTempSymbols;

    void rebuildHash();

    QgsSymbolV2* symbolForValue( QVariant value );
};


#endif // QGSCATEGORIZEDSYMBOLRENDERERV2_H
