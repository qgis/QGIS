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
 * \brief categorized renderer
*/
class CORE_EXPORT QgsRendererCategory
{
  public:

    /**
     * Constructor for QgsRendererCategory.
     */
    QgsRendererCategory() = default;

    //! takes ownership of symbol
    QgsRendererCategory( const QVariant &value, QgsSymbol *symbol SIP_TRANSFER, const QString &label, bool render = true );

    //! copy constructor
    QgsRendererCategory( const QgsRendererCategory &cat );

    QgsRendererCategory &operator=( QgsRendererCategory cat );

    QVariant value() const;
    QgsSymbol *symbol() const;
    QString label() const;

    void setValue( const QVariant &value );
    void setSymbol( QgsSymbol *s SIP_TRANSFER );
    void setLabel( const QString &label );

    /**
     * Returns true if the category is currently enabled and should be rendered.
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
    QString dump() const;

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

    const QgsCategoryList &categories() const { return mCategories; }

    //! Returns index of category with specified value (-1 if not found)
    int categoryIndexForValue( const QVariant &val );

    /**
     * Returns index of category with specified label (-1 if not found or not unique)
     * \since QGIS 2.5
     */
    int categoryIndexForLabel( const QString &val );

    bool updateCategoryValue( int catIndex, const QVariant &value );
    bool updateCategorySymbol( int catIndex, QgsSymbol *symbol SIP_TRANSFER );
    bool updateCategoryLabel( int catIndex, const QString &label );

    //! \since QGIS 2.5
    bool updateCategoryRenderState( int catIndex, bool render );

    void addCategory( const QgsRendererCategory &category );
    bool deleteCategory( int catIndex );
    void deleteAllCategories();

    //! Moves the category at index position from to index position to.
    void moveCategory( int from, int to );

    void sortByValue( Qt::SortOrder order = Qt::AscendingOrder );
    void sortByLabel( Qt::SortOrder order = Qt::AscendingOrder );

    QString classAttribute() const { return mAttrName; }
    void setClassAttribute( const QString &attr ) { mAttrName = attr; }

    //! create renderer from XML element
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
     * Takes ownership of the passed settings objects. Null pointer is a valid input that disables data-defined
     * size legend.
     * \since QGIS 3.0
     */
    void setDataDefinedSizeLegend( QgsDataDefinedSizeLegend *settings SIP_TRANSFER );

    /**
     * Returns configuration of appearance of legend when using data-defined size for marker symbols.
     * Will return null if the functionality is disabled.
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
     * If \a caseSensitive is false, then a case-insensitive match will be performed. If \a useTolerantMatch
     * is true, then non-alphanumeric characters in style and category names will be ignored during the match.
     *
     * Returns the count of symbols matched.
     *
     * \since QGIS 3.4
     */
    int matchToSymbols( QgsStyle *style, QgsSymbol::SymbolType type,
                        QVariantList &unmatchedCategories SIP_OUT, QStringList &unmatchedSymbols SIP_OUT, bool caseSensitive = true, bool useTolerantMatch = false );

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
     * Will return nullptr if no matching symbol was found for \a value, or
     * if the category corresponding to \a value is currently disabled (see QgsRendererCategory::renderState()).
     *
     * If \a foundMatchingSymbol is specified then it will be set to true if
     * a matching category was found. This can be used to differentiate between
     * a nullptr returned as a result of no matching category vs a nullptr as a result
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
