/***************************************************************************
    qgscategorizedsymbolrenderer.h
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

#include "qgssymbol.h"
#include "qgsrenderer.h"
#include "qgsexpression.h"

#include <QHash>
#include <QScopedPointer>

class QgsVectorColorRamp;
class QgsVectorLayer;

/** \ingroup core
 * \brief categorized renderer
*/
class CORE_EXPORT QgsRendererCategory
{
  public:
    QgsRendererCategory();

    //! takes ownership of symbol
    QgsRendererCategory( const QVariant& value, QgsSymbol* symbol, const QString& label, bool render = true );

    //! copy constructor
    QgsRendererCategory( const QgsRendererCategory& cat );

    ~QgsRendererCategory() {}

    QgsRendererCategory& operator=( QgsRendererCategory cat );

    QVariant value() const;
    QgsSymbol* symbol() const;
    QString label() const;

    void setValue( const QVariant &value );
    void setSymbol( QgsSymbol* s );
    void setLabel( const QString &label );

    // @note added in 2.5
    bool renderState() const;
    void setRenderState( bool render );

    // debugging
    QString dump() const;

    void toSld( QDomDocument& doc, QDomElement &element, QgsStringMap props ) const;

  protected:
    QVariant mValue;
    QScopedPointer<QgsSymbol> mSymbol;
    QString mLabel;
    bool mRender;

    void swap( QgsRendererCategory & other );
};

typedef QList<QgsRendererCategory> QgsCategoryList;

Q_NOWARN_DEPRECATED_PUSH
/** \ingroup core
 * \class QgsCategorizedSymbolRenderer
 */
class CORE_EXPORT QgsCategorizedSymbolRenderer : public QgsFeatureRenderer
{
  public:

    QgsCategorizedSymbolRenderer( const QString& attrName = QString(), const QgsCategoryList& categories = QgsCategoryList() );

    virtual ~QgsCategorizedSymbolRenderer();

    //! @note available in python as symbolForFeature2
    virtual QgsSymbol* symbolForFeature( QgsFeature& feature, QgsRenderContext& context ) override;

    //! @note available in python as originalSymbolForFeature2
    virtual QgsSymbol* originalSymbolForFeature( QgsFeature& feature, QgsRenderContext& context ) override;

    virtual void startRender( QgsRenderContext& context, const QgsFields& fields ) override;

    virtual void stopRender( QgsRenderContext& context ) override;

    virtual QList<QString> usedAttributes() override;

    virtual QString dump() const override;

    virtual QgsCategorizedSymbolRenderer* clone() const override;

    virtual void toSld( QDomDocument& doc, QDomElement &element ) const override;

    //! returns bitwise OR-ed capabilities of the renderer
    virtual Capabilities capabilities() override { return SymbolLevels | RotationField | Filter; }

    virtual QString filter( const QgsFields& fields = QgsFields() ) override;

    //! @note available in python as symbols2
    virtual QgsSymbolList symbols( QgsRenderContext& context ) override;

    /** Update all the symbols but leave categories and colors. This method also sets the source
     * symbol for the renderer.
     * @param sym source symbol to use for categories. Ownership is not transferred.
     * @see setSourceSymbol()
     */
    void updateSymbols( QgsSymbol * sym );

    const QgsCategoryList& categories() const { return mCategories; }

    //! return index of category with specified value (-1 if not found)
    int categoryIndexForValue( const QVariant& val );

    //! return index of category with specified label (-1 if not found or not unique)
    //! @note added in 2.5
    int categoryIndexForLabel( const QString& val );

    bool updateCategoryValue( int catIndex, const QVariant &value );
    bool updateCategorySymbol( int catIndex, QgsSymbol* symbol );
    bool updateCategoryLabel( int catIndex, const QString& label );

    //! @note added in 2.5
    bool updateCategoryRenderState( int catIndex, bool render );

    void addCategory( const QgsRendererCategory &category );
    bool deleteCategory( int catIndex );
    void deleteAllCategories();

    //! Moves the category at index position from to index position to.
    void moveCategory( int from, int to );

    void sortByValue( Qt::SortOrder order = Qt::AscendingOrder );
    void sortByLabel( Qt::SortOrder order = Qt::AscendingOrder );

    QString classAttribute() const { return mAttrName; }
    void setClassAttribute( const QString& attr ) { mAttrName = attr; }

    //! create renderer from XML element
    static QgsFeatureRenderer* create( QDomElement& element );

    //! store renderer info to XML element
    virtual QDomElement save( QDomDocument& doc ) override;

    //! return a list of symbology items for the legend
    virtual QgsLegendSymbologyList legendSymbologyItems( QSize iconSize ) override;

    //! return a list of item text / symbol
    //! @note not available in python bindings
    virtual QgsLegendSymbolList legendSymbolItems( double scaleDenominator = -1, const QString& rule = QString() ) override;

    //! @note added in 2.10
    QgsLegendSymbolListV2 legendSymbolItemsV2() const override;

    virtual QSet< QString > legendKeysForFeature( QgsFeature& feature, QgsRenderContext& context ) override;

    /** Returns the renderer's source symbol, which is the base symbol used for the each categories' symbol before applying
     * the categories' color.
     * @see setSourceSymbol()
     * @see sourceColorRamp()
     */
    QgsSymbol* sourceSymbol();

    /** Sets the source symbol for the renderer, which is the base symbol used for the each categories' symbol before applying
     * the categories' color.
     * @param sym source symbol, ownership is transferred to the renderer
     * @see sourceSymbol()
     * @see setSourceColorRamp()
     */
    void setSourceSymbol( QgsSymbol* sym );

    /** Returns the source color ramp, from which each categories' color is derived.
     * @see setSourceColorRamp()
     * @see sourceSymbol()
     */
    QgsVectorColorRamp* sourceColorRamp();

    /** Sets the source color ramp.
      * @param ramp color ramp. Ownership is transferred to the renderer
      * @see sourceColorRamp()
      * @see setSourceSymbol()
      */
    void setSourceColorRamp( QgsVectorColorRamp* ramp );

    //! @note added in 2.1
    bool invertedColorRamp() { return mInvertedColorRamp; }
    void setInvertedColorRamp( bool inverted ) { mInvertedColorRamp = inverted; }

    /** Update the color ramp used and all symbols colors.
      * @param ramp color ramp. Ownership is transferred to the renderer
      * @param inverted set to true to invert ramp colors
      * @note added in 2.5
      */
    void updateColorRamp( QgsVectorColorRamp* ramp, bool inverted = false );

    Q_DECL_DEPRECATED void setRotationField( const QString& fieldOrExpression ) override;
    Q_DECL_DEPRECATED QString rotationField() const override;

    void setSizeScaleField( const QString& fieldOrExpression );
    QString sizeScaleField() const;

    void setScaleMethod( QgsSymbol::ScaleMethod scaleMethod );
    QgsSymbol::ScaleMethod scaleMethod() const { return mScaleMethod; }

    //! items of symbology items in legend should be checkable
    //! @note added in 2.5
    virtual bool legendSymbolItemsCheckable() const override;

    //! item in symbology was checked
    // @note added in 2.5
    virtual bool legendSymbolItemChecked( const QString& key ) override;

    virtual void setLegendSymbolItem( const QString& key, QgsSymbol* symbol ) override;

    //! item in symbology was checked
    // @note added in 2.5
    virtual void checkLegendSymbolItem( const QString& key, bool state = true ) override;

    //! If supported by the renderer, return classification attribute for the use in legend
    //! @note added in 2.6
    virtual QString legendClassificationAttribute() const override { return classAttribute(); }

    //! creates a QgsCategorizedSymbolRenderer from an existing renderer.
    //! @note added in 2.5
    //! @returns a new renderer if the conversion was possible, otherwise 0.
    static QgsCategorizedSymbolRenderer* convertFromRenderer( const QgsFeatureRenderer *renderer );

  protected:
    QString mAttrName;
    QgsCategoryList mCategories;
    QScopedPointer<QgsSymbol> mSourceSymbol;
    QScopedPointer<QgsVectorColorRamp> mSourceColorRamp;
    bool mInvertedColorRamp;
    QScopedPointer<QgsExpression> mRotation;
    QScopedPointer<QgsExpression> mSizeScale;
    QgsSymbol::ScaleMethod mScaleMethod;
    QScopedPointer<QgsExpression> mExpression;

    //! attribute index (derived from attribute name in startRender)
    int mAttrNum;

    //! hashtable for faster access to symbols
    QHash<QString, QgsSymbol*> mSymbolHash;
    bool mCounting;

    //! temporary symbols, used for data-defined rotation and scaling
    QHash<QgsSymbol*, QgsSymbol*> mTempSymbols;

    void rebuildHash();

    QgsSymbol* skipRender();

    QgsSymbol* symbolForValue( const QVariant& value );

  private:

    /** Returns calculated classification value for a feature */
    QVariant valueForFeature( QgsFeature& feature, QgsRenderContext &context ) const;

};
Q_NOWARN_DEPRECATED_POP


#endif // QGSCATEGORIZEDSYMBOLRENDERERV2_H
