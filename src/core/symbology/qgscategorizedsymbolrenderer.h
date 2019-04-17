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
#ifndef QGSCATEGORIZEDSYMBOLRENDERER_H
#define QGSCATEGORIZEDSYMBOLRENDERER_H

#include "qgis_core.h"
#include "qgis.h"
#include "qgssymbol.h"
#include "qgsrenderer.h"
#include "qgsexpression.h"
#include "qgscolorramp.h"
#include "qgsdatadefinedsizelegend.h"

#include <QHash>

class QgsVectorLayer;
class QgsStyle;

/**
 * \ingroup core
 * \brief Represents an individual category (class) from a QgsCategorizedSymbolRenderer.
*/
class CORE_EXPORT QgsRendererCategory
{
  public:

    /**
     * Constructor for QgsRendererCategory.
     */
    QgsRendererCategory() = default;

    /**
    * Constructor for a new QgsRendererCategory, with the specified \a value and \a symbol.
    *
    * If \a value is a list, then the category will match any of the values from this list.
    *
    * The ownership of \a symbol is transferred to the category.
    *
    * The \a label argument specifies the label used for this category in legends and the layer tree.
    *
    * The \a render argument indicates whether the category should initially be rendered and appear checked in the layer tree.
    */
    QgsRendererCategory( const QVariant &value, QgsSymbol *symbol SIP_TRANSFER, const QString &label, bool render = true );

    /**
     * Copy constructor.
     */
    QgsRendererCategory( const QgsRendererCategory &cat );
    QgsRendererCategory &operator=( QgsRendererCategory cat );

    /**
     * Returns the value corresponding to this category.
     *
     * If the returned value is a list, then the category will match any of the values from this list.
     *
     * \see setValue()
     */
    QVariant value() const;

    /**
     * Returns the symbol which will be used to render this category.
     * \see setSymbol()
     */
    QgsSymbol *symbol() const;

    /**
     * Returns the label for this category, which is used to represent the category within
     * legends and the layer tree.
     * \see setLabel()
     */
    QString label() const;

    /**
     * Sets the \a value corresponding to this category.
     *
     * If \a value is a list, then the category will match any of the values from this list.
     *
     * \see value()
     */
    void setValue( const QVariant &value );

    /**
     * Sets the symbol which will be used to render this category.
     *
     * Ownership of the symbol is transferred to the category.
     *
     * \see symbol()
     */
    void setSymbol( QgsSymbol *s SIP_TRANSFER );

    /**
     * Sets the \a label for this category, which is used to represent the category within
     * legends and the layer tree.
     * \see label()
     */
    void setLabel( const QString &label );

    /**
     * Returns TRUE if the category is currently enabled and should be rendered.
     * \see setRenderState()
     * \since QGIS 2.5
     */
    bool renderState() const;

    /**
     * Sets whether the category is currently enabled and should be rendered.
     * \see renderState()
     * \since QGIS 2.5
     */
    void setRenderState( bool render );

    // debugging

    /**
     * Returns a string representing the categories settings, used for debugging purposes only.
     */
    QString dump() const;

    /**
     * Converts the category to a matching SLD rule, within the specified DOM document and \a element.
     */
    void toSld( QDomDocument &doc, QDomElement &element, QgsStringMap props ) const;

  protected:
    QVariant mValue;
    std::unique_ptr<QgsSymbol> mSymbol;
    QString mLabel;
    bool mRender = true;

    void swap( QgsRendererCategory &other );
};

typedef QList<QgsRendererCategory> QgsCategoryList;

/**
 * \ingroup core
 * \class QgsCategorizedSymbolRenderer
 */
class CORE_EXPORT QgsCategorizedSymbolRenderer : public QgsFeatureRenderer
{
  public:

    /**
     * Constructor for QgsCategorizedSymbolRenderer.
     *
     * The \a attrName argument specifies the layer's field name, or expression, which the categories will be matched against.
     *
     * A list of renderer \a categories can optionally be specified. If no categories are specified in the constructor, they
     * can be added later by calling addCategory().
     */
    QgsCategorizedSymbolRenderer( const QString &attrName = QString(), const QgsCategoryList &categories = QgsCategoryList() );

    QgsSymbol *symbolForFeature( const QgsFeature &feature, QgsRenderContext &context ) const override;
    QgsSymbol *originalSymbolForFeature( const QgsFeature &feature, QgsRenderContext &context ) const override;
    void startRender( QgsRenderContext &context, const QgsFields &fields ) override;
    void stopRender( QgsRenderContext &context ) override;
    QSet<QString> usedAttributes( const QgsRenderContext &context ) const override;
    bool filterNeedsGeometry() const override;
    QString dump() const override;
    QgsCategorizedSymbolRenderer *clone() const override SIP_FACTORY;
    void toSld( QDomDocument &doc, QDomElement &element, const QgsStringMap &props = QgsStringMap() ) const override;
    QgsFeatureRenderer::Capabilities capabilities() override { return SymbolLevels | Filter; }
    QString filter( const QgsFields &fields = QgsFields() ) override;
    QgsSymbolList symbols( QgsRenderContext &context ) const override;

    /**
     * Update all the symbols but leave categories and colors. This method also sets the source
     * symbol for the renderer.
     * \param sym source symbol to use for categories. Ownership is not transferred.
     * \see setSourceSymbol()
     */
    void updateSymbols( QgsSymbol *sym );

    /**
     * Returns a list of all categories recognized by the renderer.
     */
    const QgsCategoryList &categories() const { return mCategories; }

    /**
     * Returns the index for the category with the specified value (or -1 if not found).
     */
    int categoryIndexForValue( const QVariant &val );

    /**
     * Returns the index of the category with the specified label (or -1 if the label was not found, or is not unique).
     * \since QGIS 2.5
     */
    int categoryIndexForLabel( const QString &val );

    /**
     * Changes the value for the category with the specified index.
     *
     * If \a value is a list, then the category will match any of the values from this list.
     *
     * \see updateCategorySymbol()
     * \see updateCategoryLabel()
     * \see updateCategoryRenderState()
     */
    bool updateCategoryValue( int catIndex, const QVariant &value );

    /**
     * Changes the \a symbol for the category with the specified index.
     *
     * Ownership of \a symbol is transferred to the renderer.
     *
     * \see updateCategoryValue()
     * \see updateCategoryLabel()
     * \see updateCategoryRenderState()
     */
    bool updateCategorySymbol( int catIndex, QgsSymbol *symbol SIP_TRANSFER );

    /**
     * Changes the \a label for the category with the specified index.
     *
     * A category's label is used to represent the category within
     * legends and the layer tree.
     *
     * \see updateCategoryValue()
     * \see updateCategoryLabel()
     * \see updateCategoryRenderState()
     */
    bool updateCategoryLabel( int catIndex, const QString &label );

    /**
     * Changes the render state for the category with the specified index.
     *
     * The render state indicates whether or not the category will be rendered,
     * and is reflected in whether the category is checked with the project's layer tree.
     *
     * \see updateCategoryValue()
     * \see updateCategorySymbol()
     * \see updateCategoryLabel()
     *
     * \since QGIS 2.5
     */
    bool updateCategoryRenderState( int catIndex, bool render );

    /**
     * Adds a new \a category to the renderer.
     *
     * \see categories()
     */
    void addCategory( const QgsRendererCategory &category );

    /**
     * Deletes the category with the specified index from the renderer.
     *
     * \see deleteAllCategories()
     */
    bool deleteCategory( int catIndex );

    /**
     * Deletes all existing categories from the renderer.
     *
     * \see deleteCategory()
     */
    void deleteAllCategories();

    /**
     * Moves an existing category at index position from to index position to.
     */
    void moveCategory( int from, int to );

    /**
     * Sorts the existing categories by their value.
     *
     * \see sortByLabel()
     */
    void sortByValue( Qt::SortOrder order = Qt::AscendingOrder );

    /**
     * Sorts the existing categories by their label.
     *
     * \see sortByValue()
     */
    void sortByLabel( Qt::SortOrder order = Qt::AscendingOrder );

    /**
     * Returns the class attribute for the renderer, which is the field name
     * or expression string from the layer which will be matched against the
     * renderer categories.
     *
     * \see setClassAttribute()
     */
    QString classAttribute() const { return mAttrName; }

    /**
     * Sets the class attribute for the renderer, which is the field name
     * or expression string from the layer which will be matched against the
     * renderer categories.
     *
     * \see classAttribute()
     */
    void setClassAttribute( const QString &attr ) { mAttrName = attr; }

    /**
     * Creates a categorized renderer from an XML \a element.
     */
    static QgsFeatureRenderer *create( QDomElement &element, const QgsReadWriteContext &context ) SIP_FACTORY;

    QDomElement save( QDomDocument &doc, const QgsReadWriteContext &context ) override;
    QgsLegendSymbolList legendSymbolItems() const override;
    QSet< QString > legendKeysForFeature( const QgsFeature &feature, QgsRenderContext &context ) const override;

    /**
     * Returns the renderer's source symbol, which is the base symbol used for the each categories' symbol before applying
     * the categories' color.
     * \see setSourceSymbol()
     * \see sourceColorRamp()
     */
    QgsSymbol *sourceSymbol();

    /**
     * Sets the source symbol for the renderer, which is the base symbol used for the each categories' symbol before applying
     * the categories' color.
     * \param sym source symbol, ownership is transferred to the renderer
     * \see sourceSymbol()
     * \see setSourceColorRamp()
     */
    void setSourceSymbol( QgsSymbol *sym SIP_TRANSFER );

    /**
     * Returns the source color ramp, from which each categories' color is derived.
     * \see setSourceColorRamp()
     * \see sourceSymbol()
     */
    QgsColorRamp *sourceColorRamp();

    /**
     * Sets the source color ramp.
      * \param ramp color ramp. Ownership is transferred to the renderer
      * \see sourceColorRamp()
      * \see setSourceSymbol()
      */
    void setSourceColorRamp( QgsColorRamp *ramp SIP_TRANSFER );

    /**
     * Update the color ramp used and all symbols colors.
      * \param ramp color ramp. Ownership is transferred to the renderer
      * \since QGIS 2.5
      */
    void updateColorRamp( QgsColorRamp *ramp SIP_TRANSFER );

    bool legendSymbolItemsCheckable() const override;
    bool legendSymbolItemChecked( const QString &key ) override;
    void setLegendSymbolItem( const QString &key, QgsSymbol *symbol SIP_TRANSFER ) override;
    void checkLegendSymbolItem( const QString &key, bool state = true ) override;
    QString legendClassificationAttribute() const override { return classAttribute(); }

    /**
     * creates a QgsCategorizedSymbolRenderer from an existing renderer.
     * \returns a new renderer if the conversion was possible, otherwise 0.
     * \since QGIS 2.5
     */
    static QgsCategorizedSymbolRenderer *convertFromRenderer( const QgsFeatureRenderer *renderer ) SIP_FACTORY;

    /**
     * Configures appearance of legend when renderer is configured to use data-defined size for marker symbols.
     * This allows configuring for which values (symbol sizes) should be shown in the legend, whether to display
     * different symbol sizes collapsed in one legend node or separated across multiple legend nodes etc.
     *
     * When renderer does not use data-defined size or does not use marker symbols, these settings will be ignored.
     * Takes ownership of the passed settings objects. NULLPTR is a valid input that disables data-defined
     * size legend.
     * \since QGIS 3.0
     */
    void setDataDefinedSizeLegend( QgsDataDefinedSizeLegend *settings SIP_TRANSFER );

    /**
     * Returns configuration of appearance of legend when using data-defined size for marker symbols.
     * Will return NULLPTR if the functionality is disabled.
     * \since QGIS 3.0
     */
    QgsDataDefinedSizeLegend *dataDefinedSizeLegend() const;

    /**
     * Replaces category symbols with the symbols from a \a style that have a matching
     * name and symbol \a type.
     *
     * The \a unmatchedCategories list will be filled with all existing categories which could not be matched
     * to a symbol in \a style.
     *
     * The \a unmatchedSymbols list will be filled with all symbol names from \a style which were not matched
     * to an existing category.
     *
     * If \a caseSensitive is FALSE, then a case-insensitive match will be performed. If \a useTolerantMatch
     * is TRUE, then non-alphanumeric characters in style and category names will be ignored during the match.
     *
     * Returns the count of symbols matched.
     *
     * \since QGIS 3.4
     */
    int matchToSymbols( QgsStyle *style, QgsSymbol::SymbolType type,
                        QVariantList &unmatchedCategories SIP_OUT, QStringList &unmatchedSymbols SIP_OUT, bool caseSensitive = true, bool useTolerantMatch = false );


    /**
     * Create categories for a list of \a values.
     * The returned symbols in the category list will be a modification of \a symbol.
     *
     * If \a layer and \a fieldName are specified it will try to find nicer values
     * to represent the description for the categories based on the respective field
     * configuration.
     *
     * \since QGIS 3.6
     */
    static QgsCategoryList createCategories( const QVariantList &values, const QgsSymbol *symbol, QgsVectorLayer *layer = nullptr, const QString &fieldName = QString() );

  protected:
    QString mAttrName;
    QgsCategoryList mCategories;
    std::unique_ptr<QgsSymbol> mSourceSymbol;
    std::unique_ptr<QgsColorRamp> mSourceColorRamp;
    std::unique_ptr<QgsExpression> mExpression;

    std::unique_ptr<QgsDataDefinedSizeLegend> mDataDefinedSizeLegend;

    //! attribute index (derived from attribute name in startRender)
    int mAttrNum = -1;

    //! hashtable for faster access to symbols
    QHash<QString, QgsSymbol *> mSymbolHash;
    bool mCounting = false;

    void rebuildHash();

    /**
     * \deprecated No longer used, will be removed in QGIS 4.0
     */
    Q_DECL_DEPRECATED QgsSymbol *skipRender() SIP_DEPRECATED;

    /**
     * Returns the matching symbol corresponding to an attribute \a value.
     * \deprecated use variant which takes a second bool argument instead.
     */
    Q_DECL_DEPRECATED QgsSymbol *symbolForValue( const QVariant &value ) const SIP_DEPRECATED;

    // TODO QGIS 4.0 - rename Python method to symbolForValue

    /**
     * Returns the matching symbol corresponding to an attribute \a value.
     *
     * Will return NULLPTR if no matching symbol was found for \a value, or
     * if the category corresponding to \a value is currently disabled (see QgsRendererCategory::renderState()).
     *
     * If \a foundMatchingSymbol is specified then it will be set to TRUE if
     * a matching category was found. This can be used to differentiate between
     * NULLPTR returned as a result of no matching category vs NULLPTR as a result
     * of disabled categories.
     *
     * \note available in Python bindings as symbolForValue2
     */
    QgsSymbol *symbolForValue( const QVariant &value, bool &foundMatchingSymbol SIP_OUT ) const SIP_PYNAME( symbolForValue2 );

  private:
#ifdef SIP_RUN
    QgsCategorizedSymbolRenderer( const QgsCategorizedSymbolRenderer & );
    QgsCategorizedSymbolRenderer &operator=( const QgsCategorizedSymbolRenderer & );
#endif

    //! Returns calculated classification value for a feature
    QVariant valueForFeature( const QgsFeature &feature, QgsRenderContext &context ) const;

    //! Returns list of legend symbol items from individual categories
    QgsLegendSymbolList baseLegendSymbolItems() const;
};

#endif // QGSCATEGORIZEDSYMBOLRENDERER_H
