/***************************************************************************
    qgscategorizedsymbolrendererv2.h
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
#ifndef QGSCATEGORIZEDSYMBOLRENDERERV2_H
#define QGSCATEGORIZEDSYMBOLRENDERERV2_H

#include "qgssymbolv2.h"
#include "qgsrendererv2.h"
#include "qgsexpression.h"

#include <QHash>
#include <QScopedPointer>

class QgsVectorColorRampV2;
class QgsVectorLayer;

/* \brief categorized renderer */
class CORE_EXPORT QgsRendererCategoryV2
{
  public:
    QgsRendererCategoryV2( );

    //! takes ownership of symbol
    QgsRendererCategoryV2( QVariant value, QgsSymbolV2* symbol, QString label );

    //! copy constructor
    QgsRendererCategoryV2( const QgsRendererCategoryV2& cat );

    QgsRendererCategoryV2& operator=( QgsRendererCategoryV2 cat );

    QVariant value() const;
    QgsSymbolV2* symbol() const;
    QString label() const;

    void setValue( const QVariant &value );
    void setSymbol( QgsSymbolV2* s );
    void setLabel( const QString &label );

    // debugging
    QString dump() const;

    void toSld( QDomDocument& doc, QDomElement &element, QgsStringMap props ) const;

  protected:
    QVariant mValue;
    QScopedPointer<QgsSymbolV2> mSymbol;
    QString mLabel;

    void swap( QgsRendererCategoryV2 & other );
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

    virtual QString dump() const;

    virtual QgsFeatureRendererV2* clone();

    virtual void toSld( QDomDocument& doc, QDomElement &element ) const;

    //! returns bitwise OR-ed capabilities of the renderer
    //! \note added in 2.0
    virtual int capabilities() { return SymbolLevels | RotationField | Filter; }

    virtual QgsSymbolV2List symbols();
    //! @note added in 2.0
    void updateSymbols( QgsSymbolV2 * sym );

    const QgsCategoryList& categories() { return mCategories; }

    //! return index of category with specified value (-1 if not found)
    int categoryIndexForValue( QVariant val );

    bool updateCategoryValue( int catIndex, const QVariant &value );
    bool updateCategorySymbol( int catIndex, QgsSymbolV2* symbol );
    bool updateCategoryLabel( int catIndex, QString label );

    void addCategory( const QgsRendererCategoryV2 &category );
    bool deleteCategory( int catIndex );
    void deleteAllCategories();

    //! Moves the category at index position from to index position to.
    void moveCategory( int from, int to );

    void sortByValue( Qt::SortOrder order = Qt::AscendingOrder );
    void sortByLabel( Qt::SortOrder order = Qt::AscendingOrder );

    QString classAttribute() const { return mAttrName; }
    void setClassAttribute( QString attr ) { mAttrName = attr; }

    //! create renderer from XML element
    static QgsFeatureRendererV2* create( QDomElement& element );

    //! store renderer info to XML element
    virtual QDomElement save( QDomDocument& doc );

    //! return a list of symbology items for the legend
    virtual QgsLegendSymbologyList legendSymbologyItems( QSize iconSize );

    //! return a list of item text / symbol
    //! @note this method was added in version 1.5
    //! @note not available in python bindings
    virtual QgsLegendSymbolList legendSymbolItems( double scaleDenominator = -1, QString rule = QString() );

    QgsSymbolV2* sourceSymbol();
    void setSourceSymbol( QgsSymbolV2* sym );

    QgsVectorColorRampV2* sourceColorRamp();
    void setSourceColorRamp( QgsVectorColorRampV2* ramp );

    //! @note added in 2.1
    bool invertedColorRamp() { return mInvertedColorRamp; }
    void setInvertedColorRamp( bool inverted ) { mInvertedColorRamp = inverted; }

    //! @note added in 1.6
    void setRotationField( QString expression )
    {
      mRotation.reset( expression.isEmpty() ? NULL : new QgsExpression( expression ) );
      Q_ASSERT( !mRotation.data() || !mRotation->hasParserError() );
    }
    //! @note added in 1.6
    QString rotationField() const {  return mRotation.data() ? mRotation->expression() : QString();}

    //! @note added in 1.6
    void setSizeScaleField( QString expression )
    {
      mSizeScale.reset( expression.isEmpty() ? NULL : new QgsExpression( expression ) );
      Q_ASSERT( !mSizeScale.data() || !mSizeScale->hasParserError() );
    }
    //! @note added in 1.6
    QString sizeScaleField() const { return mSizeScale.data() ? mSizeScale->expression() : QString(); }

    //! @note added in 2.0
    void setScaleMethod( QgsSymbolV2::ScaleMethod scaleMethod );
    //! @note added in 2.0
    QgsSymbolV2::ScaleMethod scaleMethod() const { return mScaleMethod; }

  protected:
    QString mAttrName;
    QgsCategoryList mCategories;
    QScopedPointer<QgsSymbolV2> mSourceSymbol;
    QScopedPointer<QgsVectorColorRampV2> mSourceColorRamp;
    bool mInvertedColorRamp;
    QScopedPointer<QgsExpression> mRotation;
    QScopedPointer<QgsExpression> mSizeScale;
    QgsSymbolV2::ScaleMethod mScaleMethod;
    QScopedPointer<QgsExpression> mExpression;

    //! attribute index (derived from attribute name in startRender)
    int mAttrNum;

    //! hashtable for faster access to symbols
    QHash<QString, QgsSymbolV2*> mSymbolHash;

    //! temporary symbols, used for data-defined rotation and scaling
    QHash<QString, QgsSymbolV2*> mTempSymbols;

    void rebuildHash();

    QgsSymbolV2* symbolForValue( QVariant value );
};


#endif // QGSCATEGORIZEDSYMBOLRENDERERV2_H
